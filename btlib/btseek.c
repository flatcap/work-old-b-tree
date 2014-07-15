#include	<sys/types.h>
#include	<stdio.h>
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


#ifndef	lint
static char *rcsid = "$Header: /atreus/mjr/hacks/btree/btlib/RCS/btseek.c,v 1.1 89/10/24 10:09:06 mjr Rel $";
#endif

extern	char	*realloc();

bt_seekdown(b,key,len)
BT_INDEX	*b;
bt_chrp		key;
int		len;
{
	/* bt_find just seeks down to leaf level, sets the stack, ends */

	int	l = 0;
	int	sr;
	struct	bt_cache *p;

	b->stack[0].pg = b->sblk.root;

	while(1) {
		if((p = bt_rpage(b,b->stack[l].pg)) == NULL)
			return(BT_ERR);
		if(HIPT(p->p) == BT_NULL) {
			return(BT_OK);
		}
		sr = bt_srchpg(key,len,bt_dtype(b),b->cmpfn,&b->stack[l + 1].pg,&b->stack[l].ky,p->p);
		if(sr == BT_ERR) {
			bt_errno(b) = BT_PAGESRCH;
			return(BT_ERR);
		}

		/* dynamically deal w/ stack overruns */
		if(l == b->shih - 2) {
			b->shih += 3;
			b->stack = (struct bt_stack *)realloc((char *)b->stack,(unsigned)(b->shih * sizeof(struct bt_stack)));
			if(b->stack == NULL)
				return(BT_ERR);
		}

		l++;
	}
}
