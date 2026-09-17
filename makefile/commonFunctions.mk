#
# Generic Makefile for GNU Make 3.82 (MinGW port only)
#
# Some common purpose makefile functions, which are accessible from all makefiles.
#
# Help on the syntax of this makefile is got at
# http://www.gnu.org/software/make/manual/make.pdf.
#
# Copyright (C) 2012-2022 Peter Vranken (mailto:Peter_Vranken@Yahoo.de)
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
ifndef COMMON_FUNCTIONS_INCLUDED
COMMON_FUNCTIONS_INCLUDED := 1

# Return a end of line character. No parameters, can be called without $(call).
define EOL


endef
# Example:
#$(info first line$(EOL)second line)


# Return a single blank character. No parameters, can be called without $(call).
blank :=
blank := $(blank) $(blank)
# Example:
#$(info Here, we have a blank:>$(blank)<)


# Return a comma. No parameters, can be called without $(call).
comma := ,
# Example:
#$(info Here, we have a comma: $(comma))

# Return the platform path separator, ; or :. No parameters, can be called without $(call).
ifeq ($(OS),Windows_NT)
    pathSep := ;
else
    pathSep := :
endif
# Example:
#$(info This $(OS) machine uses this path separator character: $(pathSep))


# Function isDefined
#   The main makefile offers a variable "defineList", with user configurable settings for
# the build. This function supports the use of the list in conditional parts of the
# makfile, i.e., using ifeq/ifneq/ifdef/ifndef.
#   The call of this function returns the word true if $(1) is element of $(defineList) and
# nothing otherwise. Note, "true" is a symbol but not a Boolean constant. The use of this
# particular result value has no technical significance but supports having readable
# conditional makefile code. See example. Returning either a word or nothing enables the
# use of the function as condition in an expression like $(if cond,..,..), too, where
# "nothing" has the meaning of a Boolean false. See example.
#   $(1): The name of a define to be checked for presence in $(defineList).
isDefined = $(if $(filter $(1),$(defineList)),true,)
# Example:
#ifeq ($(call isDefined,DEFINE_OF_INTEREST),true)
#$(info DEFINE_OF_INTEREST is element of $(defineList))
#else
#$(info DEFINE_OF_INTEREST isn't element of $(defineList))
#endif
#$(info DEFINE_OF_INTEREST: $(if $(call isDefined,DEFINE_OF_INTEREST),is defined,is undefined))


# Helper functions: Path separator conversion in either direction. Principally, we use the
# native path convention of the make tool (UNIX style, forward shlash). When calling
# external tools it might be required to convert this style to Windows' backslash notation.
#   Only parameter is the path to convert.
u2w = $(subst /,\,$(1))
w2u = $(subst \,/,$(1))
#   Example:
# $(info mkdir $(call u2w,c:/00_HybridMaster/Components))


# Function noTrailingSlash
#   Remove a possibly existing terminating slash from a list of path or file designations.
# Designations not ending with a slash are returned unchanged.
#   $(1): Blank separated paths list ending or not ending on shlashes.
noTrailingSlash = $(patsubst %/,%,$(1))
# Example:
#$(info Cleaned path designations: $(call noTrailingSlash,/ c:/ c:/tmp/ c:/tmp/file c:/tmp/file.c c:/tmp.x/file.h c: c:/tmp))


# Function trailingSlash
#   Ensure a possibly not yet present terminating slash for a list of path designations.
# Designations already ending with a slash are returned unchanged.
#   $(1): Blank separated paths list ending or not ending on shlashes.
trailingSlash = $(addsuffix /,$(call noTrailingSlash,$(1)))
# Example:
#$(info Proper path designations: $(call trailingSlash,/ c:/ c: c:/tmp/ c:/tmp c:/tmp/file c:/tmp/file.c c:/tmp.x/file.h c: c:/tmp))


# Function pathSearch
#   Look for a single file pattern in a search path, i.e., a list of directories.
#   $(1): Blank separated path list to search through
#   $(2): File name without path to be searched in the path. Can be file pattern like *.c.
#   The call of this function returns the first match of the file pattern $(2) in one of
# the directories in the path list $(1).
pathSearch = $(firstword $(wildcard $(addsuffix $(2),$(call trailingSlash,$(1)))))
# Examples:
#$(info make is $(call pathSearch,$(subst $(pathSep),$(blank),$(PATH)),*make*))
#$(info make is $(call pathSearch,$(subst $(pathSep),$(blank),$(PATH)),make.exe))
#$(info make is $(call pathSearch,$(subst $(pathSep),$(blank),$(PATH)),mingw*make*))


# Function systemPathSearch
#   Look for a single file pattern in the system search path $(PATH).
#   $(1): File name without path to be searched in the path. Can be file pattern like *.c.
#   The call of this function returns the first match of the file pattern $(2) in one of
# the directories in the system search path $(PATH).
#   Caution: The mechanism failes for system path entries, which contain blanks.
systemPathSearch = $(call pathSearch,$(call w2u,$(subst $(pathSep),$(blank),$(PATH))),$(1))
# Examples:
#$(info make is $(call systemPathSearch,*make*))
#$(info echo is $(call systemPathSearch,echo.exe))
#$(info make is $(call systemPathSearch,make.exe))
#$(info make is $(call systemPathSearch,mingw*make*))


