
#include "badjson.h"

// no no no no #include <string>

#include "nanobase64.h"

namespace badjson
{

    bool getJSONinternal(Segment &s, drain &dest, bool isArray);

    struct Chopper
    { // these are really just glorified local variables.
        const char *str;
        const char *endP;
        int i; // an index into the string in str
        int start;
        Segment *tail = 0;
        Segment *front = 0;

        rune r;
        int runeLength;
        rune closer; // might be } or ] when recursing

        void linkToTail(Segment &s)
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

// Disable the "does not return a value in all control paths" warning.
#pragma clang diagnostic ignored "-Wreturn-type"

        // closer will be } or ] when recursing.
        // it returns the head segment, a count of the chars used, and a possible error.
        ResultsTriplette chop(const char *input, const char *inputEndP, rune acloser, int depth)
        {
            if (input >= inputEndP)
            {
                const char *err = "too short";
                return ResultsTriplette(0, 0, err);
            }
            if (depth >= 16)
            {
                const char *err = "too deep";
                return ResultsTriplette(0, 0, err);
            }
            closer = acloser;
            str = input;
            endP = inputEndP;
            i = 0;
            start = i;
            runeLength = utf8::DecodeRuneLengthInString((const unsigned char *)(str + i), endP - str);
            if (runeLength == 1)
                r = str[i];
            else
                r = (char)-1; // must match nothing below.

            while (true)
            {
                while (r == ' ')
                {
                    if (pop())
                    {
                        return ResultsTriplette(front, i, 0);
                    }
                }
                while (r == ',' || r == ':')
                {
                    if (pop())
                    {
                        return ResultsTriplette(front, i, 0);
                    }
                }
                while (r == ' ')
                {
                    if (pop())
                    {
                        return ResultsTriplette(front, i, 0);
                    }
                }
                start = i; // the beginning of our 'token'
                           // use switch ?
                if (r == closer)
                {
                    return ResultsTriplette(front, i, 0);
                }
                else if (r == '$')
                {
                    if (pop())
                    {
                        start = i;
                        goto donehexarray; // output empty array
                    }
                    start = i;
                    while (isHex())
                    {
                        if (pop())
                        {
                            break;
                        }
                    }
                donehexarray:
                    HexBytes *hb = new HexBytes();
                    hb->input = currentString();
                    linkToTail(*hb);
                }
                else if (r == '"' || r == 39) // 39 is single quot
                {
                    char quote = r;
                    if (pop())
                        break;
                    start = i;
                    // int escapeCount = 0;
                    bool hadQuoteOrSlash = false;
                    while (r != quote)
                    {
                        if (r == '\\')
                        {
                            hadQuoteOrSlash = true;
                            if (pop()) // pass the /
                            {
                                break;
                            }
                            if (r == '\'')
                            {
                                if (pop()) // pass the char
                                {
                                    break;
                                }
                            }
                        }
                        else if (r == '"')
                        {
                            hadQuoteOrSlash = true;
                        }
                        if (pop())
                        {
                            break;
                        }
                    }
                    RuneArray *ra = new RuneArray();
                    ra->input = currentString();
                    // ra->hasEscape = hasEscape;
                    // ra->needsQuote = true;
                    // = escapeCount;
                    ra->theQuote = quote;
                    ra->hadQuoteOrSlash = hadQuoteOrSlash;
                    linkToTail(*ra);
                    if (pop())
                    {
                        break;
                    }
                }
                else if (r == '=')
                {
                    slice b64slice;
                    if (pop())
                    {
                        start = i;
                        goto doneb64array; // output empty array
                    }
                    start = i;
                    while (isB64())
                    {
                        if (pop())
                        {
                            break;
                        }
                    }
                    b64slice = currentString();
                    while (r == '=')
                    { // pass any stupic  ='s at the end
                        if (pop())
                        {
                            break;
                        }
                    }
                doneb64array:
                    Base64Bytes *bb = new Base64Bytes();
                    bb->input = b64slice;
                    linkToTail(*bb);
                }
                else if (r == '{' || r == '[')
                {
                    char paren = r;
                    if (pop())
                    {
                        break;
                    }
                    start = i;
                    char closewith = ']';
                    if (paren == '{')
                    {
                        closewith = '}';
                    }
                    Chopper chopper;
                    ResultsTriplette results = chopper.chop(str + i, endP, closewith, depth + 1);

                    if (results.error)
                    {
                        results.i += i; // atw fixme
                        return results;
                    }
                    i = i + results.i;
                    Parent *parent = new Parent();
                    parent->children = results.segment;
                    parent->wasArray = paren == '[';
                    linkToTail(*parent);
                    if (str + i >= endP)
                    {
                        return ResultsTriplette(front, i, 0);
                    }
                    if (pop())
                    {
                        break;
                    }
                }
                else
                { // default:
                    // an unquoted string
                    bool hadQuoteOrSlash = false;
                    while (r != ' ' && r != ':' && r != ',' && r != closer)
                    {
                        if (r == '"' || r == '\\')
                        {
                            hadQuoteOrSlash = true;
                        }
                        if (pop())
                        {
                            break;
                        }
                    }
                    RuneArray *runes = new RuneArray();
                    runes->input = currentString();
                    // runes->hasEscape = false;
                    // runes->needsQuote = true;
                    // runes->escapeCount = 0;
                    runes->theQuote = 0;
                    runes->hadQuoteOrSlash = hadQuoteOrSlash;
                    linkToTail(*runes);
                }
            }

            int newi = 0;
            return ResultsTriplette(front, newi, 0); // dead code
        }

