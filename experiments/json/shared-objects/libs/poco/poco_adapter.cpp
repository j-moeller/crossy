#include <cstdlib>
#include <cstring>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

#include "common.h"

#include <Poco/Dynamic/Var.h>
#include <Poco/JSON/JSON.h>
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/Stringifier.h>

using namespace Poco::JSON;
using namespace Poco::Dynamic;

extern "C" {
    int run(const char* buf, size_t size, char** out_buf, size_t* out_size);
}

int run(const char* buf, size_t size, char** out_buf, size_t* out_size)
{
    std::string buf_s(buf, buf + size);
    Parser parser;

    try {
        Var result = parser.parse(buf_s);

        std::ostringstream json_string_stream;

        Poco::JSON::Stringifier::stringify(result, json_string_stream, 0, -1,
                                           Poco::JSON_WRAP_STRINGS);

        std::string json_string = json_string_stream.str();
        const char* json_string_buf = json_string.c_str();
        int json_string_buf_len = strlen(json_string_buf);

        // Use malloc() here, because we later call free() on the buffer
        char* ret_buf = static_cast<char*>(
            malloc(sizeof(char) * (json_string_buf_len + 1)));
        if (ret_buf == nullptr) {
            return TOOLCHAIN_ERROR;
        }

        ret_buf[json_string_buf_len] = 0;
        memcpy(ret_buf, json_string_buf, json_string_buf_len);

        *out_buf = ret_buf;
        *out_size = json_string_buf_len + 1;
        return PARSER_OKAY;
    } catch (Poco::JSON::JSONException& e) {
        return PARSER_ERROR;
    }
}
