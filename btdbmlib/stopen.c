#include	<stdio.h>
#include	<sys/types.h>
#include	<sys/file.h>
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
static char *rcsid = "$Header: /atreus/mjr/hacks/btree/btdbmlib/RCS/stopen.c,v 1.1 89/10/24 10:09:16 mjr Rel $";
#endif

/*
	open a store file and allocate all the buffers, etc.
*/

extern	char	*malloc();

STORE	*
store_open(path,flags,mode)
char	*path;
int	flags;
int	mode;
{
	STORE	*ret;
	int		r;

	if((ret = (STORE *)malloc(sizeof(STORE))) == NULL)
		return(NULL);

	if((store_buffer(ret) = (unsigned char *)malloc((unsigned)STO_BUFSIZ)) == NULL)
		return(NULL);
	store_bufsiz(ret) = STO_BUFSIZ;

	if((store_fileno(ret) = open(path,flags|O_RDWR,mode)) < 0)
		goto bombout;

	r = read(store_fileno(ret),(char *)&ret->sblk,sizeof(struct sto_sb));

	/* initialize */
	if(r == 0) {
		ret->sblk.magic = STO_DFLTMAGIC;
		ret->sblk.high = (off_t)sizeof(struct sto_sb);
		ret->sblk.free = STO_NULL;
		ret->sblk.label = STO_NULL;
		if(store_wsuper(ret) != STO_OK)
			goto bombout;
	}

	if(ret->sblk.magic != STO_DFLTMAGIC)
		goto bombout;

	ret->currec = STO_NULL;
	ret->curlst = STO_NULL;

	store_clearerr(ret);

	/* all is well. */
	return(ret);

	bombout:
		free((char *)ret);
		return(NULL);
}
