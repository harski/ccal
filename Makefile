
PREFIX = /usr/local
BIN = hcal
SRCDIR = src
MAKE = make

all: $(SRCDIR)/$(BIN)

$(SRCDIR)/$(BIN):
	$(MAKE) --directory=$(SRCDIR)

install: $(SRCDIR)/$(BIN)
	install -m 0755 $(SRCDIR)/$(BIN) $(PREFIX)/bin/$(BIN)

clean:
	$(MAKE) --directory=$(SRCDIR) clean

.PHONY: all clean install

