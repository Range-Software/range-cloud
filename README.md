# Cloud

Simple cloud service with HTTPs based [REST API](doc/rest_api.md) interface.

## Features

1. Store files
    * Upload
    * Replace
    * Update
    * Download
    * Set tags (alphanumeric string)
    * Set version
    * Set access rights
2. User management
    * Add user
    * Modify user (change user name, modify user groups and configure file quotas)
    * Remove user
    * Grant/revoke access to actions
3. HTTPs interface
    * Public port accessible to everybody
    * Private port accessible only to authenticated clients
    * Client authentication is based on signed certificate
4. Create custom plug-in processes
5. Submit reports

## Build guide

See [Range Cloud Build Guide](doc/build.md) for instructions on how to prepare the build environment, build, install and create installation packages.

## Deployment guides

* [Range Cloud Native Deployment Guide](doc/deploy_native.md)
* [Range Cloud Container Deployment Guide](doc/deploy_container.md)

## Download
To download already built binaries please visit http://range-software.com

## Powered by

* Qt - https://www.qt.io/
