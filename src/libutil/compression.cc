#include "compression.hh"
#include "types.hh"

#include <lzma.h>
#include <cstdio>

namespace nix {

/* RAII wrapper around lzma_stream. */
struct LzmaStream
{
    lzma_stream strm;
    LzmaStream() : strm(LZMA_STREAM_INIT) { };
    ~LzmaStream() { lzma_end(&strm); };
    lzma_stream & operator()() { return strm; }
};

std::string compressXZ(const std::string & in)
{
    LzmaStream strm;

    // FIXME: apply the x86 BCJ filter?

    lzma_ret ret = lzma_easy_encoder(
        &strm(), 6, LZMA_CHECK_CRC64);
    if (ret != LZMA_OK)
        throw Error("unable to initialise lzma encoder");

    lzma_action action = LZMA_RUN;
    uint8_t outbuf[BUFSIZ];
    string res;
    strm().next_in = (uint8_t *) in.c_str();
    strm().avail_in = in.size();
    strm().next_out = outbuf;
    strm().avail_out = sizeof(outbuf);

    while (true) {

        if (strm().avail_in == 0)
            action = LZMA_FINISH;

        lzma_ret ret = lzma_code(&strm(), action);

        if (strm().avail_out == 0 || ret == LZMA_STREAM_END) {
            res.append((char *) outbuf, sizeof(outbuf) - strm().avail_out);
            strm().next_out = outbuf;
            strm().avail_out = sizeof(outbuf);
        }

        if (ret == LZMA_STREAM_END)
            return res;

        if (ret != LZMA_OK)
            throw Error("error while decompressing xz file");
    }
}

std::string decompressXZ(const std::string & in)
{
    LzmaStream strm;

    lzma_ret ret = lzma_stream_decoder(
        &strm(), UINT64_MAX, LZMA_CONCATENATED);
    if (ret != LZMA_OK)
        throw Error("unable to initialise lzma decoder");

    lzma_action action = LZMA_RUN;
    uint8_t outbuf[BUFSIZ];
    string res;
    strm().next_in = (uint8_t *) in.c_str();
    strm().avail_in = in.size();
    strm().next_out = outbuf;
    strm().avail_out = sizeof(outbuf);

    while (true) {

        if (strm().avail_in == 0)
            action = LZMA_FINISH;

        lzma_ret ret = lzma_code(&strm(), action);

        if (strm().avail_out == 0 || ret == LZMA_STREAM_END) {
            res.append((char *) outbuf, sizeof(outbuf) - strm().avail_out);
            strm().next_out = outbuf;
            strm().avail_out = sizeof(outbuf);
        }

        if (ret == LZMA_STREAM_END)
            return res;

        if (ret != LZMA_OK)
            throw Error("error while decompressing xz file");
    }
}

}
