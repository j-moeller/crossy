#!/bin/bash

# cJSON, jansson, and poco do not return any output for large numbers
./build/poc configs/json/c-cjson.cfg configs/json/c-jansson.cfg configs/json/c-poco.cfg -i <(echo findings/02-unexpected-type-coercion/01_sample.json)