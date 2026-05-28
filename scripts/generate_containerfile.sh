#!/bin/bash

myName=$(basename "$0" .sh)

# Defaults
hostName="my-cloud-host.com"
adminAccount="admin@my-cloud-host.com"
publicPort=4011
privatePort=4012
caCountry="EU"
caState="CZ"
caLocation="Prague"
caOrganization="Range Software"
caOrgUnit="Cloud"
caEmail=""
baseImage="ubuntu:24.04"
packageFile=""
outputDir="."

print_help() {
cat << EOF
Usage: $myName.sh [OPTION]...

  Required:
    --package-file=FILENAME        Path to the range-cloud package (.tar.gz)

  Optional:
    --host-name=NAME               Host name (default: "$hostName")
    --admin-account=EMAIL          Admin account email (default: "$adminAccount")
    --public-port=PORT             Public HTTP port (default: $publicPort)
    --private-port=PORT            Private HTTP admin port (default: $privatePort)
    --ca-country=CODE              CA country code (default: "$caCountry")
    --ca-state=NAME                CA state/region (default: "$caState")
    --ca-location=NAME             CA city (default: "$caLocation")
    --ca-organization=NAME         CA organization (default: "$caOrganization")
    --ca-organization-unit=NAME    CA organizational unit (default: "$caOrgUnit")
    --ca-email=EMAIL               CA contact email (default: admin@HOST_NAME)
    --base-image=IMAGE             Base image (default: "$baseImage")
    --output-dir=DIR               Directory for generated files (default: "$outputDir")
    --help, -h                     Print this help and exit
EOF
}

extract_value() {
    echo "${1#*=}"
}

while [ $# -gt 0 ]; do
    case $1 in
        --package-file=*)         packageFile=$(extract_value "$1") ;;
        --host-name=*)            hostName=$(extract_value "$1") ;;
        --admin-account=*)        adminAccount=$(extract_value "$1") ;;
        --public-port=*)          publicPort=$(extract_value "$1") ;;
        --private-port=*)         privatePort=$(extract_value "$1") ;;
        --ca-country=*)           caCountry=$(extract_value "$1") ;;
        --ca-state=*)             caState=$(extract_value "$1") ;;
        --ca-location=*)          caLocation=$(extract_value "$1") ;;
        --ca-organization=*)      caOrganization=$(extract_value "$1") ;;
        --ca-organization-unit=*) caOrgUnit=$(extract_value "$1") ;;
        --ca-email=*)             caEmail=$(extract_value "$1") ;;
        --base-image=*)           baseImage=$(extract_value "$1") ;;
        --output-dir=*)           outputDir=$(extract_value "$1") ;;
        --help | -h | -?)         print_help; exit 0 ;;
        *) echo "Unknown parameter '$1'" >&2; exit 1 ;;
    esac
    shift
done

if [ -z "$packageFile" ]; then
    echo "Error: --package-file is required" >&2
    print_help >&2
    exit 1
fi

if [ ! -f "$packageFile" ]; then
    echo "Error: Package file '$packageFile' not found" >&2
    exit 1
fi

[ -z "$caEmail" ] && caEmail="admin@${hostName}"

packageFileName=$(basename "$packageFile")
packageDirName=$(basename "$packageFile" .tar.gz)
cloudDir="/root/range-cloud"
caDir="/root/range-ca"
hostPrivateKey="${cloudDir}/etc/ssl/certs/${hostName}-private.pem"
hostPublicKey="${cloudDir}/etc/ssl/certs/${hostName}-public.pem"
hostCertDir="${cloudDir}/etc/ssl/certs"

mkdir -p "$outputDir"

# The package must be inside the build context (output directory)
destPackage="${outputDir}/${packageFileName}"
if [ "$(realpath "$packageFile")" != "$(realpath "$destPackage" 2>/dev/null)" ]; then
    echo "Copying package to output directory..."
    cp "$packageFile" "$destPackage"
fi

# ---------------------------------------------------------------------------
# entrypoint.sh  (quoted heredoc — variables resolved at container runtime)
# ---------------------------------------------------------------------------
cat > "${outputDir}/entrypoint.sh" << 'ENTRYPOINT_EOF'
#!/bin/bash
set -e

CONFIGURED_FLAG="${CLOUD_DIR}/.configured"
SLEEP_PID=

cleanup() {
    echo "Stopping Range Cloud service..."
    [ -n "${SLEEP_PID}" ] && kill "${SLEEP_PID}" 2>/dev/null || true
    "${CLOUD_DIR}/scripts/cloud_stop.sh" 2>/dev/null || true
    exit 0
}
trap cleanup SIGTERM SIGINT

echo "Starting Range Cloud service..."
"${CLOUD_DIR}/scripts/cloud_start.sh"

