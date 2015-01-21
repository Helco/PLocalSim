#!/bin/bash

# A small script to call build.all.sh in the right working directory
PWD_BACKUP=`pwd`

cd .. # up to ide
cd .. # up to devhelp

./build.all.sh $@

cd $PWD_BACKUP
