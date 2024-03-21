## Setup

Before running the setup, you need to ensure that v8 is setup correctly. This currently needs to be done **manually**, so please follow the description in [experiments/json/shared-objects/README.v8.md](experiments/json/shared-objects/README.v8.md).

After setting up v8, you can setup the rest of the dependencies with:

```shell
sh setup_host.sh
```

This initializes the git submodules, [fetches the default corpus](scripts/setup/setup_corpus.sh), [builds the docker container from the Dockerfile](scripts/setup/setup_docker.sh) and [starts the initial container](scripts/setup/start_initial_container.sh). The initial container downloads Java dependencies, sets up the python environment and build v8 + spidermonkey.

## Usage

### Development

```shell
make
```

If you execute `make` on the host system, this will create a dev container where you can build the system in a predefined way. If you type `make` again (or `make build` or `make build/poc`) inside of the container it will build the main executable (`build/poc`).

### Evaluation

#### Gold selection

```shell
./scripts/evaluation/start_gold_selection_run.sh
```

The script [scripts/evaluation/start_gold_selection_run.sh](./scripts/evaluation/start_gold_selection_run.sh) will create a 'run directory' in the 'output' directory. In the following, we will use \{rundir} to denote this directory.

```shell
python3 scripts/evaluation/start_postprocessing.py {rundir}
```

```shell
python3 scripts/analysis/find_optimal_gold_parser.py {rundir} > {csvfile}
```

```shell
python3 scripts/analysis/analyse_gold_parser.py {csvfile}
```

#### Fuzzing Run

Define the GOLD_PARSER array in [scripts/evaluation/start_fuzzing_run.sh](scripts/evaluation/start_fuzzing_run.sh) according to the results of the gold selection. The gold parser array should be a representation for the entire set of all parsers ('common sense parser').

```shell
./scripts/evaluation/start_fuzzing_run.sh
```

This too will create a run directory in the 'output' directory. In the following, we will use \{rundir} to denote this directory.

```shell
python3 scripts/evaluation/start_postprocessing.py {rundir}
```

