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
static char *rcsid = "$Header: /atreus/mjr/hacks/btree/btlib/RCS/btio.c,v 1.1 89/10/24 10:08:59 mjr Rel $";
#endif

extern	off_t	lseek();

/*
	all read/write operations are in this file, as well as cache
	management stuff. the only exception to this is in btopen,
	where we write an initial page if creating a tree

	how caching is done in the btree routines: in case you care.
	------------------------------------------------------------
	the bt_rpage and bt_wpage functions do NOT fill a buffer for
	other btree functions to call - they return a pointer to a
	filled cache buffer, which can be operated on and then written
	back out. if an internal function requests page #BT_NULL, that
	tells bt_rpage to just get the least recently used page, OR
	any other scratch pages it has hanging around. preference is
	to re-use a scribbled-on page rather than having to flush a
	real page to disk and (possibly) have to read it right back.
	a flag is raised to mark scratch page buffers as in use, since
	there can be more than one page #BT_NULL and they are always
	maintained at the least-recently used end of the queue.
	writing is done just the opposite: if the page is not BT_NULL
	the page may be synced to disk (depending on cache flags)
	and will be re-inserted at the most-recently used end of the
	cache. if the page IS BT_NULL, any misc. flags get cleared
	(such as the 'this scratch page is in use' flag) and the
	buffer is re-enqueued at the least-recently used.
	the goal of all this stuff is to allow a calling function
	to grab a page in a buffer, request 2 scratch pages, split the
	first into the two scratches, mark the first (original) as
	junk (since the page is now invalidated) and to 'write' it as
	BT_NULL, while 'write'ing the 2 former scratch buffers as real
	pages. All without doing a damn thing but juggling pointers.
*/


bt_wsuper(b)
BT_INDEX	*b;
{
	/* write a superblock to disk if and only if it is dirty */
	/* keeps files opened O_RDONLY from choking up blood */
	if(b->dirt &&
		lseek(bt_fileno(b),0L,SEEK_SET) != 0L ||
		write(bt_fileno(b),(char *)&b->sblk,sizeof(struct bt_super)) != sizeof(struct bt_super))
			return(BT_ERR);
	b->dirt = 0;

	return(BT_OK);
}


bt_sync(b)
BT_INDEX	*b;
{
	struct	bt_cache *p1;

	/* flip through the cache and flush any pages marked dirty to disk */

	for(p1 = b->lru; p1 != NULL;) {
		if(p1->flags == BT_CHE_DIRTY && p1->num != BT_NULL) {
			if(lseek(bt_fileno(b),p1->num,SEEK_SET) != p1->num ||
				write(bt_fileno(b),(char *)p1->p,bt_pagesiz(b)) != bt_pagesiz(b))
				return(BT_ERR);
		}
		p1 = p1->next;
	}
	return(BT_OK);
}


static void
bt_requeue(b,p)
BT_INDEX	*b;
struct bt_cache	*p;
{
	/* re-assign the cache buffer to a new (or possibly the same) */
	/* location in the queue. buffers that have been stolen for */
	/* scrap will have a BT_NULL number, and should just go at the */
	/* least-recently used part of the cache, since they are junk. */
	/* buffers with legit values get moved to the most-recently. */
	/* this could all be inlined where appropriate, but gimme a */
	/* break! call it, uh, modular programming. this is still */
	/* plenty quick, I expect. */

	/* if the element is where it already belongs, waste no time */
	if((p->num == BT_NULL && p == b->lru) ||
		(p->num != BT_NULL && p == b->mru))
			return;

	/* unlink */
	if(p->prev != NULL)
		p->prev->next = p->next;
	if(p->next != NULL)
		p->next->prev = p->prev;
	if(p == b->lru)
		b->lru = p->next;
	if(p == b->mru)
		b->mru = p->prev;

	/* relink depending on number of page this buffer contains */
	if(p->num == BT_NULL) {
		b->lru->prev = p;
		p->next = b->lru;
		p->prev = NULL;
		b->lru = p;
	} else {
		b->mru->next = p;
		p->prev = b->mru;
		p->next = NULL;
		b->mru = p;
	}
}


