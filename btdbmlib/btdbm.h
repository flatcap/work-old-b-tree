#ifndef	_DEF_BTDBM_H

#ifndef	_DEF_BTREE_H
#include	<btree.h>
#endif

#ifndef	_DEF_STO_H
#include	<store.h>
#endif

/*
         (C) Copyright, 1988, 1989 Marcus J. Ranum
                    All rights reserved


          This software, its documentation,  and  supporting
          files  are  copyrighted  material  and may only be
          distributed in accordance with the terms listed in
          the COPYRIGHT document.
*/


/*
	$Header: /atreus/mjr/hacks/btree/btdbmlib/RCS/btdbm.h,v 1.1 89/10/24 10:09:19 mjr Rel $
*/

#define	BTDBM_INSERT	0
#define	BTDBM_REPLACE	1

typedef	struct	{
	bt_chrp	dptr;
	int	dsize;
} btdatum;

struct	btdbm	{
	BT_INDEX	*bt;
	STORE		*st;
	int		errno;
};
#define	BTDBM	struct	btdbm

#define	btdbm_error(db)		(db->errno)
#define	btdbm_clearerror(db)	(db->errno = 0)
#define	btdbm_btree(db)		(db->bt)
#define	btdbm_storefile(db)	(db->bt)

extern	BTDBM	*btdbm_open();
extern	btdatum	btdbm_fetch();
extern	btdatum	btdbm_nextkey();
extern	btdatum	btdbm_firstkey();

#define	_DEF_BTDBM_H
#endif
