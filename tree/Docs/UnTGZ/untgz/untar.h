/*
 * tgz_extract functions based on code within zlib library
 * No additional copyright added, KJD <jeremyd@computer.org>
 *
 *   This software is provided 'as-is', without any express or implied
 *   warranty.  In no event will the authors be held liable for any damages
 *   arising from the use of this software.
 *
 * untgz.c -- Display contents and/or extract file from
 * a gzip'd TAR file
 * written by "Pedro A. Aranda Guti\irrez" <paag@tid.es>
 * adaptation to Unix by Jean-loup Gailly <jloup@gzip.org>
 */

#ifdef __cplusplus
extern "C" {
#endif

/* mini Standard C library replacement */
//#include "miniclib.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>

/* library upon which all this work based on/requires */
#include "zlib/zlib.h"

#ifdef _WIN32 
#undef CM_NONE
#endif

#define CM_NONE 0 /* no compression */
#define CM_GZ   1 /* gz compressed */
#define CM_LZMA 2 /* lzma compressed */
#define CM_BZ2  3 /* unsupported, bzip2 compressed */
#define CM_Z    4 /* unsupported, compress compressed */

/* comment out to disable support for unneeded compression methods */
/* NONE and GZ are always enabled */
#define ENABLE_LZMA
#define ENABLE_BZ2
/* #define ENABLE_Z */

/* actual extraction routines */
int tgz_extract(gzFile tgzFile, int cm, int junkPaths, int iCnt, char *iList[], int xCnt, char *xList[]);
int tgz_extract_file(gzFile tgzFile, int cm, char *filename);

/* recursive make directory */
/* abort if you get an ENOENT errno somewhere in the middle */
/* e.g. ignore error "mkdir on existing directory" */
/* */
/* return 1 if OK */
/*        0 on error */

int makedir (char *newdir);

/* !!!USER SUPPLIED!!! */
/* wrap around whatever you want to send error messages to user, c function */
void PrintMessage(const char *msg, ...);


#ifdef __cplusplus
}
#endif
