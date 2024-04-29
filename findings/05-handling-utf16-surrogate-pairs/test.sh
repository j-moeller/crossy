#!/bin/bash

# the string "\uDBFF\uDFFF" is serialized back to the byte sequence 0xf3 0xbf 0xbf 0xbf instead of 0xf4 0x8f 0xbf 0xbf.
./build/poc configs/json/c-json-parser -i <(echo findings/05-handling-utf16-surrogate-pair/sample.json)