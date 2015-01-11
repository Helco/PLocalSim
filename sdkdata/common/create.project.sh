#!/bin/bash

PLS_SDK_ROOT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PLS_SDK_DIR=$PLS_SDK_ROOT_DIR'/PLocalSim'

if [ ! -e $PLS_SDK_DIR'/envvars.sh' ] ; then
  echo "[FAIL] Could not load envvars.sh"
  exit 1
fi
source $PLS_SDK_DIR'/envvars.sh'

# Create/Test directory
PLS_TARGET_PATH=$1
if [ -z "$PLS_TARGET_PATH" ] ; then
  echo "[FAIL] correct usage: create.project.sh <path_to_new_project>"
  exit 1
fi

mkdir -p $PLS_TARGET_PATH
if [ ! -d "$PLS_TARGET_PATH" ] ; then
  echo "[FAIL] Invalid directory: $PLS_TARGET_PATH"
  exit 1
fi

# Copy project template
cp -f -r $PLS_SDK_DIR/projectTemplate/* $PLS_TARGET_PATH'/'

if [ "$(expr substr $(uname -s) 1 10)" == "MINGW32_NT" ]; then
  # Get the sdk path in windows style
  PLS_DIR_BACKUP=`pwd`
  cd $PLS_SDK_ROOT_DIR
  PLS_SDK_DIR_WIN=`pwd -W`
  cd $PLS_DIR_BACKUP

  # Generate openShell.bat
  PLS_OPEN_SHELL_BAT='@"'$PLS_SDK_DIR_WIN'/openShell.bat'
  echo $PLS_OPEN_SHELL_BAT > $PLS_TARGET_PATH'/openShell.bat'
fi

# Generate build.local.bat
echo -e "#!/bin/bash\r\n"                    > $PLS_TARGET_PATH'/build.local.sh'
echo '"'$PLS_SDK_ROOT_DIR'/build.local.sh" $@' >> $PLS_TARGET_PATH'/build.local.sh'

# Generate run.local.bat
echo -e "#!/bin/bash\r\n"                    > $PLS_TARGET_PATH'/run.local.sh'
echo '"'$PLS_SDK_ROOT_DIR'/run.local.sh" $@'   >> $PLS_TARGET_PATH'/run.local.sh'