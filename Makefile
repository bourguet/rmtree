# Makefile
#
# Copyright (c) 2004, 2013  Jean-Marc Bourguet
# 
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
# 
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
# 
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the
#       distribution.
# 
#     * Neither the name of Jean-Marc Bourguet nor the names of the other
#       contributors may be used to endorse or promote products derived
#       from this software without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
# IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
# LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
# NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

all:

SUBDIRS=src doc
project:=$(notdir $(shell pwd))
prefix:=/opt/bourguet/$(project)
bindir:=$(prefix)/bin
mandir:=$(prefix)/man
man1dir:=$(mandir)/man1

PROGS:=rmtree
MAN1PAGES:=rmtree.1

.PHONY: all doc install 
.PHONY: clean realclean distclean
.PHONY: clean.top realclean.top distclean.top

clean realclean distclean::
	@for dir in $(SUBDIRS); do \
	   $(MAKE) -C $$dir $@; \
	done

all:
	@$(MAKE) -C src all

doc:
	@$(MAKE) -C doc all

clean:: clean.top

distclean:: distclean.top

realclean:: realclean.top

clean.top:
	@-rm *~

distclean.top: clean.top

realclean.top: distclean.top

dist:
	$(MAKE) realclean
	$(MAKE) all
	$(MAKE) distclean
	cd .. ; tar cjfX $(project)-$(VERSION).tar.bz $(project)/excluded $(project)

install: all $(DESTDIR)$(bindir) $(DESTDIR)$(man1dir)
	cd src; cp $(PROGS) $(DESTDIR)$(bindir)
	cd doc; cp $(MANPAGES) $(DESTDIR)$(man1dir)

$(DESTDIR)$(bindir):
	@mkdir -p $@

$(DESTDIR)$(man1dir):
	@mkdir -p $@
