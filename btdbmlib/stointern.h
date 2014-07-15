#ifndef	_DEF_STO_INTERN_H

/*
         (C) Copyright, 1988, 1989 Marcus J. Ranum
                    All rights reserved


          This software, its documentation,  and  supporting
          files  are  copyrighted  material  and may only be
          distributed in accordance with the terms listed in
          the COPYRIGHT document.
*/


/*
	$Header: /atreus/mjr/hacks/btree/btdbmlib/RCS/stointern.h,v 1.1 89/10/24 10:09:21 mjr Rel $
	THIS SHOULD NOT BE INCLUDED BY USER-LEVEL PROGRAMS
*/


/*
	macros to control the layout of a free list buffer
*/
struct	sto_freeent {
	sto_ptr	addr;
	int	size;
};

/* the next free list buffer in chain */
#define	STO_FREENXT(b)	(*(off_t *)(b))

/* the number of free list entries in this buffer */
#define	STO_FREECNT(b)	(*(int *)(b + sizeof(off_t)))

/* the actual data area - note this is inherently aligned to an off_t */
#define	STO_FREEBUF(b)	((struct sto_freeent *)(b + (2 * sizeof(off_t))))

/* size of a record header, for the sake of abberviation */
#define	STO_HSIZ	(sizeof(struct sto_hdr))

/* size of a freelist entry, for the sake of abberviation */
#define	STO_FSIZ	(sizeof(struct sto_freeent))

/*
size at which to start fragmenting records internally. If the record
can be split, and have at least this much left, it will. Note that
the remaining value MUST include room for a record header block
*/
#define	STO_SPLITSIZ	(265 + STO_HSIZ)

#define	_DEF_STO_INTERN_H
#endif
