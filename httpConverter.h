#pragma once

#include "badjson.h"

namespace knotfree
{
    /** We are going to consider that every command line might actually
     * be HTTP instead of just a command line with words and ending with \n
     * In particular sending commands with mychannelname.knotfree.net
     * will end up passing an HTTP GET as a MQTT message.
     * We can spot this since it starts with "GET " and ends with \n\r\n\r
     * in the case where it is HTTP we make the path into a sequence of words
     * and the part after the ? into key val pairs and also the header into key val pairs.
     *
     * If what we have in the buffer cannot be HTTP we set noWayThisIsHttp
     * If it still might be HTTP when we get more bytes then looksLikeHttpButNeedsMore is set.
     *
     * One MUST remember to call delete path and delete params if they are not null.
     *
     * eg
     *  GET /echo HTTP/1.1 CRLF
        Host: reqbin.com CRLF
        CRLF

        CRLN is \r\n


     *
     */

    struct ParsedHttp
    {
        // bool looksLikeHttpButNeedsMore = true;
        // bool noWayThisIsHttp = false;

        // eg GET /static/js/2.05e7be1f.chunk.js HTTP/1.1

        badjson::Segment *command = nullptr;
        badjson::Segment *params = nullptr; // key, val, key2, val2 so count is even
        // the params and the headers are in params.

        // we may have some params setup before we try to parse.

        badjson::Segment *tail = 0;
        badjson::Segment *front = 0;
        void linkToTail(badjson::Segment &s)
        {
            if (!front)
            {
                front = &s;
            }
            if (tail)
            {
                tail->SetNext(&s);
            }
            tail = &s;
        }
        void linkFrontToParams(){
            if ( params != nullptr ){
                linkToTail(*params);
            }
            params = front;
            front = nullptr;
            tail = nullptr;
        }
        void linkFrontToCommand(){
            if ( command != nullptr ){
                linkToTail(*command);
            }
            command = front;
            front = nullptr;
            tail = nullptr;
        }
        void addWord(slice word)
        {
            badjson::RuneArray *ra = new badjson::RuneArray();
            ra->input = word;
            ra->theQuote = '"'; // there was no quote
            ra->hadQuoteOrSlash = false;
            linkToTail(*ra);
        }

        slice pass(slice s, char what)
        {
            while (s.base[s.start] == what && s.start < s.end)
            {
                s.start++;
            }
            return s;
        }
        // pass not space or : or / or ? or whatever but ntot \n or \r
        slice passNot(slice s, char what)
        {
            while (s.base[s.start] != what && s.base[s.start] >= ' ' && s.start < s.end)
            {
                s.start++;
            }
            return s;
        }
        slice passCtrl(slice s)
        {
            while (s.base[s.start] < ' ' && s.start < s.end)
            {
                s.start++;
            }
            return s;
        }
        slice passNotCtrl(slice s)
        {
            while (s.base[s.start] >= ' ' && s.start < s.end)
            {
                s.start++;
            }
            return s;
        }
        slice passNotPathDelim(slice s)
        {
            while (s.base[s.start] != ' ' &&
                   s.base[s.start] != '/' &&
                   s.base[s.start] != '?' &&
                   s.base[s.start] != '=' &&
                   s.base[s.start] != '&' && s.start < s.end)
            {
                s.start++;
            }
            return s;
        }

        bool convert(slice s)
        {
            char buffer[333];
            if (!isWholeRequest(s))
            {
                return false; // not ok
            }
            slice pos(s);   // the position of the parse.
            pos.start += 4; // pass "GET "
            slice path = pass(pos, ' ');
            slice endOfPath = passNot(path, ' ');
            path.end = endOfPath.start;

            slice http11 = pass(endOfPath, ' ');
            slice tmp = passNot(http11, ' ');
            http11.end = tmp.start;

            if (!http11.startsWith("HTTP/1.1"))
            {
                return false;
            }
            pos.start = http11.end;
            // std::cout << "path " << path.getCstr(buffer, 333) << "\n";
            // std::cout << "http1.1" << http11.getCstr(buffer, 333) << "\n";
            // let's parse the path now. It's a shame we scan it twice

            while (path.start < path.end && !path.startsWith(" ") && !path.startsWith("?"))
            {
                slice word = pass(path, '/');
                slice tmp = passNotPathDelim(word);
                word.end = tmp.start;
                addWord(word);
                // std::cout << "word " << word.getCstr(buffer, 333) << "\n";
                path.start = word.end;
            }
            linkFrontToCommand();
            //command = front;
           // front = nullptr;
            //tail = nullptr;
            if (path.startsWith("?"))
            {
                // we have params
                while (path.size() != 0)
                {
                    path.start++;
                    slice key = path;
                    tmp = passNotPathDelim(key);
                    key.end = tmp.start;
                    path.start = key.end;
                    if (key.size() == 0)
                    {
                        break; // very bad
                    }
                    path.start++;
                    slice val = path;
                    tmp = passNotPathDelim(val);
                    val.end = tmp.start;
                    path.start = val.end;
                    if (val.size() == 0)
                    {
                        break; // very bad
                    }
                    addWord(key);
                    addWord(val);
                    // std::cout << "key " << key.getCstr(buffer, 333) << "\n";
                    // std::cout << "val " << val.getCstr(buffer, 333) << "\n";
                }
            }

            while (pos.start < pos.end && !pos.startsWith("\r\n\r\n"))
            {
                pos = passCtrl(pos);
                slice key = pos;
                slice colon = passNot(key, ':');
                key.end = colon.start;
                pos = colon;
                pos.start++;
                slice val = pass(pos, ' ');
                pos = passNot(val, ' ');
                val.end = pos.start;
                pos = pass(pos, ' ');
                // std::cout << "key " << key.getCstr(buffer, 333) << "\n";
                // std::cout << "val " << val.getCstr(buffer, 333) << "\n";
                addWord(key);
                addWord(val);
            }
            linkFrontToParams();
            //params = front;
            //front = nullptr;
            //tail = nullptr;
            return true;
        }
        // we're going to call this on every char added to s
        // so make it snappy.
        static bool isWholeRequest(slice s)
        {
            if (s.size() < 18)
            {
                return false;
            }
            slice last(s);
            last.start = last.end - 4;
            if (!last.startsWith("\r\n\r\n"))
            {
                return false;
            }
            if (!s.startsWith("GET "))// or post?
            {
                return false;
            }
            // TODO: more later
            return true;
        }
    };

    ParsedHttp IsHttp(slice input);
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
