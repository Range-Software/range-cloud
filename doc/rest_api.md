# Range Cloud - REST API

* [Service](#service)
  * [Send a test request (ping) to the cloud server](#send-a-test-request-ping-to-the-cloud-server)
  * [Cloud server statistics](#cloud-server-statistics)
  * [Stop the cloud server](#stop-the-cloud-server)
* [File store](#file-store)
  * [List files on the cloud server](#list-files-on-the-cloud-server)
  * [Get file information](#get-file-information)
  * [Upload file to the cloud server](#upload-file-to-the-cloud-server)
  * [Replace file on the cloud server](#replace-file-on-the-cloud-server)
  * [Update file on the cloud server](#update-file-on-the-cloud-server)
  * [Update file access owner on the cloud server](#update-file-access-owner-on-the-cloud-server)
  * [Update file access mode on the cloud server](#update-file-access-mode-on-the-cloud-server)
  * [Update file version on the cloud server](#update-file-version-on-the-cloud-server)
  * [Update file tags on the cloud server](#update-file-tags-on-the-cloud-server)
  * [Download file from the cloud server](#download-file-from-the-cloud-server)
  * [Remove file from the cloud server](#remove-file-from-the-cloud-server)
* [Process](#process)
  * [Start a cloud server process](#start-a-cloud-server-process)
* [Process management](#process-management)
  * [List processes on the cloud server](#list-processes-on-the-cloud-server)
  * [Update process access owner on the cloud server](#update-process-access-owner-on-the-cloud-server)
  * [Update process access mode on the cloud server](#update-process-access-mode-on-the-cloud-server)
* [Action management](#action-management)
  * [List actions on the cloud server](#list-actions-on-the-cloud-server)
  * [Update action access owner on the cloud server](#update-action-access-owner-on-the-cloud-server)
  * [Update action access mode on the cloud server](#update-action-access-mode-on-the-cloud-server)
* [User management](#user-management)
  * [List users on the cloud server](#list-users-on-the-cloud-server)
  * [Get user information](#get-user-information)
  * [Add new user](#add-new-user)
  * [Update existing user](#update-existing-user)
  * [Remove existing user](#remove-existing-user)
  * [Register new user with suggested user name](#register-new-user-with-suggested-user-name)
* [User authentication tokens management](#user-authentication-tokens-management)
  * [List authentication tokens for given user](#list-authentication-tokens-for-given-user)
  * [Generate user authentication token](#generate-user-authentication-token)
  * [Remove user authentication token](#remove-user-authentication-token)
* [Group management](#group-management)
  * [List groups on the cloud server](#list-groups-on-the-cloud-server)
  * [Get group information](#get-group-information)
  * [Add new group](#add-new-group)
  * [Remove existing group](#remove-existing-group)
* [Reporting](#reporting)
  * [Submit report](#submit-report)
* [Artificial intelligence](#artificial-intelligence)
  * [Submit AI query](#submit-ai-query)
  * [Fetch AI query result](#fetch-ai-query-result)

---

## Service

### Send a test request (ping) to the cloud server
```
POST https://<host>:<port>/test-request/
```
**Body:**
```
<Any text which will be part of the server response.>
```
**Response:**
```
<Any text which will be part of the server response.>
```

### Cloud server statistics
```
GET https://<host>:<port>/statistics/
```
**Body:**
```
<empty>
```
**Response:**
```
{
    "dateTime": {
        "current": "<curent-date-time>",
        "start": "<service-start-date-time>",
        "upTime": "<up-time>"
    },
    "general": {
        "version": "<service-software-version>"
    },
    "services": [
        {
            "index": {
                "bytes": 0,
                "files": {
                    "average": 0,
                    "maximum": 0,
                    "median": 0,
                    "minimum": 0,
                    "p05": 0,
                    "p95": 0,
                    "size": 0
                },
                "size": 0
            },
            "name": "FileService"
        },
        {
            "name": "ActionService",
            "size": 35
        },
        {
            "name": "ProcessService",
            "size": 3
        },
        {
            "name": "ReportService",
            "reports": 0
        },
        {
            "name": "UserService",
            "users": 6
        },
        {
            "name": "MailerService"
        }
    ]
}

```

### Stop the cloud server
```
GET https://<host>:<port>/stop/
```
**Body:**
```
<empty>
```
**Response:**
```
<status-message>
```

---

## File store

### List files on the cloud server
```
GET https://<host>:<port>/list-files/
```
**Body:**
```
<empty>
```
**Response:**
```
"files": [
    {
        "id": "<uid>",
        "path": "<file-path>",
        "size": "<bytes>",
        "created": "<seconds-since-epoch>",
        "updated": "<seconds-since-epoch>",
        "version": "<version>",
        "access": {
            "mode": {
                "user": <access-mode>,
                "group": <access-mode>,
                "other": <access-mode>
            },
            "owner": {
                "user": "<owner-username>",
                "group": "<owner-group>"
            }
        },
        "tags": [
            "<tag-1>",
            "<tag-2>",
            ...
            "<tag-n>"
        ]
    }
]
```

### Get file information
```
GET https://<host>:<port>/file-info/?resource-id=<uid>
```
**Body:**
```
<empty>
```
**Response:**
```
{
    "id": "<uid>",
    "path": "<file-path>",
    "size": "<bytes>",
    "created": "<seconds-since-epoch>",
    "updated": "<seconds-since-epoch>",
    "version": "<version>",
    "access": {
        "mode": {
            "user": <access-mode>,
            "group": <access-mode>,
            "other": <access-mode>
        },
        "owner": {
            "user": "<owner-username>",
            "group": "<owner-group>"
        }
    },
    "tags": [
        "<tag-1>",
        "<tag-2>",
        ...
        "<tag-n>"
    ]
}
```

### Upload file to the cloud server
```
PUT https://<host>:<port>/file-upload/?resource-name=<file-path>
```
**Body:**
```
<content of the file to be uploaded>
```
**Response:**
```
{
    "id": "<uid>",
    "path": "<file-path>",
    "size": "<bytes>",
    "created": "<seconds-since-epoch>",
    "updated": "<seconds-since-epoch>",
    "version": "<version>",
    "access": {
        "mode": {
            "user": <access-mode>,
            "group": <access-mode>,
            "other": <access-mode>
        },
        "owner": {
            "user": "<owner-username>",
            "group": "<owner-group>"
        }
    },
    "tags": [
        "<tag-1>",
        "<tag-2>",
        ...
        "<tag-n>"
    ]
}
```

### Replace file on the cloud server
All files with given file name and owned by the requester will be replaced (removed) by provided file.
_NOTE: All additional information such as tags, version and custom access rights will be reset to initial values._
```
PUT https://<host>:<port>/file-replace/?resource-name=<file-path>
```
**Body:**
```
<content of the file to be uploaded>
```
**Response:**
```
{
    "upload": {
        "id": "<uid>",
        "path": "<file-path>",
        "size": "<bytes>",
        "created": "<seconds-since-epoch>",
        "updated": "<seconds-since-epoch>",
        "version": "<version>",
        "access": {
            "mode": {
                "user": <access-mode>,
                "group": <access-mode>,
                "other": <access-mode>
            },
            "owner": {
                "user": "<owner-username>",
                "group": "<owner-group>"
            }
        },
        "tags": [
            "<tag-1>",
            "<tag-2>",
            ...
            "<tag-n>"
        ]
    },
    "remove": [
        ...
    ]
}
```

### Update file on the cloud server
```
PUT https://<host>:<port>/file-update/?resource-id=<uid>&resource-name=<file-path>
```
**Body:**
```
<content of the file to be updated>
```
**Response:**
```
{
    "id": "<uid>",
    "path": "<file-path>",
    "size": "<bytes>",
    "created": "<seconds-since-epoch>",
    "updated": "<seconds-since-epoch>",
    "version": "<version>",
    "access": {
        "mode": {
            "user": <access-mode>,
            "group": <access-mode>,
            "other": <access-mode>
        },
        "owner": {
            "user": "<owner-username>",
            "group": "<owner-group>"
        }
    },
    "tags": [
        "<tag-1>",
        "<tag-2>",
        ...
        "<tag-n>"
    ]
}
```

### Update file access owner on the cloud server
```
POST https://<host>:<port>/file-update-access-owner/?resource-id=<uid>
```
**Body:**
```
{
    "user": "<owner-username>",
    "group": "<owner-group>"
}
```
**Response:**
```
{
    "id": "<uid>",
    "path": "<file-path>",
    "size": "<bytes>",
    "created": "<seconds-since-epoch>",
    "updated": "<seconds-since-epoch>",
    "version": "<version>",
    "access": {
        "mode": {
            "user": <access-mode>,
            "group": <access-mode>,
            "other": <access-mode>
        },
        "owner": {
            "user": "<owner-username>",
            "group": "<owner-group>"
        }
    },
    "tags": [
        "<tag-1>",
        "<tag-2>",
        ...
        "<tag-n>"
    ]
}
```

### Update file access mode on the cloud server
```
POST https://<host>:<port>/file-update-access-mode/?resource-id=<uid>
```
**Body:**
```
{
    "user": <access-mode>,
    "group": <access-mode>,
    "other": <access-mode>
}
```
**`<access-mode>`:**

* 0 = No access
* 1 = `--x`
* 2 = `-w-`
* 3 = `-wx`
* 4 = `r--`
* 5 = `r-x`
* 6 = `rw-`
* 7 = `rwx`

**Response:**
```
{
    "id": "<uid>",
    "path": "<file-path>",
    "size": "<bytes>",
    "created": "<seconds-since-epoch>",
    "updated": "<seconds-since-epoch>",
    "version": "<version>",
    "access": {
        "mode": {
            "user": <access-mode>,
            "group": <access-mode>,
            "other": <access-mode>
        },
        "owner": {
            "user": "<owner-username>",
            "group": "<owner-group>"
        }
    },
    "tags": [
        "<tag-1>",
        "<tag-2>",
        ...
        "<tag-n>"
    ]
}
```

### Update file version on the cloud server
```
POST https://<host>:<port>/file-update-version/?resource-id=<uid>
```
**Body:**
```
1.2.3
```
**Response:**
```
{
    "id": "<uid>",
    "path": "<file-path>",
    "size": "<bytes>",
    "created": "<seconds-since-epoch>",
    "updated": "<seconds-since-epoch>",
    "version": "<version>",
    "access": {
        "mode": {
            "user": <access-mode>,
            "group": <access-mode>,
            "other": <access-mode>
        },
        "owner": {
            "user": "<owner-username>",
            "group": "<owner-group>"
        }
    },
    "tags": [
        "<tag-1>",
        "<tag-2>",
        ...
        "<tag-n>"
    ]
}
```

### Update file tags on the cloud server

```
POST https://<host>:<port>/file-update-tags/?resource-id=<uid>
```
**Body:**
```
<tag-1>,<tag-2>,...,<tag-n>
```
**Response:**
```
{
    "id": "<uid>",
    "path": "<file-path>",
    "size": "<bytes>",
    "created": "<seconds-since-epoch>",
    "updated": "<seconds-since-epoch>",
    "version": "<version>",
    "access": {
        "mode": {
            "user": <access-mode>,
            "group": <access-mode>,
            "other": <access-mode>
        },
        "owner": {
            "user": "<owner-username>",
            "group": "<owner-group>"
        }
    },
    "tags": [
        "<tag-1>",
        "<tag-2>",
        ...
        "<tag-n>"
    ]
}
```

### Download file from the cloud server
```
GET https://<host>:<port>/file-download/?resource-id=<uid>
```
**Body:**
```
<empty>
```
**Response:**
```
<content of the file to be uploaded>
```

### Remove file from the cloud server
```
GET https://<host>:<port>/file-remove/?resource-id=<uid>
```
**Body:**
```
<empty>
```
**Response:**
```
<uid>
```

---

## Process


### Start a cloud server process

```
POST https://<host>:<port>/process/
```
**Body:**
Following is an example of "hello-world" process.
```
{
    "name": "hello-world",
    "arguments": {
        "value1": "v1",
        "value2": "v2"
    }
}
```
**Response:**
```
<process output>
```

---

## Process management

### List processes on the cloud server
```
GET https://<host>:<port>/list-processes/
```
**Body:**
```
<empty>
```
**Response:**
```
"processes": [
    {
        "name": "<process-name>",
        "executable": "<path-to-executable>",
        "arguments": "<process-command-line-arguments>",
        "access": {
            "mode": {
                "user": <access-mode>,
                "group": <access-mode>,
                "other": <access-mode>
            },
            "owner": {
                "user": "<owner-username>",
                "group": "<owner-group>"
            }
        }
    }
]
```

### Update process access owner on the cloud server
```
POST https://<host>:<port>/process-update-access-owner/?resource-name=<resource-name>
```
**Body:**
```
{
    "user": "<owner-username>",
    "group": "<owner-group>"
}
```
**Response:**
```
{
    "name": "<process-name>",
    "executable": "<path-to-executable>",
    "arguments": "<process-command-line-arguments>",
    "access": {
        "mode": {
            "user": <access-mode>,
            "group": <access-mode>,
            "other": <access-mode>
        },
        "owner": {
            "user": "<owner-username>",
            "group": "<owner-group>"
        }
    }
}
```

### Update process access mode on the cloud server
```
POST https://<host>:<port>/process-update-access-mode/?resource-name=<resource-name>
```
**Body:**
```
{
    "user": <access-mode>,
    "group": <access-mode>,
    "other": <access-mode>
}
```
**`<access-mode>`:**

* 0 = No access
* 1 = `--x`
* 2 = `-w-`
* 3 = `-wx`
* 4 = `r--`
* 5 = `r-x`
* 6 = `rw-`
* 7 = `rwx`

**Response:**
```
{
    "name": "<process-name>",
    "executable": "<path-to-executable>",
    "arguments": "<process-command-line-arguments>",
    "access": {
        "mode": {
            "user": <access-mode>,
            "group": <access-mode>,
            "other": <access-mode>
        },
        "owner": {
            "user": "<owner-username>",
            "group": "<owner-group>"
        }
    }
}
```

---

## Action management

### List actions on the cloud server
```
GET https://<host>:<port>/list-actions/
```
**Body:**
```
<empty>
```
**Response:**
```
"actions": [
    {
        "name": "<action-name>".
        "access": {
            "mode": {
                "user": <access-mode>,
                "group": <access-mode>,
                "other": <access-mode>
            },
            "owner": {
                "user": "<owner-username>",
                "group": "<owner-group>"
            }
        }
    }
]
```

### Update action access owner on the cloud server
```
POST https://<host>:<port>/action-update-access-owner/?resource-name=<resource-name>
```
**Body:**
```
{
    "user": "<owner-username>",
    "group": "<owner-group>"
}
```
**Response:**
```
{
    "name": "<action-name>".
    "access": {
        "mode": {
            "user": <access-mode>,
            "group": <access-mode>,
            "other": <access-mode>
        },
        "owner": {
            "user": "<owner-username>",
            "group": "<owner-group>"
        }
    }
}
```

### Update action access mode on the cloud server
```
POST https://<host>:<port>/action-update-access-mode/?resource-name=<resource-name>
```
**Body:**
```
{
    "user": <access-mode>,
    "group": <access-mode>,
    "other": <access-mode>
}
```
**`<access-mode>`:**

* 0 = No access
* 1 = `--x`
* 2 = `-w-`
* 3 = `-wx`
* 4 = `r--`
* 5 = `r-x`
* 6 = `rw-`
* 7 = `rwx`

**Response:**
```
{
    "name": "<action-name>".
    "access": {
        "mode": {
            "user": <access-mode>,
            "group": <access-mode>,
            "other": <access-mode>
        },
        "owner": {
            "user": "<owner-username>",
            "group": "<owner-group>"
        }
    }
}
```

---

## User management

### List users on the cloud server
```
GET https://<host>:<port>/list-users/
```
**Body:**
```
<empty>
```
**Response:**
```
"users": [
    {
        "name": "<user-name>",
        "groups": [
            "<group-name-1>",
            "<group-name-2>",
            ...
            "<group-name-n>"
        ],
        "quotas": {
            "files": {
                "fileCount": "100",
                "fileSize": "104857600",
                "storeSize": "104857600"
            }
        }
    }
]
```

### Get user information
```
GET https://<host>:<port>/user-info/?resource-name=<user-name>
```
**Body:**
```
<empty>
```
**Response:**
```
{
    "name": "<user-name>",
    "groups": [
        "<group-name-1>",
        "<group-name-2>",
        ...
        "<group-name-n>"
    ],
    "quotas": {
        "files": {
            "fileCount": "100",
            "fileSize": "104857600",
            "storeSize": "104857600"
        }
    }
}
```
If no `<user-name>` is provided caller user information will be returned.

### Add new user
```
GET https://<host>:<port>/user-add/?resource-name=<user-name>
```
**Body:**
```
<empty>
```
**Response:**
```
{
    "name": "<user-name>",
    "groups": [
        "<group-name-1>",
        "<group-name-2>",
        ...
        "<group-name-n>"
    ],
    "quotas": {
        "files": {
            "fileCount": "100",
            "fileSize": "104857600",
            "storeSize": "104857600"
        }
    }
}
```

### Update existing user
```
POST https://<host>:<port>/user-update/?resource-name=<user-name>
```
**Body:**
```
{
    "name": "<user-name>",
    "groups": [
        "<group-name-1>",
        "<group-name-2>",
        ...
        "<group-name-n>"
    ],
    "quotas": {
        "files": {
            "fileCount": "100",
            "fileSize": "104857600",
            "storeSize": "104857600"
        }
    }
}

```
_NOTE: If **"quotas:"** object is not provided or empty, default values will be set._
**Response:**
```
{
    "name": "<user-name>",
    "groups": [
        "<group-name-1>",
        "<group-name-2>",
        ...
        "<group-name-n>"
    ],
    "quotas": {
        "files": {
            "fileCount": "100",
            "fileSize": "104857600",
            "storeSize": "104857600"
        }
    }
}
```

### Remove existing user
```
GET https://<host>:<port>/user-remove/?resource-name=<user-name>
```
**Body:**
```
<empty>
```
**Response:**
```
<user-name>
```

### Register new user with suggested user name
```
GET https://<host>:<port>/user-register/?resource-name=<user-name>
```
**Body:**
```
<empty>
```
**Response:**
```
{
    "user": {
        "name": "<user-name>",
        "groups": [
            "<group-name-1>",
            "<group-name-2>"
        ]
    },
    "quotas": {
        "files": {
            "fileCount": "100",
            "fileSize": "104857600",
            "storeSize": "104857600"
        }
    },
    "tokens": [
        {
            "content": "<token>",
            "id": "<uid>",
            "resource": "<user-name>",
            "validity": "<timestamp>"
        }
    ]
}
```

---

## User authentication tokens management

### List authentication tokens for given user
```
GET https://<host>:<port>/list-user-tokens/
```
**Body:**
```
<empty>
```
**Response:**
```
"tokens": [
    {
        "content": "<token>",
        "id": "<uid>",
        "resource": "<user-name>",
        "validity": "<timestamp>"
    }
]
```

### Generate user authentication token
```
GET https://<host>:<port>/user-token-generate/?resource-id=<uid>&resource-name=<user-name>
```
**Body:**
```
<empty>
```
**Response:**
```
{
    "content": "<token>",
    "id": "<uid>",
    "resource": "<user-name>",
    "validity": "<timestamp>"
}
```

### Remove user authentication token
```
GET https://<host>:<port>/user-token-remove/?resource-id=<uid>&resource-name=<user-name>
```
**Body:**
```
<empty>
```
**Response:**
```
<uid>
```

---

## Group management

### List groups on the cloud server
```
GET https://<host>:<port>/list-groups/
```
**Body:**
```
<empty>
```
**Response:**
```
"groups": [
    {
        "name": "<group-name>"
    }
]
```

### Get group information
```
GET https://<host>:<port>/group-info/?resource-name=<group-name>
```
**Body:**
```
<empty>
```
**Response:**
```
{
    "name": "<group-name>"
}
```

### Add new group
```
GET https://<host>:<port>/group-add/?resource-name=<group-name>
```
**Body:**
```
<empty>
```
**Response:**
```
{
    "name": "<group-name>"
}
```

### Remove existing group
```
GET https://<host>:<port>/group-remove/?resource-name=<group-name>
```
**Body:**
```
<empty>
```
**Response:**
```
<group-name>
```

---

## Reporting

### Submit report
```
GET https://<host>:<port>/submit-report/
```
**Body:**
```
{
    "created": "1.1.1970"
    "report": "Report text"
    "comment": "Additional comments"
}
```
**Response:**
```
<report-status>
```

---

## Artificial intelligence

### Submit AI query

Forwards the query, via `range-ai-lib`, to the AI service configured on the cloud server
(`aiType`, `aiApiUrl`, `aiApiKey`, `aiModel`, `aiMaxTokens` in the server configuration).
The query is processed asynchronously: the request is acknowledged immediately with a
query `id` and `status` `pending`, and the generated answer is retrieved afterwards with
[Fetch AI query result](#fetch-ai-query-result).

```
POST https://<host>:<port>/ai-query/
```
**Body:**
The `application` field selects a server-side context and guardrails for the query.
Recognized values are defined in the server's `etc/aiqueries.json` file (each entry has a
`name`, optional `aliases`, and a `systemPrompt`; the file is created with default entries
`family-tree` and `finite-element-analysis` on first start). Any other value (or an empty
field) uses the `defaultSystemPrompt` from the same file. The `model` field is optional;
when empty the server default model is used.

The `query` object carries the question itself plus optional supporting fields:
`fileId` (UUID of a resource file stored in Range Cloud), `fileDescription`
(free-form client-supplied description of the referenced file, e.g. what it is and
why it is attached), `context` (free-form text supplied by the client to ground the
answer) and `language` (preferred response language). Only `question` is mandatory.

When `fileId` is set, the server reads the file through the file service under the
authenticated executor's access rights (read permission is required; an unauthorized
or unknown id yields an error response) and embeds an in-memory snapshot of its
content into the query context. The snapshot is taken at submission time — later
changes to the stored file do not affect a running query. The file is embedded into
the agent prompt inside a fenced block together with its metadata (name, the
optional `fileDescription`, version, tags, size, update time) and a configurable
preamble (`filePromptPreamble` in `etc/aiqueries.json`) instructing the model to
treat the content as data. Files larger than the server's `aiMaxFileContextSize`
configuration value (default 262144 bytes) and files that are not valid UTF-8 text
are rejected. An empty or absent `fileId` is valid and means no file is accessed.
```
{
    "application": "family-tree",
    "model": "claude-sonnet-4-6",
    "query": {
        "question": "Summarize the contents of my latest report.",
        "fileId": "123e4567-e89b-12d3-a456-426614174000",
        "fileDescription": "My latest report.",
        "context": "",
        "language": "en"
    }
}
```
**Response:**
```
{
    "id": "d7a749a0-0d3b-4e4c-8d29-c1abb036150f",
    "status": "pending",
    "request": {
        "application": "family-tree",
        "model": "claude-sonnet-4-6",
        "query": {
            "question": "Summarize the contents of my latest report.",
            "fileId": "123e4567-e89b-12d3-a456-426614174000",
            "fileDescription": "My latest report.",
            "context": "",
            "language": "en"
        }
    },
    "response": ""
}
```

### Fetch AI query result

Fetches the result of a previously submitted AI query by the `id` returned from
[Submit AI query](#submit-ai-query). While the query is still being processed the
response carries `status` `pending` and an empty `response`; clients should poll
until `status` becomes `completed`. A completed result is returned exactly once and
is then discarded on the server; unfetched results are discarded after one hour.
Only the user who submitted the query (or `root`) may fetch its result. An unknown,
already fetched, or expired id yields an error response.

```
GET https://<host>:<port>/ai-query-result/?resource-id=<query-id>
```
**Response:**
```
{
    "id": "d7a749a0-0d3b-4e4c-8d29-c1abb036150f",
    "status": "completed",
    "request": {
        "application": "family-tree",
        "model": "claude-sonnet-4-6",
        "query": {
            "question": "Summarize the contents of my latest report.",
            "fileId": "123e4567-e89b-12d3-a456-426614174000",
            "fileDescription": "My latest report.",
            "context": "",
            "language": "en"
        }
    },
    "response": "<AI generated answer>"
}
```
