#!/bin/bash
set -e

# In each container we have to set up the python environment separately as the
# dependencies are not written to the file system. We could use a virtual env
# to install the dependencies, however we would need to activate the venv in
# each container manually, as the venv activation does not propagate through
# the entrypoint.sh's exec call.
if [ -f "experiments/json/python/setup.sh" ]; then
    cd experiments/json/python && ./setup.sh && cd -
fi

exec "$@"