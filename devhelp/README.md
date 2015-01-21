# Developer Helper

## Overview
This is a small tool to make the development of the library easier. It essentially is a small stand-alone SDK that is ready-to-work-with when you could install a SDK (means all compilers and libraries are there). Also there are few optimizations (especially the library compilation caching) to speed up the work with this tool instead of rebuilding and reinstalling an entire SDK for one development iteration.
<sup>As this is not intended to be used externally, please don't try to use it other as intended (for example from another folder than devhelp/ ), doing otherwise could have undefined behaviour.</sup>

## Usage
This tool only works if a call to install.sh would also work (compiler and libraries are installed). If it does you navigate with a shell to the `devhelp` folder (for Windows you can just open `openShell.bat`). Then you can use:

* `build.sh` - for building just the example project located in the `devhelp` folder
* `build.all.sh` - for building the library, the resource compiler (c version) and the example projec
* `build.all.sh --rebuild` - to delete the cache and make sure that everything will be compiled from the ground up
* `run.sh` - for starting the built project 
* `run.sh --debug` - for starting the project within gdb
