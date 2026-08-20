#
# Generic Makefile for GNU Make 3.82 (MinGW port only)
#
# Inspect environment variables and system search path to find out, where the executables,
# compiler, linker, etc., are located.
#
# Help on the syntax of this makefile is got at
# http://www.gnu.org/software/make/manual/make.pdf.
#
ifndef LOCATE_TOOLS_INCLUDED
LOCATE_TOOLS_INCLUDED := 1

# Compose a path to a tool from an environment variable with the installation directory,
# $(1), and the name of the executable, $(2). Note, $(1) may contain blanks.
getToolPath = $(call u2w,$(1)$(2).exe)

# Check for the presence of a file, $(1). Note, $(1) may contains blanks.
#   The function returns either the (confirmed) file-path or nothing if the file is not
# existent.
checkForFile = $(wildcard $(call escapeBlanks,$(call w2u,$(1))))
#$(info This file exists: $(if $(call checkForFile,makefile/locateTools.mk),true (as $(call checkForFile,makefile/locateTools.mk)),false))

# Check if a tool named $(2) can be found and started from directory $(1). $(1) may contain
# blanks and it ends with a trailing backslash
getAndValidateToolPath = $(call quoteExecutable,$(or $(call checkForFile,$(call getToolPath,$(1),$(2))),$(error Configured tool $(call getToolPath,$(1),$(2)) doesn't exist. Please check the specified installation path $(1))))

# Different releases of GCC use different executable names.
gcc-prefix :=

# The required tools are located using the configuration settings, particularly the
# installation directories.
gcc := $(call getAndValidateToolPath,$(MINGW_HOME)\bin\,$(gcc-prefix)gcc)
g++ := $(call getAndValidateToolPath,$(MINGW_HOME)\bin\,$(gcc-prefix)g++)
ar := $(call getAndValidateToolPath,$(MINGW_HOME)\bin\,$(gcc-prefix)ar)
objcopy := $(call getAndValidateToolPath,$(MINGW_HOME)\bin\,$(gcc-prefix)objcopy)
size := $(call getAndValidateToolPath,$(MINGW_HOME)\bin\,$(gcc-prefix)size)
windres := $(call getAndValidateToolPath,$(MINGW_HOME)\bin\,$(gcc-prefix)windres)

#java := $(call getAndValidateToolPath,$(JAVA_BIN)\,java)

echo := $(call getAndValidateToolPath,$(UNIX_TOOLS_BIN)\,echo)
mkdir := $(call getAndValidateToolPath,$(UNIX_TOOLS_BIN)\,mkdir)
#pwd := $(call getAndValidateToolPath,$(UNIX_TOOLS_BIN)\,pwd)
rm := $(call getAndValidateToolPath,$(UNIX_TOOLS_BIN)\,rm)
rmdir := $(call getAndValidateToolPath,$(UNIX_TOOLS_BIN)\,rmdir)

# The make tool is different: We need to use the same one as has been invoked by the user
# and as is executing this makefile fragment.
make := $(call quoteExecutable,$(MAKE))

.PHONY: versionGCC helpGCC builtinMacrosGCC
versionGCC:
	$(info GCC: $(call u2w,$(gcc)))
	$(gcc) --version
helpGCC:
	$(gcc) -v --help
builtinMacrosGCC:
	$(gcc) -dM -E - < nul

# Temporarily uncomment the next line to double check the correct selection of tool
# binaries.
#$(info make: $(make), gcc: $(gcc), java: $(java), mkdir: $(mkdir), echo: $(echo), SubWCRev: $(SubWCRev))

else
$(error This makefile shouldn't be called twice. There's a problem in your makefile structure)
endif # LOCATE_TOOLS_INCLUDED
