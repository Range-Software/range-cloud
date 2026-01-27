#!/bin/bash

myName=$(basename $0 .sh)

keyFile=
csrFile=
oldCertFile=
keyCommonName=
keyCountry="CZ"
keyState="Prague"
keyLocation="Prague"
keyOrganization="Range"
keyOrganizationUnit="Cloud"
keySize=4096

keyPassword="12345678"

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

extract_certificate_field()
{
    local _subject=$1
    local _field=$2
    assert_nonempty "$_subject" "Certificate subject not specified"
    assert_nonempty "$_field" "Certificate field not specified"
    local _temp="${_subject#*${_field}=}"
    echo "${_temp%%,*}"
}

print_help()
{
cat <<End-of-help
Usage: $myName.sh [OPTION]...

  mandatory

    --key-file=[PATH]             Directory where key will be stored
    --csr-file=[PATH]             Directory where CSR will be stored

    --common-name=[STRING]        Common name

  optional

    --reuse-cert-file=[PATH]      Reuse certificate values from existing certificate (default=$oldCertFile)

    --country=[STRING]            Country (default=$keyCountry)
    --state=[STRING]              State (default=$keyState)
    --location=[STRING]           Location (default=$keyLocation)
    --organization=[STRING]       Organization (default=$keyOrganization)
    --organization-unit=[STRING]  Organization unit (default=$keyOrganizationUnit)

    --key-size=[NUMBER]           Size of key in bits (default=$keySize)

    --password=[STRING]           Key encryption password (default=$keyPassword)

    --help, -h, -?                Print this help and exit

End-of-help
}

while [ $# -gt 0 ]
do
    case $1 in
        --key-file=*)
            keyFile=$(extract_cmd_parameter_value "$1")
            ;;
        --csr-file=*)
            csrFile=$(extract_cmd_parameter_value "$1")
            ;;
        --reuse-cert-file=*)
            oldCertFile=$(extract_cmd_parameter_value "$1")
            ;;
        --common-name=*)
            keyCommonName=$(extract_cmd_parameter_value "$1")
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
        --key-size=*)
            keySize=$(extract_cmd_parameter_value "$1")
            ;;
        --password=*)
            keyPassword=$(extract_cmd_parameter_value "$1")
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

assert_nonempty "$keyFile" "Key file not specified"
assert_nonempty "$csrFile" "CSR file not specified"

if [ -n "$oldCertFile" ] && [ -f "$oldCertFile" ]
then
    oldCertSubject=$(openssl x509 -in "$oldCertFile" -noout -subject)
    assert_success $? "Failed to extract subject from certificate file \"$oldCertFile\""

    keyCommonName=$(extract_certificate_field "$oldCertSubject" "CN")
    keyCountry=$(extract_certificate_field "$oldCertSubject" "C")
    keyState=$(extract_certificate_field "$oldCertSubject" "ST")
    keyLocation=$(extract_certificate_field "$oldCertSubject" "L")
    keyOrganization=$(extract_certificate_field "$oldCertSubject" "O")
    keyOrganizationUnit=$(extract_certificate_field "$oldCertSubject" "OU")
fi

assert_nonempty "$keyCommonName" "Common name not specified"
assert_nonempty "$keyCountry" "Country not specified"
assert_nonempty "$keyState" "State not specified"
assert_nonempty "$keyLocation" "Location not specified"
assert_nonempty "$keyOrganization" "Organization not specified"
assert_nonempty "$keyOrganizationUnit" "Organization unit not specified"

assert_int $keySize "Invalid key size"

subject="/CN=${keyCommonName}/C=${keyCountry}/ST=${keyState}/L=${keyLocation}/O=${keyOrganization}/OU=${keyOrganizationUnit}"

if [ -f "$keyFile" ]; then rm -v "$keyFile"; fi
if [ -f "$csrFile" ]; then rm -v "$csrFile"; fi

echo "Generate key '$keyFile'"
openssl genpkey -algorithm RSA -pkeyopt rsa_keygen_bits:$keySize -out "$keyFile" -pass "pass:$keyPassword"
assert_success $? "Failed to generate key"

echo "Generate CSR '$csrFile'"
openssl req -key "$keyFile" -passin "pass:$keyPassword" -new -sha256 -out "$csrFile" -subj "$subject" -batch -verbose
assert_success $? "Failed to generate CSR"
