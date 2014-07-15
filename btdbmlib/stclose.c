#include	<stdio.h>
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
static char *rcsid = "$Header: /atreus/mjr/hacks/btree/btdbmlib/RCS/stclose.c,v 1.1 89/10/24 10:09:13 mjr Rel $";
#endif

/*
	close a store file and deallocate any buffers, etc
*/

store_close(r)
STORE	*r;
{
	int	rv = STO_OK;

	if(close(store_fileno(r)) != 0)
		rv = STO_ERR;

	if(store_buffer(r) != NULL)
		free((char *)store_buffer(r));
	free((char *)r);
	return(rv);
}
