# 
#
#/*
#         (C) Copyright, 1988, 1989 Marcus J. Ranum
#                    All rights reserved
#
#
#          This software, its documentation,  and  supporting
#          files  are  copyrighted  material  and may only be
#          distributed in accordance with the terms listed in
#          the COPYRIGHT document.
#*/
#
# $Header: /atreus/mjr/hacks/btree/Makefile,v 1.1 89/10/24 10:09:36 mjr Rel $
#

MAKE= make

LIB=	libbtree.a

SRCDIR= /users/mjr/hacks/btree

all:	
	cd btlib; $(MAKE)
	cd btdbmlib; $(MAKE)
	cd utils; $(MAKE)
	cd doc; $(MAKE)

clean:
	cd btlib; $(MAKE) clean
	cd btdbmlib; $(MAKE) clean
	cd utils; $(MAKE) clean
	cd doc; $(MAKE) clean

ci:
	cd btlib; $(MAKE) ci
	cd btdbmlib; $(MAKE) ci
	cd utils; $(MAKE) ci
	cd doc; $(MAKE) ci
	ci -u Makefile README < /dev/null

lint:	
	cd btlib; $(MAKE) lint
	cd btdbmlib; $(MAKE) lint
