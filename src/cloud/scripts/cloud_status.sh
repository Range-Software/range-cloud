#!/bin/bash

myName=$(basename $0 .sh)
myPath=$(dirname $0)

cloudDir="$(realpath $myPath/..)"
varDir="$cloudDir/var"
pidFile="$varDir/cloud.pid"

assert_success()
{
    if [ $1 -ne 0 ]
    then
        echo "$2" >&2
        exit 1
    fi
}

print_help()
{
cat <<End-of-help
Usage: $myName.sh [OPTION]...

  mandatory

  optional

    --help, -h, -?           Print this help and exit

End-of-help
}

while [ $# -gt 0 ]
do
    case $1 in
        --help | -h | -?)
            print_help
            exit 0
            ;;
        *)
            echo "Unknown parameter '$1'" >&2
            exit 1
            ;;
    esac
    shift
done

if [ ! -f "$pidFile" ]
then
    echo "PID file \"$pidFile\" does not exist"
    echo "Cloud is probably not running"
    exit 1
fi

lastPid=$(cat "$pidFile")
kill -0 $lastPid
retVal=$?

if [ $retVal -eq 0 ]
then
    echo "Cloud process (pid=$lastPid) is running"
    exit 0
else
    echo "Cloud process (pid=$lastPid) is not running"
    exit $retVal
fi
