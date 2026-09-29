#
# Generic Makefile for GNU Make 3.82 (MinGW port only)
#
# Core of build: Compilation and run of assembler. Linkage of C(++) code.
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
ifndef COMPILE_AND_LINK_INCLUDED
COMPILE_AND_LINK_INCLUDED := 1

# General settings for the makefile.
#$(info Makeprocessor in use is $(MAKE)[$(MAKELEVEL)] $(MAKEFLAGS) $(MAKECMDGOALS))


# Concept of compilation configurations:
#
# Configuration PRODCUTION:
# - no self-test code
# - no debug output
# - no assertions
#
# Configuration DEBUG:
# + all self-test code
# + debug output possible
# + all assertions active
#
ifeq ($(MAKELEVEL),0)
    $(info Making $(target) \
           $(if $(filter PRODUCTION,$(CONFIG)),for production,for debugging) \
     )
endif
ifeq ($(CONFIG),PRODUCTION)
    cDefines := -DPRODUCTION -DNDEBUG
else ifeq ($(CONFIG),DEBUG)
    cDefines := -DDEBUG
else
    $(error Please set CONFIG to either PRODUCTION or DEBUG)
endif
#$(info $(CONFIG) $(cDefines))

# The lists of files to be processed have been elaborated before by findFiles.mk. They can
# be printed for reference and for debugging purpose.
#$(info incDirList := $(incDirList))
#$(info cFileList := $(cFileList))
#$(info objFileList := $(objFileList))
#$(info objDirList := $(objFileList))

# Include the dependency files. Do this with a failure tolerant include operation - the
# files are not available after a clean.
ifneq ($(IGN_DEP),1)
    ifdef INC_DEP
        #$(info Load dependencies)
        -include $(patsubst %.o,%.d,$(objFileList))
    else
        #$(info Don't load dependencies on this make level yet)
    endif
