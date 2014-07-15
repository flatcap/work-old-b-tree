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
static char *rcsid = "$Header: /atreus/mjr/hacks/btree/btlib/RCS/btdelete.c,v 1.1 89/10/24 10:08:56 mjr Rel $";
#endif


bt_delete(b,key,len)
BT_INDEX	*b;
bt_chrp		key;
int		len;
{
	struct	bt_cache	*op;
	int	staklev;
	off_t	dpag;		/* page to delete from */
	int	keyat;		/* key # to delete */
	off_t	ptj;		/* junk */
	int	sr;

	/* 90% of this code is straight from bt_insert(). the only */
	/* difference is that we need to ensure an exact match at */
	/* leaf level only, otherwise we just track our way up the */
	/* tree, deleting keys. page concatenation and redistribution */
	/* are not implemented, though if they are to be, they should */
	/* fit fairly easily in this framework. good luck, deletes suck */

	/* the usual consistency checks */
	if(len > BT_MAXK(b)) {
		bt_errno(b) = BT_KTOOBIG;
		return(BT_ERR);
	}

	if(len <= 0) {
		bt_errno(b) = BT_ZEROKEY;
		return(BT_ERR);
	}

	if(bt_seekdown(b,key,len) == BT_ERR)
		return(BT_ERR);

	/* set current stack level */
	staklev = b->sblk.levs - 1;
	dpag = b->stack[staklev].pg;

	if((op = bt_rpage(b,dpag)) == NULL)
		return(BT_ERR);

	/* locate first del point, check if not there */
	/* there must not be anything to delete. error. */
	sr = bt_srchpg(key,len,bt_dtype(b),b->cmpfn,&ptj,&keyat,op->p);
	if(sr == BT_NF)
		return(BT_NF);
	if(sr == BT_ERR) {
		bt_errno(b) = BT_PAGESRCH;
		return(BT_ERR);
	}

	/* invalidate current notion of where we are in the tree */
	b->cpag = BT_NULL;

	/* this loop should be comfortably repeated until we perform a */
	/* simple delete without emptying a page. if a page concatentation */
	/* function is added, we would look until we no longer concatenate */
	while(1) {

		/* oddball special case. if the page is root and */
		/* there is no key left to delete, the tree is */
		/* (by definition) empty, having only a high pointer */
		/* left. so, the high pointer is dropped, and */
		/* turned to BT_NULL, instantly converting root */
		/* back to a leaf page. we then return. done */
		if(dpag == b->sblk.root && KEYCNT(op->p) == 0) {
			HIPT(op->p) = BT_NULL;
			KEYLEN(op->p) = 0;
			op->flags = BT_CHE_DIRTY;
			if(bt_wpage(b,op) == BT_ERR)
				return(BT_ERR);
			b->sblk.levs = 1;
			if(bt_wsuper(b) == BT_ERR)
				return(BT_ERR);
			return(BT_OK);
		}

		/* if the page has more than one key OR is an index with */
		/* one key, just drop one key from the page, do not free it */
		/* also, if the page is the root page, do not do free it */
		if(KEYCNT(op->p) > 1 || dpag == b->sblk.root ||
			(KEYCNT(op->p) == 1 && HIPT(op->p) != BT_NULL)) {
			struct	bt_cache *tp;	/* temp copy page */

			if((tp = bt_rpage(b,BT_NULL)) == NULL)
				return(BT_ERR);
			bt_delpg(keyat,op->p,tp->p);

			/* swap the page numbers, invalidate the old, */
			/* mark the new as dirty to force a write */
			tp->num = op->num;
			tp->flags = BT_CHE_DIRTY;
			op->num = BT_NULL;

			if(bt_wpage(b,op) == BT_ERR || bt_wpage(b,tp) == BT_ERR)
				return(BT_ERR);

			/* mark all as well with tree */
			bt_clearerr(b);
			return(BT_OK);
		} else {
			off_t	savlft;
			off_t	savrt;

			savlft = LSIB(op->p);
			savrt = RSIB(op->p);

			if(savlft != BT_NULL) {
				if((op = bt_rpage(b,savlft)) == NULL)
					return(BT_ERR);
				RSIB(op->p) = savrt;
				op->flags = BT_CHE_DIRTY;
				if(bt_wpage(b,op) == BT_ERR)
					return(BT_ERR);
			}

			if(savrt != BT_NULL) {
				if((op = bt_rpage(b,savrt)) == NULL)
					return(BT_ERR);
				LSIB(op->p) = savlft;
				op->flags = BT_CHE_DIRTY;
				if(bt_wpage(b,op) == BT_ERR)
					return(BT_ERR);
			}

			/* sibs are fixed, now free the page */
			if(bt_freepage(b,dpag) == BT_ERR)
				return(BT_ERR);
		}

		/* still going, pop stack level */
		staklev--;

		/* set key location for next delete */
		keyat = b->stack[staklev].ky;
		dpag = b->stack[staklev].pg;

		/* allocate a scratch page and read the old */
		if((op = bt_rpage(b,dpag)) == NULL)
			return(BT_ERR);

	}
}
