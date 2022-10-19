
#include "mqtt5nanoParse.h"
#include "nanobase64.h"

namespace mqtt5nano
{

    // FIXME: return ok, not fail.
    bool mqttPacketPieces::parse(const slice body, const unsigned char _packetType, const int len)
    {
        bool fail = false;
        reset();

        packetType = (_packetType >> 4);
        QoS = (_packetType >> 1) & 3;

        if (packetType < CtrlConn || packetType > CtrlAuth)
        {
            fail = true;
            return fail;
        }

        slice pos = body;
        // pos.printhex();
        if (packetType == CtrlPublish)
        {
            // ok. parse a pub.
            TopicName = pos.getBigFixedLenString();
            // TopicName.printstr();
            // pos.printhex();
            if (QoS)
            {
               PacketID = pos.getBigFixLenInt();// this might be wrong endian FIXME:
            }
            int propLen = pos.getBigEndianVarLenInt();

            props.base = pos.base;
            props.start = pos.start;
            props.end = props.start + propLen;
            pos.start = props.end;

            // props.printhex();
            if (props.size())
            {
                int userIndex = 0;
                int maxUserIndex = userKeyValLen;
                // parse the props
                slice ptmp = props;
                while (ptmp.empty() == false)
                {
                    int key = ptmp.readByte();
                    if (key == propKeyRespTopic)
                    {
                        RespTopic = ptmp.getBigFixedLenString();
                        // RespTopic.printstr("resp");
                    }
                    else if (key == propKeyCorrelationData)
                    {
                        CorrelationData = ptmp.getBigFixedLenString();
                        // RespTopic.printstr("corr");
                    }
                    else if (key == propKeyUserProps)
                    {
                        slice k = ptmp.getBigFixedLenString();
                        // k.printstr();
                        slice v = ptmp.getBigFixedLenString();
                        // v.printstr();
                        if (userIndex < maxUserIndex)
                        {
                            UserKeyVal[userIndex] = k;
                            UserKeyVal[userIndex + 1] = v;
                            userIndex += 2;
                        }
                    }
                    else
                    {
                        // what happens now?
                        char code = getPropertyLenCode(key);
                        if (code & 0x0F)
                        {
                            if (code == char(0xFF))
                            {
                                fail = true;
                                return fail; // we're done and broken.
                            }
                            if (code == 0x0F)
                            {
                                int dummy = ptmp.getBigEndianVarLenInt();
                            }
                            else
                            { // pass 'code' bytes.
                                ptmp.start += code;
                            }
                        }
                        else
                        {
                            code = code / 16;
                            // pass 'code' strings.
                            for (int i = 0; i < code; i++)
                            {
                                slice aslice = ptmp.getBigFixedLenString();
                            }
                        }
                    }
                }
            }

            Payload = pos;
            Payload.end = body.start + len;
            // and we're done.
        }
        else
        {
            // worry about the body of the other packets later. TODO:
        }
        return fail;
    }

    // reset simply has to zero all the base pointers for the slices
    // and that makes them 'empty'.
    void mqttPacketPieces::reset()
    {
        for (int i = 0; i < userKeyValLen; i++)
        {
            UserKeyVal[i].base = 0;
        }
        TopicName.base = 0;
        Payload.base = 0;
        props.base = 0;
        RespTopic.base = 0;
        CorrelationData.base = 0;
        QoS = 0;
    }

    bool mqttPacketPieces::outputConnect(sink assemblyBuffer, drain *destination,
                                         slice clientID, slice user, slice pass)
    {
        // We're filling a buffer. We'll actually generate an array
        // of slices that are in the buffer that may have small gaps between them.
        // the problem is that the parts have variable length and we don't know
        // what it is until we output and then go back and patch it.
        // Since the var len can be as long as 3 bytes we have to leave
        // some space at the end of each segment for the length of the next.

        //  setby caller:
        packetType = CtrlConn;
        QoS = 0;

        slice fixedHeader = assemblyBuffer;
        fixedHeader.start = assemblyBuffer.start;
       
        assemblyBuffer.writeByte(char(packetType * 16) + (QoS * 2));
        fixedHeader.end = assemblyBuffer.start;
        //
        assemblyBuffer.start += 4; // leave some space
        //
        slice varHeader = assemblyBuffer;
        varHeader.start = assemblyBuffer.start;

        assemblyBuffer.writeByte(0); // todo: shorten this.
        assemblyBuffer.writeByte(4);
        assemblyBuffer.writeByte('M');
        assemblyBuffer.writeByte('Q');
        assemblyBuffer.writeByte('T');
        assemblyBuffer.writeByte('T');
        assemblyBuffer.writeByte(5);
        assemblyBuffer.writeByte(0xC2); // c2 -- connect flags. user, pass, and clean start
        assemblyBuffer.writeByte(0);    // msb keep alive
        assemblyBuffer.writeByte(0x3C); // lsb keep alive
        //
        assemblyBuffer.writeByte(0); // length of props
                                     // we could have props here. see outputPubOrSub
        varHeader.end = assemblyBuffer.start;
        //
        assemblyBuffer.start += 4; // leave some space
        //
        slice payload = assemblyBuffer;
        payload.start = assemblyBuffer.start;

        assemblyBuffer.writeFixedLenStr(clientID);
        assemblyBuffer.writeFixedLenStr(user);
        assemblyBuffer.writeFixedLenStr(pass);
        payload.end = assemblyBuffer.start;

        // now the body length
        // at the tail of fixedHeader
        int bodylen = varHeader.size() + payload.size();
        sink tmp = assemblyBuffer;      
        tmp.start = fixedHeader.end;
        tmp.end = tmp.start + 4;
        tmp.writeLittleEndianVarLenInt(bodylen);
        fixedHeader.end = tmp.start;

        if (assemblyBuffer.full() == true)
        {
            return true; // failed
        }
        // now, write out fixedHeader,varHeader, payload
        bool fail = false;
        fail |= destination->write(fixedHeader);
        fail |= destination->write(varHeader);
        fail |= destination->write(payload);

        return fail;
    };