if [ ! -f "${CONFIGURED_FLAG}" ]; then
    echo "Waiting for service to be available..."
    retries=30
    while [ "${retries}" -gt 0 ]; do
        "${CLOUD_DIR}/scripts/cloud_status.sh" 2>/dev/null && break
        retries=$((retries - 1))
        sleep 2
    done

    if [ "${retries}" -eq 0 ]; then
        echo "Error: service did not become ready in time" >&2
        "${CLOUD_DIR}/scripts/cloud_stop.sh" 2>/dev/null || true
        exit 1
    fi

    echo "Configuring admin account: ${ADMIN_ACCOUNT}"

    "${CLOUD_DIR}/bin/cloud-tool" \
        --http-port="${PRIVATE_PORT}" \
        --host-key="${HOST_PUBLIC_KEY}" \
        --private-key="${CLOUD_DIR}/cert/clients/root.key.pem" \
        --private-key-password="" \
        --public-key="${CLOUD_DIR}/cert/clients/root.cert.pem" \
        --user-add \
        --resource-name="${ADMIN_ACCOUNT}"

    "${CLOUD_DIR}/bin/cloud-tool" \
        --http-port="${PRIVATE_PORT}" \
        --host-key="${HOST_PUBLIC_KEY}" \
        --private-key="${CLOUD_DIR}/cert/clients/root.key.pem" \
        --private-key-password="" \
        --public-key="${CLOUD_DIR}/cert/clients/root.cert.pem" \
        --user-update \
        --resource-name="${ADMIN_ACCOUNT}" \
        --json-content="{ \"name\": \"${ADMIN_ACCOUNT}\", \"groups\": [ \"users\", \"root\" ] }"

    touch "${CONFIGURED_FLAG}"
    echo "Admin account configured successfully"
fi

echo "Range Cloud is running"
while true; do
    sleep 60 &
    SLEEP_PID=$!
    wait "${SLEEP_PID}"
done
ENTRYPOINT_EOF

chmod +x "${outputDir}/entrypoint.sh"

# ---------------------------------------------------------------------------
# Containerfile  (unquoted heredoc — generator variables expanded here)
# ---------------------------------------------------------------------------
cat > "${outputDir}/Containerfile" << CONTAINERFILE_EOF
FROM ${baseImage}

# Runtime environment passed to entrypoint
ENV CLOUD_DIR=${cloudDir}
ENV CA_DIR=${caDir}
ENV ADMIN_ACCOUNT=${adminAccount}
ENV PRIVATE_PORT=${privatePort}
ENV HOST_PUBLIC_KEY=${hostPublicKey}

RUN apt-get update && \\
    apt-get install -y --no-install-recommends openssl && \\
    rm -rf /var/lib/apt/lists/*

COPY ${packageFileName} /tmp/${packageFileName}

# Extract package, set up CA, generate self-signed host certificates, set up cloud
RUN set -e && \\
    EXTRACT_DIR=\$(mktemp -d) && \\
    tar -xzf /tmp/${packageFileName} -C "\${EXTRACT_DIR}" && \\
    PKGDIR="\${EXTRACT_DIR}/${packageDirName}" && \\
    "\${PKGDIR}/range-ca/scripts/ca_setup.sh" \\
        --ca-dir="${caDir}" \\
        --country="${caCountry}" \\
        --state="${caState}" \\
        --location="${caLocation}" \\
        --organization="${caOrganization}" \\
        --organization-unit="${caOrgUnit}" \\
        --common-name="${hostName}" \\
        --email="${caEmail}" && \\
    mkdir -p "${hostCertDir}" && \\
    "${caDir}/scripts/ca_create_signed_certificate.sh" \\
        --key="${hostPrivateKey}" \\
        --cert="${hostPublicKey}" \\
        --extension="server_cert" \\
        --country="${caCountry}" \\
        --state="${caState}" \\
        --location="${caLocation}" \\
        --organization="${caOrganization}" \\
        --organization-unit="${caOrgUnit}" \\
        --common-name="${hostName}" \\
        --email="${caEmail}" && \\
    "\${PKGDIR}/scripts/cloud_setup.sh" \\
        --cloud-directory="${cloudDir}" \\
        --range-ca="${caDir}" \\
        --public-http-port=${publicPort} \\
        --private-http-port=${privatePort} \\
        --private-key="${hostPrivateKey}" \\
        --public-key="${hostPublicKey}" \\
        --country="${caCountry}" \\
        --state="${caState}" \\
        --location="${caLocation}" \\
        --organization="${caOrganization}" \\
        --organization-unit="${caOrgUnit}" \\
        --common-name="${hostName}" \\
        --email="${caEmail}" && \\
    rm -rf "\${EXTRACT_DIR}" /tmp/${packageFileName}

COPY entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

EXPOSE ${publicPort} ${privatePort}

ENTRYPOINT ["/entrypoint.sh"]
CONTAINERFILE_EOF

echo ""
echo "Generated files in: ${outputDir}"
echo "  Containerfile"
echo "  entrypoint.sh"
echo "  ${packageFileName}"
echo ""
echo "Build:"
echo "  docker build -t range-cloud ${outputDir}"
echo "  podman build -t range-cloud ${outputDir}"
echo ""
echo "Run:"
echo "  docker run -d --name range-cloud -p ${publicPort}:${publicPort} -p ${privatePort}:${privatePort} range-cloud"
echo "  podman run -d --name range-cloud -p ${publicPort}:${publicPort} -p ${privatePort}:${privatePort} range-cloud"
