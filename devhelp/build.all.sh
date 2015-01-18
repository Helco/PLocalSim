#!/bin/bash

# As this environment is basically a small standalone sdk, we have to use the c resource compiler

# Disable unicode (should speed up everything)
LC_ALL_BACKUP=$LC_ALL
export LC_ALL=C

# Prepare cache
if [ "$1" == "--rebuild" ]; then
  echo "[INFO] Remove library cache"
  rm -f ./build/tempLocal/libCache/*
else
  mkdir -p ./build/tempLocal/libCache
fi

# Building
cd ..
echo "[INFO] Building libray"
if ./scripts/build.library.sh --debug ./devhelp/build/tempLocal/libCache ; then
  echo "[INFO] Building resource compiler"
  if ./scripts/build.resCompiler.sh -c --debug ; then
    cd devhelp
    echo "[INFO] Building project"
    if ./build.sh ; then # no --debug needed
      echo "[INFO] Everything built"
    else
      echo "[FAIL] Project compilation failed"
    fi
  else
    echo "[FAIL] Resource compiler compilation failed"
  fi
else
  echo "[FAIL] Library compilation failed"
fi

# Reenable unicode
export LC_ALL=$LC_ALL_BACKUP
