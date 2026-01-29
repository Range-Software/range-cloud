#!/bin/bash

myName=$(basename $0 .sh)
myPath=$(dirname $0)

cloudDir="$(realpath $myPath/..)"
varDir="$cloudDir/var"
pidFile="$varDir/cloud.pid"

runInteractive="false"
additionalParameters=

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

    --interactive            Run in interactive mode (no background process)

    --log-debug              Switch on debug log level
    --log-trace              Switch on trace log level
    --log-ssl                Switch on ssl debug log level
    --log-qt                 Switch on qt debug log level

    --help, -h, -?           Print this help and exit

End-of-help
}

while [ $# -gt 0 ]
do
    case $1 in
        --interactive)
            runInteractive="true"
            ;;
        --log-debug | --log-trace | --log-ssl | --log-qt)
            additionalParameters+=" $1"
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

binDir="${cloudDir}/bin"
libDir="${cloudDir}/lib"

if [ -z "$LD_LIBRARY_PATH" ]
then
    export LD_LIBRARY_PATH="$libDir"
else
    export LD_LIBRARY_PATH="$libDir:$LD_LIBRARY_PATH"
fi

count_statements()
{
    local _logFile="$cloudDir/log/Cloud.log"

    _nStatements=$[0]
    if [ -f "$_logFile" ]
    then
        _nStatements=$(grep -c "\[Application\] All services are ready." "$_logFile")
    fi
    echo $_nStatements
}

if [ "$runInteractive" = "true" ]
then
    $binDir/cloud --cloud-directory="$cloudDir" $additionalParameters
else
    if [ -f "$pidFile" ]
    then
        echo "Found PID file \"$pidFile\""
        lastPid=$(cat "$pidFile")
        kill -0 $lastPid &> /dev/null
        if [ $? -eq 0 ]
        then
            echo "Cloud is probably running with PID=\"$lastPid\""
            exit
        fi
    fi

    nOldStatements=$(count_statements)

    $binDir/cloud --cloud-directory="$cloudDir" $additionalParameters &> /dev/null &
    lastPid=$!
    echo $lastPid > "$pidFile"
    echo "Cloud is running with pid=$lastPid"

    isReady="false"

    while [ "$isReady" = "false" ]
    do
        kill -0 $lastPid
        if [ $? -ne 0 ]
        then
            echo "Cloud start failed. pid=$lastPid is not running" >&2
            exit 1
        fi
        echo "Checking readiness ..."
        nStatements=$(count_statements)
        if [ $nOldStatements -lt $nStatements ]
        then
            echo "Cloud is ready"
            isReady="true"
        fi
        sleep 1
    done
fi
