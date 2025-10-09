# Range Cloud - REST API

---

## Service

### Test request
```
POST https://<host>:<port>/test-request
```
**Body:**
```
<Any text which will be part of the server response.>
```

### Service statistics
```
GET https://<host>:<port>/statistics
```
**Body:**
```
<empty>
```

### Stop service
```
GET https://<host>:<port>/stop
```
**Body:**
```
<empty>
```

---

## File store

### List files
```
GET https://<host>:<port>/list-files
```
**Body:**
```
<empty>
```

### Get file information
```
GET https://<host>:<port>/file-info?resource-id=<uid>
```
**Body:**
```
<empty>
```

### Upload file
```
PUT https://<host>:<port>/file-upload?resource-name=<resource-name>
```
**Body:**
```
<content of the file to be uploaded>
```

### Update remote file
```
POST https://<host>:<port>/file-update?resource-id=<uid>&resource-name=<resource-name>
```
**Body:**
```
<content of the file to be updated>
```

### Update file access owner
```
POST https://<host>:<port>/file-update-access-owner?resource-id=<uid>
```
**Body:**
```
{
    "user": "<owner-username>",
    "group": "<owner-group>"
}
```

### Update file access mode
```
POST https://<host>:<port>/file-update-access-mode?resource-id=<uid>
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

### Update file version
```
POST https://<host>:<port>/file-update-version?resource-id=<uid>
```
**Body:**
```
1.2.3
```

### Update file tags

```
POST https://<host>:<port>/file-update-tags?resource-id=<uid>
```
**Body:**
```
tag1,tag2,tag3
```

### Download file
```
GET https://<host>:<port>/file-download?resource-id=<uid>
```
**Body:**
```
<empty>
```

### Remove file
```
GET https://<host>:<port>/file-remove?resource-id=<uid>
```
**Body:**
```
<empty>
```

---

## Process


### Run process

```
POST https://<host>:<port>/process
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

---

## Process management

### List processes
```
GET https://<host>:<port>/list-processes
```
**Body:**
```
<empty>
```

### Update process access owner
```
POST https://<host>:<port>/process-update-access-owner?resource-name=<resource-name>
```
**Body:**
```
{
    "user": "<owner-username>",
    "group": "<owner-group>"
}
```

### Update process access mode
```
POST https://<host>:<port>/process-update-access-mode?resource-name=<resource-name>
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

---

## Action management

### List actions
```
GET https://<host>:<port>/list-actions
```
**Body:**
```
<empty>
```

### Update action access owner
```
POST https://<host>:<port>/action-update-access-owner?resource-name=<resource-name>
```
**Body:**
```
{
    "user": "<owner-username>",
    "group": "<owner-group>"
}
```

### Update action access mode
```
POST https://<host>:<port>/action-update-access-mode?resource-name=<resource-name>
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

---

## User management

### List users
```
GET https://<host>:<port>/list-users
```
**Body:**
```
<empty>
```

### Get user information
```
GET https://<host>:<port>/user-info?resource-name=<user-name>
```
**Body:**
```
<empty>
```

### Add user
```
GET https://<host>:<port>/user-add?resource-name=<user-name>
```
**Body:**
```
<empty>
```

### Update user
```
POST https://<host>:<port>/user-update?resource-name=<user-name>
```
**Body:**
```
{
    "name": "<user-name>",
    "groups": [
        "<group-name-1>",
        "<group-name-2>"
    ]
}

```

### Remove user
```
GET https://<host>:<port>/user-remove?resource-name=<user-name>
```
**Body:**
```
<empty>
```

### Register user
```
GET https://<host>:<port>/user-register?resource-name=<user-name>
```
**Body:**
```
{
    "user": {
        "name": "<user-name>",
        "groups": [
            "<group-name-1>",
            "<group-name-2>"
        ]
    },
    "tokens": [
        {
            "content": "<token>",
            "id": "<uuid>",
            "resource": "<user-name>",
            "validity": "<timestamp>"
        }
    ]
}
```

---

## Group management

### List groups
```
GET https://<host>:<port>/list-groups
```
**Body:**
```
<empty>
```

### Get group information
```
GET https://<host>:<port>/group-info?resource-name=<group-name>
```
**Body:**
```
<empty>
```

### Add group
```
GET https://<host>:<port>/group-add?resource-name=<group-name>
```
**Body:**
```
<empty>
```

### Remove group
```
GET https://<host>:<port>/group-remove?resource-name=<group-name>
```
**Body:**
```
<empty>
```

---

## Reporting

### Submit report
```
GET https://<host>:<port>/submit-report
```
**Body:**
```
{
    "created": "1.1.1970"
    "report": "Report text"
    "comment": "Additional comments"
}
```
