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
#     * Neither the name of the <ORGANIZATION> nor the names of its
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

all:: rmtree

.PHONY: clean distclean realclean test

SUBDIRS=test

PROGS=$(PROG1)

PROG1=rmtree
OBJ1=rmtree.o

CXXFLAGS=-O2
CXXDBGFLAGS=-g

ifeq ($(notdir $(CXX)),g++)
GCCWARN=-Wall -Wextra
CXXFLAGS+=-MMD $(GCCWARN)
CXXDBGFLAGS+=-MMD $(GCCWARN)
endif

$(PROG1): $(OBJ1)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $(PROG1) $(OBJ1) $(LOADLIBES) $(LDLIBS)

debug: CXXFLAGS=$(CXXDBGFLAGS)
debug: all

test:
	@$(MAKE) -C test test

clean realclean distclean::
	@for dir in $(SUBDIRS) ; do \
	   $(MAKE) -C $$dir $@ || true ; \
	done;

clean::
	@-rm *.o *~

distclean:: clean
	@-rm $(PROGS)
	@-rm *.d

realclean:: distclean

-include $(OBJ1:.o=.d) $(OBJ2:.o=.d)
