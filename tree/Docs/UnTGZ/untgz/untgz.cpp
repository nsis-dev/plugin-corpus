/*
  untgz - unzip like replacement plugin, except for tarballs
  KJD <jeremyd@computer.org> 2002-2005

  Initial plugin and TAR extraction logic derived from untgz.c
  included with zlib.
  * written by "Pedro A. Aranda Guti\irrez" <paag@tid.es>
  * adaptation to Unix by Jean-loup Gailly <jloup@gzip.org>

  Copyright and license:
  I personally add no additional copyright, so the license is the
  combination of NSIS exDLL (example plugin) and decompression
  libraries used.  For basic gzip'd tarballs (which rely on zlib),
  this is essentially MIT/BSD licensed.  Support for lzma compressed
  tarballs requires the LZMA files and their LGPL/CPL with exception
  license.  Please see the included readme and/or libraries for
  complete copyright and license information.
  
  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors (or copyright holders) be
  held liable for any damages arising from the use of this software.

*/


/*
  USAGE:
  untgz::extract [-j] [-d basedir] [-k] [-z<type>] tarball.tgz
    extracts files from tarball.tgz
	if -j is specified then ignore paths in tarball (junkpaths)
	if -d is specified will extract relative to basedir
	if -k is specified will not overwrite existing files (keep)
      if -z is specified determines compression used, see below
  untgz::extractV [-j] [-d basedir] [-k] [-z<type>] tarball.tgz [-i {iList}] [-x {xList}] --
    extracts files from tarball.tgz
	if -j is specified then ignore paths in tarball (junkpaths)
	if -d is specified will extract relative to basedir
	if -k is specified will not overwrite existing files (keep)
      if -z is specified determines compression used, see below
	if -i is specified will only extract files whose filename matches
	if -x is specified will NOT extract files whose filename matches
	the -- is required and marks the end of the file lists
  untgz::extractFile [-d basedir] [-z<type>] tarball.tgz file
    extracts just the file specified
	path information is ignored, implictly -j is specified (may also be explicit)

  For compatibility with tar command, the following option specifiers may be
  used, however, they are simply ignored.
      -x indicates action to perform is extraction (extract)
      -f archive-name indicates name of tarball (filename), note
         even when used, the filename must be last argument

  If the tarball uses a compression other than gz, then the -z<type> option
  must be specified to indicate how tar file is compressed.
  -z      indicates gzip (.tgz/.tar.gz) compression, uses zlib, 
          this is the default if -z<type> option is omitted
  -znone  indicates uncompressed tar file (.tar)
  -zlzma  indicates lzma (.tlz/.tar.lzma) compression
  -zbz2   indicates bzip2 (.tbz/.tar.bz2) compression, NOT YET IMPLEMENTED
  

  NOTES:
    Without -j there is a security issue as no checking is done to paths,
    allowing untrusted tarballs to overwrite arbitrary files (e.g. /bin/*).
    Also no checking is done to directory or file names.  In untar.c there
    is a hook so custom versions can modify/strip filepaths prior to opening.
    The -d option is currently implemented by a chdir to indicatd directory
    prior to extraction; future versions may instead prepend to extracted path.
*/


// plugin specific headers
#include "untar.h"
#include "nsisUtils.h"

// standard headers
#include <stdarg.h>  /* va_list, va_start, va_end */
#include <stdio.h>

/* The exported API without name mangling */
extern "C" {

__declspec(dllexport) void extract(HWND hwndParent, int string_size, char *variables, stack_t **stacktop);
__declspec(dllexport) void extractV(HWND hwndParent, int string_size, char *variables, stack_t **stacktop);
__declspec(dllexport) void extractFile(HWND hwndParent, int string_size, char *variables, stack_t **stacktop);


/* DLL entry function, needs to be __stdcall, but must be extern "C" for proper decoration (name mangling) */
BOOL WINAPI _DllMainCRTStartup(HANDLE _hModule, DWORD ul_reason_for_call, LPVOID lpReserved);

}


// global variables
HINSTANCE g_hInstance;

HWND g_hwndParent;
HWND g_hwndList;


