#!/usr/bin/env bash
set -e

SONAR_SCANNER_URL="https://binaries.sonarsource.com/Distribution/sonar-scanner-cli/sonar-scanner-cli-4.2.0.1873-linux.zip"
SONAR_SCANNER_FILE="sonarscanner.zip"
SONAR_SCANNER_DIR="sonar_scanner_unzip"
BUILDWRAPPER_FILE="../third_party/SonarQube/build-wrapper-3.11.zip"

echo "Downloading sonar scanner"
wget -q ${SONAR_SCANNER_URL} -O ${SONAR_SCANNER_FILE}
echo "Unzipping sonar scanner"
unzip -o ${SONAR_SCANNER_FILE} -d ${SONAR_SCANNER_DIR}
echo "Unzipping Build Wrapper"
unzip -o ${BUILDWRAPPER_FILE} -d ${SONAR_SCANNER_DIR}

echo "Fixing BuildRoot for Haswell Platform"
pushd ${SONAR_SCANNER_DIR}/build-wrapper-3.11/linux-x86-64/
ln -sv libinterceptor-x86_64.so libinterceptor-haswell.so