#include	<stdio.h>
#include	<sys/types.h>
#include	"btconf.h"
#include	"btree.h"
#include	"btintern.h"

/*
         (C) Copyright, 1988, 1989 Marcus J. Ranum
                    All rights reserved


          This software, its documentation,  and  supporting
          files  are  copyrighted  material  and may only be
          distributed in accordance with the terms listed in
          the COPYRIGHT document.
*/


#ifdef	YES_BT_DEBUG
#ifndef	lint
static char *rcsid = "$Header: /atreus/mjr/hacks/btree/btlib/RCS/btdebug.c,v 1.1 89/10/24 10:08:55 mjr Rel $";
#endif

void
bt_dump(b,p)
BT_INDEX	*b;
struct	bt_cache *p;
{
	int	x;
	int	rl;
	off_t	rv;
	char	buf[BT_DFLTPSIZ];

	if(KEYUSE(p->p) > bt_pagesiz(b))
		(void)printf("WARNING overfull page!!!\n");


	(void)printf("Dump:page=%ld, cnt=%d, len=%d, ",
		p->num,KEYCNT(p->p),KEYLEN(p->p));

	(void)printf("lsib=%ld, rsib=%ld, high=%ld, used=%d\n",
		LSIB(p->p),RSIB(p->p),HIPT(p->p),KEYUSE(p->p));

	if(BT_MAXK(b) > sizeof(buf)) {
		(void)printf("page size too big to dump keys (sorry)\n");
		return;
	}

	/* print keys/len/chld index */
	for(x = 0; x < KEYCNT(p->p); x++) {
		if(bt_keyof(x,p->p,(bt_chrp)buf,&rl,&rv,(int)sizeof(buf)) == BT_ERR) {
			(void)printf("cannot access key #%d\n",x);
			return;
		}

		switch(bt_dtype(b)) {
			case	BT_STRING:
				buf[rl] = '\0';
				(void)printf("key=\"%s\",len=%d,rrv=%ld",buf,rl,rv);
				break;

			case	BT_INT:
				(void)printf("key=%d,len=%d,rrv=%ld",*(int *)buf,rl,rv);
				break;

			case	BT_LONG:
				(void)printf("key=%ld,len=%d,rrv=%ld",*(long *)buf,rl,rv);
				break;

			case	BT_FLOAT:
				(void)printf("key=%f,len=%d,rrv=%ld",*(float *)buf,rl,rv);
				break;

			case	BT_USRDEF:
				(void)printf("key=<usrdef>,len=%d,rrv=%ld",rl,rv);
				break;
		}

		if(x % 4 == 3)
			(void)printf("\n");
		else
			if(x != KEYCNT(p->p) - 1)
				(void)printf(", ");
	}
	(void)printf("\n");
}
#endif