// DLL entry point
BOOL WINAPI _DllMainCRTStartup(HANDLE _hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
  g_hInstance=(HINSTANCE)_hModule;
  //mCRTinit();	/* init out mini clib, mostly just stdin/stdout/stderr */
  return TRUE;
}


// macro sets R0 to exit status, "success" or error message
#define setExitStatus(status) setuservariable(INST_R0, status)

// macro to display error status, and sets error value
#define setErrorStatus(status)	\
{	\
    PrintMessage(status);	\
    setExitStatus(status);	\
}

// macro to display command line, sets error value, & then returns
#define exitWithError(status, optSvar)	\
{	\
    PrintMessage(cmdline);	\
    PrintMessage(status " %s", optSvar);	\
    setExitStatus(status);	\
    return;	\
}


// macro that attempts to pop argument off stack
// if one is not available will display (via LogMessage)
// current command line followed by msg (which should
// be a description of the error regarding the
// expected argument that is missing) ...
#define poparg(buf, msg) \
{ \
  if (popstring(buf)) \
  { \
    if (tgzFile) gzclose(tgzFile); \
    exitWithError(msg, "") \
  } \
}

// error messages
#define ERR_SUCCESS "success"  /* DO NOT CHANGE */
#define ERR_OPEN_FAILED "Error: Could not open tarball."
#define ERR_READ "Error: Failure reading from tarball."
#define MESG_DONE "extraction complete."

#define ERR_NO_TARBALL "Error: tarball not specified."
#define ERR_DOPT_MISSING_DIR "Error: -d option given but base directory not specified!"
#define ERR_BAD_INCLUDE_LIST "Error: -i unable to obtain include file list!"
#define ERR_BAD_EXCLUDE_LIST "Error: -x unable to obtain exclude file list!"
#define ERR_MISSING_INCLUDE_EXCLUDE_TERMINATOR "Error: -- include/exclude end marker is missing!"
#define ERR_MISSING_FILE "Error: file to extract not specified!"
#define ERR_UNSUPPORTED_COMPRESSION "Error: Unsupported compression format."
#define ERR_UNKNOWN_OPTION "Error: unknown option specified!"
#define WARN_INVALID_OPTION "WARNING: invalid option (%s), ignoring!"


