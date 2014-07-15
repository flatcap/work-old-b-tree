#include	<stdio.h>
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
static char *rcsid = "$Header: /atreus/mjr/hacks/btree/btdbmlib/RCS/streallocbuf.c,v 1.1 89/10/24 10:09:18 mjr Rel $";
#endif


extern	char	*realloc();

store_reallocbuf(r,siz)
STORE	*r;
int	siz;
{
	store_buffer(r) = (unsigned char *)realloc((char *)store_buffer(r),(unsigned)siz);
	if(store_buffer(r) == NULL)
		return(STO_ERR);
	store_bufsiz(r) = siz;
	return(STO_OK);
}