        // utility functions

        // advances the index and returns true if we're done here
        bool pop()
        {
            i += runeLength;
            if (str + i < endP)
            {
                runeLength = utf8::DecodeRuneLengthInString((const unsigned char *)(str + i), endP - str);
                if (runeLength == 1)
                    r = str[i];
                else
                    r = (char)-1; // should match nothing
            }
            else
            {
                r = closer;
                return true; // we're out of input
            }
            return done();
        }
        bool done()
        {
            return str + i >= endP || runeLength == 0;
        }
        slice currentString()
        {
            slice s(str, start, i);
            return s;
        }
        bool isHex()
        {
            return runeLength == 1 && hex::isHex(str[i]);
        }
        bool isB64()
        {
            return runeLength == 1 && base64::isB64(str[i]);
        }
    };

    ResultsTriplette Chop(const char *inputLineOfText, int length)
    {

        Chopper chopper;
        const char *endP = inputLineOfText + length;
        ResultsTriplette results = chopper.chop(inputLineOfText, endP, (char)0, 0);
        if (results.error != nullptr)
        {
            return results;
        }
        if (results.segment == nullptr)
        {
            results.segment = new Segment();
            return results;
        }
        else if (results.segment->Next() == nullptr)
        {
            // so it's just one Segment
            // is it a parent type? no dynamic cast in Arduino
            // denied: Parent *p = dynamic_cast<Parent *>(results.segment);
            Segment *children = results.segment->GetChildren();
            if (children && results.segment->WasArray())
            {
                // We don't care for the case when it's [ contents ] but not { contents }
                // so we'll just change it to return the contents
                results.segment = children;
            }
        }
        return results;
        // btw. Since we don't use the i in the ResultsTriplette we could make another type.
    }

    bool Segment::GetQuoted(drain &s)
    {
        return true;
    }

    bool Segment::Raw(drain &s)
    {
        return true;
    }

    bool Parent::GetQuoted(drain &s)
    {
        bool ok = true;
        if (children)
        {
            bool ok = getJSONinternal(*children, s, wasArray);
        }
        return ok;
    }

    bool Parent::Raw(drain &s)
    {
        bool ok = true;
        if (children)
        {
            bool ok = getJSONinternal(*children, s, wasArray);
        }
        return ok;
    }

    //
    bool xxxxGetRawString(RuneArray &b, drain &s)

    // I expect \a to become \a and \"  to become "
    // so this is not right
    {
        bool failed = false;
        if (false) // b.escapeCount)
        {
            bool passed = false;
            const char *cP = b.input.base;
            int i = b.input.start;
            while (i < b.input.end)
            {
                int runeLen = utf8::DecodeRuneLengthInString((const unsigned char *)(cP + i), b.input.end - i);
                if (runeLen == 1)
                {
                    char r = cP[i];
                    if (r == '\\' && !passed)
                    {
                        passed = true;
                    }
                    else
                    {
                        passed = false;
                        failed |= s.writeByte(cP[i]);
                    }
                }
                else
                {
                    failed |= s.writeBytes(cP + i, runeLen);
                }
                if (failed)
                    break;
                i += runeLen;
            }
        }
        else
        {
            failed = s.write(b.input);
        }
        return failed;
    }

    // a good example of interating utf8 byes
    int xxxNeedsEscape(RuneArray &b)
    {
        int count = 0;
        const char *cP = b.input.base;
        int i = b.input.start;
        while (i < b.input.end)
        {
            int runeLen = utf8::DecodeRuneLengthInString((const unsigned char *)(cP + i), b.input.end - i);
            // break this up. it's too hard to read.
            if (runeLen == 1 && cP[i] == '\\' || cP[i] == '\'' || cP[i] == '"')
            {
                count++;
            }
            i += runeLen;
        }
        return count;
    }

    // EscapeDoubleQuotes outputs the string with all the \ and " having a \ before them
    void EscapeDoubleQuotes(RuneArray &b, drain &s)
    {
        const char *cP = b.input.base;
        int i = b.input.start;
        while (i < b.input.end)
        {
            int runeLen = utf8::DecodeRuneLengthInString((const unsigned char *)(cP + i), b.input.end - i);
            if (runeLen == 1)
            {
                if (cP[i] == '\\' || cP[i] == '"')
                {
                    s.writeByte('\\');
                }
            }
            s.writeBytes(cP + i, runeLen);
            i += runeLen;
        }
    }

