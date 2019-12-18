# Installation

## Ubuntu 19.10

Versions earlier than 19.10 are not supported

### Requirements
First install the following packages:

```sudo apt install build-essential pkg-config qtchooser wget unzip desktop-file-utils git cmake qt5-default \```
```libqt5svg5-dev qtmultimedia5-dev libavutil-dev libavformat-dev libavcodec-dev libavfilter-dev libavutil-dev \```
```libswscale-dev libfmt-dev ffmpeg```

Add the audiowaveform ppa and install:

```sudo add-apt-repository ppa:chris-needham/ppa```

```sudo apt-get update```

```sudo apt-get install audiowaveform```

### Build

Run:

```qmake```

Then:

```make -j{JOBS}``` 

Replace {JOBS} with the amount of logical cores on your build system + 1

### Install

By default, Chestnut will install to /usr/local

```sudo make install```

### Uninstall

```sudo make uninstall```

## Manjaro/Arch Linux

There is a package of the latest release available via AUR

```pacaur -S chestnut```