    // Output a mqtt5 Subscribe packet using values previously set.
    // outputBuffer is a buffer. The final result is copied to 'assemblyBuffer'.
    // we don't have to buffer payload.
    // NOTE: when generating Subscribe packets the topic must be in the TopicName.

    bool mqttPacketPieces::outputPubOrSub(sink assemblyBuffer, drain *destination)
    {
        // We're filling a buffer. We'll actually generate an array
        // of slices that are in the buffer that may have small gaps between them.

        // setby caller:  packetType = CtrlSubscribe;

        if (packetType == CtrlSubscribe)
        {
            Payload = TopicName;
            TopicName.base = 0;
        }

        slice fixedHeader = assemblyBuffer;
        fixedHeader.start = assemblyBuffer.start;

        assemblyBuffer.writeByte(char(packetType * 16) + (QoS * 2));
        fixedHeader.end = assemblyBuffer.start;
        //
        assemblyBuffer.start += 4; // leave some space
        //
        slice varHeader = assemblyBuffer;
         varHeader.start = assemblyBuffer.start;
        if (packetType == CtrlPublish)
        {
            assemblyBuffer.writeFixedLenStr(TopicName);
        }
        assemblyBuffer.writeByte(PacketID / 16); //  .
        assemblyBuffer.writeByte(PacketID);
        varHeader.end = assemblyBuffer.start;
        //
        assemblyBuffer.start += 4; // leave some space
        //
        slice props = assemblyBuffer;
         props.start = assemblyBuffer.start;
        if (RespTopic.empty() == false)
        {
            assemblyBuffer.writeByte(propKeyRespTopic);
            assemblyBuffer.writeFixedLenStr(RespTopic);
        }
        for (int i = 0; i < userKeyValLen; i += 2)
        {
            if (UserKeyVal[i].empty() == false)
            {
                assemblyBuffer.writeByte(propKeyUserProps);
                assemblyBuffer.writeFixedLenStr(UserKeyVal[i]);
                assemblyBuffer.writeFixedLenStr(UserKeyVal[i + 1]);
            }
        }
        props.end = assemblyBuffer.start;

        // don't buffer the payload.
        int payloadSize = Payload.size();
        if (packetType == CtrlSubscribe)
        {
            payloadSize += 2; // for it's length bytes
            payloadSize += 1; // because subscribe adds the qos
        }
        // put the lengths in.
        // put the props len at the tail of varHeader
        int propslen = props.size();
        sink tmp= assemblyBuffer;
        tmp.start = varHeader.end;
        tmp.writeLittleEndianVarLenInt(propslen);
        varHeader.end = tmp.start;

        // now the body length
        // at the tail of fixedHeader
        int bodylen = varHeader.size() + props.size() + payloadSize;
     
        tmp.start = fixedHeader.end;
        tmp.writeLittleEndianVarLenInt(bodylen);
        fixedHeader.end = tmp.start;

        if (assemblyBuffer.full() == true)
        {
            return true; // failed
        }

        // now, write out fixedHeader,varHeader, props, payload
        bool fail = false;
        fail |= destination->write(fixedHeader);
        fail |= destination->write(varHeader);
        fail |= destination->write(props);

        if (packetType == CtrlSubscribe)
        {
            fail |= destination->writeFixedLenStr(Payload);
            fail |= destination->writeByte(QoS);
        }
        else
        { // packetType == CtrlPublish
            fail |= destination->write(Payload);
        }
        return fail;
    };

    // return a value if key found else return a 'done' slice.
    slice mqttPacketPieces::findKey(const char *key)
    {
        int size = userKeyValLen;
        for (int i = 0; i < size; i += 2)
        {
            slice s = UserKeyVal[i];
            if (s.equals(key))
            {
                return s;
            }
        }
        slice bads;
        bads.base = 0;
        return bads;
    };

