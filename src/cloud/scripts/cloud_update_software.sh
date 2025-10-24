#!/bin/bash

myName=$(basename $0 .sh)
myPath=$(dirname $0)

cloudPackageDir="$(realpath $myPath/..)"

binDir="${cloudPackageDir}/bin"
scriptsDir="${cloudPackageDir}/scripts"
processesDir="${cloudPackageDir}/processes"

binCloud="${binDir}/cloud"
cloudDir=

assert_success()
{
    if [ $1 -ne 0 ]
    then
        echo "$2" >&2
        exit 1
    fi
}

assert_nonempty()
{
    if [ -z "$1" ]
    then
        echo "Value is empty. $2" >&2
        exit 1
    fi
}

extract_cmd_parameter_value()
{
    echo "${1#*=}"
}

print_help()
{
cat <<End-of-help
Usage: $myName.sh [OPTION]...

  mandatory

    --cloud-directory=[PATH]        Directory where data directory structure will be created

  optional

    --help, -h, -?                  Print this help and exit

End-of-help
}

while [ $# -gt 0 ]
do
    case $1 in
        --cloud-directory=*)
            cloudDir=$(extract_cmd_parameter_value "$1")
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

assert_nonempty "$cloudDir" "Path to Cloud directory not specified"

if [ -d "$cloudDir" ]
then
    cloudDir=$(realpath "$cloudDir")
fi

if [ "$cloudDir" != "$cloudPackageDir" ]
then
    dstBinDir="$cloudDir/bin"
    dstScriptsDir="$cloudDir/scripts"
    dstProcessesDir="$cloudDir/processes"

    mkdir -pv "$dstBinDir" && \
    cp -v "$binDir/"* "$dstBinDir/" && \
    mkdir -pv "$dstScriptsDir" && \
    cp -v "$scriptsDir/cloud_create_csr.sh" "$dstScriptsDir/" && \
    cp -v "$scriptsDir/cloud_start.sh" "$dstScriptsDir/" && \
    cp -v "$scriptsDir/cloud_status.sh" "$dstScriptsDir/" && \
    cp -v "$scriptsDir/cloud_stop.sh" "$dstScriptsDir/" && \
    cp -v "$scriptsDir/cloud_tool.sh" "$dstScriptsDir/" && \
    mkdir -pv "$dstProcessesDir" && \
    cp -v "$processesDir/"* "$dstProcessesDir/"
    assert_success $? "Failed to prepare Cloud directory: \"$cloudDir\""

    # Linux specific directories
    if [ -d "$cloudPackageDir/lib" ]
    then
        cp -Rv "$cloudPackageDir/lib" "$cloudDir"
        assert_success $? "Failed to copy lib directory"
    fi
    if [ -d "$cloudPackageDir/plugins" ]
    then
        cp -Rv "$cloudPackageDir/plugins" "$cloudDir"
        assert_success $? "Failed to copy plugins directory"
    fi

    # MacOS specific directories
    if [ -d "$cloudPackageDir/Frameworks" ]
    then
        cp -Rv "$cloudPackageDir/Frameworks" "$cloudDir"
        assert_success $? "Failed to copy Frameworks directory"
    fi
    if [ -d "$cloudPackageDir/PlugIns" ]
    then
        cp -Rv "$cloudPackageDir/PlugIns" "$cloudDir"
        assert_success $? "Failed to copy PlugIns directory"
    fi
fi

