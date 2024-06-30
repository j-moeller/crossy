#include <cstdlib>

#include "jsoncpp/include/json/json.h"
#include "jsoncpp/include/json/reader.h"

#include "common.h"

extern "C" {
    int run(const char* buf, size_t size, char** out_buf, size_t* out_size);
}

int run(const char* buf, size_t size, char** out_buf, size_t* out_size)
{
    Json::Value json;
    Json::CharReaderBuilder char_reader_builder;
    std::unique_ptr<Json::CharReader> const char_reader(
        char_reader_builder.newCharReader());
    if (!char_reader->parse(buf, buf + size, &json, nullptr)) {
        return PARSER_ERROR;
    }

    // use a specialized string builder to generate output without newlines and
    // indents
    Json::StreamWriterBuilder stream_writer_builder;
    stream_writer_builder["indentation"] = "";
    stream_writer_builder["emitUTF8"] = true;
    std::string s = Json::writeString(stream_writer_builder, json);

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
