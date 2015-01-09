#!/bin/bash

if [ "$(expr substr $(uname -s) 1 10)" == "MINGW32_NT" ]; then
  PLS_GCC='./sdkdata/windows/PLocalSim/MinGW/bin/gcc.exe'
  PLS_AR='./sdkdata/windows/PLocalSim/MinGW/bin/ar.exe'
  PLS_DIR_SDL='./sdkdata/windows/PLocalSim/SDL'
else
  PLS_GCC='gcc'
  PLS_AR='ar'
fi
