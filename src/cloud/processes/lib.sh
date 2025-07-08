#!/bin/bash

if [ -z "$CLOUD_PROCESS_LOG_FILE" ]
then
    exit 1
fi

pinfo()
{
    echo "$@" >> "$CLOUD_PROCESS_LOG_FILE"
}

pwarning()
{
    echo "WARNING: $@" >> "$CLOUD_PROCESS_LOG_FILE"
}

perror()
{
    echo "ERROR: $@" >> "$CLOUD_PROCESS_LOG_FILE"
}

assert_success()
{
    if [ $1 -ne 0 ]
    then
        perror "$2"
        if [ "$3" = "true" ]
        then
            echo "$2"
        fi
        exit 1
    fi
}

assert_nonempty()
{
    if [ -z "$1" ]
    then
        perror "$2"
        exit 1
    fi
}

extract_cmd_parameter_value()
{
    echo "${1#*=}"
}

in_csv_list()
{
    local _text=$1
    local _list=$2

    local _retVal=1
    local _IFS_OLD=$IFS
    IFS=','
    local _item=
    for _item in $_list
    do
        pinfo "$_item $_text"
        if [ "$_item" = "$_text" ]
        then
            _retVal=0
            break
        fi
    done
    IFS=$_IFS_OLD
    return $_retVal
}

authorize_user()
{
    # Test if executor is ownner or if executor's group is also owner's group
    local _ownerUser=$(echo "$CLOUD_PROCESS_OWNER" | cut -d':' -f1)
    local _ownerGroups=$(echo "$CLOUD_PROCESS_OWNER" | cut -d':' -f2)
    local _executorUser=$(echo "$CLOUD_PROCESS_EXECUTOR" | cut -d':' -f1)
    local _executorGroups=$(echo "$CLOUD_PROCESS_EXECUTOR" | cut -d':' -f2)

    if [ "$_executorUser" = "$_ownerUse" ]
    then
        return 0
    fi

    local _retVal=1
    local _IFS_OLD=$IFS
    IFS=','
    local _item=
    for _item in $_executorGroups
    do
        if in_csv_list "$_item" "$_ownerGroups"
        then
            _retVal=0
            break
        fi
    done
    IFS=$_IFS_OLD
    return $_retVal
}

print_header()
{
    echo >> "$CLOUD_PROCESS_LOG_FILE"
    echo "################################################################################" >> "$CLOUD_PROCESS_LOG_FILE"
    echo "--------------------------------------------------------------------------------" >> "$CLOUD_PROCESS_LOG_FILE"
    echo "$(date +%d-%b-%Y) $(date +%T)" >> "$CLOUD_PROCESS_LOG_FILE"
    echo "$@" >> "$CLOUD_PROCESS_LOG_FILE"
    echo >> "$CLOUD_PROCESS_LOG_FILE"
}

print_header "$(basename $0)"

assert_nonempty "$CLOUD_PROCESS_WORK_DIR" '$CLOUD_PROCESS_WORK_DIR is not defined'
assert_nonempty "$CLOUD_PROCESS_RANGE_CA_DIR" '$CLOUD_PROCESS_RANGE_CA_DIR is not defined'
assert_nonempty "$CLOUD_PROCESS_EXECUTOR" '$CLOUD_PROCESS_EXECUTOR is not defined'
assert_nonempty "$CLOUD_PROCESS_OWNER" '$CLOUD_PROCESS_OWNER is not defined'
