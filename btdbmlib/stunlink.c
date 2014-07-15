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
static char *rcsid = "$Header: /atreus/mjr/hacks/btree/btdbmlib/RCS/stunlink.c,v 1.1 89/10/24 10:09:18 mjr Rel $";
#endif

/*
	break the links on both sides of a record, making them point to
	the correct places.
*/

store_unlink(r,victim)
STORE	*r;
sto_ptr		victim;
{
	struct	sto_hdr	 pbuf;
	struct	sto_hdr	 nbuf;
	struct	sto_hdr	 vbuf;
	sto_ptr	savprev;
	sto_ptr	savnext;
	char	pnul = 0;
	char	nnul = 0;

	if(store_gethdr(r,victim,&vbuf) == STO_ERR)
		return(STO_ERR);

	if((savnext = vbuf.next) != STO_NULL)
		nnul++;
	if((savprev = vbuf.prev) != STO_NULL)
		pnul++;

	if(nnul && store_gethdr(r,vbuf.next,&nbuf) == STO_ERR)
		return(STO_ERR);

	if(pnul && store_gethdr(r,vbuf.prev,&pbuf) == STO_ERR)
		return(STO_ERR);

	if(nnul)
		nbuf.prev = vbuf.prev;
	if(pnul)
		pbuf.next = vbuf.next;

	vbuf.prev = vbuf.next = STO_NULL;

	if(store_puthdr(r,victim,&vbuf) == STO_ERR)
		return(STO_ERR);

	if(pnul && store_puthdr(r,savprev,&pbuf) == STO_ERR)
		return(STO_ERR);

	if(nnul && store_puthdr(r,savnext,&nbuf) == STO_ERR)
		return(STO_ERR);

	return(STO_OK);
}
