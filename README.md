# screensaver.asterwave addon for Kodi

This is a [Kodi](https://kodi.tv) screensaver addon.

[![License: GPL-2.0-or-later](https://img.shields.io/badge/License-GPL%20v2+-blue.svg)](LICENSE.md)
[![Build Status](https://travis-ci.org/xbmc/screensaver.asterwave.svg?branch=Matrix)](https://travis-ci.org/xbmc/screensaver.asterwave/branches)
[![Build Status](https://dev.azure.com/teamkodi/binary-addons/_apis/build/status/xbmc.screensaver.asterwave?branchName=Matrix)](https://dev.azure.com/teamkodi/binary-addons/_build/latest?definitionId=41&branchName=Matrix)
[![Build Status](https://jenkins.kodi.tv/view/Addons/job/xbmc/job/screensaver.asterwave/job/Matrix/badge/icon)](https://jenkins.kodi.tv/blue/organizations/jenkins/xbmc%2Fscreensaver.asterwave/branches/)
<!--- [![Build Status](https://ci.appveyor.com/api/projects/status/github/xbmc/screensaver.asterwave?branch=Matrix&svg=true)](https://ci.appveyor.com/project/xbmc/screensaver-asterwave?branch=Matrix) -->

## Build instructions

When building the addon you have to use the correct branch depending on which version of Kodi you're building against.
For example, if you're building the `master` branch of Kodi you should checkout the `master` branch of this repository.
Also make sure you follow this README from the branch in question.

### Linux

The following instructions assume you will have built Kodi already in the `kodi-build` directory
suggested by the README.

1. `git clone --branch master https://github.com/xbmc/xbmc.git`
2. `git clone https://github.com/xbmc/screensaver.asterwave.git`
3. `cd screensaver.asterwave && mkdir build && cd build`
4. `cmake -DADDONS_TO_BUILD=screensaver.asterwave -DADDON_SRC_PREFIX=../.. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=../../xbmc/kodi-build/addons -DPACKAGE_ZIP=1 ../../xbmc/cmake/addons`
5. `make`

The addon files will be placed in `../../xbmc/kodi-build/addons` so if you build Kodi from source and run it directly
the addon will be available as a system addon.
