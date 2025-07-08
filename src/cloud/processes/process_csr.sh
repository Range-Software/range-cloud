#!/bin/bash

readonly MY_PATH="$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")"

. "$MY_PATH/lib.sh"

print_help()
{
cat <<End-of-help
Usage: $myName.sh [OPTION]...

  mandatory

    --csr-base64=[STRING]             CSR content

  optional

    --help, -h, -?                    Print this help and exit

End-of-help
}

csrContentBase64=

while [ $# -gt 0 ]
do
    case $1 in
        --csr-base64=*)
            csrContentBase64=$(extract_cmd_parameter_value "$1")
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

assert_nonempty "$csrContentBase64" 'CSR was not provided'

tempCsrB64=$(mktemp cloud.csr54.XXXXXXXXXX)
tempCsr=$(mktemp cloud.csr.XXXXXXXXXX)
tempCert=$(mktemp cloud.csr.XXXXXXXXXX)

pinfo "CSR (base64): \"$tempCsrB64\""
pinfo "CSR: \"$tempCsr\""
pinfo "Certificate: \"$tempCert\""

echo "$csrContentBase64" > $tempCsrB64 && base64 -d -i "$tempCsrB64" > "$tempCsr"
assert_success $? "Failed to decode CSR from base64 string"

certCommonName=$(openssl req -noout -subject -in "$tempCsr" | grep -o '\<CN\s*=\s*[a-zA-Z0-9@_.\-]*' | sed -n 's/CN\s*=\s*//p')
pinfo "Owner: \"$CLOUD_PROCESS_OWNER\""
pinfo "User: \"$CLOUD_PROCESS_EXECUTOR\""
pinfo "CSR CN: \"$certCommonName\""

executorUser=$(echo "$CLOUD_PROCESS_EXECUTOR" | cut -d':' -f1)

# Only user themselves or owner of the process can process CSR
if [ "$executorUser" == "$certCommonName" ] || authorize_user
then
    signCsrCommand="$CLOUD_PROCESS_RANGE_CA_DIR/scripts/ca_sign_certificate.sh --extension=client_cert --csr=$tempCsr --cert=$tempCert"
    pinfo
    pinfo "Sign CSR command: $signCsrCommand"
    pinfo
    $signCsrCommand >> "$CLOUD_PROCESS_LOG_FILE" 2>&1
    assert_success $? "Failed to sign CSR"

    if [[ "$OSTYPE" == "darwin"* ]]; then
        # Mac OSX
        base64 -i $tempCert
    else
        base64 -w 0 -i $tempCert
    fi
    assert_success $? "Failed to encode CSR to base64 string"
else
    warningMessage="User \"$executorUser\" is not authorized to request CSR for \"$certCommonName\""
    pwarning "$warningMessage"
    echo "$warningMessage"
fi

rm -vf $tempCsrB64 $tempCsr $tempCert 2>&1 >> "$CLOUD_PROCESS_LOG_FILE"
assert_success $? "Failed to remove temporary files"
