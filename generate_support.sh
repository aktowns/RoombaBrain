#!/usr/bin/env bash

INCLUDE_DIRS=$(find $IDF_PATH -iname "include" -type d | sed -e "s@$IDF_PATH@$IDF_PATH@i")

echo "cmake_minimum_required(VERSION 3.10)"

echo "SET(CMAKE_SYSTEM_NAME "xtensa-esp32-elf-")"
echo "SET(CMAKE_C_COMPILER \"$IDF_TOOLCHAIN/bin/xtensa-esp32-elf-gcc\")"
echo "SET(CMAKE_CXX_COMPILER \"$IDF_TOOLCHAIN/bin/xtensa-esp32-elf-g++\")"
echo "SET(CMAKE_CXX_FLAGS_DISTRIBUTION \"-fno-rtti -fno-exceptions -std=gnu++11 -Wall -Werror=all -Wno-error=deprecated-declarations -Wextra\")"
echo "SET(CMAKE_C_FLAGS_DISTRIBUTION \"-Wno-old-style-declaration -std=gnu99 -Wall -Werror=all -Wno-error=deprecated-declarations -Wextra -W\")"
echo "SET(CMAKE_CXX_STANDARD 11)"
echo "SET(CMAKE_C_STANDRD 11)"

declare -a DEFNS
DEFNS=("WITH_POSIX" "ESP32" "ESP_PLATFORM" "HAVE_CONFIG_H") 

for d in ${DEFNS[@]}; do 
  echo "add_definitions(-D'$d')"
done

echo "include_directories(\"\${CMAKE_CURRENT_SOURCE_DIR}/build/include\")"

echo "include_directories(\"$IDF_TOOLCHAIN/xtensa-esp32-elf/include\")"

for d in ${INCLUDE_DIRS[@]}; do 
  echo "include_directories(\"$d\")"
done

echo "include_directories(\"$IDF_PATH/components/json/cJSON\")"
echo "include_directories(\"$IDF_PATH/components/coap/libcoap/include/coap\")"
echo "include_directories(\"$IDF_PATH/components/coap/port/include/coap/\")" 
echo "include_directories(\"$IDF_PATH/components/lwip/include/lwip\")"
echo "include_directories(\"$IDF_PATH/components/lwip/include/lwip/port\")"
echo "include_directories(\"$IDF_PATH/components/lwip/include/lwip/posix\")"
echo "include_directories(\"$(pwd)/components/nanopb/\")"