# Helper for wildcard searches and UNIX shell calls if the searched paths potentially
# contain blanks. All blanks are unconditionally escaped by backslash, which is a format
# accepted by $(wildcard).
#   $(1): A single path, which may contain one or more blanks.
#   Note, this function must not be called for a list of paths. Such a list would use
# additional blanks as element separator and, consequently, this function would join all paths
# from a list.
#   The function returns the escaped path designation with UNIX path separator characters.
escapeBlanks = $(subst $(blank),\$(blank),$(call w2u,$(1)))
# Example:
#$(info This is an escaped path: $(call escapeBlanks,c:/Path with blanks/path/again blanks/file.mk))

# Helper for wildcard searches if the searched paths potentially contain blanks. All
# blanks, which had been escaped using function escapeBlanks are restored.
#   $(1): A single path, as got from escapeBlanks.
restoreBlanks = $(subst \$(blank),$(blank),$(1))

# Helper for processing paths, which potentially contain blanks: Does a path designation
# contain at least one blank?
#   $(1): The file or path designation to check.
#   The function returns the word true if at least one blanks is found or nothing
# otherwise. It can thus be used in a condition like
# $(if $(call hasBlanks,$(file)),needsQuoting)
hasBlanks = $(if $(word 2,$(1)),true,)

# Helper for starting external tools if they are installed in a path, which potentially
# contains blanks. The file-path designation of the executable is quoted.
#   $(1): The designation of a single file. It may contain blanks.
#   The function returns the quoted designation. In Windows it is embedded in double
# quotes, in UNIX the backslash is used to escape the blanks.
quoteExecutable = $(if $(call hasBlanks,$(1)),$(if $(call eq,$(OS),Windows_NT),"$(call u2w,$(1))",$(call escapeBlanks,$(call w2u,$(1)))),$(if $(call eq,$(OS),Windows_NT),$(call u2w,$(1)),$(call w2u,$(1))))

# Recursive search for files. Use cases are similar to those of built-in function wildcard.
# This basis of this code has been gratefully taken from
# http://blog.jgc.org/2011/07/gnu-make-recursive-wildcard-function.html (1.4.2017). A small
# change supports the search of a list of directories all at once.
#   $(1): A blank separated list of directories, ending with slash. The empty field means the
# current working directory and yields the same file as ./ All of these directories are
# searched for all files matching against any of the wildcards given in $(2).
#   $(2): A blank separated list of wildcards in the form *.<extension>
rwildcard = $(strip $(foreach d,$(wildcard $(addsuffix *,$(1))),$(call rwildcard,$(d)/,$(2)) $(filter $(subst *,%,$(2)),$(d))))
# Usage is as follows. First, all C files in the current directory (or below).
#$(info $(call rwildcard,,*.c))
#$(info $(call rwildcard,./,*.c))
# Then all C files in c:/tmp:
#$(info $(call rwildcard,c:/tmp/,*.c))
# Multiple patterns can be used. Here are all C and H files:
# Multiple directories and patterns can be used. Here are all C and H files in two directories:
#$(info $(call rwildcard,test/helloWorld/ code/,*.c *.h))


# Test for equality: A wrapper around the required ugly string comparison operations.
#   The macro returns either the empty string (condition is false) or the string "true".
# The macro can thus be used as condition c of $(if c, ... , ...)
#   $(1) and $(2) are the two compared strings.
eq = $(if $(1)$(2),$(and $(findstring $(1),$(2)),$(findstring $(2),$(1)),true),true)
#$(info 2==3: $(call eq,2,3), 2==2: $(call eq,2,2), ""=="": $(call eq,,), 1=="": $(call eq,1,),""==1: $(call eq,,1))


# Test if a file is in a list of such. Can be used in conditional expressions to handle
# such a file specifically.
#   The file is addressed by $(notdir $(1)), i.e., it is addressed to by only its raw file
# name with extension.
#   $(1): The file (can also be a list of such). The macro expands to non-empty if the file
# (or one or more of the files in the list) matches one of the elements of the list $(2).
# Otherwise it expands to nothing. The macro can thus be used as condition c of
# $(if c, ... , ...).
#   $(2): The list of file names to test. Only provide raw file names with extension.
isFileInList = $(if $(filter $(2), $(notdir $(1))),true,)
#$(info "$(call isFileInList, a/x/test.c path/c, a b xtest.c)")

# Test if an option is enabled. A makefile variable, which can have the values 0 or is
# considered an option.
#   The macro returns either the empty string (option variable has value 0) or the string
# "true" (option variable has value 1). The macro can thus be used as condition c of $(if
# c, ... , ...)
#   $(1) is the name of the tested option variable.
isEnabled = $(if $(filter-out 0 1,$(strip $($1))),$(error Boolean switch $1 has unsupported value "$($1)". Use 0 and 1 only),$(if $(filter 1,$(strip $($1))),true,))

# Create a fragment of the C compiler command line with #defines for a number of options. The
# returned text contains the pattern -D<name>=<0or1> for each option in the list $(1) of
# options.
getDefines = $(foreach def,$(1),-D$(def)=$(if $(call isEnabled,$(def)),1,0))

# Test if a particular option or at least one out of a particular set of those has been
# given on the make command line. The macro returns the word true is one or more matching
# options have been identified and nothing if non of the tested options are on the command
# line. The macro can thus be used as condition c of $(if c, ... , ...).
#   Caution, the function is based on inspecting the internal make variable $(MAKEFLAGS).
# This variable contains the options with argument value and often this will be a not
# explicitly given default value. The test will succeed only if the query asks for option
# plus argument.
#   $(1): The option(s) to test without the leading dash
isOption = $(if $(filter $(1),$(MAKEFLAGS)),true,)
#$(info -s: $(call isOption,s), -xxx: $(call isOption,xxx), -s or -O: $(if $(call isOption,s O), At least one is given,None of these is given))

endif # COMMON_FUNCTIONS_INCLUDED