#!/bin/bash

myName=$(basename $0 .sh)
myPath=$(dirname $0)

cloudPackageDir="$(realpath $myPath/..)"
rangeCaDir="$cloudPackageDir/range-ca"

keyCountry=
keyState=
keyLocation=
keyOrganization=
keyOrganizationUnit=
keyCommonName=
keyEmail=
keyPassword=

extendPath () {
    local _path=$1
    local _add_path=$2
    if [ -z "$_path" ]
    then
        echo "$_add_path"
    else
        echo "$_path:$_add_path"
    fi
}

binDir="${cloudPackageDir}/bin"
libDir="${cloudPackageDir}/lib"
scriptsDir="${cloudPackageDir}/scripts"
processesDir="${cloudPackageDir}/processes"

if [ -z "$LD_LIBRARY_PATH" ]
then
    export LD_LIBRARY_PATH="$libDir"
else
    export LD_LIBRARY_PATH="$libDir:$LD_LIBRARY_PATH"
fi

binCloud="${binDir}/cloud"
cloudDir=$($binCloud --print-settings | grep "\"cloudDirectory\"" | cut -d\" -f4)
publicHttpPort=$($binCloud --print-settings | grep "\"publicHttpPort\"" | cut -d\" -f4)
privateHttpPort=$($binCloud --print-settings | grep "\"privateHttpPort\"" | cut -d\" -f4)
fileStore=
publicKey=
privateKey=

assert_success()
{
    if [ $1 -ne 0 ]
    then
        echo "$2" >&2
        exit 1
    fi
}

assert_int()
{
    local _value=$1
    if ! [[ $_value =~ ^[-]?[0-9]+$ ]]; then
        echo "Value is not an integer. $2" >&2
        exit 1
    fi
}

assert_port()
{
    local _value=$1
    assert_int "$1" "Value \"$_value\" is not an integer. $2"
    if [ "$_value" -lt 0 ] && [ "$_value" -gt 65535 ]
    then
        echo "Value \"$_value\" is not in valid range [0-65535]. $2" >&2
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

touch_dir()
{
    if [ ! -d "$1" ]
    then
        mkdir -p "$1"
        return $?
    fi
    return 0
}

assert_touch_dir()
{
    assert_nonempty "$1" "Directory not provided"
    touch_dir "$1"
    assert_success $? "Failed to touch the directory \"$1\""
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

  optional

    --cloud-directory=[PATH]        Directory where data directory structure will be created (default=$cloudDir)
    --file-store-path=[PATH]        Path to direcory with file store (default=<cloud-directory>/store)

    --range-ca=[PATH]               Path to Range CA top directory (default=$rangeCaDir)

    --public-http-port=[NUMBER]     Public http port (default=$publicHttpPort)
    --private-http-port=[NUMBER]    Private http port (default=$privateHttpPort)

    --country=[STRING]              Country (default=$keyCountry)
    --state=[STRING]                State (default=$keyState)
    --location=[STRING]             Location (default=$keyLocation)
    --organization=[STRING]         Organization (default=$keyOrganization)
    --organization-unit=[STRING]    Organization unit (default=$keyOrganizationUnit)
    --common-name=[STRING]          Common name (default=$keyCommonName)
    --email=[STRING]                E-mail address (default=$caEmail)

    --private-key=[PATH]            Path to server private key to be reused
    --public-key=[PATH]             Path to server public key (certificate) to be reused

    --password=[STRING]             Key encryption password (default=$keyPassword)

    --help, -h, -?                  Print this help and exit

End-of-help
}

while [ $# -gt 0 ]
do
    case $1 in
        --cloud-directory=*)
            cloudDir=$(extract_cmd_parameter_value "$1")
            ;;
        --file-store-path=*)
            fileStore=$(extract_cmd_parameter_value "$1")
            ;;
        --range-ca=*)
            rangeCaDir=$(extract_cmd_parameter_value "$1")
            ;;
        --public-http-port=*)
            publicHttpPort=$(extract_cmd_parameter_value "$1")
            ;;
        --private-http-port=*)
            privateHttpPort=$(extract_cmd_parameter_value "$1")
            ;;
        --country=*)
            keyCountry=$(extract_cmd_parameter_value "$1")
            ;;
        --state=*)
            keyState=$(extract_cmd_parameter_value "$1")
            ;;
        --location=*)
            keyLocation=$(extract_cmd_parameter_value "$1")
            ;;
        --organization=*)
            keyOrganization=$(extract_cmd_parameter_value "$1")
            ;;
        --organization-unit=*)
            keyOrganizationUnit=$(extract_cmd_parameter_value "$1")
            ;;
        --common-name=*)
            keyCommonName=$(extract_cmd_parameter_value "$1")
            ;;
        --email=*)
            keyEmail=$(extract_cmd_parameter_value "$1")
            ;;
        --password=*)
            keyPassword=$(extract_cmd_parameter_value "$1")
            ;;
        --private-key=*)
            privateKey=$(extract_cmd_parameter_value "$1")
            ;;
        --public-key=*)
            publicKey=$(extract_cmd_parameter_value "$1")
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
assert_nonempty "$rangeCaDir" "Path to Range CA not specified"
assert_nonempty "$keyCommonName" "Common name not specified"
assert_nonempty "$keyCountry" "Country not specified"
assert_nonempty "$keyState" "State not specified"
assert_nonempty "$keyLocation" "Location not specified"
assert_nonempty "$keyOrganization" "Organization not specified"
assert_nonempty "$keyOrganizationUnit" "Organization unit not specified"
assert_port "$publicHttpPort" "Not a valid port number \"$publicHttpPort\""
assert_port "$privateHttpPort" "Not a valid port number \"$privateHttpPort\""

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
    cp -v "$scriptsDir/cloud_start.sh" "$dstScriptsDir/" && \
    cp -v "$scriptsDir/cloud_stop.sh" "$dstScriptsDir/" && \
    cp -v "$scriptsDir/cloud_status.sh" "$dstScriptsDir/" && \
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

