#!/bin/sh
convert $1 `basename $1 jpg`png
git-rm $1
