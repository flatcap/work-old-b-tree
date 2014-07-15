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
static char *rcsid = "$Header: /atreus/mjr/hacks/btree/btdbmlib/RCS/stalloc.c,v 1.1 89/10/24 10:09:12 mjr Rel $";
#endif

/*
	storage allocation for the store library. allocation is performed
	as follows:
	1) if there is a free header list, read it sequentially and take
	the first fit.
		a)if the first fitting record is bigger than some compiled-in
		value, it is split into 2 subrecords, and the left-over
		is placed back into the free list. 
		b)if the first fitting record is not large enough to split
		without excessive fragmentation, return the whole thing
		and assume the user will be happy.
	2) if there is no free header list, allocate a new record chunk
	from the end-of-file and initialize it as empty.

	the free header list is stored as a record, and is accessed and
	updated by library routines. it's somewhat self-referential, but
	works nicely. the only disadvantage of this approach is that it
	probably does more seeks than it should in an ideal world.
*/


sto_ptr
store_alloc(r,bytes)
STORE	*r;
int		bytes;
{
	sto_ptr	ret = STO_NULL;
	struct	sto_hdr	rbuf;

	if(bytes <= 0)
		return(STO_NULL);

	if(r->sblk.free != STO_NULL) {
		struct	sto_freeent	fry;
		struct	sto_freeent	*freep;
		int	byt;
		off_t	currof;
		int	ismore;
		int	fcnt;
		int	junk;

		currof = (off_t)(2 * sizeof(long));

		while(1) {

			ismore = store_read(r,r->sblk.free,currof,store_buffer(r),store_bufsiz(r),&byt);
			if(ismore == STO_ERR)
				return(STO_ERR);

			if(currof == 0L) {
				freep = STO_FREEBUF(store_buffer(r));
				fcnt = ((byt - (2 * sizeof(off_t))) / STO_FSIZ); 
			} else {
				freep = (struct sto_freeent *)store_buffer(r);
				fcnt = byt / STO_FSIZ; 
			}

			currof += (off_t)byt;

			for(junk = 0; junk < fcnt; junk++) {
				if(bytes <= freep->size) {

					/* allocate it */
					ret = freep->addr;

					/* if the block is big enough, split */						/* it into a record of the desired */						/* size and one consisting of the */
					/* left-overs. if the size of the */
					/* table scraps is small, just give */
					/* the customer a bit extra */
					if(freep->size - bytes >= STO_SPLITSIZ) {
						fry.addr = freep->addr + STO_HSIZ + bytes;
						
						fry.size = freep->size - (2 * STO_HSIZ) - bytes;
						rbuf.size = bytes; 
					} else {
						fry.addr = STO_NULL;
						fry.size = 0;
						rbuf.size = freep->size; 
					}

					if(store_write(r,r->sblk.free,(off_t)(2 * sizeof(off_t) + (junk * STO_FSIZ)),(unsigned char *)&fry,STO_FSIZ) == STO_ERR)
						return(STO_NULL);

					/* duh, break out of the loop */
					junk = fcnt + 1;
				}
				freep++;
			}
			if(ismore != STO_MORE)
				break;
		}
		
	}

	if(ret == STO_NULL) {
		ret = r->sblk.high;
		r->sblk.high += bytes + STO_HSIZ;
		rbuf.size = bytes; 
		rbuf.used = 0; 
		rbuf.refs = 1; 
		rbuf.next = rbuf.prev = STO_NULL;
		if(store_wsuper(r) != STO_OK)
			return(STO_NULL);
	}

	rbuf.magic = STO_DFLTMAGIC; 
	rbuf.used = 0; 
	rbuf.refs = 1; 
	rbuf.next = rbuf.prev = STO_NULL;

	if(store_puthdr(r,ret,&rbuf) == STO_ERR)
		return(STO_NULL);

	return(ret);
}
