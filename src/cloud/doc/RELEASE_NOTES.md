## Version 1.0.5

### Improvements

- Added max body size configuration to http server

### Bug Fixes

- Guard process signal handlers against double-emit/use-after-move crash

### Submodules

- range-ai-lib @ v1.0.0
- range-base-lib @ v1.0.1
- range-build-tools @ v1.0.0
- range-cloud-lib @ v1.0.2

---

## Version 1.0.4

### Bug Fixes

- Fix SSL error on macOS by seeding system CA certificates

### Submodules

- range-ai-lib @ v1.0.0
- range-base-lib @ v1.0.0
- range-build-tools @ v1.0.0
- range-cloud-lib @ v1.0.0

---

## Version 1.0.3

- Fixed various memory leaks

---

## Version 1.0.2

- Improved HTTP server thread safety, timeout handling, and resource cleanup
- Added certificate expiry validation to RHttpServer and RHttpClient
- Added script to renew expired certificate for local accounts
- Log Qt debug messages

---

## Version 1.0.1

- Print Qt library and core application info on startup

---

## Version 1.0.0

Initial release.
