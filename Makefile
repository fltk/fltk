SHELL=/bin/sh

all : src fluid test
shared : srcshared fluid test
lib : SRC
src : SRC

SRC : makeinclude src/makedepend
	cd src && $(MAKE)
src/makedepend :
	touch src/makedepend
srcshared : makeinclude src/makedepend
	cd src && $(MAKE) shared

fluid : FLUID
FLUID : makeinclude fluid/makedepend
	cd fluid && $(MAKE)
fluid/makedepend :
	touch fluid/makedepend

test : TEST
TEST : makeinclude
	cd test && $(MAKE)

install : SRC FLUID
	cd src && $(MAKE) install
	cd fluid && $(MAKE) install

uninstall : makeinclude src/makedepend fluid/makedepend
	cd src && $(MAKE) uninstall
	cd fluid && $(MAKE) uninstall

clean :
	-@ rm -f *.o core *~
	cd src && $(MAKE) clean
	cd fluid && $(MAKE) clean
	cd test && $(MAKE) clean

distclean : clean
	-@ rm config.* makeinclude

realclean : distclean

include ./version
dist :
	./makedist $(VERSION).$(REVISION)

makeinclude :
	./configure
