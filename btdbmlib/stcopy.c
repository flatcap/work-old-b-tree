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
static char *rcsid = "$Header: /atreus/mjr/hacks/btree/btdbmlib/RCS/stcopy.c,v 1.1 89/10/24 10:09:13 mjr Rel $";
#endif

/*
	make a copy of a record. returns an error if there is not enough
	space for the copy. otherwise it will read and write buffers as
	necessary to copy the entire IN-USE portion of a record. bytes
	that are not in-use are not copied, which allows records to be
	"shrunk" in this manner.
*/

store_copy(r,rec1,rec2)
STORE	*r;
sto_ptr		rec1;
sto_ptr		rec2;
{
	off_t	currof = 0L;

	if(rec1 == rec2)
		return(STO_OK);

	while(1) {
		int	ismore;
		int	byts;

		ismore = store_read(r,rec1,currof,store_buffer(r),store_bufsiz(r),&byts);
		if(ismore == STO_ERR)
			return(STO_ERR);

		if(store_write(r,rec2,currof,store_buffer(r),byts) == STO_ERR)
			return(STO_ERR);

		currof += (off_t)byts;

		if(ismore == STO_OK)
			break;
	}
	return(STO_OK);
}
