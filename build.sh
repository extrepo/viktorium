#!/bin/bash

# Проверяем зависимости
DEPLIST="cmake gcc g++ pkgconf"
for d in $DEPLIST ; do
  s=`dpkg -l $d | grep 'ii'`
  if [ -z "$s" ]; then
    echo "Error: package '$d' not installed!!"
    exit 1
  fi
done

# Параметры CMake
CMAKEDEF="-DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release -DAPP_BUILD_TESTS=NO"

# Экспорт всех APP_ переменных окружения в cmake
VARS=`declare -p "${!APP_@}" | sed -E "/.*(APP_[^=]*)=.*/s//\1/"`
for VAR in $VARS ; do
  CMAKEDEF="$CMAKEDEF -D$VAR='${!VAR}'"
done

# Сборка
echo "Building application"
rm -rf bin
rm -rf build
rm -rf artifacts
cmake -S . -B build $CMAKEDEF
[ $? -ne 0 ] && exit 1
cmake --build build -j$(nproc)
[ $? -ne 0 ] && exit 1
cpack -G DEB --config build/CPackConfig.cmake -B artifacts
[ $? -ne 0 ] && exit 1
rm -rf artifacts/_*

exit 0
