#!/bin/bash

# PLS_ENV_PREFIX is currently only used by devhelp
# If you use custom variables, don't use PLS_ENV_PREFIX

if [ "$(expr substr $(uname -s) 1 10)" == "MINGW32_NT" ]; then
  PLS_GCC=$PLS_ENV_PREFIX'./sdkdata/windows/PLocalSim/MinGW/bin/gcc.exe'
  PLS_AR=$PLS_ENV_PREFIX'./sdkdata/windows/PLocalSim/MinGW/bin/ar.exe'
  PLS_GDB=$PLS_ENV_PREFIX'./sdkdata/windows/PLocalSim/MinGW/bin/gdb.exe'
  PLS_DIR_SDL=$PLS_ENV_PREFIX'./sdkdata/windows/PLocalSim/SDL2'
else
  PLS_GCC='gcc'
  PLS_AR='ar'
  PLS_GDB='gdb'
fi
