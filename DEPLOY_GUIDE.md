# Range Cloud Deployment Guide

## Overview

This guide describes how to deploy a Range Cloud instance:

| Name | Purpose |
|------|---------|
| Range Cloud | User files |

---

## Prerequisites

- Linux x86\_64 system
- The Range Cloud package file: `range-cloud-<version>-linux-x86_64.tar.gz`
- Write access to `$HOME` and to the directory where host SSL certificates will be stored
- `tar` and `openssl` available on the system

---

## Configuration

The following values are used throughout this guide. Adjust them to match your environment before running any commands.

```bash
# Your deployment
HOST_NAME="your-host-name.com"
ADMIN_ACCOUNT="admin.name@your-host-name.com"

# Directories
CLOUD_DIR="$HOME/range-cloud"
CA_DIR="$HOME/range-ca"

# Host SSL certificate paths (existing certificates from your CA/Let's Encrypt)
HOST_PRIVATE_KEY="/etc/ssl/certs/${HOST_NAME}-privkey.pem"
HOST_PUBLIC_KEY="/etc/ssl/certs/${HOST_NAME}-fullchain.pem"

# Certificate Authority identity details
CA_COUNTRY="US"
CA_STATE="TX"
CA_LOCATION="Your City"
CA_ORGANIZATION="Your Company"
CA_ORG_UNIT="Cloud"
CA_COMMON_NAME="$HOST_NAME"
CA_EMAIL="tomas.soltys@${HOST_NAME}"

# Package file path
PACKAGE_FILE="/path/to/range-cloud-<version>-linux-x86_64.tar.gz"
```

---

## Directory and Port Layout

After deployment, the following directory structure is created:

```
$HOME/
├── range-ca/                  # Certificate Authority
└── range-cloud/               # Range Cloud instance
    ├── bin/
    ├── cert/
    │   └── clients/
    │       ├── root.key.pem
    │       └── root.cert.pem
    └── scripts/
```

Network ports:

| Purpose | Port |
|---------|------|
| Public HTTP | `4080` |
| Private HTTP (admin) | `4443` |

Ensure these ports are open in your firewall. The public port is used by clients; the private port is used for administration via `cloud-tool`.

---

## Step 1: Extract the Package

Extract the package to a temporary working directory. Keep `TMPDIR` and `PKGDIR` set for use in subsequent steps.

```bash
TMPDIR=$(mktemp -d -t "RANGE-CLOUD.XXXXXXXXXX")
PKGDIR="$TMPDIR/$(basename "$PACKAGE_FILE" .tar.gz)"

tar -xzvf "$PACKAGE_FILE" -C "$TMPDIR"
```

> **Note:** `PKGDIR` is referenced in several later steps. Keep this shell session open or re-export the variable if you open a new terminal.

---

## Step 2: Set Up the Certificate Authority

The Range CA is a local certificate authority used to sign internal service certificates. It only needs to be created once. If `$CA_DIR` already exists, skip this step.

```bash
if [ ! -d "$CA_DIR" ]; then
    "$PKGDIR/range-ca/scripts/ca_setup.sh" \
        --ca-dir="$CA_DIR" \
        --country="$CA_COUNTRY" \
        --state="$CA_STATE" \
        --location="$CA_LOCATION" \
        --organization="$CA_ORGANIZATION" \
        --organization-unit="$CA_ORG_UNIT" \
        --common-name="$CA_COMMON_NAME" \
        --email="$CA_EMAIL"
fi
```

---

## Step 3: Prepare Host SSL Certificates

The cloud instance requires a private key and a public certificate (full chain) for the host. Choose one of the two options below.

### Option A: Use Existing Certificates (Recommended)

