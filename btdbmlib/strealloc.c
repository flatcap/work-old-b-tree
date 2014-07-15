#include	<sys/types.h>
#include	"stoconf.h"
#include	"store.h"


/*
         (C) Copyright, 1988, 1989 Marcus J. Ranum
                    All rights reserved


          This software, its documentation,  and  supporting
          files  are  copyrighted  material  and may only be
          distributed in accordance with the terms listed in
          the COPYRIGHT document.
*/


#ifndef	lint
static char *rcsid = "$Header: /atreus/mjr/hacks/btree/btdbmlib/RCS/strealloc.c,v 1.1 89/10/24 10:09:17 mjr Rel $";
#endif

/*
	record reallocator. this has to do a lot of ugly stuff because of
	the linked-list management. first it remembers the sibs of a record,
	allocates a new one, copies the old one into it, frees the old one,
	patches links and references and returns.
*/

sto_ptr
store_realloc(r,rec,bytes)
STORE		*r;
sto_ptr		rec;
int		bytes;
{
	sto_ptr	ret;
	struct	sto_hdr	rbuf;
	struct	sto_hdr	nbuf;

	/* remember this so we can patch any links that existed */
	if(store_gethdr(r,rec,&rbuf) == STO_ERR)
		return(STO_ERR);

	ret = store_alloc(r,bytes);
	if(ret == STO_NULL)
		return(STO_NULL);

	if(store_copy(r,rec,ret) == STO_ERR)
		return(STO_NULL);

	if(store_free(r,rec) == STO_ERR)
		return(STO_NULL);

	/* re-establish next links */
	if(rbuf.next != STO_NULL) {
		if(store_gethdr(r,rbuf.next,&nbuf) == STO_ERR)
			return(STO_ERR);
		nbuf.prev = ret;
		if(store_puthdr(r,rbuf.next,&nbuf) == STO_ERR)
			return(STO_ERR);
	}

	/* re-establish prev links */
	if(rbuf.prev != STO_NULL) {
		if(store_gethdr(r,rbuf.prev,&nbuf) == STO_ERR)
			return(STO_ERR);
		nbuf.next = ret;
		if(store_puthdr(r,rbuf.prev,&nbuf) == STO_ERR)
			return(STO_ERR);
	}

	if(store_gethdr(r,ret,&nbuf) == STO_ERR)
		return(STO_ERR);

	nbuf.refs = rbuf.refs;
	nbuf.next = rbuf.next;
	nbuf.prev = rbuf.prev;

	if(store_puthdr(r,ret,&nbuf) == STO_ERR)
		return(STO_ERR);
	return(ret);
}
