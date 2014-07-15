#ifndef	_DEF_BT_INTERN_H

/*
         (C) Copyright, 1988, 1989 Marcus J. Ranum
                    All rights reserved


          This software, its documentation,  and  supporting
          files  are  copyrighted  material  and may only be
          distributed in accordance with the terms listed in
          the COPYRIGHT document.
*/


/*
	$Header: /atreus/mjr/hacks/btree/btlib/RCS/btintern.h,v 1.1 89/10/24 10:09:08 mjr Rel $
	THIS SHOULD NOT BE INCLUDED BY USER-LEVEL PROGRAMS
*/

#define	BT_NULL	((off_t)-1L)
#define	BT_FREE	((off_t)-2L)

/* make life easier with function pointers */
typedef	int	(*FUNCP)();

/* cache flags - not needed by user code */
#define	BT_CHE_CLEAN	0
#define	BT_CHE_DIRTY	1
#define	BT_CHE_LOCKD	2

/* forward decls for internal funcs only */
extern	struct bt_cache *bt_rpage();
extern	off_t		bt_newpage();
extern	void		bt_inspg();
extern	void		bt_splpg();

#ifndef	NO_BT_DEBUG
extern	void		bt_dump();
#endif

/*
minumum allowable page cache. since page sizes are variable,
the cache is also used to provide buffers for insertion and
deletion, or splitting. if there are not enough, nothing works
this value was determined from intimate knowledge of the code
*/
#define	BT_MINCACHE	4


/*
 Macros used in manipulating btree pages.

 this stuff is the machine specific part - if these macros are
 not right, you are guaranteed to know about it right away when
 this code is run. If you know exact values, you can plug them
 in directly, otherwise, we guess. getting it wrong will waste
 some space, is all. note that the off_ts are clustered at the
 beginning of the page, so that there is less likely to be a
 problem with the ints following. this may cause trouble in some
 odd architectures, and if so, fix it, and let me know.

 debugging a page is hard to do, by its very nature, and it is
 equally hard to build consistency checks into the code to
 validate pages as they are read and written. there is some
 code in bt_rpage() and bt_wpage() that looks for glaringly hosed
 pages, but if something gets past them, serious problems are
 sure to follow.

	Don't even *THINK* about running this past "lint -h" !!!!!

	These macros wind up nesting ~5 layers deep and get pretty
	hairy. If your c-preprocessor is not gutsy enough, have fun
	with the cut and paste !

	Makeup of a page:
	a page is an unstructured mess stuffed into a character buffer
	with all locations being determined via pointer arithmetic.
	this structure is loosely based on Zoellic and Folk's text
	"File Structures: a conceptual toolkit". further, there is
	no distinction between internal pages and leaf pages except
	that the value in the high pointer will be BT_NULL

	fields are (in order)
	- how many -- data type (size) ---------value/description-----
	1	|	off_t	|	page left sibling
	1	|	off_t	|	page right sibling
	1	|	off_t	|	page "high" (overflow) ptr
	1	|	int	|	count of keys in page - keycnt
	1	|	int	|	total length of keys in page
	keycnt	|	int	|	length index (one per key)
	keycnt	|	off_t	|	child/data ptrs (one per key)

	Ideally, pages should be flagged depending on type, and if
	the page contains fixed-size objects (structs, or ints, etc)
	the length index should be left out, saving sizeof(int) 
	bytes/key, which would be susbstantial improvement. This is
	an enhancement considered for release 2.0 if ever, or is
	left as an exercise for the reader.

	--mjr();
*/

/* alignment boundary for off_ts */
#define	ALIGN	sizeof(off_t)
/*
#define	ALIGN	(sizeof(off_t)/sizeof(char))
*/

/*
the actual alignments - the bit with the u_long may break on
some systems. Whatever, it needs to be a size that can give a valid
modulo operator on an address (Suns don't like modulo of pointers)
*/
#define	DOALIGN(s)	(s + (ALIGN - ((u_long)s % ALIGN)))

/* page siblings */
#define	LSIB(p)		(*(off_t *)(p))
#define	RSIB(p)		(*(off_t *)(p + sizeof(off_t)))
#define	HIPT(p)		(*(off_t *)(p + (2 * sizeof(off_t))))

/* count of keys in page, stored in first integer value */
#define	KEYCNT(p)	(*(int *)(p + (3 * sizeof(off_t))))

/* length of keys in page, stored in second integer value */
#define	KEYLEN(p)	(*(int *)(p + sizeof(int) + (3 * sizeof(off_t))))

/* address of key string (pointer to char) */
#define	KEYADR(p)	(p + (2 * sizeof(int)) + (3 * sizeof(off_t)))

/* address of first (int) key index value */
#define	KEYIND(p)	DOALIGN((KEYADR(p) + KEYLEN(p)))

/* address of first (off_t) child pointer value */
#define	KEYCLD(p)	(KEYIND(p) + (KEYCNT(p) * sizeof(int)))

/* # of bytes used by page pointers, sibs, etc */
#define PTRUSE		((2 * sizeof(int)) + (4 * sizeof(off_t)))

/* # of bytes used out of a page */
#define KEYUSE(p)	((KEYCNT(p) * (sizeof(off_t) + sizeof(int))) + PTRUSE + KEYLEN(p))

#define	_DEF_BT_INTERN_H
#endif