struct bt_cache *
bt_rpage(b,num)
BT_INDEX	*b;
off_t		num;
{
	struct bt_cache	*p;

	/* sanity check - only BT_NULL is allowed to be a page less */
	/* than 0 and no pages are allowed to be read past EOF. */
	if(num == 0L || num >= b->sblk.high || (num < 0L && num != BT_NULL) || (num != BT_NULL && (num % bt_pagesiz(b)) != 0)) {
		bt_errno(b) = BT_PTRINVAL;
		return(NULL);
	}
	
	/* scan the cache for the desired page. if it is not there, */
	/* load the desired stuff into the lru. if the requested page */
	/* is BT_NULL we could probably cheat by just using the lru, */
	/* but keeping the code smaller and cleaner is probably nicer */
	for(p = b->mru; p != NULL; p = p->prev) {

		/* if the page number matches and the buffer is not */
		/* marked as an allocated scratch buffer, return it */
		if(num == p->num && p->flags != BT_CHE_LOCKD) {
			/* if it is a scratch buffer, lock it */
			if(p->num == BT_NULL)
				p->flags = BT_CHE_LOCKD;

			/* recache the page (moves it to mru) */
			bt_requeue(b,p);
			return(p);
		}
	}
	
	/* obviously, we didnt find it, so flush the lru and read */
	/* one complication is to make sure we dont clobber allocated */
	/* scratch page buffers. we have to seek backwards until we */
	/* get a page that is not locked */
	for(p = b->lru; p != NULL; p = p->next)
		if(p->flags != BT_CHE_LOCKD)
			break;
	
	/* sanity check */
	if(p == NULL) {
		bt_errno(b) = BT_NOBUFFERS;
		return(NULL);
	}

	/* flush if the page is dirty and not a scratch buffer */
	if(p->num != BT_NULL && p->flags == BT_CHE_DIRTY) {
		if(lseek(bt_fileno(b),p->num,SEEK_SET) != p->num ||
			write(bt_fileno(b),(char *)p->p,bt_pagesiz(b)) != bt_pagesiz(b))
			return(NULL);
	}

	/* read new data if not a scratch buffer */
	if(num != BT_NULL) {
		int	ku;

		if(lseek(bt_fileno(b),num,SEEK_SET) != num ||
			read(bt_fileno(b),(char *)p->p,bt_pagesiz(b)) != bt_pagesiz(b))
			return(NULL);
		p->flags = BT_CHE_CLEAN;
		p->num = num;

		/* sanity check */
		ku = KEYUSE(p->p);
		if(ku > bt_pagesiz(b) || ku < 0 || KEYLEN(p->p) < 0) {
			bt_errno(b) = BT_BADPAGE;
			return(NULL);
		}
	} else {
		p->flags = BT_CHE_LOCKD;
		p->num = BT_NULL;
	}

	bt_requeue(b,p);
	return(p);
}



bt_wpage(b,p)
BT_INDEX	*b;
struct bt_cache	*p;
{

	if(p->num != BT_NULL) {
		int	ku;

		/* sanity check */
		ku = KEYUSE(p->p);
		if(ku > bt_pagesiz(b) || ku < 0 || KEYLEN(p->p) < 0) {
			bt_errno(b) = BT_BADPAGE;
			return(BT_ERR);
		}

		if(b->cflg != BT_RWCACHE && p->flags == BT_CHE_DIRTY) {
			if(lseek(bt_fileno(b),p->num,SEEK_SET) != p->num ||
				write(bt_fileno(b),(char *)p->p,bt_pagesiz(b)) != bt_pagesiz(b))
				return(BT_ERR);
			p->flags = BT_CHE_CLEAN;
		}
	} else {
		/* unlock scratch buffer */
		p->flags = BT_CHE_CLEAN;
	}

	bt_requeue(b,p);
	return(BT_OK);
}



bt_freepage(b,pag)
BT_INDEX	*b;
off_t		pag;
{
	/* simple free page management is done with a linked */
	/* list linked to the left sibling of each page */
	struct	bt_cache *p;

	if((p = bt_rpage(b,pag)) == NULL)
		return(BT_ERR);
	
	LSIB(p->p) = b->sblk.free;

	/* note - this value is SET but never checked. Why ? because the */
	/* in-order insert can't be bothered to reset all the free pointers */
	/* in the free list. this could be useful, however, if someone ever */
	/* writes a tree-patcher program, or something like that. */
	HIPT(p->p) = BT_FREE;

	p->flags = BT_CHE_DIRTY;
	if(bt_wpage(b,p) == BT_ERR)
		return(BT_ERR);

	b->sblk.free = pag;
	b->dirt = 1;
	return(bt_wsuper(b));
}


off_t
bt_newpage(b)
BT_INDEX	*b;
{
	off_t	ret = BT_NULL;
	struct	bt_cache *p;

	if(b->sblk.free == BT_NULL) {
		ret = b->sblk.high;
		b->sblk.high += bt_pagesiz(b);
	} else {
		if((p = bt_rpage(b,b->sblk.free)) != NULL) {
			ret = b->sblk.free;
			b->sblk.free = LSIB(p->p);
		} else {
			return(BT_NULL);
		}
	}
	b->dirt = 1;
	if(bt_wsuper(b) == BT_ERR)
		return(BT_ERR);
	return(ret);
}