If you already have valid SSL certificates for your domain (e.g., from Let's Encrypt), point the variables to those files:

```bash
HOST_PRIVATE_KEY="/etc/ssl/certs/${HOST_NAME}-privkey.pem"
HOST_PUBLIC_KEY="/etc/ssl/certs/${HOST_NAME}-fullchain.pem"
```

Verify both files exist before proceeding:

```bash
ls -la "$HOST_PRIVATE_KEY" "$HOST_PUBLIC_KEY"
```

### Option B: Generate Self-Signed Certificates via the Range CA

Use this option if you do not have external certificates. The Range CA created in Step 2 is used to sign the host certificate.

```bash
HOST_PRIVATE_KEY="$CLOUD_DIR/etc/ssl/certs/${HOST_NAME}-private.pem"
HOST_PUBLIC_KEY="$CLOUD_DIR/etc/ssl/certs/${HOST_NAME}-public.pem"

mkdir -p "$(dirname "$HOST_PRIVATE_KEY")"
mkdir -p "$(dirname "$HOST_PUBLIC_KEY")"

"$CA_DIR/scripts/ca_create_signed_certificate.sh" \
    --key="$HOST_PRIVATE_KEY" \
    --cert="$HOST_PUBLIC_KEY" \
    --extension="server_cert" \
    --country="$CA_COUNTRY" \
    --state="$CA_STATE" \
    --location="$CA_LOCATION" \
    --organization="$CA_ORGANIZATION" \
    --organization-unit="$CA_ORG_UNIT" \
    --common-name="$CA_COMMON_NAME" \
    --email="$CA_EMAIL"
```

> The generated key and certificate files must not already exist. If they do, remove them first.

---

## Step 4: Deploy Range Cloud

Run the setup script. This creates and configures the service directory at `$CLOUD_DIR`.

```bash
"$PKGDIR/scripts/cloud_setup.sh" \
    --cloud-directory="$CLOUD_DIR" \
    --range-ca="$CA_DIR" \
    --public-http-port=4021 \
    --private-http-port=4022 \
    --private-key="$HOST_PRIVATE_KEY" \
    --public-key="$HOST_PUBLIC_KEY" \
    --country="$CA_COUNTRY" \
    --state="$CA_STATE" \
    --location="$CA_LOCATION" \
    --organization="$CA_ORGANIZATION" \
    --organization-unit="$CA_ORG_UNIT" \
    --common-name="$CA_COMMON_NAME" \
    --email="$CA_EMAIL"
```

---

## Step 5: Start the Service

```bash
"$CLOUD_DIR/scripts/cloud_start.sh"
```

---

## Step 6: Configure the Admin Account

After the service is running, create and configure the administrator account. The `cloud-tool` binary connects via the private HTTP port and authenticates using the root client certificates generated during setup.

```bash
# Add the admin user
"$CLOUD_DIR/bin/cloud-tool" \
    --http-port=4022 \
    --host-key="$HOST_PUBLIC_KEY" \
    --private-key="$CLOUD_DIR/cert/clients/root.key.pem" \
    --private-key-password="" \
    --public-key="$CLOUD_DIR/cert/clients/root.cert.pem" \
    --user-add \
    --resource-name="$ADMIN_ACCOUNT"

# Assign the admin user to the 'users' and 'root' groups
"$CLOUD_DIR/bin/cloud-tool" \
    --http-port=4022 \
    --host-key="$HOST_PUBLIC_KEY" \
    --private-key="$CLOUD_DIR/cert/clients/root.key.pem" \
    --private-key-password="" \
    --public-key="$CLOUD_DIR/cert/clients/root.cert.pem" \
    --user-update \
    --resource-name="$ADMIN_ACCOUNT" \
    --json-content="{ \"name\": \"$ADMIN_ACCOUNT\", \"groups\": [ \"users\", \"root\" ] }"
```

---

## Step 7: Clean Up Temporary Files

Remove the temporary extraction directory created in Step 1.

```bash
rm -rf "$TMPDIR"
```

---

## Verification

Check that the service is running and healthy:

```bash
"$CLOUD_DIR/scripts/cloud_status.sh"
```

---

## Ongoing Operations

All operational commands run directly against the deployed instance directory. No package file is required except for updates.

### Start the Service

```bash
"$CLOUD_DIR/scripts/cloud_start.sh"
```

### Stop the Service (Graceful)

```bash
"$CLOUD_DIR/scripts/cloud_stop.sh"
```

### Kill the Service (Force)

```bash
"$CLOUD_DIR/scripts/cloud_stop.sh" --force
```

### Check Service Status

```bash
"$CLOUD_DIR/scripts/cloud_status.sh"
```

### Update the Software

Updating stops the service, replaces the software, and restarts it. You need a new package file.

```bash
# Re-extract the new package
TMPDIR=$(mktemp -d -t "RANGE-CLOUD.XXXXXXXXXX")
PKGDIR="$TMPDIR/$(basename "$PACKAGE_FILE" .tar.gz)"
tar -xzvf "$PACKAGE_FILE" -C "$TMPDIR"

# Stop, update, and restart
"$CLOUD_DIR/scripts/cloud_stop.sh"
"$PKGDIR/scripts/cloud_update_software.sh" --cloud-directory="$CLOUD_DIR"
"$CLOUD_DIR/scripts/cloud_start.sh"

# Clean up
rm -rf "$TMPDIR"
```

### Clear / Erase the Environment

> **Warning:** This permanently deletes all instance data. It cannot be undone.

```bash
"$CLOUD_DIR/scripts/cloud_stop.sh"
rm -rf "$CLOUD_DIR"
```

To also remove the Certificate Authority:

```bash
rm -rf "$CA_DIR"
```
