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
static char *rcsid = "$Header: /atreus/mjr/hacks/btree/btdbmlib/RCS/stdecref.c,v 1.1 89/10/24 10:09:13 mjr Rel $";
#endif

/*
	decrement the reference count in a record header.
*/

store_decrefcnt(r,rec)
STORE	*r;
sto_ptr		rec;
{
	struct	sto_hdr	 rbuf;
	if(store_gethdr(r,rec,&rbuf) == STO_ERR)
		return(STO_ERR);
	rbuf.refs--;
	if(store_puthdr(r,rec,&rbuf) == STO_ERR)
		return(STO_ERR);
	return(STO_OK);
}
