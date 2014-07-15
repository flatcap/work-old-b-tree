#ifndef	_DEF_STO_H

/*
         (C) Copyright, 1988, 1989 Marcus J. Ranum
                    All rights reserved


          This software, its documentation,  and  supporting
          files  are  copyrighted  material  and may only be
          distributed in accordance with the terms listed in
          the COPYRIGHT document.
*/


/*
	$Header: /atreus/mjr/hacks/btree/btdbmlib/RCS/store.h,v 1.1 89/10/24 10:09:20 mjr Rel $
*/

#define	STO_NULL	-1L

#define	STO_OK		0
#define	STO_ERR		1
#define	STO_MORE	2

/* STARTING size of the store file internal buffer */
/* this is also the size of the initial free list block */
#define	STO_BUFSIZ	1024

typedef	off_t	sto_ptr;

/* a record header. one before each record in the file */
struct	sto_hdr {
	off_t	magic;
	int	size;
	int	used;
	int	refs;
	sto_ptr	next;
	sto_ptr	prev;
};

/* store file super block */
struct	sto_sb {
	off_t	magic;
	off_t	high;
	sto_ptr	free;
	sto_ptr	label;
};

struct	store_hand {
	int		fd;
	int		errno;
	sto_ptr		currec;		/* current record */
	sto_ptr		curlst;		/* current list head */
	struct		sto_sb	sblk;
	int		bufsiz;	/* buffer size */
	unsigned	char	*buf;	/* general porpoise buffer */
					/* this buffer may "stretch" to */
					/* accomodate data */
};
#define	STORE struct store_hand

/* pseudo-functions */
#define	store_errno(r)	(r->errno)
#define	store_clearerr(r)		(r->errno = STO_NOERROR)
#define	store_fileno(r)	(r->fd)
#define	store_buffer(r)	(r->buf)
#define	store_bufsiz(r)	(r->bufsiz)
#define	store_currec(r)	(r->currec)
#define	store_label(r)	(r->sblk.label)

/* forward decls */
extern	STORE		*store_open();
extern	sto_ptr		store_alloc();
extern	sto_ptr		store_realloc();
extern	void		store_perror();

/* error codes and messages */
extern	char	*sto_errs[];

/* error constants */
#define	STO_NOERROR	0
#define	STO_BADHDR	1
#define	STO_IOERR	2
#define	STO_NOREC	3
#define	STO_TOOSMALL	4

#define	_DEF_STO_H
#endif