void argParse(HWND hwndParent, int string_size, char *variables, stack_t **stacktop, 
              char *cmd, char *cmdline, gzFile *tgzFile, int *compressionMethod,
              int *junkPaths, char *basePath)
{
  char buf[1024];     /* used for argument processor or other temp buffer */
  char iPath[1024];   /* initial (base) directory for extraction */

  /* setup stack and other general NSIS plugin stuff */
  pluginInit(hwndParent, string_size, variables, stacktop);

  /* initialize our logmessage with command called */
  strcpy(cmdline, "untgz::");
  strcat(cmdline, cmd);  /* e.g. "extract", "extractV", or "extractFile" */
  strcat(cmdline, " ");

  /* usage: untgz::extract* [-j] [-d basedir] tarball.tgz ...other arguments... */

  /* sets INST_R0 to "success" if all goes well, else will be set to an error msg */
  setExitStatus(ERR_SUCCESS);

  /* initialize optional arguments to their general defaults */
  *tgzFile = 0;
  *compressionMethod = CM_GZ;
  *junkPaths = 0;       /* keep path information by default    */
  if (basePath != NULL)
    *basePath = '\0';   /* default to current directory ""     */
  *iPath = '\0';        /* default to current directory ""     */


  /* get 1st optional argument or the tarball itself */
  poparg(buf, ERR_NO_TARBALL);

  /* cycle through handling options */
  while (*buf == '-')
  {
    if (strcmp(buf, "-d") == 0)       /* see if optional basedir specified */
    {
      strcat(cmdline, "-d ");
      poparg(buf, ERR_DOPT_MISSING_DIR);

      /* copy over base directory to our logmesage */
      strcat(cmdline, "'");
      strcat(cmdline, buf);
      strcat(cmdline, "' ");

	  /* if basepath is not NULL then copy it over for callee */
	  if (basePath != NULL) strcpy(basePath, buf);

      /* store so we can set as current directory after opening tarball */
      strcpy(iPath, buf);
    }
    else if (strcmp(buf, "-j") == 0)  /* see if optional junkpaths specified */
    {
      /* indicate junkpaths specified in our logmessage */
      strcat(cmdline, "-j ");

      *junkPaths = 1;
    }
    else if (strcmp(buf, "-z") == 0)
    {
      strcat(cmdline, "-z "); /* update logmessage */
      *compressionMethod = CM_GZ;
    }
    else if (strcmp(buf, "-znone") == 0)
    {
      strcat(cmdline, "-znone "); /* update logmessage */
      *compressionMethod = CM_NONE;
    }
    else if (strcmp(buf, "-zlzma") == 0)
    {
      strcat(cmdline, "-zlzma "); /* update logmessage */
      *compressionMethod = CM_LZMA;
    }
    else if (strcmp(buf, "-zbz2") == 0)
    {
      strcat(cmdline, "-zbz2 "); /* update logmessage */
      *compressionMethod = CM_BZ2;
    }
    else if (strcmp(buf, "-zZ") == 0)
    {
      strcat(cmdline, "-zZ "); /* update logmessage */
      *compressionMethod = CM_Z;
    }
    else if ((strcmp(buf,"-x")==0)||(strcmp(buf,"-f")==0))  /* ignored options */
    {
      /* update our logmessage */
      strcat(cmdline, buf);
      strcat(cmdline, " ");
    }
    else                              /* else invalid optional argument specified */
    {
      strcat(cmdline, "<<");
      strcat(cmdline, buf);
      strcat(cmdline, ">>");
	  PrintMessage(WARN_INVALID_OPTION, buf);
      //exitWithError(ERR_UNKNOWN_OPTION);
    }

    /* get next optional argument or the tarball itself */
    poparg(buf, ERR_NO_TARBALL);
  }

  /* copy over tarball file name to our logmessage */
  strcat(cmdline, "'");
  strcat(cmdline, buf);  
  strcat(cmdline, "' ");

  /* check if compression method requested is supported */
  if ((*compressionMethod == CM_Z)
#ifndef ENABLE_BZ2
      || (*compressionMethod == CM_BZ2)
#endif
#ifndef ENABLE_LZMA
      || (*compressionMethod == CM_LZMA)
#endif
     )
  {
    exitWithError(ERR_UNSUPPORTED_COMPRESSION, buf);
  }
  

  /* open tarball so can read/decompress it */
  if (*compressionMethod == CM_BZ2) {
	  if ((*tgzFile = fopen(buf,"rb")) ==NULL)
	    exitWithError(ERR_OPEN_FAILED, buf);
  } else {

	if ((*tgzFile = gzopen(buf,"rb")) == NULL)
	    exitWithError(ERR_OPEN_FAILED, buf);
  }
  /* set working dir (after opening tarball) to base
     directory user specified (or leave as current),
     but 1st try to create if it doesn't exist yet.
  */
  if (*iPath) /* != '\0' if not specified, ie current */
  {
    makedir(iPath);
    SetCurrentDirectory(iPath);
  }
}

void extract(HWND hwndParent, int string_size, char *variables, stack_t **stacktop)
{
  char cmdline[1024];     /* just used to display to user */
  int junkPaths;          /* default to extracting with paths -- highly insecure */
  int compressionMethod;  /* gzip or other compressed tar file */
  gzFile tgzFile = NULL;  /* the opened tarball (assuming argParse returns successfully) */

  /* do common stuff including parsing arguments up to filename to extract */
  argParse(hwndParent, string_size, variables, stacktop, 
           "extract", cmdline, &tgzFile, &compressionMethod, &junkPaths, NULL);

  /* check if everything up to now processed ok, exit if not */
  if (strcmp(getuservariable(INST_R0), ERR_SUCCESS) != 0) return;

  /* show user cmdline */
  PrintMessage(cmdline);

  /* actually perform the extraction */
  if (tgz_extract(tgzFile, compressionMethod, junkPaths, 0, NULL, 0, NULL) == -1)
    setErrorStatus(ERR_READ)
  else
    PrintMessage(MESG_DONE);
}

