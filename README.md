# Generic Image Processing (GIP) Framework

C++ framework for developing image processing applications

Copyright (C) 2001-2002, 2019-2020 by René Møller Fonseca <rene.m.fonseca+gip@gmail.com>

Primary hosting at https://dev.azure.com/renefonseca/gip

Reference API documentation is hosted at https://rene-fonseca.github.io/gip-api


## Status

[![Windows x64](https://dev.azure.com/renefonseca/gip/_apis/build/status/rene-fonseca.gip?branchName=master&jobName=windows_x64&label=Windows%20x64)](https://dev.azure.com/renefonseca/gip/_build/latest?definitionId=6&branchName=master)
[![Windows x86](https://dev.azure.com/renefonseca/gip/_apis/build/status/rene-fonseca.gip?branchName=master&jobName=windows_x86&label=Windows%20x86)](https://dev.azure.com/renefonseca/gip/_build/latest?definitionId=6&branchName=master)
[![Windows ARM64](https://dev.azure.com/renefonseca/gip/_apis/build/status/rene-fonseca.gip?branchName=master&jobName=windows_arm64&label=Windows%20ARM64)](https://dev.azure.com/renefonseca/gip/_build/latest?definitionId=6&branchName=master)
[![Windows ARM32](https://dev.azure.com/renefonseca/gip/_apis/build/status/rene-fonseca.gip?branchName=master&jobName=windows_arm32&label=Windows%20ARM32)](https://dev.azure.com/renefonseca/gip/_build/latest?definitionId=6&branchName=master)
[![Ubuntu 18.04](https://dev.azure.com/renefonseca/gip/_apis/build/status/rene-fonseca.gip?branchName=master&jobName=ubuntu18_04&label=Ubuntu%2018.04)](https://dev.azure.com/renefonseca/gip/_build/latest?definitionId=6&branchName=master)
[![Ubuntu 16.04](https://dev.azure.com/renefonseca/gip/_apis/build/status/rene-fonseca.gip?branchName=master&jobName=ubuntu16_04&label=Ubuntu%2016.04)](https://dev.azure.com/renefonseca/gip/_build/latest?definitionId=6&branchName=master)
[![macOS](https://dev.azure.com/renefonseca/gip/_apis/build/status/rene-fonseca.gip?branchName=master&jobName=macos&label=macOS)](https://dev.azure.com/renefonseca/gip/_build/latest?definitionId=6&branchName=master)


## Dependencies

Base Framework at https://dev.azure.com/renefonseca/base

PNG and JPEG are optional packages. For Ubuntu install:

```shell
sudo apt-get install libpng-dev
sudo apt-get install libjpeg-dev
```

## Build

Make sure GIT, cmake, make, and compiler (e.g. g++) are installed.

### Debug build

Use -DCMAKE_PREFIX_PATH to set location of base install folder.

```shell
git clone https://dev.azure.com/renefonseca/gip/_git/gip
cd gip
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH=base -DCMAKE_INSTALL_PREFIX=install
cmake --build .. --config Debug --target install -- -j 4
ctest . -C Debug
```

### Release build

Use -DCMAKE_PREFIX_PATH to set location of base install folder.

```shell
git clone https://dev.azure.com/renefonseca/gip/_git/gip
cd gip
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=base -DCMAKE_INSTALL_PREFIX=install
cmake --build .. --config Release --target install -- -j 4
ctest . -C Release
```
