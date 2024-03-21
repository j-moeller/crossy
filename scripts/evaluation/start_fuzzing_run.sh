#!/bin/bash
set -e

N_JOBS=16
TOTAL_TIME=28800
GOLD_PARSER=(
    'configs/json/c-jsmn.cfg' \
    'configs/json/c-jsonh.cfg'  \
    'configs/json/c-libjson.cfg'
)
PARSER=(
    "configs/json/c-ccan.cfg" \
    "configs/json/c-cjson.cfg" \
    "configs/json/c-frozen.cfg" \
    "configs/json/c-jansson.cfg" \
    "configs/json/c-jsmn.cfg" \
    "configs/json/c-json-c.cfg" \
    "configs/json/c-json-parser.cfg" \
    "configs/json/c-jsonh.cfg" \
    "configs/json/c-libjson.cfg" \
    "configs/json/c-poco.cfg" \
    "configs/json/c-yajl.cfg" \
    "configs/json/cpp-boost.cfg" \
    "configs/json/cpp-json.cfg" \
    "configs/json/cpp-jsoncpp.cfg" \
    "configs/json/cpp-rapidjson.cfg" \
    "configs/json/cpp-spidermonkey.cfg" \
    "configs/json/cpp-v8.cfg" \
    "configs/json/java-gson.cfg" \
    "configs/json/java-jackson.cfg" \
    "configs/json/py-json.cfg" \
    "configs/json/py-simplejson.cfg" \
    "configs/json/rust-serde.cfg"
)

# Generated 
python3 scripts/evaluation/start_fuzzing_run.py \
    ${PARSER[*]} \
    -g ${GOLD_PARSER[*]} \
    --n_jobs ${N_JOBS} \
    --max_total_time ${TOTAL_TIME} \
    --analysis_criterium='all' \
    --max_length 512 \
    --seed 12345678 \
    --initial_corpus corpus-final \
    --writable_corpus
