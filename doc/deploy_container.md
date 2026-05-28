# Range Cloud Container Deployment Guide

## Overview

This guide describes how to build and run a Range Cloud instance as a Docker or Podman container. The process has two stages:

1. **Generate** — run `scripts/generate_containerfile.sh` to produce a ready-to-use build context (Containerfile + entrypoint script) with all deployment parameters baked in.
2. **Build and run** — build the container image and start the container using Docker or Podman.

All Range Cloud setup steps (Certificate Authority creation, self-signed host certificate generation, cloud service configuration) happen during `docker build` / `podman build`. The admin account is configured automatically on the first container start.

---

## Prerequisites

- Docker or Podman installed on the build host
- The Range Cloud package file: `range-cloud-<version>-linux-x86_64.tar.gz`
- Bash

---

## Script Reference — `generate_containerfile.sh`

### Synopsis

```
scripts/generate_containerfile.sh --package-file=FILENAME [OPTION]...
```

### Parameters

| Parameter | Required | Default | Description |
|-----------|----------|---------|-------------|
| `--package-file=FILENAME` | Yes | — | Path to the range-cloud `.tar.gz` package |
| `--host-name=NAME` | No | `your-host-name.com` | Host name embedded in generated certificates |
| `--admin-account=EMAIL` | No | `admin@your-host-name.com` | Administrator account email address |
| `--public-port=PORT` | No | `4080` | Public HTTP port exposed by the container |
| `--private-port=PORT` | No | `4443` | Private HTTP port used for administration |
| `--ca-country=CODE` | No | `EU` | Two-letter country code for the CA certificate |
| `--ca-state=NAME` | No | `CZ` | State or region for the CA certificate |
| `--ca-location=NAME` | No | `Prague` | City for the CA certificate |
| `--ca-organization=NAME` | No | `Range Software` | Organization for the CA certificate |
| `--ca-organization-unit=NAME` | No | `Cloud` | Organizational unit for the CA certificate |
| `--ca-email=EMAIL` | No | `admin@HOST_NAME` | Contact email embedded in the CA certificate |
| `--base-image=IMAGE` | No | `ubuntu:24.04` | Base image for the generated Containerfile |
| `--output-dir=DIR` | No | `.` (current directory) | Directory where generated files are written |
| `--help`, `-h` | No | — | Print help and exit |

### Generated files

The script writes three files to `--output-dir`:

| File | Description |
|------|-------------|
| `Containerfile` | Build instructions for Docker / Podman |
| `entrypoint.sh` | Container startup script |
| `<package>.tar.gz` | Copy of the package (required in the build context) |

### What the Containerfile does at build time

1. Installs `openssl` on the base image.
2. Extracts the package to a temporary directory.
3. Runs `ca_setup.sh` to create a local Certificate Authority at `/root/range-ca`.
4. Runs `ca_create_signed_certificate.sh` to generate a self-signed host key and certificate at `/root/range-cloud/etc/ssl/certs/`.
5. Runs `cloud_setup.sh` to install and configure the Range Cloud service at `/root/range-cloud`.
6. Cleans up the extracted package and temporary files.

All configuration values (`--host-name`, `--ca-*`, ports) are expanded and embedded in the Containerfile at generation time — no build arguments are required when building the image.

### What the entrypoint does at container start

**Every start:**
- Starts the Range Cloud service via `cloud_start.sh`.
- Registers a signal handler: on `SIGTERM` or `SIGINT` it calls `cloud_stop.sh` before the container exits.

**First start only** (detected via a `.configured` sentinel file):
- Polls `cloud_status.sh` up to 30 times (2-second intervals) until the service is ready.
- Creates the administrator account using `cloud-tool --user-add`.
- Assigns the administrator to the `users` and `root` groups using `cloud-tool --user-update`.
- Writes a `.configured` marker so this step is skipped on all subsequent starts.

---

## Deployment

### Step 1: Generate the build context

Run the script from the project root, pointing to the package file and providing your deployment values.

```bash
scripts/generate_containerfile.sh \
    --package-file=/path/to/range-cloud-<version>-linux-x86_64.tar.gz \
    --host-name=myhost.com \
    --admin-account=admin@myhost.com \
    --output-dir=./container-build
```

The script prints the exact `build` and `run` commands to use once it completes.

### Step 2: Build the image

```bash
# Docker
docker build -t range-cloud ./container-build

# Podman
podman build -t range-cloud ./container-build
```

The build may take several minutes while the CA and cloud service are configured inside the image.

### Step 3: Run the container

```bash
# Docker
docker run -d \
    --name range-cloud \
    -p 4080:4080 \
    -p 4443:4443 \
    range-cloud

# Podman
podman run -d \
    --name range-cloud \
    -p 4080:4080 \
    -p 4443:4443 \
    range-cloud
```

On first start, the container configures the admin account before becoming fully operational. Allow a few seconds after `docker run` returns before connecting.

---

## Container Management

### View logs

```bash
docker logs -f range-cloud
podman logs -f range-cloud
```

### Stop gracefully

Sends `SIGTERM`; the entrypoint calls `cloud_stop.sh` before exiting.

```bash
docker stop range-cloud
podman stop range-cloud
```

### Restart

```bash
docker restart range-cloud
podman restart range-cloud
```

Admin account configuration is skipped on restart because the `.configured` sentinel persists inside the container.

### Remove the container

```bash
docker rm -f range-cloud
podman rm -f range-cloud
```

---

## Data Persistence

By default, all service data (including the `.configured` sentinel, user data, and certificates) lives inside the container at `/root/range-cloud`. Removing the container with `docker rm` permanently deletes this data.

To persist data across container recreations, mount a volume over the cloud data directory:

```bash
docker run -d \
    --name range-cloud \
    -p 4080:4443 \
    -p 4443:4443 \
    -v range-cloud-data:/root/range-cloud \
    range-cloud
```

> **Note:** On the very first run with an empty named volume, Docker copies the image contents into the volume. On subsequent runs the volume contents are used as-is, including the `.configured` sentinel, so the admin account setup is not repeated.

---

## Customization Examples

### Different host name and ports

```bash
scripts/generate_containerfile.sh \
    --package-file=range-cloud-1.0.4-linux-x86_64.tar.gz \
    --host-name=cloud.example.org \
    --admin-account=sysadmin@example.org \
    --public-port=8080 \
    --private-port=8443 \
    --output-dir=./container-build
```

### Custom CA identity

```bash
scripts/generate_containerfile.sh \
    --package-file=range-cloud-1.0.4-linux-x86_64.tar.gz \
    --host-name=cloud.example.org \
    --admin-account=sysadmin@example.org \
    --ca-country=US \
    --ca-state=California \
    --ca-location="San Francisco" \
    --ca-organization="Example Corp" \
    --ca-organization-unit=Engineering \
    --ca-email=pki@example.org \
    --output-dir=./container-build
```

### Alternative base image

```bash
scripts/generate_containerfile.sh \
    --package-file=range-cloud-1.0.4-linux-x86_64.tar.gz \
    --base-image=debian:12 \
    --output-dir=./container-build
```

> **Note:** The generated Containerfile installs `openssl` via `apt-get`. If you change to a non-Debian base image you will need to edit the generated `Containerfile` to use the appropriate package manager.
