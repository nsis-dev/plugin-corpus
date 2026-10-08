#ifndef _BZ2_H_
#define _BZ2_H_

#define BZ_NO_STDIO

#include "bzlib.h"

typedef void BZFILE;


int bz2_init( BZFILE * in, BZFILE ** out);
void bz2_cleanup(BZFILE *bz2File);
int bz2_read(BZFILE *bz2File, void *buffer, int size);


#endif /* _BZ2_H_ */
