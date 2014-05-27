#!/usr/bin/python2
import sys
import os

wd = os.path.dirname(os.path.realpath(__file__))

# this should be a symlink to git-mbed that contains at least
# build/mbed and workspace_tools
mbeddir = wd + "/mbed-lib"
sys.path.append(mbeddir)

import workspace_tools.targets
import workspace_tools.build_api

projdir = sys.argv[1]

libdir = mbeddir + "/build/mbed"
srcdir = wd + "/src/" + projdir
#builddir = wd + "/builds/" + projdir
builddir = wd + "/builds"
target = workspace_tools.targets.TARGET_MAP["NUCLEO_F401RE"]
toolname = "GCC_ARM"
v = False

workspace_tools.build_api.build_project(srcdir, builddir, target,
        toolname, [libdir], clean=True, verbose=v)
