#include "bzlib.h"
#include "nsisUtils.h"

static int bzerror;

int bz2_init( BZFILE * in, BZFILE ** out) {


	*out = BZ2_bzReadOpen( &bzerror, in, 
                        0 , 0,
                        NULL , 0 );
	return bzerror;
}

void bz2_cleanup(BZFILE *bz2File) {

	BZ2_bzReadClose(&bzerror, bz2File);
}

int bz2_read(BZFILE * bz2File, void *buffer, int BLOCKSIZE) {

	int len;

	len = BZ2_bzRead ( &bzerror, bz2File , buffer, BLOCKSIZE );

	return len;
}

