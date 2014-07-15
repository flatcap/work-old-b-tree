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
# $Header: /atreus/mjr/hacks/btree/doc/RCS/Makefile,v 1.5 89/10/23 17:19:35 mjr Rel $
#

# formatter to use
FMT= psroff

# formatter flags
FMTFLAGS= -man

# where to install the man pages.
MANDIR=	/usr/man/man3
MAN=	$(MANDIR)/btree.3

DOCS=	btdbm.3 \
	btree.3 \
	store.3

all:	

clean:
	rm -f store.txt btree.txt

ci:	clean
	ci -u $(DOCS) Makefile  < /dev/null

print:
	$(FMT) $(FMTFLAGS) $(DOCS)

ascii:
	nroff $(FMTFLAGS) btree.3 > btree.txt
	nroff $(FMTFLAGS) store.3 > store.txt
