#!/bin/sh

myPath=$(dirname $0)

cloudDir="$(realpath $myPath/..)"

binDir="${cloudDir}/bin"
libDir="${cloudDir}/lib"

if [ -z "$LD_LIBRARY_PATH" ]
then
    export LD_LIBRARY_PATH="$libDir"
else
    export LD_LIBRARY_PATH="$libDir:$LD_LIBRARY_PATH"
fi

$binDir/cloud-tool $@