else
    $(warning $$(IGN_DEP)=$(IGN_DEP): Dependency files aren't loaded)
endif

# Ensure existence of target directory.
.PHONY: makeDir
makeDir: | $(objDirList)
$(objDirList):
	-$(mkdir) -pv $@

# Create software version information based on revision information found in .svn.
#   Note, we execute tool GitWCRev.exe unconditionally in each build. We can't write a
# dependency rule as execution depends on some internal information in .git, which is not
# decodable by make. However, fortunately the tool itself has a built-in make-alike rule:
# If the template expansion yields the same file as already existent then it won't
# re-create the file or even update the file date. Therefore, make won't see a new file and
# won't trigger re-compilation and linkage.
.PHONY: generateVersionFile
generateVersionFile:
	$(info Calling $(GitWCRev))
	-$(GitWCRev) . src/rev_softwareRevision.c.template src/rev_softwareRevision.c $(if $(call isOption,s),> nul,)

# Pattern rules for assembler language source files.
asmFlags = -Wall                                                                            \
           -MMD -MP                                                                         \
           $(foreach path,$(call noTrailingSlash,$(incDirList) $(srcDirInUseList)),-I$(path))\
           $(cDefines) $(foreach def,$(defineList),-D$(def))                                \
           -Wa,-g -Wa,-gdwarf-2
ifeq ($(SAVE_TMP),1)
    # Debugging the build: Put assembler listing in the output directory.
    asmFlags += -Wa,-a=$(patsubst %.o,%.lst,$@)
endif
ifneq ($(CONFIG),DEBUG)
    asmFlags += $(productionCodeOptimization)
endif
#$(info asmFlags := $(asmFlags))

$(targetDir)obj/%.o: %.S
	$(info Assembling source file $<)
	$(gcc) -c $(asmFlags) -o $@ $<


# Pattern rules for compilation of, C and C++ source files.

# TODO Consider adding -fno-asynchronous-unwind-tables to compile switches
cFlags = -fno-common -fno-exceptions -ffunction-sections -fdata-sections                    \
         -fshort-enums -fdiagnostics-show-option -finline-functions -fmessage-length=0      \
         -fzero-initialized-in-bss -fno-tree-loop-optimize                                  \
         -Wall -Wno-main -Wno-old-style-declaration -Wextra -Wstrict-overflow=4             \
         -Werror=missing-declarations -Wno-parentheses -Wdiv-by-zero -Wcast-align -Wformat  \
         -Wformat-security -Wignored-qualifiers -Wsign-conversion -Wsign-compare            \
         -Werror=implicit-function-declaration -Werror=maybe-uninitialized                  \
         -Wno-nested-externs -Werror=int-to-pointer-cast -Werror=pointer-sign               \
         -Werror=pointer-to-int-cast -Werror=return-local-addr -Werror=missing-prototypes   \
         -Werror=missing-field-initializers -Werror=overflow -fwrapv                        \
         -MMD -MP -std=gnu11                                                                \
         $(foreach path,$(call noTrailingSlash,$(incDirList) $(srcDirInUseList)),-I$(path)) \
         $(cDefines) $(foreach def,$(defineList),-D$(def))
ifeq ($(SAVE_TMP),1)
    # Debugging the build: Put preprocessed C file and assembler listing in the output
    # directory.
    cFlags += -Wa,-a=$(patsubst %.o,%.lst,$@) -save-temps=obj -fverbose-asm -fstack-usage
endif
# Debug settings see https://gcc.gnu.org/onlinedocs/gcc/Debugging-Options.html#Debugging-Options
ifeq ($(CONFIG),DEBUG)
    cFlags += -g3 -gdwarf-2 -Og #-Winline 
    
    # The Embedded Coder generated source files suffer from frequent occurances of unused
    # variables. This spoils the console output and may obscure other, relevant messages.
    # We turn it off at least in DEBUG compilation. PRRODUCTION code should however be kept
    # clear of this issue and we need the warning.
    cFlags += -Wno-unused-but-set-variable
else
    cFlags += -g1 -gdwarf-2 $(productionCodeOptimization)
    ifneq ($(productionCodeOptimization),-Os)
        cflags += -Winline
    endif
endif
#$(info cFlags := $(cFlags))

$(targetDir)obj/%.o: %.c
	$(info Compiling C file $<)
	$(gcc) -c $(cFlags) -o $@ $<

$(targetDir)obj/%.o: %.cpp
	$(info Compiling C++ file $<)
	$(gcc) -c $(cFlags) -o $@ $<

# Create a preprocessed source file, which is convenient to debug complex nested macro
# expansion, or an assembler listing, which is convenient to understand the
# close-to-hardware parts of the code.
#   Note, different to the compilers does the assembler not take argument -o. It writes the
# output to stdout and we need redirection.
%.i %.asm: %.S
	$(info Preprocessing assembler file $(notdir $<) to text file $(patsubst %.S,%$(suffix $@),$<))
	$(gcc) $(if $(call eq,$(suffix $@),.i),-E,-S) $(filter-out -MMD -MP,$(cFlags)) $< $(if $(call eq,$(suffix $@),.i),-o, >) $(patsubst %.S,%$(suffix $@),$<)

%.i %.asm: %.c
	$(info Preprocessing C file $(notdir $<) to text file $(patsubst %.c,%$(suffix $@),$<))
	$(gcc) $(if $(call eq,$(suffix $@),.i),-E,-S) $(filter-out -MMD -MP,$(cFlags)) -o $(patsubst %.c,%$(suffix $@),$<) $<

%.i %.asm: %.cpp
	$(info Preprocessing C++ file $(notdir $<) to text file $(patsubst %.cpp,%$(suffix $@),$<))
	$(gcc) $(if $(call eq,$(suffix $@),.i),-E,-S) $(filter-out -MMD -MP,$(cFlags)) -o $(patsubst %.cpp,%$(suffix $@),$<) $<


# Windows only: A global resource file is compiled to a binary representation of the
# application's icons. The binary representation is linked with the executable. This makes
# Windows show the application with its own icon. Furthermore, the user can create an
# association of the file name extension of the application's input files with one of its
# icons.
#   No according code is supported for the other environments. Here, no icons are available
# as part of the executable file. The functionality (the actual code of the application) is
# not affected at all.
ifeq ($(OS),Windows_NT)
    # A single (compiled) resource file is demanded for the project if it is build under
    # Windows.
    projectResourceFile := $(targetDir)obj/$(target).res
    
    # A general auto rule for compiling resource files under Windows is added.
    #   TODO The rule is insufficient. The prerequisite is the *.rc file which references
    # external files, e.g. icon files. These external files should also be prerequisites.
    # Directly specifying these files in a rule would break the concept of a generic
    # makefile. We need a working hypothesis similar to the C/C++ code: Look for all icon
    # files in all input directories and add these as prerequisites. At the moment, a
    # change of an icon file won't be considered in the next build.
    $(targetDir)obj/$(target).res: src/linNet/$(target).rc
		$(info Compile Windows resource file $<)
		$(windres) $< -O coff -o $@
else
    # Empty variable: A (compiled) resource file is not known under this operating system.
    projectResourceFile :=
endif


## A general rule enforces rebuild if one of the configuration files changes
#$(objFileList): GNUmakefile ../shared/makefile/compileAndLink.mk                        \
#                    ../shared/makefile/locateTools.mk ../shared/makefile/commonFunctions.mk \
#                    ../shared/makefile/parallelJobs.mk


# Compile all C source files and assemble the *.S. Archive all object files. Target
# directory tree needs to exist.
$(targetDir)$(target).a: $(objFileList)
	$(ar) r $@ $^


# Let the linker create the Windows executable.
lFlags = -Wl,--gc-sections -Wl,-sort-common -Wl,-Map="$(targetDir)$(target).map" -Wl,--cref \
         -Wl,--warn-common,--warn-once -Wl,-g -Wl,--print-memory-usage

$(targetDir)$(target).exe: $(objFileList) $(libList) $(projectResourceFile)
	$(info Linking project. Mapfile is $(targetDir)$(target).map)
	$(gcc) $(lFlags) -o $@ $(objFileList) $(projectResourceFile) -Wl,--start-group $(libList) -lm -Wl,--end-group
	$(size) $@

# Delete all dependency files ignoring (-) the return code from Windows because of non
# existing files.
.PHONY: cleanDep
cleanDep:
	-$(rm) -f $(patsubst %.o,%.d,$(objFileList))

# Delete all application products ignoring (-) the return code from Windows because of non
# existing files.
.PHONY: clean
clean:
	-$(rm) -rf $(targetDir)obj
	-$(rm) -f $(targetDir)*$(target).*

else
$(error This makefile shouldn't be called twice. There's a problem in your makefile structure)
endif # COMPILE_AND_LINK_INCLUDED
