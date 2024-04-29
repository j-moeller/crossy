#!/bin/bash

# for very large numbers (> 10^100) gson no longer returns a float, but a string representation
./build/poc configs/json/c-cjson.cfg configs/json/c-jansson.cfg configs/json/c-poco.cfg -i <(echo findings/02-unexpected-type-coercion/02_sample.json)