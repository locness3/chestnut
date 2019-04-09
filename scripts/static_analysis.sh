#!/usr/bin/env bash
set +x

build-wrapper-linux-x86-64 --out-dir bw-output make clean all

pushd app/

sonar-scanner

