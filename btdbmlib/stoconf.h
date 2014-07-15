#ifndef	_DEF_STO_CONF_H

/*
         (C) Copyright, 1988, 1989 Marcus J. Ranum
                    All rights reserved


          This software, its documentation,  and  supporting
          files  are  copyrighted  material  and may only be
          distributed in accordance with the terms listed in
          the COPYRIGHT document.
*/


/*
	$Header: /atreus/mjr/hacks/btree/btdbmlib/RCS/stoconf.h,v 1.1 89/10/24 10:09:20 mjr Rel $

	THIS SHOULD NOT BE INCLUDED BY USER-LEVEL PROGRAMS

	All reasonably user-configurable options are in this file.

	If there is something that needs to be special-cased for
	a system, put it here, PLEASE!
*/


/*
default magic number to use for a record store file
*/
#define	STO_DFLTMAGIC	0x72252


/*
SEEK_SET should be whatever your systems version of lseek() takes
to tell it to go to an exact offset. zero is pretty standard.
*/
#ifndef	SEEK_SET
#define	SEEK_SET	0
#endif

#define	_DEF_STO_CONF_H
#endif
