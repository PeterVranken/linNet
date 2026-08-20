#
# Generic Makefile for GNU Make 3.82 (MinGW port only)
#
# Build of project.
#
# Help on the syntax of this makefile is got at
# http://www.gnu.org/software/make/manual/make.pdf.
#
# Copyright (C) 2012-2026 Peter Vranken (mailto:Peter_Vranken@Yahoo.de)
#
# This program is free software: you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License as published by the
# Free Software Foundation, either version 3 of the License, or any later
# version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License
# for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#
# Preconditions
# =============
#
# The makefile is intended to be executed by the GNU make utility 3.82. The MinGW is
# required, the Cygwin/msys port handles paths under Windows differently and will fail. The
# easiest way to ensure the right port is an executable named mingw32-make.exe. In case of
# doubts, run "make --version" to see.
#   This makefile does not handle blanks in any paths or file names. Please rename your
# paths and files accordingly prior to using this makefile.
#
# Targets and Options
# ===================
#
# The makefile provides several targets, which can be combined on the command line.
#   Some options can be set on either command line or down below in this main makefile. For
# example, the build confiuration, DEBUG or PRODUCTION, can be set.
#   Get some help on the available targets and options by invoking the makefile using:
#
#   make help
#
# Input Files
# ===========
#
# The makefile compiles and links all source files which are located in a given list of
# source directories. The list of directories is configured in this main makefile, please
# look for the setting of srcDirList below.
#   A second list of files is found as cFileListExcl. These C/C++ files are excluded from
# build.
#

# The name of the project.
project := linNet

# The name stem of the final build artifacts.
target := $(project)

# Specify a blank separated list of directories holding source files. The directories are
# searched recursively for C/C++ and assembler source files.
#   Caution, absolute paths are not supported. Use paths only, which relate to the path of
# this makefile.
srcDirList := src/

# Exclusion list: Edit the list of excluded files. A blank separated list of C or assembler
# source files is expected (all file designations with relative path), which are excluded
# from build.
cFileListExcl :=

# Additional include directories (besides the source directories and common, project
# independent paths).
incDirList :=

# A blank separated list of C defines for compilation, with or without value.
defineList :=

# Blanks separated list of sub-ordinated archives to link with the application.
libList :=

# Now go into the project independent implementation of the compile and link rules using
# the project specific settings made above.
sharedMakefilePath = makefile/
include $(sharedMakefilePath)buildProject.mk
