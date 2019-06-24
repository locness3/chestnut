#!/usr/bin/env python3
## 
 # Chestnut. Chestnut is a free non-linear video editor for Linux.
 # Copyright (C) 2019
 #
 # This program is free software: you can redistribute it and/or modify
 # it under the terms of the GNU General Public License as published by
 # the Free Software Foundation, either version 3 of the License, or
 # (at your option) any later version.
 #
 # This program is distributed in the hope that it will be useful,
 # but WITHOUT ANY WARRANTY; without even the implied warranty of
 # MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 # GNU General Public License for more details.
 #
 # You should have received a copy of the GNU General Public License
 # along with this program. If not, see <http://www.gnu.org/licenses/>.
##

"""
    Locate the libraries of the built exec and copy to 
    somewhere local for appimage to use.
"""
from subprocess import Popen, PIPE
import pathlib
from shutil import copy

PROG_PATH="../../../app/main/build/release/chestnut"
LIB_DIR="lib/"

p = Popen(['ldd', PROG_PATH], stdin=PIPE, stdout=PIPE, stderr=PIPE)
output, err = p.communicate()
rc = p.returncode

pathlib.Path(LIB_DIR).mkdir(parents=True, exist_ok=True)

for l in output.split():
    entry = l.decode("utf-8")
    if entry.startswith("/lib") or entry.startswith("/usr/lib/"):
        print(entry)
        copy(entry, LIB_DIR)