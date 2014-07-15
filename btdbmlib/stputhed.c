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
static char *rcsid = "$Header: /atreus/mjr/hacks/btree/btdbmlib/RCS/stputhed.c,v 1.1 89/10/24 10:09:17 mjr Rel $";
#endif

/*
	update a record header. this function is also called a lot and
	should be (maybe) made to cache and synchronise with store_gethdr().
*/

extern	off_t	lseek();

store_puthdr(r,rec,hdr)
STORE	*r;
sto_ptr		rec;
struct	sto_hdr		*hdr;
{
	if(lseek(store_fileno(r),rec,SEEK_SET) != rec ||
		write(store_fileno(r),(char *)hdr,STO_HSIZ) != STO_HSIZ) {
		store_errno(r) = STO_IOERR;
		return(STO_ERR);
	}
	return(STO_OK);
}
