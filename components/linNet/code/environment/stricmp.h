#ifndef STRICMP_INCLUDED
#define STRICMP_INCLUDED
/**
 * @file stricmp.h
 *   Implementation of stricmp for environments, which do not provide this standard
 * function.
 *
 * Copyright (C) 2014 Peter Vranken (mailto:Peter_Vranken@Yahoo.de)
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */
/* Module interface
 *   stricmp
 */

/*
 * Include files
 */

#include <string.h>
#include <ctype.h>
#include "types.h"


/*
 * Defines
 */

/* Find out if we have a revision of GCC, which requires a local implementation of stricmp:
   Try the build with and without the local implementation and see if you run into a linker
   error. Use the local implementation only if compiler or linker complain about the
   missing function. */
#if defined(__GNUC__) 
# if 10000*__GNUC__ + 100*__GNUC_MINOR__ + __GNUC_PATCHLEVEL__ <= 40801 \
     && !defined(__MINGW64__)
#  define USE_LOCAL_STRICMP
# elif 10000*__GNUC__ + 100*__GNUC_MINOR__ + __GNUC_PATCHLEVEL__ == 80100
    /* GCC 8.1.0 for Windows contains the needed function in both variants, 32 and 64 Bit.
       (Checked for mingw-i686-8.1.0-release-win32-dwarf-rt_v6-rev0 and
       mingw-w64-x86_64-8.1.0-posix-seh-rt_v6-rev0.) */
# elif 10000*__GNUC__ + 100*__GNUC_MINOR__ + __GNUC_PATCHLEVEL__ == 160100
# else
#  error Double-check if local implementation of stricmp is still needed for this revision \
         of GCC
# endif
#else
# error Double-check if local implementation of stricmp is needed for this compiler or c \
        library
#endif

/*
 * Global type definitions
 */


/*
 * Global data declarations
 */


/*
 * Global inline interface
 */

#ifdef USE_LOCAL_STRICMP
/**
 * Case insensitive string compare.
 *   @return
 * The difference of the first differing pair of characters at the same position in the two
 * strings.
 *   @param a
 * First operand of the returned difference.
 *   @param b
 * Second operand of the returned difference.
 *   @remark
 * Based on http://stackoverflow.com/questions/5820810/case-insensitive-string-comp-in-c,
 * visited on Feb 1, 2014.
 */

static signed int stricmp(char const *a, char const *b)
{
    while(true)
    {
        signed int d = tolower(*a) - tolower(*b);
        if(d != 0 || *a == 0)
            return d;

        ++ a;
        ++ b;
    }

} /* End of stricmp */
#endif /* function not available in GCC for mingw32 */



/*
 * Global prototypes
 */




#endif  /* STRICMP_INCLUDED */
