#!/bin/bash

readonly MY_PATH="$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")"

. "$MY_PATH/lib.sh"

print_help()
{
cat <<End-of-help
Usage: $myName.sh [OPTION]...

  mandatory

    --report-base64=[STRING]          Report content

  optional

    --help, -h, -?                    Print this help and exit

End-of-help
}

reportContentBase64=

while [ $# -gt 0 ]
do
    case $1 in
        --report-base64=*)
            reportContentBase64=$(extract_cmd_parameter_value "$1")
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

assert_nonempty "$reportContentBase64" 'Report was not provided'

tempReportB64=$(mktemp cloud.report54.XXXXXXXXXX)
tempReport=$(mktemp cloud.report.XXXXXXXXXX)

pinfo "Report (base64): \"$tempReportB64\""
pinfo "Report: \"$tempReport\""

echo "$reportContentBase64" > $tempReportB64 && base64 -d -i "$tempReportB64" > "$tempReport"
assert_success $? "Failed to decode Report from base64 string"

pinfo "Owner: \"$CLOUD_PROCESS_OWNER\""
pinfo "User: \"$CLOUD_PROCESS_EXECUTOR\""

report_file_name="$(date "+%Y%m%d%H%M%S")-report.txt"
cp -v "$tempReport" "$report_file_name" 2>&1 >> "$CLOUD_PROCESS_LOG_FILE"
assert_success $? "Failed to copy report file"

echo "Report was processed"

rm -vf $tempReportB64 $tempReport 2>&1 >> "$CLOUD_PROCESS_LOG_FILE"
assert_success $? "Failed to remove temporary files"
