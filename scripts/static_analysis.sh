#!/usr/bin/env bash
set +e

PLATFORM="x86-64"

SONAR_VER="3.3.0.1492-linux"
BUILD_WRAPPER_VER="3.11"

BUILD_WRAPPER="scripts/sonar_scanner_unzip/build-wrapper-${BUILD_WRAPPER_VER}/linux-${PLATFORM}/"
SONAR_SCANNER="scripts/sonar_scanner_unzip/sonar-scanner-${SONAR_VER}/bin/"

PATH+=":`pwd`/${BUILD_WRAPPER}"
PATH+=":`pwd`/${SONAR_SCANNER}"
export PATH
echo ${PATH}

build-wrapper-linux-x86-64 --out-dir bw-output make clean all

pushd app/

sonar-scanner

