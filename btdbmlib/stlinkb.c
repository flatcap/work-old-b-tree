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
static char *rcsid = "$Header: /atreus/mjr/hacks/btree/btdbmlib/RCS/stlinkb.c,v 1.1 89/10/24 10:09:16 mjr Rel $";
#endif

/*
	manipulate the link pointers of a header to link the record numbered
	'pred' BEFORE the record numbered 'victim'. Note that no attempt is
	made to UNLINK the victim if it is already linked into a list.
	that must be controlled at a higher level, or we get into a kind
	of recursive linking and unlinking loop and never return.
*/

store_linkbefore(r,victim,next)
STORE	*r;
sto_ptr		victim;
sto_ptr		next;
{
	struct	sto_hdr	 nbuf;
	struct	sto_hdr	 pbuf;
	struct	sto_hdr	 vbuf;
	char	pnul = 0;

	if(next == STO_NULL) {
		store_errno(r) = STO_NOREC;
		return(STO_ERR);
	}

	if(store_gethdr(r,victim,&vbuf) == STO_ERR || store_gethdr(r,next,&nbuf) == STO_ERR)
		return(STO_ERR);

	if(nbuf.prev != STO_NULL)
		pnul++;

	if(pnul && store_gethdr(r,nbuf.prev,&pbuf) == STO_ERR)
		return(STO_ERR);

	if(pnul)
		pbuf.next = victim;
	vbuf.next = next;
	vbuf.prev = nbuf.prev;
	nbuf.prev = victim;

	if(store_puthdr(r,victim,&vbuf) == STO_ERR || store_puthdr(r,next,&nbuf) == STO_ERR)
		return(STO_ERR);

	if(pnul && store_puthdr(r,vbuf.prev,&pbuf) == STO_ERR)
		return(STO_ERR);

	return(STO_OK);
}
