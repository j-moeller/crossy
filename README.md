# Crossy

This is the code repository accompanying our ACM AsiaCCS '24 paper ["Cross-Language Differential Testing of JSON Parsers"](https://dl.acm.org/doi/pdf/10.1145/3634737.3657003).

_**Note:** We are still in the process of publishing this repository. Currently, the build process and preliminary fuzzing of C/C++/Rust/Java projects are possible. We unfortunately encountered some problems with Python projects, and the poco parser which we are going to fix in the following weeks._

## Setup

This repository consists of two collections of source code. The first is our crossy framework that is located in `src/`. The second are the JSON parsers located in `experiments/json`. By running

```
make
```

you spawn a series of self-contained docker containers that build the respective projects (e.g., `Dockerfile.crossy` for Crossy and `experiments/json/shared-objects/libs/jsmn/Dockerfile.jsmn` for the jsmn parser).

## Fuzzing

_**Note:** We are still in the process of publishing this repository. We are going to add our evaluation scripts once we adapted them for the published version. For now, please see `src/cli.cpp` for a list of available options for crossy._


Afterwards, you can run:

```
make run
```

to spawn a docker container for running the fuzzer. In the docker container you can run:

```
./build/crossy \
    configs/json/* \
    -o ./output/ \
    -- \
    corpus \
    -detect_leaks=0 \
    -artifact_prefix=./output/
```