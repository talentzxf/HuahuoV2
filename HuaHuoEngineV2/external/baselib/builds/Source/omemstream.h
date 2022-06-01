#pragma once

#include <iomanip>
#include <ostream>
#include <algorithm>

// This is ostream that writes to provided buffer, guaranteed to be null terminated.
// If piped more data that buffer can accommodate, will not set EOF flag,
// instead will count them in overflow byte counter.
// Total amount of bytes that would have been written can be accessed via totalBytes.
class omemstream : public std::ostream
{
public:
    omemstream(char* buffer, size_t bufferSize) :
        std::ostream(&membuffer),
        membuffer(buffer, bufferSize)
    {
    }

    size_t totalBytes() const
    {
        return membuffer.totalBytes();
    }

protected:

    class memstreambuf : public std::streambuf
    {
    public:
        memstreambuf(char* buffer, size_t bufferSize)
        {
            if (buffer && (bufferSize > 0))
            {
                // leave space for null terminator in the end
                setp(buffer, buffer + bufferSize - 1);
                std::fill_n(buffer, bufferSize, 0);
            }
        }

        size_t totalBytes() const
        {
            return pptr() - pbase() + overflowbytes;
        }

    protected:
        size_t overflowbytes = 0;

        int overflow(int c)
        {
            ++overflowbytes;
            // never set EOF here
            // so we get all extra bytes passed via overflow
            return c;
        }
    };

    memstreambuf membuffer;
};
