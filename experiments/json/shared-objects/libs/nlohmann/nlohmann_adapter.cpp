#include <cstddef>
#include <cstdlib>
#include <cstring>

#include "common.h"

#include "nlohmann/single_include/nlohmann/json.hpp"

// for convenience
using json = nlohmann::json;

extern "C" {
    int run(const char* buf, size_t size, char** out_buf, size_t* out_size);
}

int run(const char* buf, size_t size, char** out_buf, size_t* out_size)
{
    std::vector<char> buf_vec(buf, buf + size);

    // parse json, exceptions disabled, otherwise must catch json::parse_error&
    json j = json::parse(buf_vec.begin(), buf_vec.end(), nullptr, false, false);
    if (j.is_discarded()) {
        return PARSER_ERROR;
    }

    std::string s;
    try {
        // write json to string
        s = j.dump();
    } catch (const json::type_error&) {
        return PARSER_ERROR;
    }

    const char* s_buf = s.c_str();
    int s_buf_len = strlen(s_buf);

    // Use malloc() here, because we later call free() on the buffer
    char* ret_buf = static_cast<char*>(malloc(sizeof(char) * (s_buf_len + 1)));
    if (ret_buf == NULL) {
        return TOOLCHAIN_ERROR;
    }
    ret_buf[s_buf_len] = 0;
    memcpy(ret_buf, s_buf, s_buf_len);

    *out_buf = ret_buf;
    *out_size = s_buf_len + 1;

    return PARSER_OKAY;
}