if [ -z "$fileStore" ]
then
    fileStore="${cloudDir}/store"
fi

certDir="${cloudDir}/cert"

serverKeyDir="${certDir}/server"
serverCsrDir="${certDir}/server"
serverCertDir="${certDir}/server"
caCertDir="${certDir}/ca"
clientKeyDir="${certDir}/clients"
clientCsrDir="${certDir}/clients"
clientCertDir="${certDir}/clients"

assert_touch_dir "$serverKeyDir"
assert_touch_dir "$serverCertDir"
assert_touch_dir "$caCertDir"
assert_touch_dir "$clientKeyDir"
assert_touch_dir "$clientCsrDir"
assert_touch_dir "$clientCertDir"

# Setup Range-CA
echo
if [ -d "$rangeCaDir" ]
then
    echo "Reusing existing Certificate Authority (range-ca)"
else
    echo "Configuring new Certificate Authority (range-ca)"
    "$rangeCaDir"/scripts/ca_setup.sh \
        --ca-dir="$rangeCaDir" \
        --country="$keyCountry" \
        --state="$keyState" \
        --location="$keyLocation" \
        --organization="$keyOrganization" \
        --organization-unit="$keyOrganizationUnit" \
        --common-name="$keyCommonName" \
        --email=$keyEmail
    assert_success $? "Failed to setup Certificate Authority"
fi

caChainCert="$caCertDir/ca-chain.cert.pem"

# Copy CA chain to CA cert directory
cp -v "$rangeCaDir/intermediateCA/certs/ca-chain.cert.pem" "$caChainCert"
assert_success $? "Failed to copy CA chain certificate"

if [ -f "$publicKey" ] && [ -f "$privateKey" ]
then
    # Use provided server key and signed certificate.
    echo
    echo "Reusing provided server key and certificate"
    serverKey="$serverKeyDir/$(basename $privateKey)"
    serverCert="$serverCertDir/$(basename $publicKey)"

    ln -s "$publicKey" "$serverCert" && \
    ln -s "$privateKey" "$serverKey"
    assert_success $? "Failed to copy provided private and public keys"
else
    # Create server key and signed certificate.
    echo
    echo "Creating server key and signed certificate"
    serverKey="$serverKeyDir/$keyCommonName.key.pem"
    serverCert="$serverCertDir/$keyCommonName.cert.pem"

    serverCsr="$serverCsrDir/$keyCommonName.csr.pem"
    $scriptsDir/cloud_create_csr.sh \
        --common-name="$keyCommonName" \
        --country="$keyCountry" \
        --state="$keyState" \
        --location="$keyLocation" \
        --organization="$keyOrganization" \
        --organization-unit="$keyOrganizationUnit" \
        --key-file="$serverKey" \
        --csr-file="$serverCsr" \
        --password=$keyPassword
    assert_success $? "Failed to create key and CSR for server '$keyCommonName'"

    $rangeCaDir/scripts/ca_sign_certificate.sh \
        --extension="server_cert" \
        --csr="$serverCsr" \
        --cert="$serverCert"
    assert_success $? "Failed to sign certificate for account '$keyCommonName'"
fi

accounts="root guest"

for account in $accounts
do
    echo
    echo "Creating account '$account'"
    clientKey="$clientKeyDir/$account.key.pem"
    clientCsr="$clientCsrDir/$account.csr.pem"
    clientCert="$clientCertDir/$account.cert.pem"

    $scriptsDir/cloud_create_csr.sh \
        --common-name="$account" \
        --country="$keyCountry" \
        --state="$keyState" \
        --location="$keyLocation" \
        --organization="$keyOrganization" \
        --organization-unit="$keyOrganizationUnit" \
        --key-file="$clientKey" \
        --csr-file="$clientCsr" \
        --password=$keyPassword
    assert_success $? "Failed to create key and CSR for account '$account'"

    $rangeCaDir/scripts/ca_sign_certificate.sh \
        --extension="client_cert" \
        --csr="$clientCsr" \
        --cert="$clientCert"
    assert_success $? "Failed to sign certificate for account '$account'"
done

# Run cloud to store initial settings
$binCloud \
    --cloud-directory="$cloudDir" \
    --file-store-path="$fileStore" \
    --public-http-port="$publicHttpPort" \
    --private-http-port="$privateHttpPort" \
    --private-key-password="$keyPassword" \
    --private-key="$serverKey" \
    --public-key="$serverCert" \
    --ca-public-key="$caChainCert" \
    --range-ca-directory="$rangeCaDir" \
    --store-settings
assert_success $? "Failed to initialize cloud settings"

