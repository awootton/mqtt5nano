
#include "slices.h"
#include "nanobase64.h"

namespace mqtt5nano {

    const char *slice::emptyStr = "";

    slice::slice(sink s) {
        base = s.base;
        start = 0;
        end = s.start;
    }

    char *slice::gethexstr(char *buffer, int max) {
        buffer[0] = 0;
        if (empty()) {
            buffer[0] = 0;
            return &buffer[0];
        }
        int amt = hex::encode(&base[start], size(), buffer, max - 1);
        buffer[amt] = 0;
        return &buffer[0];
    };

    void slice::gethexstr(sink dest) {
        if (empty()) {
            return;
        }
        dest.writeByte(0);
        int amt = hex::encode(&base[start], size(), &dest.base[dest.start], dest.remaining());
        dest.start += amt;
    };

    char *slice::getCstr(char *buffer, int max) {
        if (empty() == false) {
            int amount = size();
            if (amount + 1 > max) {
                amount = max - 1;
            }
            int i = 0;
            for (; i < amount; i++) {
                buffer[i] = base[start + i];
            }
            buffer[i] = 0;
        }
        return &buffer[0];
    }

    // copy the slice into the buffer, return the buffer.
    char *slice::copy(char *buffer, int max) {
        if (empty() == false) {
            int amount = size();
            if (amount > max) {
                amount = max;
            }
            int i = 0;
            for (; i < amount; i++) {
                buffer[i] = base[start + i];
            }
        }
        return &buffer[0];
    }

    slice slice::b64Decode(sink *buffer) {
        slice s = *this;
        sink d = *buffer;
        int amt = base64::decode(s.base + s.start, s.size(), d.base + d.start, d.remaining());
        slice result(d.base + d.start, d.start, d.start + amt);
        if (buffer->start + amt > buffer->end) {
            buffer->start = buffer->end;
        } else {
            buffer->start += amt;
        }
        return result;
    }

    slice slice::b64Encode(sink *buffer) {
        slice s = *this;
        sink d = *buffer;
        int amt = base64::encode(s.base + s.start, s.size(), d.base + d.start, d.remaining());
        slice result(d.base + d.start, d.start, d.start + amt);
        if (buffer->start + amt > buffer->end) {
            buffer->start = buffer->end;
        } else {
            buffer->start += amt;
        }

        return result;
    }

} // namespace mqtt5nano

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
