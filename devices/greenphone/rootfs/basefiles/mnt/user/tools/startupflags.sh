#!/bin/sh

GLOBAL_CONF="/mnt/user/etc/StartupFlags.conf"
LOCAL_CONF="$HOME/Settings/Trolltech/StartupFlags.conf"

parseConf()
{
    awk 'BEGIN { group = "" } \
/^\[.*\]$/ { group = toupper(gensub("[\][]", "", "g")) } \
/^State=/ && group != "" { printf "%s_FLAG=%s\n", group, gensub("^State=", "", "") }' $1
}

[ -r $GLOBAL_CONF ] && parseConf $GLOBAL_CONF
[ -r $LOCAL_CONF ] && parseConf $LOCAL_CONF

