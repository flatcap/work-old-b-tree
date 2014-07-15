#ifndef	_DEF_BTREE_H

/*
         (C) Copyright, 1988, 1989 Marcus J. Ranum
                    All rights reserved


          This software, its documentation,  and  supporting
          files  are  copyrighted  material  and may only be
          distributed in accordance with the terms listed in
          the COPYRIGHT document.
*/


/*
	$Header: /atreus/mjr/hacks/btree/btlib/RCS/btree.h,v 1.1 89/10/24 10:09:07 mjr Rel $
*/


/*
there are some potential problems with just using char * instead of
unsigned char *, because of sign extension and so on. the following
typedef indicates the best type to use as a pointer to an arbitrary
chunk of data for the system. since a lot of pointer math is done on
these, it should be either a signed or unsigned char, or odd results
may occur.
typedef	char *		bt_chrp;
*/
typedef	unsigned char *	bt_chrp;


/* return codes */
#define	BT_OK	0
#define	BT_ERR	-1
#define	BT_NF	1
#define	BT_EOF	2
#define	BT_BOF	3

/* arguments to bt_optopen */
#define	BT_PATH		1
#define	BT_PSIZE	2
#define	BT_CACHE	3
#define	BT_CFLAG	4
#define	BT_OMODE	5
#define	BT_OPERM	6
#define	BT_MAGIC	7
#define	BT_DTYPE	8
#define	BT_LABEL	9

/* cache modes, acceptable argument to BT_CFLAG */
#define	BT_NOCACHE	0
#define	BT_ROCACHE	1
#define	BT_RWCACHE	2

/* recognized data types, acceptable argument to BT_DTYPE */
#define	BT_STRING	0
#define	BT_INT		1
#define	BT_LONG		2
#define	BT_FLOAT	3
#define	BT_USRDEF	4

/* cache handle */
struct	bt_cache {
	char	flags;
	bt_chrp	p;
	off_t	num;
	struct	bt_cache *next;
	struct	bt_cache *prev;
};

/* super block */
struct	bt_super {
	long	magic;
	int	psiz;
	int	levs;
	int	dtyp;
	off_t	root;
	off_t	free;
	off_t	high;
};

struct	bt_stack {
	off_t	pg;
	int	ky;
};

/* b+tree index handle */
struct	bt_index {
	int	fd;
	int	errno;
	char	dirt;
	int	cflg;
	int	cky;
	off_t	cpag;
	int	(*cmpfn)();
	struct	bt_super sblk;
	struct	bt_cache *lru;
	struct	bt_cache *mru;
	struct	bt_stack *stack;
	int	shih;
};
#define	BT_INDEX	struct bt_index

/* pseudo functions */
#define	bt_fileno(b)	(b->fd)
#define	bt_pagesiz(b)	(b->sblk.psiz)
#define	bt_dtype(b)	(b->sblk.dtyp)
#define	bt_cmpfn(b)	(b->cmpfn)
#define	bt_errno(b)	(b->errno)
#define	bt_clearerr(b)	(b->errno = BT_NOERROR)

/* biggest size of a key - depends on the page size of the tree */
#define	BT_MAXK(b)	(((b->sblk.psiz-(4*sizeof(int))-(5*sizeof(off_t)))/2) - sizeof(off_t))

/* size of page 0 label - depends on the page size and sblk size */
#define	BT_LABSIZ(b)	(b->sblk.psiz - (sizeof(struct bt_super) + 1))

/* forward decls. */
extern	BT_INDEX	*bt_open();
extern	BT_INDEX	*bt_optopen();
extern	void		bt_perror();

/* btree error codes and messages */
extern	char	*bt_errs[];

/* error constants */
#define	BT_NOERROR	0
#define	BT_KTOOBIG	1
#define	BT_ZEROKEY	2
#define	BT_DUPKEY	3
#define	BT_PTRINVAL	4
#define	BT_NOBUFFERS	5
#define	BT_LTOOBIG	6
#define	BT_BTOOSMALL	7
#define	BT_BADPAGE	8
#define	BT_PAGESRCH	9
#define	BT_BADUSERARG	10

#define	_DEF_BTREE_H
#endif
