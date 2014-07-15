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
static char *rcsid = "$Header: /atreus/mjr/hacks/btree/btdbmlib/RCS/stread.c,v 1.1 89/10/24 10:09:17 mjr Rel $";
#endif

/*
	function to read a record into a user-provided buffer. it is somewhat
	like the read(2) system call except it accepts an offset, and returns
	the number of bytes read in an integer variable provided by the caller.
*/

extern	off_t	lseek();

store_read(r,rec,offset,buf,siz,rsiz)
STORE	*r;
sto_ptr		rec;
off_t		offset;
unsigned char	*buf;
int		siz;
int		*rsiz;
{
	struct	sto_hdr	rbuf;
	int	ret = STO_OK;
	int	toread;

	if(store_gethdr(r,rec,&rbuf) == STO_ERR)
		return(STO_ERR);

	if(rbuf.used > siz + offset)
		ret = STO_MORE;

	if(siz + offset > rbuf.used)
		toread = rbuf.used - offset;
	else
		toread = siz;

	if(offset > 0 && lseek(store_fileno(r),rec + STO_HSIZ + offset,SEEK_SET) != rec + STO_HSIZ + offset) {
		store_errno(r) = STO_IOERR;
		return(STO_ERR);
	}

	if((*rsiz = read(store_fileno(r),(char *)buf,toread)) != toread)
		return(STO_ERR);
	return(ret);
}
