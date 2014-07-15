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
static char *rcsid = "$Header: /atreus/mjr/hacks/btree/btdbmlib/RCS/stwsuper.c,v 1.1 89/10/24 10:09:19 mjr Rel $";
#endif

/*
	synchronise a store file superblock.
*/

extern	off_t	lseek();

store_wsuper(r)
STORE	*r;
{
	if(lseek(store_fileno(r),0L,SEEK_SET) != 0L ||
		write(store_fileno(r),(char *)&r->sblk,sizeof(struct sto_sb)) != sizeof(struct sto_sb))
		return(STO_ERR);
	return(STO_OK);
}
