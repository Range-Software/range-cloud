# Range Cloud Build Guide

## Prepare build environment

Initialize all sub modules
```
git submodule init && git submodule update --remote
```
Following command will attempt to download and install all required packages, therefore it must be executed under privileged (root) user
```
sudo ./src/range-build-tools/prereqs.sh
```
_NOTE: In case your OS does not provide you with Qt version 6.8 or newer download and install it from [https://www.qt.io/download/](https://www.qt.io/download/)._

## Build

```
cmake -S src -B build-Release && \
cmake --build build-Release --parallel
```

## Install

```
cmake --install build-Release --prefix <install-dir>
```
Where `<install-dir>` is a directory where software binaries and deployment script and configuration will be installed.

## Create an installation packages and installers (optional)

```
cmake --build build-Release --target package
```
## Cloud as a background process

### Setup Cloud

Following command will prepare complete directory structure, certificates and configuration for Cloud.
```
$ <install-dir>/scripts/cloud_setup.sh \
        --cloud-directory=<deploy-dir> \
        --public-http-port=4011 \
        --private-http-port=4012 \
        --range-ca=/path/to/range-ca \
        --country=US \
        --state=TX \
        --location=Houston \
        --organization=My Cloud \
        --organization-unit=Cloud \
        --common-name=my-cloud-host.com \
        --email=my.name@my-cloud-host.com
```
Where `<deploy-dir>` is a directory where cloud instance will be deployed.
_NOTE: There can be multiple deployments on the same host._

### Start/stop cloud as a background process

Following command will start Cloud as a background process.
```
$ <deploy-dir>/scripts/cloud_start.sh
```

Following command will stop Cloud running as a background process.
```
$ <deploy-dir>/scripts/cloud_stop.sh
```

## Send a test request (ping) to the cloud server

```
$ <deploy-dir>/bin/cloud-tool --host-key=<path_to_public_host_key> \
                              --private-key=<path_to_client_private_key> \
                              --private-key-password=<client_private_key_password> \
                              --public-key=<path_to_client_public_signed_key> \
                              --test-request
```

## Hello world plugin process example

Plugin processes are stored in `/<path_to_cloud>/processes/`.
To trigger `hello_world` plug-in process execute following command using `cloud-tool`.

```
$ <deploy-dir>/bin/cloud-tool --host-key=<path_to_public_host_key> \
                              --private-key=<path_to_client_private_key> \
                              --private-key-password=<client_private_key_password> \
                              --public-key=<path_to_client_public_signed_key> \
                              --process --json-content='{ "name": "hello-world", "arguments": { "<value2>": "value2", "<value1>": "value1" } }'
```
