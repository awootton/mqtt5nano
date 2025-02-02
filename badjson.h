
#pragma once

#include "slices.h"

using namespace mqtt5nano; // we'll use slices

namespace badjson {
    struct ResultsTriplette; // below

    extern int segmentsAllocated;

    // Chop is how we use the badjson parser.
    // pass a pointer to some text and the length
    // and it will pass back the first object of a linked list.
    ResultsTriplette Chop(const char *, int);

    // the virtual base class. Aka the interface.
    struct Segment {
        Segment *next;
        slice input;

        Segment() {
            segmentsAllocated++;
            input.base = nullptr;
            next = nullptr;
        }
        virtual ~Segment() {
            segmentsAllocated--;
            if (next)
                delete next;
        }

        Segment *Next() { return next; }
        void SetNext(Segment *s) { next = s; };

        // these return false if it failed.
        // aka they return if it's ok.
        virtual bool GetQuoted(Destination &s);
        virtual bool Raw(Destination &s);
        virtual Segment *GetChildren(); // always nullptr unless parent
        virtual bool WasArray();        // if parent true means it was [] and false means {}
    };

    // Parent has a sub-list
    struct Parent : Segment {
        Segment *children;
        bool wasArray; // eg. started with [ and not {

        Parent() {
            children = 0;
        }
        virtual ~Parent() {
            if (children) {
                delete (children);
            }
        };

        bool GetQuoted(Destination &s) override;
        bool Raw(Destination &s) override;
        Segment *GetChildren() override;
        bool WasArray() override;
    };

    // RuneArray aka string
    struct RuneArray : Segment {
        // bool hasEscape;
        // bool needsQuote;

        // int escapeCount;
        char theQuote;
        bool hadQuoteOrSlash; // had double quote or '\' when scanned.

        RuneArray() {
            theQuote = 0;
            hadQuoteOrSlash = false;
        }
        virtual ~RuneArray(){
        };

        // returns false if not ok.
        bool GetQuoted(Destination &s) override;
        bool Raw(Destination &s) override;
        Segment *GetChildren() override;
        bool WasArray() override;
    };

    struct Base64Bytes : Segment {
        Base64Bytes() {
        }
        virtual ~Base64Bytes(){
        };
        bool GetQuoted(Destination &s) override;
        bool Raw(Destination &s) override;
        Segment *GetChildren() override;
        bool WasArray() override;
    };

    struct HexBytes : Segment {
        HexBytes() {
        }
        virtual ~HexBytes(){
        };
        bool GetQuoted(Destination &s) override;
        bool Raw(Destination &s) override;
        Segment *GetChildren() override;
        bool WasArray() override;
    };

    // ResultsTriplette is used because C++ can't return multiple values
    struct ResultsTriplette {
        Segment *segment;  // if nullptr there should be an error.
        int i;             // index in input string.
        const char *error; // usually null

        ResultsTriplette(Segment *segment, int i, const char *err)
            : segment(segment), i(i), error(err){};
    };

    // ToString will wrap the list with `[` and `]` and output like child list.
    // evrything is double quoted.
    bool ToString(Segment *segment, Destination &dest); // in the cpp

    typedef unsigned char rune;

    // getJSONinternal will write the whole thing in json format. TODO: rename
    // use ToString instead
    // bool getJSONinternal(Segment &s, drain &dest, bool isArray);

} // namespace badjson

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
