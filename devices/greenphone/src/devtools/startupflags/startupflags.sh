#!/bin/sh

GLOBAL_CONF="/mnt/user/etc/StartupFlags.conf"
LOCAL_CONF="$HOME/Settings/Trolltech/StartupFlags.conf"

parseConf()
{
    awk 'BEGIN { group = "" } \
/^\[.*\]$/ { group = toupper(gensub("[\][]", "", "g")) "_FLAG" } \
/^State=/ && group != "" { printf "%s=%s\n", group, gensub("^State=", "", "") }' $1
}

modifyConf()
{
    awk 'BEGIN { group = "" } \
/^\[.*\]$/ { group = toupper(gensub("[\][]", "", "g")) "_FLAG" } \
/^State=/ && group == "'$2'" { printf "State=%d\n", '$3' ; continue } \
{ print $0 }' $1
}

if [ $# -eq 0 ]; then
    # Read all startup flags
    
    [ -r $GLOBAL_CONF ] && parseConf $GLOBAL_CONF
    [ -r $LOCAL_CONF ] && parseConf $LOCAL_CONF
elif [ $# -eq 2 ]; then
    # Modify the value of a startup flag
    
    TEMPFILE=`mktemp /tmp/startupflags.XXXXXX`
    
    if [ -r $GLOBAL_CONF ]; then
        if parseConf $GLOBAL_CONF | grep $1 >/dev/null; then
            modifyConf $GLOBAL_CONF $1 $2 > $TEMPFILE
            cp $TEMPFILE $GLOBAL_CONF
            rm -f $TEMPFILE
        fi
    fi        
    
    if [ -r $LOCAL_CONF ]; then
        if parseConf $LOCAL_CONF | grep $1; then
            modifyConf $LOCAL_CONF $1 $2 > $TEMPFILE
            cp $TEMPFILE $LOCAL_CONF
            rm -f $TEMPFILE
        fi
    fi
else
    echo "Usage: $0 [Flag] [Value]"
fi

