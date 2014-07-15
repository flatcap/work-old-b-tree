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
static char *rcsid = "$Header: /atreus/mjr/hacks/btree/btlib/RCS/btfind.c,v 1.1 89/10/24 10:08:57 mjr Rel $";
#endif

bt_find(b,key,len,rrn)
BT_INDEX	*b;
bt_chrp		key;
int		len;
off_t		*rrn;
{
	struct	bt_cache *op;	/* old page */
	int	sr;

	if(len > BT_MAXK(b)) {
		bt_errno(b) == BT_KTOOBIG;
		return(BT_ERR);
	}

	if(len <= 0) {
		bt_errno(b) == BT_ZEROKEY;
		return(BT_ERR);
	}

	if(bt_seekdown(b,key,len) == BT_ERR)
		return(BT_ERR);

	if((op = bt_rpage(b,b->stack[b->sblk.levs - 1].pg)) == NULL)
		return(BT_ERR);

	b->cpag = op->num;

	/* mark all as well with tree */
	bt_clearerr(b);

	sr = bt_srchpg(key,len,bt_dtype(b),b->cmpfn,rrn,&b->cky,op->p);
	if(sr == BT_OK)
		return(BT_OK);
	if(sr == BT_ERR) {
		bt_errno(b) = BT_PAGESRCH;
		return(BT_ERR);
	}
	b->cky--;
	return(BT_NF);
}
