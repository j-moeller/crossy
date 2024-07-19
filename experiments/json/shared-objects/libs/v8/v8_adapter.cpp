// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <limits.h>
#include <stddef.h>
#include <stdint.h>

#include "include/v8-context.h"
#include "include/v8-exception.h"
#include "include/v8-isolate.h"
#include "include/v8-json.h"
#include "include/v8-local-handle.h"
#include "include/v8-primitive.h"
#include "test/fuzzer/fuzzer-support.h"

#include "common.h"

#include <chrono>
#include <iostream>
#include <numeric>

// TODO: Make compilation automated
// Compile with: ninja -C out/paco_component

static v8_fuzzer::FuzzerSupport* support;
static v8::Isolate* isolate;
static std::unique_ptr<v8::Isolate::Scope> isolate_scope;

__attribute__((constructor)) extern "C" int init()
{
    // TODO: Hardcoded path
    char path[] = "./build/crossy";
    size_t path_size = strlen(path);

    int argc = 1;
    char** argv = new char*[2];
    argv[0] = new char[path_size + 1];
    for (size_t i = 0; i < path_size; i++) {
        argv[0][i] = path[i];
    }
    argv[0][path_size] = '\0';
    argv[1] = NULL;

    v8_fuzzer::FuzzerSupport::InitializeFuzzerSupport(&argc, &argv);

    support = v8_fuzzer::FuzzerSupport::Get();
    isolate = support->GetIsolate();
    isolate_scope = std::make_unique<v8::Isolate::Scope>(isolate);

    delete[] argv[0];
    delete[] argv;

    return 0;
}

extern "C" int run(const char* data, size_t size, char** out_buf,
                   size_t* out_size)
{
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Context> context = support->GetContext();
    // v8::Context::Scope context_scope(context);
    // v8::TryCatch try_catch(isolate);

    if (size > INT_MAX) {
        return 0;
    }

    v8::Local<v8::String> source;
    if (!v8::String::NewFromUtf8(isolate, data, v8::NewStringType::kNormal,
                                    static_cast<int>(size))
             .ToLocal(&source)) {
        return TOOLCHAIN_ERROR;
    }

    v8::Local<v8::Value> json;
    if (!v8::JSON::Parse(context, source).ToLocal(&json)) {
        return PARSER_ERROR;
    }

    // back to string
    v8::Local<v8::String> stringified;
    if (!v8::JSON::Stringify(context, json).ToLocal(&stringified)) {
        return PARSER_ERROR;
    }

    // Convert the result to an UTF8 string
    v8::String::Utf8Value utf8(isolate, stringified);
    char* ret_buf = static_cast<char*>(malloc(utf8.length() + 1));
    if (ret_buf == nullptr) {
        return TOOLCHAIN_ERROR;
    }

    ret_buf[utf8.length()] = 0;
    memcpy(ret_buf, *utf8, utf8.length());

    *out_buf = ret_buf;
    *out_size = utf8.length();

    // isolate->RequestGarbageCollectionForTesting(v8::Isolate::kFullGarbageCollection);

    return 0;
}
