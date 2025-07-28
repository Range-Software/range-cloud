# Cloud

Simple cloud service with HTTPs based [REST API](doc/rest_api.md) interface.

## Features

1. Store files
    * Upload
    * Update
    * Download
    * Set tags (alphanumeric string)
    * Set version
    * Set access rights
2. User management
    * Add user
    * Modify user
    * Remove user
    * Grant/revoke access to actions
3. HTTPs interface
    * Public port accessible to everybody
    * Private port accessible only to authenticated clients
    * Client authentication is based on signed certificate
4. Create custom plug-in processes
5. Submit reports

## Prepare build environment
Initialize all submodules
```
git submodule init && git submodule update --remote
```
Following command will attempt to download and install all required packages, therefore it must be executed under priviledged (root) user
```
sudo ./src/range-build-tools/prereqs.sh
```
_NOTE: In case your OS does not provide you with Qt version 6.8 or newer download and installit from [https://www.qt.io/download/](https://www.qt.io/download/)._
## Build
```
cmake -S src -B build
cmake --build build --parallel
```
## Install
```
cmake --install build --prefix <install-dir>
```
Where `<install-dir>` is a directory where software binaries and deployment script and configuration will be installed.
## Create an installation packages and installers (optional)
```
cmake --build build --target package
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

### Start/stop cloud as a beckground process

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

## Download
To download already built binaries please visit http://range-software.com

## Powered by

* Qt - https://www.qt.io/
