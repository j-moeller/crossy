#!/bin/bash
set -e

print_usage() {
    echo "Usage: $0 --host_output_dir <dir> --host_corpus_dir <dir> [--name <name>] [--writable_corpus] [--detach] <command>"
}

while [ $# -gt 0 ]; do
    case $1 in
        --host_output_dir)
            shift
            host_output_dir=$1
            ;;
        --host_corpus_dir)
            shift
            host_corpus_dir=$1
            ;;
        --name)
            shift
            name=$1
            ;;
        --writable_corpus)
            writable_corpus=true
            ;;
        --detach)
            detach=true
            ;;
        -*|--*)
            echo "Unknown option $1"
            exit 1
            ;;
        *)
            command=$1
            ;;
    esac
    shift
done

if [ -z "${host_corpus_dir}" ]; then
    echo "Error: Host corpus dir missing"
    print_usage
    exit 1
fi

if [ -z "${host_output_dir}" ]; then
    echo "Error: Host output dir missing"
    print_usage
    exit 1
fi

if [ -z "${command}" ]; then
    echo "Error: Command missing"
    print_usage
    exit 1
fi

if [ -z "${writable_corpus}" ]; then
    corpus_flag=":ro"
else
    corpus_flag=""
fi

if [ -z "${detach}" ]; then
    detach_flag="-it"
else
    detach_flag="-d"
fi

if [ -z "${name}" ]; then
    name_flag=""
else
    name_flag="--name ${name}"
fi

docker run --rm ${detach_flag} \
    --env JAVA_HOME="/usr/lib/jvm/java-17-openjdk-amd64" \
    -v ${host_corpus_dir}:/app/corpus${corpus_flag} \
    -v $(pwd)/configs:/app/configs:ro \
    -v $(pwd)/experiments:/app/experiments:ro \
    -v $(pwd)/experiments/json/java/.m2:/root/.m2:ro \
    -v $(pwd)/ext/llvm-project:/app/ext/llvm-project:ro \
    -v $(pwd)/scripts:/app/scripts:ro \
    -v $(pwd)/src:/app/src:ro \
    -v $(pwd)/Makefile:/app/Makefile:ro \
    -v $(pwd)/suppr.txt:/app/suppr.txt:ro \
    \
    -v $(pwd)/build:/app/build:ro \
    -v ${host_output_dir}:/app/output \
    ${name_flag} \
    mlsec-crossy sh -c "${command}"
