#include <cstdlib>
#include <cstring>
#include <string>

#include "common.h"

#include <boost/json/src.hpp>

using namespace boost::json;

extern "C" {
    int run(const char* buf, size_t size, char** out_buf, size_t* out_size);
}

int run(const char* buf, size_t size, char** out_buf, size_t* out_size)
{
    std::string buf_s(buf, buf + size);

    try {
        error_code ec;
        value json = parse(buf_s, ec);
        if (ec) {
            return PARSER_ERROR;
        }

        std::string s = serialize(json);
        const char* ss_buf = s.c_str();
        int ss_buf_len = strlen(ss_buf);

        // Use malloc() here, because we later call free() on the buffer
        char* ret_buf =
            static_cast<char*>(malloc(sizeof(char) * (ss_buf_len + 1)));
        if (ret_buf == nullptr) {
            return TOOLCHAIN_ERROR;
        }

        ret_buf[ss_buf_len] = 0;
        memcpy(ret_buf, ss_buf, ss_buf_len);

        *out_buf = ret_buf;
        *out_size = ss_buf_len + 1;
        return PARSER_OKAY;

    } catch (std::bad_alloc const& e) {
        return PARSER_ERROR;
    }
}
