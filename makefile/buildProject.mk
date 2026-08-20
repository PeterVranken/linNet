#
# Generic Makefile for GNU Make 3.82 (MinGW port only)
#
# Control the serialization of build steps to make the complete application or project,
# which must happen sequentially even if make is commanded to do parallel processing (i.e.
# jobs=n is given).
#
# Help on the syntax of this makefile is got at
# http://www.gnu.org/software/make/manual/make.pdf.
ifndef BUILD_PROJECT_INCLUDED
BUILD_PROJECT_INCLUDED := 1

# A normal GNU makefile cannot safely be run with the make command line switch -j if it
# combines targets to sequences of such. A common example is a derived target "rebuild",
# which depends on "clean" and "build" in this order. The order of prerequisites is however
# regarded only in the standard sequential mode of make. If -j N is given, the
# prerequisites are made in parallel; in our example "clean" was not be guaranteed to be
# completed before the build begins.
#   make considers all prerequisites as long as independent and will make them in parallel
# as long as they are not explicitly coupled by inter-dependencies. Such inter-dependencies
# can however not always be expressed with the term "target: prerequisite". The result of
# clean is to have not any file left, so which file to state as prerequisite of a rule
# actually depending on clean? Moreover: Such a dependency is typically only optional, we
# do not want to do a clean always; a regular, always-present prerequisite "clean" of
# target build would obviously be counterproductive.
#   The only found way out of this is to use the makefile recursively. Now the build steps
# are properly serialized as steps of a recipe. Each step may benefit from parallelism but
# the execution of steps remains sequential. Each step is a single rule with fully expressed
# dependency hierarchy. The prerequisite(s) of the phony rules get the character of a .BEGIN
# rule, like some make derivatives offer. Caution, these are again not serialized in a
# defined order if make is invoked with -j.

# Load the makefile, the targets of which are run in a safe parallel way.
include $(sharedMakefilePath)help.mk
include $(sharedMakefilePath)commonFunctions.mk
include $(sharedMakefilePath)globalVariables.mk
include $(sharedMakefilePath)locateTools.mk
include $(sharedMakefilePath)findFiles.mk
include $(sharedMakefilePath)compileAndLink.mk

# Locate tools, which are required on project level only.
#SubWCRev := $(call getAndValidateToolPath,../../tools/SubWCRev/,SubWCRev)

# Run the complete build process with compilation, linkage and a2l and binary file
# modifications. However, the sub-ordinated archives are not built, their up-to-date
# presence is a not checked pre-requisite.
.PHONY: build
build: makeDir #generateVersionFile
	$(MAKE) INC_DEP=1 $(targetDir)$(target).exe

# Rebuild the application.
#   Note, the denpedencies don't need to be loaded in case of a re-build.
.PHONY: rebuild
rebuild: clean #generateVersionFile
	$(MAKE) makeDir
	$(MAKE) $(targetDir)$(target).exe

.PHONY: help_buildProducts
help_buildProducts:
	$(info Additional targets on project build level:$(EOL)\
           - build: Build the flashable binary without updating the included archives$(EOL)\
           - rebuild: Target clean followed by target build$(EOL)\
           - clean: Delete application build artifacts but leave the included archives$(EOL))
	$(info )
h help targets usage: help_buildProducts

else
$(error This makefile shouldn't be called twice. There's a problem in your makefile structure)
endif # BUILD_PROJECT_INCLUDED
