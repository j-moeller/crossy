#!/bin/bash

# gson omitts entries where the values is null
./build/poc configs/json/java-gson.cfg -i <(echo findings/01-omission-of-null-values/sample.json)