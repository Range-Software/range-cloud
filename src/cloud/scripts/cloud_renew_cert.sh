#!/bin/bash

myName=$(basename $0 .sh)
myPath=$(dirname $0)

cloudDir="$(realpath $myPath/..)"
rangeCaDir="${HOME}/range-ca"
scriptsDir="${cloudDir}/scripts"

account=
keyPassword="12345678"

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

    --account=[STRING]            Account name which a certificate will be renewed

  optional

    --password=[STRING]           Key encryption password (default=$keyPassword)

    --range-ca=[PATH]             Path to Range CA top directory (default=$rangeCaDir)

    --help, -h, -?                Print this help and exit

End-of-help
}

while [ $# -gt 0 ]
do
    case $1 in
        --account=*)
            account=$(extract_cmd_parameter_value "$1")
            ;;
        --password=*)
            keyPassword=$(extract_cmd_parameter_value "$1")
            ;;
        --range-ca=*)
            rangeCaDir=$(extract_cmd_parameter_value "$1")
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

assert_nonempty "$account" "Common name not specified"
assert_nonempty "$rangeCaDir" "Path to Range CA not specified"

certDir="${cloudDir}/cert"

clientKeyDir="${certDir}/clients"
clientCsrDir="${certDir}/clients"
clientCertDir="${certDir}/clients"

clientKey="$clientKeyDir/$account.key.pem"
clientCsr="$clientCsrDir/$account.csr.pem"
clientCert="$clientCertDir/$account.cert.pem"

$scriptsDir/cloud_create_csr.sh \
    --key-file="$clientKey" \
    --csr-file="$clientCsr" \
    --reuse-cert-file="$clientCert" \
    --password=$keyPassword

$rangeCaDir/scripts/ca_sign_certificate.sh \
    --extension="client_cert" \
    --csr="$clientCsr" \
    --cert="$clientCert"
assert_success $? "Failed to sign certificate for account '$account'"
