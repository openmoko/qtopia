#!/bin/sh

getvar()
{
    gv_varname="$1"
    shift
    while [ $# -gt 0 ]; do
        if [ "${1##$gv_varname=}" != "$1" ]; then
            echo "${1##$gv_varname=}"
        fi
        shift
    done
}

DEVICE_CONFIG_PATH=$(getvar DEVICE_CONFIG_PATH "$@")

[ -n "$DEVICE_CONFIG_PATH" ] && [ -f "$DEVICE_CONFIG_PATH/environment" ] && . "$DEVICE_CONFIG_PATH/environment"

exec env "$@"

