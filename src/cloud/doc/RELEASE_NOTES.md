## Version 1.2.0

### Improvements

- New configuration settings: `maxConnections` (default 1000) and `maxConnectionsPerHost` (default 20) limit simultaneous connections to the HTTP servers
- `maxBodySize`, `maxConnections` and `maxConnectionsPerHost` are now applied to both public and private HTTP servers
- Qt 6.12 is now the minimum required version (CMake 3.25 minimum); CI builds with Qt 6.12.0

### Bug Fixes

- `maxBodySize` default was read from an uninitialized value when not set in the configuration file

### Submodules

- range-ai-lib @ v1.1.0
- range-base-lib @ v1.1.0
- range-build-tools @ v1.0.0
- range-cloud-lib @ v1.1.1

---

## Version 1.1.0

### Improvements

- New AI query service: the `ai-query` action (cloud-tool: `--ai-query`) forwards a structured question — `question` plus optional `fileId`, `fileDescription`, `context` and `language` — to an external AI service
- AI queries are processed asynchronously: submission returns a query id immediately and the answer is fetched with the new `ai-query-result` action (cloud-tool polls transparently); each result is delivered once, only to the submitting user or root, and unfetched results are discarded after one hour
- A query can reference a stored file, read under the caller's access rights: for Claude (Anthropic) it is attached via the Anthropic Files API (PDF, PNG, JPEG, GIF, WebP and UTF-8 text) with content-hash re-use, replace-on-change and least-recently-used eviction against the remote store capacity; otherwise its text is inlined, and oversized or non-text files are rejected
- Per-application AI contexts and guardrails are configurable in `etc/aiqueries.json` (created with defaults on first start)
- New configuration settings: `aiType`, `aiApiUrl`, `aiApiKey`, `aiModel`, `aiMaxTokens`, `aiMaxFileContextSize` and `aiRemoteFileStoreSize` (Anthropic file store capacity, default 100 GB)
- File store writes are atomic: an interrupted write can no longer corrupt a stored file
- Emails are now properly formatted (From/To/Subject headers)
- Access rights changes are saved immediately
- Server log file renamed from `Cloud.log` to `cloud.log` to match the binary name

### Bug Fixes

- Fixed crash on application shutdown
- `cloud_tool.sh` no longer word-splits quoted arguments (e.g. `--json-content` with spaces) when forwarding them to `cloud-tool`
- Fixed possible deadlock on machines with few CPU cores
- Unknown actions now receive an error response instead of leaving the request unanswered
- Authentication token removal now verifies that the token belongs to the requesting user
- Renaming a user to an already existing user name is rejected
- File replace no longer aborts on first removal failure and reports the complete outcome

### Submodules

- range-ai-lib @ v1.1.0
- range-base-lib @ v1.1.0
- range-build-tools @ v1.0.0
- range-cloud-lib @ v1.1.0

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
