#!/bin/bash

print_usage() {
    echo "Usage: $0 [--command <command>]"
}

while [ $# -gt 0 ]; do
    case $1 in
        --command)
            shift
            command=$1
            ;;
        -*|--*)
            echo "Unknown option $1"
            exit 1
            ;;
        *)
            echo "Unknown argument $1"
            exit 1
            ;;
    esac
    shift
done

if [ -z "${command}" ]; then
    command_flag="/bin/bash"
else
    command_flag="${command}"
fi

start_container() {
    CONTAINER_HASH=$(./scripts/dev/start_dev_container.sh)
    echo ${CONTAINER_HASH} > ${DOCKER_CONTAINER_FILE}
}

DOCKER_CONTAINER_FILE=.dockercontainer
CONTAINER_HASH=$(cat ${DOCKER_CONTAINER_FILE} 2> /dev/null)

if [ $? -eq 0 ]; then
    STATE=$(docker container inspect -f '{{.State.Running}}' ${CONTAINER_HASH})
    if [ $? -ne 0 -o "${STATE:-}" = "false" ]; then
        start_container
    fi
else
    start_container
fi

docker exec -it ${CONTAINER_HASH} /bin/bash -c "${command_flag}"
exit 0
