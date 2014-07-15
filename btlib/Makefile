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
# $Header: /atreus/mjr/hacks/btree/btlib/RCS/Makefile,v 1.1 89/10/24 10:09:08 mjr Rel $
#

# compiler to use
CC= cc

# compiler flags
#CFLAGS= -p
CFLAGS= -O

LDFLAGS= -g

# where to build the library. must be set in testrack/Makefile
# if you change it from here.
LIBDIR=..
LIB=	$(LIBDIR)/libbtree.a

CFILES=	btclose.c \
	btdebug.c \
	btdelete.c \
	bterrors.c \
	btfind.c \
	btgoto.c \
	btinsert.c \
	btio.c \
	btlabel.c \
	btload.c \
	btopen.c \
	btoopen.c \
	btpage1.c \
	btpage2.c \
	btravrs.c \
	btseek.c \
	btzap.c

HFILES=	btconf.h \
	btree.h \
	btintern.h

OFILES=	btclose.o \
	btdebug.o \
	btdelete.o \
	bterrors.o \
	btfind.o \
	btgoto.o \
	btio.o \
	btinsert.o \
	btlabel.o \
	btload.o \
	btopen.o \
	btoopen.o \
	btpage1.o \
	btpage2.o \
	btravrs.o \
	btseek.o \
	btzap.o

FILES=	$(CFILES) \
	$(HFILES) \
	README \
	Makefile

$(LIB):	$(OFILES) 
	ar crv $(LIB) $(OFILES)
	ranlib $(LIB)

$(OFILES): $(HFILES)

clean:
	rm -f *.o $(LIB) core tags

ci:
	ci -u $(FILES) < /dev/null

tags:
	ctags $(CFILES)

lint:	$(CFILES)
	lint -u $(CFILES) | sed '/pointer alignment/d'
