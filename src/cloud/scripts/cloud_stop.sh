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

force="false"

print_help()
{
cat <<End-of-help
Usage: $myName.sh [OPTION]...

  mandatory

  optional

    --force                  Force stop (kill process if needed)

    --help, -h, -?           Print this help and exit

End-of-help
}

while [ $# -gt 0 ]
do
    case $1 in
        --force)
            force="true"
            ;;
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
    exit
fi

lastPid=$(cat "$pidFile")
kill -0 $lastPid
if [ $? -eq 0 ]
then
    sigKill=
    if [ "$force" = "true" ]
    then
        sigKill="-9"
    fi
    kill $sigKill $lastPid
    assert_success $? "Failed to kill $sigKill process with PID=\"$lastPid\""
fi
while true
do
    kill -0 $lastPid &> /dev/null
    if [ $? -ne 0 ]
    then
        break
    fi
    sleep 1
done
rm -v "$pidFile"
assert_success $? "Failed to remove PID file \"$pidFile\""
echo "Cloud process has stopped"