    // EscapeDoubleQuotesUnSingle unescapes all the \' and escapes all the " and \
    // todo make combined routine to do EscapeDoubleQuotes depending on flag.
    // to save code.
    void EscapeDoubleQuotesUnSingle(RuneArray &b, drain &s)
    {
        const char *cP = b.input.base;
        int i = b.input.start;
        while (i < b.input.end)
        {
            int runeLen = utf8::DecodeRuneLengthInString((const unsigned char *)(cP + i), b.input.end - i);
            if (runeLen == 1)
            {
                if (cP[i] == '\\' && (i + 1) < b.input.end)
                { // skip the \ if followed by '
                    if (cP[i + 1] == '\'')
                    {
                        i++;
                        continue;
                    }
                }
            }
            s.writeBytes(cP + i, runeLen);
            i += runeLen;
        }
    }

    // String returns the JSON string. That is, it's double quoted and escaped.
    bool RuneArray::GetQuoted(drain &s)
    {
        s.writeByte('"');
        if (this->theQuote == '"')
        {
            // the original text was properly quoted already in the input.
            s.write(input);
        }
        else if (this->theQuote == '\'')
        {
            // we have to unescape all the single quotes and escape all the double q
            // on the fly!
            EscapeDoubleQuotesUnSingle(*this, s);
        }
        else
        {
            // it was an unquoted string in the input
            if (this->hadQuoteOrSlash)
            {
                // we have to escape the double quotes
                EscapeDoubleQuotes(*this, s);
            }
            else
            {
                bool ok = s.write(input);
            }
        }
        bool ok = s.writeByte('"');
        return ok;
    }

    // FIXME: these are not finished

    // returns the unescaped string
    bool RuneArray::Raw(drain &s)
    {
        if (this->theQuote == '"')
        {
            // it was quoted
            if (hadQuoteOrSlash)
            {
                // we'll have to un escape it.
            }
            else
            {
                s.write(input);
            }
        }
        else if (this->theQuote == '\'')
        {
        }
        else
        {
            // wasn't quoted.
        }

        if (!this->theQuote)
        {
            s.write(input);
            return true; // !s.full();
        }

        return true;
    }

    bool Base64Bytes::GetQuoted(drain &s)
    {
        return true;
    }

    bool Base64Bytes::Raw(drain &s)
    {
        return true;
    }

    bool HexBytes::GetQuoted(drain &s)
    {
        return true;
    }

    bool HexBytes::Raw(drain &s)
    {
        return true;
    }

    // expresses a list of Segment's as JSON, Is the String() of the Parent object.
    bool getJSONinternal(Segment &s, drain &dest, bool isArray)
    {
        // what if s is null ?
        // eg. for debugging dest.write("in getJSONinternal\n");
        char oddDelimeter = ',';
        bool isok = true;
        if (isArray)
        {
            dest.writeByte('[');
        }
        else
        {
            dest.writeByte('{');
            oddDelimeter = ':';
        }
        // if (dest.full()) // fixme
        // {
        //     return false;
        // }
        int i;
        Segment *child = &s;
        for (i = 0; child != nullptr; child = child->Next())
        {
            if (i != 0)
            {
                if ((i & 1) != 1)
                {
                    dest.writeByte(',');
                }
                else
                {
                    dest.writeByte(oddDelimeter);
                }
            }

            bool ok = child->GetQuoted(dest);
            if (!ok)
            {
                return ok;
            }
            i++;
        }
        if (isArray)
        {
            dest.writeByte(']');
        }
        else
        {
            dest.writeByte('}');
        }
        return true; // fixme collect actual errors
    }

    // ToString will wrap the list with `[` and `]` and output like child list.
    // todo: rename to ToJsString
    bool ToString(Segment &segment, drain &dest)
    {
        bool ok = getJSONinternal(segment, dest, true);
        return ok;
    }

    Segment *Segment::GetChildren()
    {
        return 0;
    }
    Segment *Parent::GetChildren()
    {
        return children;
    }
    Segment *RuneArray::GetChildren()
    {
        return 0;
    }
    Segment *Base64Bytes::GetChildren()
    {
        return 0;
    }
    Segment *HexBytes::GetChildren()
    {
        return 0;
    }

    bool Segment::WasArray()
    {
        return false;
    }
    bool Parent::WasArray()
    {
        return wasArray;
    }
    bool RuneArray::WasArray()
    {
        return false;
    }
    bool Base64Bytes::WasArray()
    {
        return false;
    }
    bool HexBytes::WasArray()
    {
        return false;
    }

} // namespace


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
