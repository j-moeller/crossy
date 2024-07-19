#include <cassert>
#include <cstdlib>
#include <iostream>
#include <locale>
#include <string.h>
#include <string>
#include <vector>

#include <cuchar>

#include <jsapi.h>

#include <js/CharacterEncoding.h>
#include <js/CompilationAndEvaluation.h>
#include <js/Exception.h>
#include <js/Initialization.h>
#include <js/JSON.h>
#include <js/SourceText.h>

static JSClassOps global_ops = {
    nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, JS_GlobalObjectTraceHook};

/* The class of the global object. */
static JSClass global_class = {"global", JSCLASS_GLOBAL_FLAGS, &global_ops};

static JSContext* cx = nullptr;
static JSObject* global = nullptr;

__attribute__((constructor)) extern "C" int init()
{
    if (!JS_Init()) {
        return 1;
    }

    cx = JS_NewContext(JS::DefaultHeapMaxBytes);

    if (!cx) {
        assert(false);
        return 1;
    }

    if (!JS::InitSelfHostedCode(cx)) {
        assert(false);
        return 1;
    }

    JS::RealmOptions options;
    global = JS_NewGlobalObject(cx, &global_class, nullptr,
                                JS::FireOnNewGlobalHook, options);

    return 0;
}

__attribute__((destructor)) extern "C" int shutdown()
{
    if (global != nullptr) {
        // JSFree(global)???
    }

    if (cx != nullptr) {
        JS_DestroyContext(cx);
        cx = nullptr;
    }
    JS_ShutDown();
    return 0;
}

struct Data {
    JSContext* cx;
    struct {
        char** buf;
        size_t* size;
    } str;
};

bool callback(const char16_t* buf, uint32_t len, void* data_)
{
    if (len == 4) {
        if (buf[0] == 'n' && buf[1] == 'u' && buf[2] == 'l' && buf[3] == 'l') {
            return false;
        }
    }

    Data* data = (Data*)data_;

    JSContext* cx = data->cx;
    char** out_buf = data->str.buf;
    size_t* out_size = data->str.size;

    JS::RootedString str(cx, JS_NewUCStringCopyN(cx, buf, len));
    JS::UniqueChars chars = JS_EncodeStringToUTF8(cx, str);

    if (chars) {
        *out_buf = strdup(chars.get());
        *out_size = strlen(chars.get());
        return true;
    }

    return false;
}

inline JSString* useUFT8Chars(JSContext* cx, const char* buf, size_t size)
{
    std::string str(buf, buf+size);
    JS::UTF8Chars utf8chars(str.c_str(), str.size());
    return JS_NewStringCopyUTF8N(cx, utf8chars);
}

extern "C" int run(const char* buf, size_t size, char** out_buf,
                   size_t* out_size)
{
    assert(cx != nullptr && global != nullptr);

    if (size == 0) {
        return 0;
    }

    // JS_MaybeGC(cx);
    JS_ClearPendingException(cx);

    {
        JSAutoRealm realm(cx, global);

        JSString* s = useUFT8Chars(cx, buf, size);
        if (s == nullptr) {
            return 0;
        }

        JS::RootedString str(cx, s);

        JS::RootedValue parsed(cx);
        if (!JS_ParseJSON(cx, str, &parsed)) {
            return 0;
        }

        Data data;
        data.cx = cx;
        data.str.buf = out_buf;
        data.str.size = out_size;

        bool ok = JS_Stringify(cx, &parsed, nullptr, JS::NullHandleValue,
                               callback, &data);

        if (!ok) {
            *out_buf = nullptr;
            *out_size = 0;
        }

        return 0;
    }

    return 0;
}