#include	<sys/types.h>
#include	"stoconf.h"
#include	"store.h"
#include	"stointern.h"


/*
         (C) Copyright, 1988, 1989 Marcus J. Ranum
                    All rights reserved


          This software, its documentation,  and  supporting
          files  are  copyrighted  material  and may only be
          distributed in accordance with the terms listed in
          the COPYRIGHT document.
*/


#ifndef	lint
static char *rcsid = "$Header: /atreus/mjr/hacks/btree/btdbmlib/RCS/stgethed.c,v 1.1 89/10/24 10:09:15 mjr Rel $";
#endif

/*
	read a record header. this might benefit from some caching,
	because this function is called A LOT. a record header is
	read, then checked (as much as we can - using a magic number)
	for validity. note that the file descriptor should be left
	pointing to the beginning of the record's data segment. the
	functions store_read() and store_write() rely on this.
*/

extern	off_t	lseek();

store_gethdr(r,rec,hdr)
STORE	*r;
sto_ptr		rec;
struct	sto_hdr		*hdr;
{
	if(rec == STO_NULL) {
		store_errno(r) = STO_NOREC;
		return(STO_ERR);
	}

	if(lseek(store_fileno(r),rec,SEEK_SET) != rec) {
		store_errno(r) = STO_IOERR;
		return(STO_ERR);
	}

	switch(read(store_fileno(r),(char *)hdr,STO_HSIZ)) {
		case	0:
			store_errno(r) = STO_NOREC;
			return(STO_ERR);

		case	STO_HSIZ:
			if(hdr->magic != STO_DFLTMAGIC) {
				store_errno(r) = STO_NOREC;
				return(STO_ERR);
			}
			return(STO_OK);

		default:
			return(STO_ERR);
	}

}
