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
static char *rcsid = "$Header: /atreus/mjr/hacks/btree/btdbmlib/RCS/stfree.c,v 1.1 89/10/24 10:09:14 mjr Rel $";
#endif

/*
	free list management and creation. the free list is maintained as a
	stored record using the library's own internal routines. when a 
	request is made to free something, the order of operations is:

	1) if the record is the free list, refuse to free it !!!
	2)if there is more than one reference to the record, just
	decrement the reference count and return.
	3) if there is not a currently defined free list:
		a)one is allocated via store_alloc(), which can actually
		be safely called because it will not look at the nonexistent
		free list. if the free-list management is changed, this will
		probably get very weird.
		b)make a free list entry and paste it into the new free list
		c)update the superblock
		d)return - done.
	4) if there WAS a free list, 
		a)if the whole allocated block of the free list is not in
		currently marked as in-use, just paste the new free list entry
		at the end of the free list block and return.
	- or -
		b)scan through the allocated block looking for an empty slot
		and if one is found, paste it directly in. this operation is
		less inefficient than it may seem because the scan is buffered
		pretty intelligently.
	- if 'b' fails -
		c)we know we now have a full free list block with no empty
		slots, so we bite the bullet, and allocate a bigger free
		list block at the end of file, and copy the old one into
		it. we then have enough free space to paste the new free
		list entry directly at the end of the block, as in '3'
		above. this should presumably happen fairly rarely. it
		costs some, but the copy is pretty well buffered.
*/

store_free(r,rec)
STORE	*r;
sto_ptr		rec;
{
	struct	sto_freeent	fry;
	struct	sto_freeent	*freep;
	struct	sto_hdr	rbuf;
	struct	sto_hdr	nbuf;
	sto_ptr	newf;
	int		fcnt;
	int		junk;
	int		byts;

	/* may as well make sure everything is OK before we do work */
	if(store_gethdr(r,rec,&rbuf) == STO_ERR)
		return(STO_ERR);

	/* sneaky dog ! */
	if(rec == r->sblk.free) {
		store_errno(r) = STO_NOREC;
		return(STO_ERR);
	}

	/* if there are is than one reference to this record, just decrement */
	/* it and we are happy-like, even though it could be bad */
	if(--rbuf.refs > 0) {
		return(store_puthdr(r,rec,&rbuf));
	}

	/* if it is linked to siblinks, unlink them */
	if(rbuf.next != STO_NULL || rbuf.prev != STO_NULL) {
		if(store_unlink(r,rec) == STO_ERR)
			return(STO_ERR);

		/* one flaw in the design: we have to re-read the header */
		/* since the call to unlink changes pointers and all that */
		if(store_gethdr(r,rec,&rbuf) == STO_ERR)
			return(STO_ERR);
	}

	fry.addr = rec;
	fry.size = rbuf.size;

	/* if there is not existing free page, create one */
	if(r->sblk.free == STO_NULL) {
		r->sblk.free = store_alloc(r,STO_BUFSIZ);
		if(r->sblk.free == STO_NULL)
			return(STO_ERR);
		STO_FREENXT(store_buffer(r)) = STO_NULL;
		STO_FREECNT(store_buffer(r)) = 0;

		if(store_write(r,r->sblk.free,0L,store_buffer(r),2 * sizeof(off_t)) != STO_OK ||
			store_wsuper(r) != STO_OK)
				return(STO_ERR);
	}

	/* now grab the free record header */
	if(store_gethdr(r,r->sblk.free,&rbuf) == STO_ERR)
		return(STO_ERR);

	/* is there room for a simple insert, or do we need to get mean ? */
	if(rbuf.used + STO_FSIZ < rbuf.size) {
		if(store_write(r,r->sblk.free,(off_t)rbuf.used,(unsigned char *)&fry,STO_FSIZ) == STO_ERR)
			return(STO_ERR);
		return(STO_OK);
	} else {
		off_t	currof;

		currof = (off_t)(2 * sizeof(off_t));
		while(1) {
			int	ismore;

			ismore = store_read(r,r->sblk.free,currof,store_buffer(r),store_bufsiz(r),&byts);
			if(ismore == STO_ERR)
				return(STO_ERR);

			if(currof == 0L) {
				freep = STO_FREEBUF(store_buffer(r));
				fcnt = ((byts - (2 * sizeof(off_t))) / STO_FSIZ); 
			} else {
				freep = (struct sto_freeent *)store_buffer(r);
				fcnt = byts / STO_FSIZ; 
			}

			currof += (off_t)byts;

			for(junk = 0; junk < fcnt; junk++) {
				if(freep->addr == STO_NULL) {
					if(store_write(r,r->sblk.free,(off_t)(2 * sizeof(off_t) + (junk * STO_FSIZ)),(unsigned char *)&fry,sizeof(fry)) == STO_ERR)
						return(STO_ERR);
				}
				freep++;
			}
			if(ismore != STO_MORE)
				break;
		}
	}

	/* brutal but effective. Allocate a bigger free block at EOF */
	nbuf.size = rbuf.size * 2; 
	newf = r->sblk.high;
	r->sblk.high += nbuf.size + STO_HSIZ;
	nbuf.magic = STO_DFLTMAGIC; 
	nbuf.used = rbuf.used; 
	nbuf.refs = 1; 
	nbuf.next = nbuf.prev = STO_NULL;

	if(store_puthdr(r,newf,&nbuf) == STO_ERR)
		return(STO_ERR);

	if(store_copy(r,r->sblk.free,newf) == STO_ERR)
		return(STO_ERR);

	if(store_write(r,newf,(off_t)nbuf.used,(unsigned char *)&fry,sizeof(fry)) == STO_ERR)
		return(STO_ERR);

	/* kludge #2 - free the old free page */
	fry.addr = r->sblk.free;
	fry.size = rbuf.size;

	if(store_write(r,newf,(off_t)nbuf.used,(unsigned char *)&fry,sizeof(fry)) == STO_ERR)
		return(STO_ERR);

	r->sblk.free = newf;

	return(store_wsuper(r));
}
