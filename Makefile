
PREFIX = /usr/local
BIN = ccal
SRCDIR = src
MAKE = make

all: $(SRCDIR)/$(BIN)

$(SRCDIR)/$(BIN):
	$(MAKE) --directory=$(SRCDIR)

install: $(SRCDIR)/$(BIN)
	install -m 0755 $(SRCDIR)/$(BIN) $(PREFIX)/bin/$(BIN)

clean:
	$(MAKE) --directory=$(SRCDIR) clean

distclean: clean
	$(MAKE) --directory=$(SRCDIR) distclean
	rm $(SRCDIR)/Makefile

.PHONY: all clean distclean install

