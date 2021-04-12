#!/usr/bin/env bash

target="val@val00"
## this script will sync the project to the remote server
rsync -i -rtuv $PWD/. $target:~/rococo
 