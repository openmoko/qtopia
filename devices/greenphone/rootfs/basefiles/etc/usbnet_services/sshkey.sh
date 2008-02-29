#!/bin/sh

SSHKEY_PORT=10022

case $1 in
start)
    HOME=$( awk -F : '/^root/ { print $6 }' < /etc/passwd )

    if [ ! -f $HOME/.ssh/authorized_keys ]; then
        echo "Waiting for ssh key on $SSHKEY_PORT"

        mkdir -p $HOME/.ssh
        nc -l -p $SSHKEY_PORT > $HOME/.ssh/authorized_keys &
    fi
    ;;
stop)
    kill $( ps ax | grep '[n]c -l -p $SSHKEY_PORT' | awk '{print $1}' ) 2>/dev/null
    ;;
esac

exit 0