void extractV(HWND hwndParent, int string_size, char *variables, stack_t **stacktop)
{
  char cmdline[1024];     /* just used to display to user */
  int junkPaths;          /* default to extracting with paths -- highly insecure */
  int compressionMethod;  /* gzip or other compressed tar file */
  gzFile tgzFile = NULL;  /* the opened tarball (assuming argParse returns successfully) */

  char buf[1024];         /* used for argument processor or other temp buffer */
  int iCnt=0, xCnt=0;     /* count for elements in list */
  char **iList=NULL,      /* (char *) list[Cnt] for list of files to extract */
       **xList=NULL;      /* (char *) list[Cnt] for list of files to NOT extract */

  /* do common stuff including parsing arguments up to filename to extract */
  argParse(hwndParent, string_size, variables, stacktop, 
           "extractV", cmdline, &tgzFile, &compressionMethod, &junkPaths, NULL);

  /* check if everything up to now processed ok, exit if not */
  if (strcmp(getuservariable(INST_R0), ERR_SUCCESS) != 0) return;

  /* get next optional argument or end marker */
  poparg(buf, ERR_MISSING_INCLUDE_EXCLUDE_TERMINATOR);

  while ((*buf == '-') && (strcmp(buf, "--") != 0))
  {
    if (strcmp(buf, "-i") == 0)
	{
      strcat(cmdline, "-i ");

      /* get include file list */
      if (getArgList(&iCnt, &iList, cmdline) != 0)
        exitWithError(ERR_BAD_INCLUDE_LIST, "");
	}
	else if (strcmp(buf, "-x") == 0)
    {
      strcat(cmdline, "-x ");

      /* get exclude file list */
      if (getArgList(&xCnt, &xList, cmdline) != 0)
        exitWithError(ERR_BAD_EXCLUDE_LIST, "");
    }
	else                              /* else invalid optional argument specified */
	{
      strcat(cmdline, "<<");
      strcat(cmdline, buf);
      strcat(cmdline, ">>");
	  PrintMessage(WARN_INVALID_OPTION, buf);
      //exitWithError(ERR_UNKNOWN_OPTION);
	}

    /* get next optional argument or end marker */
    poparg(buf, ERR_MISSING_INCLUDE_EXCLUDE_TERMINATOR);
  }

  if (strcmp(buf, "--") == 0)
	  strcat(cmdline, "--");
  else
	  strcat(cmdline, "?--?");


  /* show user cmdline */
  PrintMessage(cmdline);

  /* actually perform the extraction */
  if (tgz_extract(tgzFile, compressionMethod, junkPaths, iCnt, iList, xCnt, xList) == -1)
    setErrorStatus(ERR_READ)
  else
    PrintMessage(MESG_DONE);
}

void extractFile(HWND hwndParent, int string_size, char *variables, stack_t **stacktop)
{
  char cmdline[1024];     /* just used to display to user */
  char filename[1024];    /* holds filename of file to extract */
  int dummy;              /* holds junkpath, except always true for us, so ignored */
  int compressionMethod;  /* gzip or other compressed tar file */
  gzFile tgzFile = NULL;  /* the opened tarball (assuming argParse returns successfully) */

  /* do common stuff including parsing arguments up to filename to extract */
  argParse(hwndParent, string_size, variables, stacktop, 
           "extractFile", cmdline, &tgzFile, &compressionMethod, &dummy, NULL);

  /* check if everything up to now processed ok, exit if not */
  if (strcmp(getuservariable(INST_R0), ERR_SUCCESS) != 0) return;

  /* actually get the file to extract */
  poparg(filename, ERR_MISSING_FILE);

  /* copy over file name to extract to our logmessage */
  strcat(cmdline, "'");
  strcat(cmdline, filename);
  strcat(cmdline, "'");

  /* show user cmdline */
  PrintMessage(cmdline);

  /* actually perform the extraction */
  if (tgz_extract_file(tgzFile, compressionMethod, filename) == -1)
    setErrorStatus(ERR_READ)
  else
    PrintMessage(MESG_DONE);
}

