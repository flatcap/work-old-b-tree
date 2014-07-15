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
static char *rcsid = "$Header: /atreus/mjr/hacks/btree/btlib/RCS/btgoto.c,v 1.1 89/10/24 10:08:58 mjr Rel $";
#endif


extern	char	*realloc();

bt_goto(b,d)
BT_INDEX	*b;
int		d;
{
	int	l = 0;
	struct	bt_cache *p;

	b->stack[0].pg = b->sblk.root;

	while(1) {
		if((p = bt_rpage(b,b->stack[l].pg)) == NULL)
			return(BT_ERR);

		/* dynamically deal w/ stack overruns */
		if(l == b->shih - 2) {
			b->shih += 3;
			b->stack = (struct bt_stack *)realloc((char *)b->stack,(unsigned)(b->shih * sizeof(struct bt_stack)));
			if(b->stack == NULL)
				return(BT_ERR);
		}

		/* set the key to be one AFTER or BEFORE the real */
		/* end of the tree. we preincrement keys when doing */
		/* next so this is OK to do, though a bit hokey */
		if(HIPT(p->p) == BT_NULL) {
			b->cpag = p->num;
			if(d == BT_EOF)
				b->cky =  KEYCNT(p->p);
			else
				b->cky =  -1;

			/* mark all as well with tree */
			bt_clearerr(b);
			return(BT_OK);
		}
	
		/* have not yet hit a leaf - go high or low, as approp. */
		if(d == BT_EOF || KEYCNT(p->p) == 0) {
			b->stack[++l].pg = HIPT(p->p);
		} else {
			b->stack[++l].pg = *(off_t *)KEYCLD(p->p);
		}
	}
}
