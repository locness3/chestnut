#!/usr/bin/env bash

PATH+=:/home/jon/Downloads/sonar-scanner-3.3.0.1492-linux/bin/:/home/jon/Downloads/build-wrapper-linux-x86/
export PATH

build-wrapper-linux-x86-64 --out-dir bw-output make clean all 
sonar-scanner 
