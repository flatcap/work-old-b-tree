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
static char *rcsid = "$Header: /atreus/mjr/hacks/btree/btdbmlib/RCS/stwrite.c,v 1.1 89/10/24 10:09:19 mjr Rel $";
#endif

/*
	write(2)-like interface to write a record. accepts an offset, which
	is treated as the point at which to start writing. if the write
	extends the in-use space of the block, the header may be adjusted.
*/

extern	off_t	lseek();

store_write(r,rec,offset,buf,siz)
STORE	*r;
sto_ptr		rec;
off_t		offset;
unsigned char	*buf;
int		siz;
{
	struct	sto_hdr	rbuf;

	if(store_gethdr(r,rec,&rbuf) == STO_ERR)
		return(STO_ERR);

	if(siz > rbuf.size || (int)offset + siz > rbuf.size) {
		store_errno(r) = STO_TOOSMALL;
		return(STO_ERR);
	}

	if(offset > 0 && lseek(store_fileno(r),rec + STO_HSIZ + offset,SEEK_SET) != rec + STO_HSIZ + offset) {
		store_errno(r) = STO_IOERR;
		return(STO_ERR);
	}

	if(write(store_fileno(r),(char *)buf,siz) != siz)
		return(STO_ERR);

	/* has the buffer grown any ? */
	if(offset + siz > rbuf.used) {
		rbuf.used = offset + siz;
		if(store_puthdr(r,rec,&rbuf) == STO_ERR)
			return(STO_ERR);
	}
	return(STO_OK);
}