    // PropKeyConsumes is to look up a code for every prop key. The code will be how many bytes to pass, in the lower
    // nibble or else how many strings to pass in the upper nibble.
    char PropKeyConsumes[43];

    void init()
    {
        PropKeyConsumes[propKeyPayloadFormatIndicator] = 0x01; // byte, Packet: Will, Publish
        PropKeyConsumes[propKeyMessageExpiryInterval] = 0x04;  // Uint (4 bytes), Packet: Will, Publish
        PropKeyConsumes[propKeyContentType] = 0x10;            // utf-8, Packet: Will, Publish
        PropKeyConsumes[propKeyRespTopic] = 0x10;              // utf-8, Packet: Will, Publish
        PropKeyConsumes[propKeyCorrelationData] = 0x10;        // binary data, Packet: Will, Publish
        PropKeyConsumes[propKeySubID] = 0x0F;                  // uint (variable bytes), Packet: Publish, Subscribe
        PropKeyConsumes[propKeySessionExpiryInterval] = 0x04;  // uint (4 bytes), Packet: Connect, ConnAck, DisConn
        PropKeyConsumes[propKeyAssignedClientID] = 0x10;       // utf-8, Packet: ConnAck
        PropKeyConsumes[propKeyServerKeepalive] = 0x02;        // uint (2 bytes), Packet: ConnAck
        PropKeyConsumes[propKeyAuthMethod] = 0x10;             // utf-8, Packet: Connect, ConnAck, Auth
        PropKeyConsumes[propKeyAuthData] = 0x10;               // binary data, Packet: Connect, ConnAck, Auth
        PropKeyConsumes[propKeyReqProblemInfo] = 0x01;         // byte, Packet: Connect
        PropKeyConsumes[propKeyWillDelayInterval] = 0x04;      // uint (4 bytes), Packet: Will
        PropKeyConsumes[propKeyReqRespInfo] = 0x01;            // byte, Packet: Connect
        PropKeyConsumes[propKeyRespInfo] = 0x10;               // utf-8, Packet: ConnAck
        PropKeyConsumes[propKeyServerRef] = 0x10;              // utf-8, Packet: ConnAck, DisConn
        PropKeyConsumes[propKeyReasonString] = 0x10;           // utf-8, Packet: ConnAck, PubAck, PubRecv, PubRel, PubComp, SubAck, UnSubAck, DisConn, Auth
        PropKeyConsumes[propKeyMaxRecv] = 0x02;                // uint (2 bytes), Packet: Connect, ConnAck
        PropKeyConsumes[propKeyMaxTopicAlias] = 0x02;          // uint (2 bytes), Packet: Connect, ConnAck
        PropKeyConsumes[propKeyTopicAlias] = 0x02;             // uint (2 bytes), Packet: Publish
        PropKeyConsumes[propKeyMaxQos] = 0x01;                 // byte, Packet: ConnAck
        PropKeyConsumes[propKeyRetainAvail] = 0x01;            // byte, Packet: ConnAck
        PropKeyConsumes[propKeyUserProps] = 0x20;              // utf-8 string pair, Packet: Connect, ConnAck, Publish, Will, PubAck, PubRecv, PubRel, PubComp, Subscribe, SubAck, UnSub, UnSubAck, DisConn, Auth
        PropKeyConsumes[propKeyMaxPacketSize] = 0x04;          // uint (4 bytes), Packet: Connect, ConnAck
        PropKeyConsumes[propKeyWildcardSubAvail] = 0x01;       // byte, Packet: ConnAck
        PropKeyConsumes[propKeySharedSubAvail] = 0x01;         // byte, Packet: ConnAck
    }

    unsigned char getPropertyLenCode(int i)
    {
        if (PropKeyConsumes[0] == 0)
        {
            init();
        }
        if (i <= sizeof(PropKeyConsumes))
        {
            return PropKeyConsumes[i];
        }
        return 0xFF;
    };

    // slice mqttBuffer::XXXloadFromFount(fount &f, int amount)
    // {
    //     bool ok = true;
    //     slice extent;
    //     extent.start = 0;
    //     extent.base = buffer;
    //     extent.end = 0;
    //     for (int i = 0; i < amount; i++)
    //     {
    //         extent.end = i;
    //         if (f.empty() == true)
    //         {
    //             return extent;
    //         }
    //         unsigned char c = f.readByte();
    //         buffer[extent.end] = c;
    //         extent.end++;
    //     };
    //     return extent;
    // };

    slice mqttBuffer::loadHexString(const char *hexstr)
    {
        slice extent;
        extent.start = 0;
        extent.base = buffer;
        int amount = hex::decode((unsigned char *)hexstr, 0x7FFF, buffer, size);
        extent.end = amount;
        return extent;
    };

} // namespace mqtt5nano

// Copyright 2020,2021,2022 Alan Tracey Wootton
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

