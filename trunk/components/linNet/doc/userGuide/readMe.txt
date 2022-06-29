The file linNet-UserGuide.pdf is the main documentation of linNet. You
are strongle encouraged to read it before starting with linNet.

The user guide of linNet is open source. To build it from the source
files, the build environment (GNU make tool) needs to be set up as
described for linNet itself in the user guide.

The user guide is written in LaTeX. The TeX compiler applied on a Windows
system is MikTeX MiKTeX-pdfTeX, Version 3.141592653-2.6-1.40.24 (MiKTeX
21.12.10 Portable). This tool needs to be installed; pdflatex.exe needs to
be found on the Windows search path. Building the manual on other systems
has not been tried so far.

Once the environment is set up you just need to open a shell, cd here into
directory userGuide and type:

make

To get more information type

make help

A valid MikTeX installation can be found at https://miktex.org/download;
see https://miktex.org/howto/portable-edition for how to use it (both
visited Jan 2022).
