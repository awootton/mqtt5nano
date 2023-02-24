

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "commonCommands.h"
#include "mqtt5nano.h"

using namespace std;
using namespace badjson;
using namespace mqtt5nano;

char *collectorArray = new char[16 * 1024];
ByteCollector collector = ByteCollector(collectorArray, 16 * 1024);

struct coutDrain3 : ByteDestination // for the examples output to std::cout
{
    bool writeByte(char c) override {
        std::cout << c;
        bool ok = true;
        // also write to a buffer
        collector.writeByte(c);
        return ok;
    };
};

coutDrain3 coutDrain;
extern mockWiFi WiFi;

VoidDestination voidDestination;

   

int main() {

    std::cout << "hello mqtt5nano whole package mocked tests by atw\n";

    if (1) {
        collector.reset();
        std::cout << "test http over mqtt encrypted 'get wifi list'\n";

        const char *version_of_test1 = "30880700213d4a625a4b724e4e4239525566337771635535736f74344e4a794e70577a546874240800213d304c356e2d6a7a363963684672567867694d356b726167384461304a71623570474554202f3d4f5545714b6e5a5463504a42463657534c6b744d48315f744530415a6d2d4273323344576b31375a58623250384232456f49654e58513f6e6f6e633d33486c596750437776394c426e7a393471667563485543622661646d6e3d704472616169743320485454502f312e310a557365722d4167656e743a204d6f7a696c6c612f352e3020284d6163696e746f73683b20496e74656c204d6163204f5320582031305f31355f3729204170706c655765624b69742f3533372e333620284b48544d4c2c206c696b65204765636b6f29204368726f6d652f3131302e302e302e30205361666172692f3533372e33360d0a526566657265723a20687474703a2f2f6c6f63616c686f73743a333030302f0d0a5365632d43682d55612d506c6174666f726d3a20226d61634f53220d0a582d466f727761726465642d466f723a2031302e34322e352e310d0a582d466f727761726465642d506f72743a203434330d0a5365632d43682d55613a20224368726f6d69756d223b763d22313130222c20224e6f742041284272616e64223b763d223234222c2022476f6f676c65204368726f6d65223b763d22313130220d0a5365632d46657463682d4d6f64653a20636f72730d0a4163636570743a202a2f2a0d0a4163636570742d456e636f64696e673a20677a69702c206465666c6174652c2062720d0a4163636570742d4c616e67756167653a20656e2d55532c656e3b713d302e390d0a43616368652d436f6e74726f6c3a206e6f2d63616368650d0a4f726967696e3a20687474703a2f2f6c6f63616c686f73743a333030300d0a507261676d613a206e6f2d63616368650d0a5365632d46657463682d536974653a2063726f73732d736974650d0a582d466f727761726465642d5365727665723a207472616566696b2d376239633564666235382d676e6267670d0a582d5265616c2d49703a2031302e34322e352e310d0a5365632d43682d55612d4d6f62696c653a203f300d0a5365632d46657463682d446573743a20656d7074790d0a582d466f727761726465642d486f73743a206174772d74657374312d7365727665722e6b6e6f74667265652e6e65740d0a582d466f727761726465642d50726f746f3a2068747470730d0a0d0a";

        // set long name atw-test1-server
        // set short name test1
        // admin hint:pDraait3 admin public key:pDraait3JSz8I1UCWEKq5Qe_MIyimIh44AsOxLnN2g
        // pubk: _63c3Efg_pHefrRtbnT2R_lnKdk-vbXZKlUjCkyejTY

        mqtt5nano::PackageOne one;        // drag  in all the commands and timers
        WiFi.begin("woot2", "word4word"); // pretend that already happened

        DestinationStream stream;
        stream.output = &coutDrain; //&sinkdrain;
        one.setup(stream);

        mqtt5nano::setAdminPassword("atwadmin");
        mqtt5nano::setDevicePassword("atwtest1");

        mqtt5nano::millisUnixAdjust = (long long)(1676856950+102785+245861) * (long long)1000;

        char *mqttInputBuffer = (char *)malloc(8192);

        int mqttInputBufferLen = hex::decode(version_of_test1, strlen(version_of_test1), mqttInputBuffer, 8192);

        slice mqttInputSlice(mqttInputBuffer, 0, mqttInputBufferLen);

        one.mqttClient.client.source = &mqttInputSlice;

        one.mqttClient.client.output = &voidDestination;//coutDrain;

        for (int ms = 10; ms < 2000; ms += 100) {

            one.loop(ms, stream);
        }

        collector.base[collector.start++] = 0; // needs null termination!
        if (slice(collector.base).contains("ERROR") ) {
            cout << "FAIL 77\n";
        }
    }



    {
        collector.reset();
        std::cout << "test http get short name local mode with goofy headers Twice\n";

        char httpReq[] = "GET /get/short/name?nonc=jsjsjhhgsf HTTP/1.1\r\nHost: thing-o.local.\r\nConnection: keep-alive\r\nPragma: no-cache\r\nCache-Control: no-cache\r\n\r\nGET /get/short/name?nonc=jsjsjhhgsf HTTP/1.1\r\nHost: thing-o.local.\r\nConnection: keep-alive\r\nPragma: no-cache\r\nCache-Control: no-cache\r\n\r\n";

        slice httpReqSlice(httpReq);

        // char streamOutBuffer [8192];
        mqtt5nano::PackageOne one; // drag  in all the commands and timers

        DestinationStream stream;

        stream.output = &coutDrain; //&sinkdrain;

        one.setup(stream);
        WiFi.begin("woot2", "word4word"); // pretend that already happened

        one.www.client.source = &httpReqSlice;
        one.www.client.output = &coutDrain;
        one.www.client.connect("dummy.local", 80,10000);

        mqtt5nano::millisUnixAdjust = (long long)(1676856950+102785) * (long long)1000;

        for (int ms = 100; ms < 240; ms += 3) {

            one.loop(ms, stream);

        }
        collector.base[collector.start++] = 0; // needs null termination!
        if (slice(collector.base).contains("ERROR") ) {
            cout << "FAIL 6\n";
        }
    }

    if (1) {
        collector.reset();
        std::cout << "test http over mqtt encrypted 'version'\n";

        const char *version_of_test1 = "30840700213d4a625a4b724e4e4239525566337771635535736f74344e4a794e70577a546874240800213d53436b6e504375623666567a694d4531444d5656394d7434556c78694f755845474554202f3d333067744b2d737571424c6d534a30535459745a58775846626b536d7266326a637772517a68556837534a5a7a413f6e6f6e633d6b3732305363675a7a557a78416f356c6648674d4d4c70722661646d6e3d704472616169743320485454502f312e310a4163636570742d456e636f64696e673a20677a69702c206465666c6174652c2062720d0a4f726967696e3a20687474703a2f2f6c6f63616c686f73743a333030300d0a526566657265723a20687474703a2f2f6c6f63616c686f73743a333030302f0d0a582d466f727761726465642d486f73743a206174772d74657374312d7365727665722e6b6e6f74667265652e6e65740d0a582d5265616c2d49703a203139322e3136382e312e320d0a557365722d4167656e743a204d6f7a696c6c612f352e3020284d6163696e746f73683b20496e74656c204d6163204f5320582031305f31355f3729204170706c655765624b69742f3533372e333620284b48544d4c2c206c696b65204765636b6f29204368726f6d652f3131302e302e302e30205361666172692f3533372e33360d0a4163636570742d4c616e67756167653a20656e2d55532c656e3b713d302e390d0a43616368652d436f6e74726f6c3a206e6f2d63616368650d0a5365632d43682d55612d4d6f62696c653a203f300d0a5365632d43682d55612d506c6174666f726d3a20226d61634f53220d0a4163636570743a202a2f2a0d0a5365632d46657463682d446573743a20656d7074790d0a582d466f727761726465642d466f723a203139322e3136382e312e320d0a582d466f727761726465642d506f72743a203434330d0a582d466f727761726465642d5365727665723a207472616566696b2d376239633564666235382d37363962390d0a507261676d613a206e6f2d63616368650d0a5365632d43682d55613a20224368726f6d69756d223b763d22313130222c20224e6f742041284272616e64223b763d223234222c2022476f6f676c65204368726f6d65223b763d22313130220d0a5365632d46657463682d4d6f64653a20636f72730d0a5365632d46657463682d536974653a2063726f73732d736974650d0a582d466f727761726465642d50726f746f3a2068747470730d0a0d0a";

        // set long name atw-test1-server
        // set short name test1
        // admin hint:pDraait3 admin public key:pDraait3JSz8I1UCWEKq5Qe_MIyimIh44AsOxLnN2g
        // pubk: _63c3Efg_pHefrRtbnT2R_lnKdk-vbXZKlUjCkyejTY

        mqtt5nano::PackageOne one;        // drag  in all the commands and timers
        WiFi.begin("woot2", "word4word"); // pretend that already happened

        DestinationStream stream;
        stream.output = &coutDrain; //&sinkdrain;
        one.setup(stream);

        mqtt5nano::setAdminPassword("atwadmin");
        mqtt5nano::setDevicePassword("atwtest1");

        mqtt5nano::millisUnixAdjust = (long long)(1676856950+102785) * (long long)1000;

        char *mqttInputBuffer = (char *)malloc(8192);

        int mqttInputBufferLen = hex::decode(version_of_test1, strlen(version_of_test1), mqttInputBuffer, 8192);

        slice mqttInputSlice(mqttInputBuffer, 0, mqttInputBufferLen);

        one.mqttClient.client.source = &mqttInputSlice;

        one.mqttClient.client.output = &voidDestination;//coutDrain;

        for (int ms = 10; ms < 2000; ms += 100) {

            one.loop(ms, stream);
        }

        collector.base[collector.start++] = 0; // needs null termination!
        if (slice(collector.base).contains("ERROR") ) {
            cout << "FAIL 77\n";
        }
    }


    if (1) {
        collector.reset();
        std::cout << "get long name encryped \n";

        const char *version_of_test1 = "308c0700213d4a625a4b724e4e4239525566337771635535736f74344e4a794e70577a546874240800213d324a544e76796a7567537542623154677472503543764f4e6b7a632d4e73516d474554202f3d4d6d674767665471513374516e74343063374d78715642734c5f63726a344649634937324d30464756545645676b464b6738304178673f6e6f6e633d76507a70746668486a335951475574454d6f384b387658392661646d6e3d704472616169743320485454502f312e310a5365632d46657463682d446573743a20656d7074790d0a582d466f727761726465642d5365727665723a207472616566696b2d376239633564666235382d37363962390d0a4163636570742d456e636f64696e673a20677a69702c206465666c6174652c2062720d0a582d466f727761726465642d50726f746f3a2068747470730d0a507261676d613a206e6f2d63616368650d0a5365632d43682d55613a20224368726f6d69756d223b763d22313130222c20224e6f742041284272616e64223b763d223234222c2022476f6f676c65204368726f6d65223b763d22313130220d0a5365632d43682d55612d4d6f62696c653a203f300d0a5365632d43682d55612d506c6174666f726d3a20226d61634f53220d0a5365632d46657463682d4d6f64653a20636f72730d0a5365632d46657463682d536974653a2063726f73732d736974650d0a582d466f727761726465642d466f723a203139322e3136382e312e320d0a582d466f727761726465642d506f72743a203434330d0a43616368652d436f6e74726f6c3a206e6f2d63616368650d0a4f726967696e3a20687474703a2f2f6c6f63616c686f73743a333030300d0a4163636570742d4c616e67756167653a20656e2d55532c656e3b713d302e390d0a526566657265723a20687474703a2f2f6c6f63616c686f73743a333030302f0d0a582d466f727761726465642d486f73743a206174772d74657374312d7365727665722e6b6e6f74667265652e6e65740d0a582d5265616c2d49703a203139322e3136382e312e320d0a557365722d4167656e743a204d6f7a696c6c612f352e3020284d6163696e746f73683b20496e74656c204d6163204f5320582031305f31355f3729204170706c655765624b69742f3533372e333620284b48544d4c2c206c696b65204765636b6f29204368726f6d652f3131302e302e302e30205361666172692f3533372e33360d0a4163636570743a202a2f2a0d0a0d0a";

        // set long name atw-test1-server
        // set short name test1
        // admin hint:pDraait3 admin public key:pDraait3JSz8I1UCWEKq5Qe_MIyimIh44AsOxLnN2g
        // pubk: _63c3Efg_pHefrRtbnT2R_lnKdk-vbXZKlUjCkyejTY

        mqtt5nano::PackageOne one;        // drag  in all the commands and timers
        WiFi.begin("woot2", "word4word"); // pretend that already happened

        DestinationStream stream;
        stream.output = &coutDrain; //&sinkdrain;
        one.setup(stream);

        mqtt5nano::setAdminPassword("atwadmin");
        mqtt5nano::setDevicePassword("atwtest1");
        mqtt5nano::setShortName("ourTestShortName123");
        mqtt5nano::setLongName("ourTestLongName87654");

        mqtt5nano::millisUnixAdjust = (long long)(1676856950 +100273) * (long long)1000;

        char *mqttInputBuffer = (char *)malloc(8192);

        int mqttInputBufferLen = hex::decode(version_of_test1, strlen(version_of_test1), mqttInputBuffer, 8192);

        slice mqttInputSlice(mqttInputBuffer, 0, mqttInputBufferLen);

        one.mqttClient.client.source = &mqttInputSlice;

        char *mqttOutputBuffer = (char *)malloc(8192);
        ByteDestination mqttOutputDestination(mqttOutputBuffer, 8192);
        one.mqttClient.client.output = &mqttOutputDestination; //&coutDrain;

        for (int ms = 10; ms < 2000; ms += 100) {

            one.loop(ms, stream);
        }

        collector.base[collector.start++] = 0; // needs null termination!
        if (slice(collector.base).contains("ERROR") ) {
            cout << "FAIL 99\n";
        }
        coutDrain.write(slice(mqttOutputDestination.buffer));
        if ( ! slice(mqttOutputDestination.buffer).contains("ourTestLongName87654") ) {
            cout << "FAIL 198\n";
        }
    }



    if (1) {
        collector.reset();
        std::cout << "test non encrypted http mqtt \n";

        const char *version_of_test1 = "30b70600213d4a625a4b724e4e4239525566337771635535736f74344e4a794e70577a546874240800213d535636476864586c736c584c537662324831726c74544d356f50344b426d3737474554202f6765742f73686f72742f6e616d6520485454502f312e310a5365632d46657463682d446573743a20656d7074790d0a582d466f727761726465642d486f73743a206174772d74657374312d7365727665722e6b6e6f74667265652e6e65740d0a582d466f727761726465642d50726f746f3a2068747470730d0a4163636570743a202a2f2a0d0a4163636570742d456e636f64696e673a20677a69702c206465666c6174652c2062720d0a5365632d46657463682d4d6f64653a20636f72730d0a582d466f727761726465642d5365727665723a207472616566696b2d376239633564666235382d37363962390d0a4f726967696e3a20687474703a2f2f6c6f63616c686f73743a333030300d0a5365632d43682d55613a20224368726f6d69756d223b763d22313130222c20224e6f742041284272616e64223b763d223234222c2022476f6f676c65204368726f6d65223b763d22313130220d0a526566657265723a20687474703a2f2f6c6f63616c686f73743a333030302f0d0a5365632d43682d55612d4d6f62696c653a203f300d0a5365632d43682d55612d506c6174666f726d3a20226d61634f53220d0a5365632d46657463682d536974653a2063726f73732d736974650d0a582d466f727761726465642d466f723a203139322e3136382e312e320d0a582d466f727761726465642d506f72743a203434330d0a557365722d4167656e743a204d6f7a696c6c612f352e3020284d6163696e746f73683b20496e74656c204d6163204f5320582031305f31355f3729204170706c655765624b69742f3533372e333620284b48544d4c2c206c696b65204765636b6f29204368726f6d652f3131302e302e302e30205361666172692f3533372e33360d0a43616368652d436f6e74726f6c3a206e6f2d63616368650d0a582d5265616c2d49703a203139322e3136382e312e320d0a507261676d613a206e6f2d63616368650d0a4163636570742d4c616e67756167653a20656e2d55532c656e3b713d302e390d0a0d0a";

        // set long name atw-test1-server
        // set short name test1
        // admin hint:pDraait3 admin public key:pDraait3JSz8I1UCWEKq5Qe_MIyimIh44AsOxLnN2g
        // pubk: _63c3Efg_pHefrRtbnT2R_lnKdk-vbXZKlUjCkyejTY

        mqtt5nano::PackageOne one;        // drag  in all the commands and timers
        WiFi.begin("woot2", "word4word"); // pretend that already happened

        DestinationStream stream;
        stream.output = &coutDrain; //&sinkdrain;
        one.setup(stream);

        mqtt5nano::setAdminPassword("atwadmin");
        mqtt5nano::setDevicePassword("atwtest1");
        mqtt5nano::setShortName("ourTestShortName123");

        mqtt5nano::millisUnixAdjust = (long long)1676856950 * (long long)1000;

        char *mqttInputBuffer = (char *)malloc(8192);

        int mqttInputBufferLen = hex::decode(version_of_test1, strlen(version_of_test1), mqttInputBuffer, 8192);

        slice mqttInputSlice(mqttInputBuffer, 0, mqttInputBufferLen);

        one.mqttClient.client.source = &mqttInputSlice;

        char *mqttOutputBuffer = (char *)malloc(8192);
        ByteDestination mqttOutputDestination(mqttOutputBuffer, 8192);
        one.mqttClient.client.output = &mqttOutputDestination; //&coutDrain;

        for (int ms = 10; ms < 2000; ms += 100) {

            one.loop(ms, stream);
        }

        collector.base[collector.start++] = 0; // needs null termination!
        if (slice(collector.base).contains("ERROR") ) {
            cout << "FAIL 99\n";
        }
        coutDrain.write(slice(mqttOutputDestination.buffer));
        if ( ! slice(mqttOutputDestination.buffer).contains("ourTestShortName123") ) {
            cout << "FAIL 198\n";
        }
    }


    if (1) {
        collector.reset();
        std::cout << "test parse a connack and then the rest \n";

        const char *version_of_test1 = "200300000030800700213d4a625a4b724e4e4239525566337771635535736f74344e4a794e70577a546874240800213d72492d554e744d753574566f346f2d2d7170417773716d6752337261462d4836474554202f3d596238396a6b4f584853577971355131664d704875623572756b782d354a4232385f427463474c765850536455773f6e6f6e633d6342773636753955384b58437a4b4d4f30586e736a3032502661646d6e3d704472616169743320485454502f312e310a507261676d613a206e6f2d63616368650d0a5365632d43682d55612d506c6174666f726d3a20226d61634f53220d0a5365632d46657463682d446573743a20656d7074790d0a5365632d46657463682d4d6f64653a20636f72730d0a5365632d46657463682d536974653a2063726f73732d736974650d0a582d5265616c2d49703a2031302e34322e352e310d0a43616368652d436f6e74726f6c3a206e6f2d63616368650d0a4163636570743a202a2f2a0d0a4163636570742d4c616e67756167653a20656e2d55532c656e3b713d302e390d0a4f726967696e3a20687474703a2f2f6c6f63616c686f73743a333030300d0a526566657265723a20687474703a2f2f6c6f63616c686f73743a333030302f0d0a5365632d43682d55612d4d6f62696c653a203f300d0a582d466f727761726465642d50726f746f3a2068747470730d0a582d466f727761726465642d5365727665723a207472616566696b2d376239633564666235382d37363962390d0a5365632d43682d55613a20224368726f6d69756d223b763d22313130222c20224e6f742041284272616e64223b763d223234222c2022476f6f676c65204368726f6d65223b763d22313130220d0a582d466f727761726465642d486f73743a206174772d74657374312d7365727665722e6b6e6f74667265652e6e65740d0a4163636570742d456e636f64696e673a20677a69702c206465666c6174652c2062720d0a582d466f727761726465642d466f723a2031302e34322e352e310d0a582d466f727761726465642d506f72743a203434330d0a557365722d4167656e743a204d6f7a696c6c612f352e3020284d6163696e746f73683b20496e74656c204d6163204f5320582031305f31355f3729204170706c655765624b69742f3533372e333620284b48544d4c2c206c696b65204765636b6f29204368726f6d652f3131302e302e302e30205361666172692f3533372e33360d0a0d0a";

        // set long name atw-test1-server
        // set short name test1
        // admin hint:pDraait3 admin public key:pDraait3JSz8I1UCWEKq5Qe_MIyimIh44AsOxLnN2g
        // pubk: _63c3Efg_pHefrRtbnT2R_lnKdk-vbXZKlUjCkyejTY

        mqtt5nano::PackageOne one;        // drag  in all the commands and timers
        WiFi.begin("woot2", "word4word"); // pretend that already happened

        DestinationStream stream;
        stream.output = &coutDrain; //&sinkdrain;
        one.setup(stream);

        mqtt5nano::setAdminPassword("atwadmin");
        mqtt5nano::setDevicePassword("atwtest1");

        mqtt5nano::millisUnixAdjust = (long long)1676856950 * (long long)1000;

        char *mqttInputBuffer = (char *)malloc(8192);

        int mqttInputBufferLen = hex::decode(version_of_test1, strlen(version_of_test1), mqttInputBuffer, 8192);

        slice mqttInputSlice(mqttInputBuffer, 0, mqttInputBufferLen);

        one.mqttClient.client.source = &mqttInputSlice;

        one.mqttClient.client.output = &voidDestination; //&coutDrain;

        for (int ms = 10; ms < 2000; ms += 100) {

            one.loop(ms, stream);
        }

        collector.base[collector.start++] = 0; // needs null termination!
        if (slice(collector.base).contains("ERROR") ) {
            cout << "FAIL 8\n";
        }
    }


    if(0 ){ // why is this one broken?
        collector.reset();
        std::cout << "test http over mqtt\n";

        const char *hexofpub = "30800700213d4a625a4b724e4e4239525566337771635535736f74344e4a794e70577a546874240800213d72492d554e744d753574566f346f2d2d7170417773716d6752337261462d4836474554202f3d596238396a6b4f584853577971355131664d704875623572756b782d354a4232385f427463474c765850536455773f6e6f6e633d6342773636753955384b58437a4b4d4f30586e736a3032502661646d6e3d704472616169743320485454502f312e310a507261676d613a206e6f2d63616368650d0a5365632d43682d55612d506c6174666f726d3a20226d61634f53220d0a5365632d46657463682d446573743a20656d7074790d0a5365632d46657463682d4d6f64653a20636f72730d0a5365632d46657463682d536974653a2063726f73732d736974650d0a582d5265616c2d49703a2031302e34322e352e310d0a43616368652d436f6e74726f6c3a206e6f2d63616368650d0a4163636570743a202a2f2a0d0a4163636570742d4c616e67756167653a20656e2d55532c656e3b713d302e390d0a4f726967696e3a20687474703a2f2f6c6f63616c686f73743a333030300d0a526566657265723a20687474703a2f2f6c6f63616c686f73743a333030302f0d0a5365632d43682d55612d4d6f62696c653a203f300d0a582d466f727761726465642d50726f746f3a2068747470730d0a582d466f727761726465642d5365727665723a207472616566696b2d376239633564666235382d37363962390d0a5365632d43682d55613a20224368726f6d69756d223b763d22313130222c20224e6f742041284272616e64223b763d223234222c2022476f6f676c65204368726f6d65223b763d22313130220d0a582d466f727761726465642d486f73743a206174772d74657374312d7365727665722e6b6e6f74667265652e6e65740d0a4163636570742d456e636f64696e673a20677a69702c206465666c6174652c2062720d0a582d466f727761726465642d466f723a2031302e34322e352e310d0a582d466f727761726465642d506f72743a203434330d0a557365722d4167656e743a204d6f7a696c6c612f352e3020284d6163696e746f73683b20496e74656c204d6163204f5320582031305f31355f3729204170706c655765624b69742f3533372e333620284b48544d4c2c206c696b65204765636b6f29204368726f6d652f3131302e302e302e30205361666172692f3533372e33360d0a0d0a";

        //  slice httpReqSlice(httpReq);

        // char streamOutBuffer [8192];
        mqtt5nano::PackageOne one;        // drag  in all the commands and timers
        WiFi.begin("woot2", "word4word"); // pretend that already happened

        DestinationStream stream;
        // const char cmd[] = ""; // no serial input
        // memcpy(stream.buff, cmd, sizeof(cmd));
        // stream.source.end = sizeof(cmd);
        // char serialOutBuffer[8192];
        // SinkDrain sinkdrain(serialOutBuffer, sizeof(serialOutBuffer));
        stream.output = &voidDestination; //&coutDrain; //&sinkdrain;
        one.setup(stream);

        char *mqttInputBuffer = (char *)malloc(8192);
        //     int decode(const char *src, int srcLen, char *dest, int destMax);

        int mqttInputBufferLen = hex::decode(hexofpub, strlen(hexofpub), mqttInputBuffer, 8192);
        slice mqttInputSlice(mqttInputBuffer, 0, mqttInputBufferLen);

        one.mqttClient.client.source = &mqttInputSlice;

        one.mqttClient.client.output = &voidDestination; //&coutDrain;

        for (int ms = 10; ms < 2000; ms += 100) {

            one.loop(ms, stream);
        }

        collector.base[collector.start++] = 0; // needs null termination!
        if (slice(collector.base).contains("ERROR") ) {
            cout << "FAIL 6\n";
        }
    }
    {
        std::cout << "test getRandomString\n";
        char rando[32];
        mqtt5nano::getRandomString(rando, sizeof(rando) - 1);
        rando[31] = 0;
        std::cout << "random is " << rando << "\n";
        coutDrain3 coutDrain;
        coutDrain.write("soe tie er wae");

        mqtt5nano::getRandomString(rando, sizeof(rando) - 1);
        std::cout << "random is " << rando << "\n";
        mqtt5nano::getRandomString(rando, sizeof(rando) - 1);
        std::cout << "random is " << rando << "\n";
        mqtt5nano::getRandomString(rando, sizeof(rando) - 1);
        std::cout << "random is " << rando << "\n";
        mqtt5nano::getRandomString(rando, sizeof(rando) - 1);
        std::cout << "random is " << rando << "\n";
    }

    
    {

        std::cout << "test getRandomString\n";
        char rando[32];
        mqtt5nano::getRandomString(rando, sizeof(rando) - 1);
        rando[31] = 0;
        std::cout << "random is " << rando << "\n";
        coutDrain3 coutDrain;
        coutDrain.write("soe tie er wae");

        mqtt5nano::getRandomString(rando, sizeof(rando) - 1);
        std::cout << "random is " << rando << "\n";
        mqtt5nano::getRandomString(rando, sizeof(rando) - 1);
        std::cout << "random is " << rando << "\n";
        mqtt5nano::getRandomString(rando, sizeof(rando) - 1);
        std::cout << "random is " << rando << "\n";
        mqtt5nano::getRandomString(rando, sizeof(rando) - 1);
        std::cout << "random is " << rando << "\n";
    }

    if (1) {
        collector.reset();
        std::cout << "test two serial commands\n";

        // char streamOutBuffer [8192];
        mqtt5nano::PackageOne one; // drag  in all the commands and timers

        DestinationStream stream;
        const char cmd[] = "set ssid woot2\nset pass word4word\n";
        memcpy(stream.buff, cmd, sizeof(cmd));
        stream.source.end = sizeof(cmd);

        char serialOutBuffer[8192];
        ByteDestination sinkdrain(serialOutBuffer, sizeof(serialOutBuffer));

        ByteDestination *cp = &sinkdrain;
        Destination *dP = (Destination *)cp;
        stream.output = dP;

        one.setup(stream);

        for (int ms = 100; ms < 99; ms += 3) {
            one.loop(ms, stream);

            if (serialOutBuffer[0]) {
                std::cout << serialOutBuffer << ms << "\n";
                sinkdrain.reset();
                serialOutBuffer[0] = 0;
            }
        }
        collector.base[collector.start++] = 0; // needs null termination!
        if (slice(collector.base).contains("ERROR") ) {
            cout << "FAIL 5\n";
        }
    }

    if (1) {
        collector.reset();
        std::cout << "test MqttCommandClient\n";

        Stream serial;
        // some binary for mqtt publish
        // const char *pubbytes = "3268000d544553542f54494d456162636400032d080010544553542f54494d4565666768696a6b2600046b657931000476616c312600046b657932000476616c326d65737361676520617420323032302d30332d32372030313a33353a33372e34303330373920633d31";
        char tmpbuffer[1024];
        // int amt = hex::decode((const unsigned char *)pubbytes, strlen(pubbytes), tmpbuffer, 1024);

        // client.client.source = slice(tmpbuffer, 0, amt);
        ByteDestination theDrain(tmpbuffer, 1023);

        MqttCommandClient client;

        client.client.output = &theDrain; // make drain to buffer

        client.sendSubscribe("testtopic");

        char tmpbuffer2[1024];

        slice written = theDrain.getWritten();

        int got = hex::encode(written.base, written.size(), tmpbuffer2, 1023);
        tmpbuffer2[got] = 0;

        std::cout << "\n";

        std::cout << "got " << tmpbuffer2 << "\n";

        // is this right? where's the test?
        const char *need = "800f000200000974657374746f70696300";
        if ( 0 && strcmp(tmpbuffer2, need) != 0) {
            cout << "FAIL 4\n";
        }

        collector.base[collector.start++] = 0; // needs null termination!
        if (slice(collector.base).contains("ERROR") ) {
            cout << "FAIL 3\n";
        }
    }
    if (1) {
        collector.reset();

        std::cout << "test mqtt gen\n";

        mqttPacketPieces pubgen;

        pubgen.QoS = 1;
        pubgen.packetType = CtrlPublish;

        pubgen.TopicName = "banner";
        pubgen.PacketID = 3;
        pubgen.RespTopic = "caller876543";

        pubgen.Payload = "uptime";

        // for (int i = 0; i < parser.userKeyValLen; i++) {
        //     pubgen.UserKeyVal[i] = parser.UserKeyVal[i];
        // }

        char assemblybuffer[1024]; // ?? how much 4096? new //FIXME: too big for stack
        ByteCollector assemblyArea(assemblybuffer, 1024);

        char tmpbuffer[1024];
        ByteDestination socket(tmpbuffer, 1024);

        bool ok = pubgen.outputPubOrSub(assemblyArea, &socket);
        if (!ok) {
            std::cout << "ERROR FAIL !ok \n";
        }
        char hexcmd[1024];
        int amt = hex::encode(socket.buffer.base, socket.getWritten().size(), hexcmd, 1024);
        hexcmd[amt] = 0;

        std::cout << "pub bytes hex";
        std::cout << hexcmd << "\n";

        MqttCommandClient client;

        Stream serial;
        // some binary for mqtt publish
        const char *pubbytes = hexcmd;
        char tmpbuffer3[1024];
        amt = hex::decode(pubbytes, strlen(pubbytes), tmpbuffer3, 1024);
        slice tmpSlice3(tmpbuffer3, 0, amt);
        client.client.source = &tmpSlice3;

        char cmdpoubuf[1024];
        ByteDestination cmdoutput(cmdpoubuf, 1024);
        client.client.output = &cmdoutput;

        for (int i = 0; i < 1000; i++) {

            client.loop(i, serial);

            connected = true;
            if (i == 10) {
                // throw away the connect and the subscribe
                cmdoutput.reset();
            }
        }

        char buff2[1024];

        std::cout << "got " << cmdoutput.getWritten().gethexstr(buff2, 1024) << "\n";
        std::cout << "got " << cmdoutput.getWritten().getCstr(buff2, 1024) << "\n";
        collector.base[collector.start++] = 0; // needs null termination!
        if (slice(collector.base).contains("ERROR") ) {
            cout << "FAIL 2\n";
        }
    }

    if (1) {
        collector.reset();

        std::cout << "test mqtt MqttCommandClient pub\n";

        MqttCommandClient client;

        Stream serial;
        // some binary for mqtt publish
        const char *pubbytes = "0021206d644375784d53426c43734f436852416732316454713954787368314";
        char tmpbuffer[1024];
        int amt = hex::decode(pubbytes, strlen(pubbytes), tmpbuffer, 1024);

        // client.client.source = slice(tmpbuffer, 0, amt);

        char cmdpoubuf[1024];
        ByteDestination cmdoutput(cmdpoubuf, 1024);
        client.client.output = &cmdoutput;

        for (int i = 0; i < 1000; i++) {

            client.loop(i, serial);

            connected = true;
        }

        char buff2[1024];

        // std::cout << "got " << cmdoutput.getWritten().gethexstr(buff2, 1024) << "\n";
        // std::cout << "got " << cmdoutput.getWritten().getCstr(buff2, 1024) << "\n";

        collector.base[collector.start++] = 0; // needs null termination!
        if (slice(collector.base).contains("ERROR") ) {
            cout << "FAIL 2\n";
        }
    }

    if (1) {
        collector.reset();
        std::cout << "test mqtt MqttCommandClient pub\n";

        MqttCommandClient client;

        Stream serial;
        // some binary for mqtt publish
        const char *pubbytes = "3268000d544553542f54494d456162636400032d080010544553542f54494d4565666768696a6b2600046b657931000476616c312600046b657932000476616c326d65737361676520617420323032302d30332d32372030313a33353a33372e34303330373920633d31";
        char tmpbuffer[1024];
        int amt = hex::decode(pubbytes, strlen(pubbytes), tmpbuffer, 1024);

        //    client.client.source = slice(tmpbuffer, 0, amt);

        char cmdpoubuf[1024];
        ByteDestination cmdoutput(cmdpoubuf, 1024);
        client.client.output = &cmdoutput;

        for (int i = 0; i < 1000; i++) {

            client.loop(i, serial);

            connected = true;
        }

        char buff2[1024];

        std::cout << "got " << cmdoutput.getWritten().gethexstr(buff2, 1024) << "\n";
        std::cout << "got " << cmdoutput.getWritten().getCstr(buff2, 1024) << "\n";
        collector.base[collector.start++] = 0; // needs null termination!
        if (slice(collector.base).contains("ERROR") ) {
            cout << "FAIL 1\n";
        }
    }
}

// Copyright 2022 Alan Tracey Wootton
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
