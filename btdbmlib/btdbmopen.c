#include	<stdio.h>
#include	<sys/types.h>
#include	<btdbm.h>


/*
         (C) Copyright, 1988, 1989 Marcus J. Ranum
                    All rights reserved


          This software, its documentation,  and  supporting
          files  are  copyrighted  material  and may only be
          distributed in accordance with the terms listed in
          the COPYRIGHT document.
*/


#ifndef	lint
static char *rcsid = "$Header: /atreus/mjr/hacks/btree/btdbmlib/RCS/btdbmopen.c,v 1.1 89/10/24 10:09:12 mjr Rel $";
#endif


extern	char	*malloc();
extern	char	*strcat();
extern	char	*strcpy();

BTDBM	*
btdbm_open(path,type,flags,mode)
char	*path;
int	type;
int	flags;
int	mode;
{
	BTDBM	*ret;
	char	nbuf[BUFSIZ];

	if((ret = (BTDBM *)malloc(sizeof(BTDBM))) == NULL)
		return(NULL);

	(void)strcpy(nbuf,path);
	(void)strcat(nbuf,".ndx");

	if((ret->bt = bt_optopen(	BT_PATH,	nbuf,
					BT_DTYPE,	type,
					BT_OMODE,	flags,
					BT_OPERM,	mode,
		0)) == NULL)
			goto bombout;

	(void)strcpy(nbuf,path);
	(void)strcat(nbuf,".dat");

	if((ret->st = store_open(nbuf,flags,mode)) == NULL)
		goto bombout;

	/* OK */
	btdbm_clearerror(ret);
	return(ret);

bombout:
	free((char *)ret);
	return(NULL);
}
