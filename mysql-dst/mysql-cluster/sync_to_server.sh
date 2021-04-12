#!/usr/bin/env bash

target="wxd@cube1"
## this script will sync the project to the remote server
rsync -i -rtuv $PWD $target:/raid/wxd/mysql
 