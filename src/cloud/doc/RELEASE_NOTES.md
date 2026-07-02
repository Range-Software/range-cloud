## Version 1.0.6

### Improvements

- Replaced polling loops in FileManager and Mailer with event-driven task queues (wait conditions)
- Mailer now produces properly formatted email messages (From/To/Subject headers)
- File checksum is computed from in-memory content instead of re-reading the stored file
- Action and process access rights changes are persisted immediately

### Bug Fixes

- Fix double-free of FileManager and Mailer on application shutdown
- Fix possible request starvation/deadlock of the global thread pool on machines with few cores
- Unknown actions now receive an error response instead of leaving the HTTP request unanswered
- Authentication token removal now verifies that the token belongs to the authorized resource name
- Renaming a user to an already existing user name is rejected
- Group removal now persists the fully updated state (group stripped from users before write)
- File replace no longer aborts on first removal failure and reports the complete outcome
- Protect file index and statistics with a dedicated mutex (statistics reads vs. worker thread)
- Fixed wrong placeholders in unauthorized-token error messages
- Fixed groups being counted as users in user manager statistics

### Submodules

- range-ai-lib @ v1.0.0
- range-base-lib @ v1.0.1
- range-build-tools @ v1.0.0
- range-cloud-lib @ v1.0.3

---

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
