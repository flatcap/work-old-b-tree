#include	<sys/types.h>
#include	<sys/file.h>
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
static char *rcsid = "$Header: /atreus/mjr/hacks/btree/btlib/RCS/btopen.c,v 1.1 89/10/24 10:09:00 mjr Rel $";
#endif

extern	char	*malloc();
extern	off_t	lseek();


BT_INDEX	*
bt_open(path,flags,mode)
char	*path;
int	flags;
int	mode;
{
	BT_INDEX	*ret;
	struct bt_cache *cp1;
	struct bt_cache *cp2;
	int		r;

	if((ret = (BT_INDEX *)malloc(sizeof(BT_INDEX))) == NULL)
		return(NULL);

	/* clear error */
	bt_errno(ret) = BT_NOERROR;

	/* set funcp to something inoffensive */
	bt_cmpfn(ret) = 0;

	if((bt_fileno(ret) = open(path,(flags|O_RDWR),mode)) < 0)
		goto bombout;

	r = read(bt_fileno(ret),(char *)&ret->sblk,sizeof(struct bt_super));

	/* failure to read anything - initialize tree */
	if(r == 0) {
		char	*jnk;
		if((jnk = malloc((unsigned)BT_DFLTPSIZ)) != NULL) {
			ret->sblk.magic = BT_DFLTMAGIC;
			bt_pagesiz(ret) = BT_DFLTPSIZ;
			bt_dtype(ret) = BT_DFLTDTYPE;
			ret->sblk.levs = 1;
			ret->sblk.root = (off_t)BT_DFLTPSIZ;
			ret->sblk.free = BT_NULL;
			ret->sblk.high = (off_t)(2 * BT_DFLTPSIZ);

			/* mark super block as dirty and sync */
			ret->dirt = 1;

			/* write and pretend we read a legit superblock */
			if(bt_wsuper(ret) == BT_OK)
				r = sizeof(struct bt_super);

			/* now make jnk into an empty first page */
			KEYCNT(jnk) = 0;
			KEYLEN(jnk) = 0;
			LSIB(jnk) = RSIB(jnk) = BT_NULL;
			HIPT(jnk) = BT_NULL;

			/* now, since the cache is not set up yet, we */
			/* must directly write the page. */
			if(lseek(bt_fileno(ret),(off_t)BT_DFLTPSIZ,SEEK_SET) != (off_t)BT_DFLTPSIZ ||
				write(bt_fileno(ret),jnk,BT_DFLTPSIZ) != BT_DFLTPSIZ)
				r = 0;
			(void)free(jnk);
		}
	}

	/* yet another sanity check */
	if(r != sizeof(struct bt_super) || ret->sblk.magic != BT_DFLTMAGIC)
		goto bombout;

	/* initialize locator stack */
	ret->shih = ret->sblk.levs + 2;
	ret->stack = (struct bt_stack *)malloc((unsigned)(ret->shih * sizeof(struct bt_stack)));
	if(ret->stack == NULL)
		goto bombout;

	/* initialize cache */
	cp2 = ret->lru = NULL;
	for(r = 0; r < BT_MINCACHE + BT_DFLTCACHE; r++) {
		cp1 = (struct bt_cache *)malloc(sizeof(struct bt_cache)); 
		if(cp1 == NULL)
			goto bombout;

		if(ret->lru == NULL)
			ret->lru = cp1;
		cp1->prev = cp2;
		cp1->num = BT_NULL;
		cp1->next = NULL;
		cp1->flags = BT_CHE_CLEAN;
		if((cp1->p = (bt_chrp)malloc((unsigned)bt_pagesiz(ret))) == NULL)
			goto bombout;
		if(cp2 != NULL)
			cp2->next = cp1;

		cp2 = cp1;
	}
	ret->mru = cp1;

	/* set cache type flag */
	ret->cflg = BT_DFLTCFLAG;

	/* no valid current key */
	ret->cpag = BT_NULL;

	/* all is well */
	return(ret);

bombout:
	(void)bt_close(ret);
	return(NULL);
}
