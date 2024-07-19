#include <cstdlib>
#include <iostream>
#include <sstream>

#include "rapidjson/include/rapidjson/document.h"
#include "rapidjson/include/rapidjson/stringbuffer.h"
#include "rapidjson/include/rapidjson/writer.h"

#include "common.h"

extern "C" {
    int run(const char* buf, size_t size, char** out_buf, size_t* out_size);
}

int run(const char* buf, size_t size, char** out_buf, size_t* out_size)
{
    rapidjson::Document d;
    d.Parse(buf, size);

    if (d.HasParseError()) {
        return PARSER_ERROR;
    }

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    d.Accept(writer);

    std::stringstream ss;
    ss << buffer.GetString();

    std::string s = ss.str();
    const char* ss_buf = s.c_str();
    int ss_buf_len = strlen(ss_buf);

    // Use malloc() here, because we later call free() on the buffer
    char* ret_buf = static_cast<char*>(malloc(sizeof(char) * (ss_buf_len + 1)));
    if (ret_buf == nullptr) {
        return TOOLCHAIN_ERROR;
    }

    ret_buf[ss_buf_len] = 0;
    memcpy(ret_buf, ss_buf, ss_buf_len);

    *out_buf = ret_buf;
    *out_size = ss_buf_len + 1;

    return PARSER_OKAY;
}
