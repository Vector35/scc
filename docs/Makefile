
HTML := $(patsubst %.txt,%.html,$(wildcard *.txt))
TXT := $(wildcard *.txt)

all: $(HTML)
.PHONY: all clean

$(HTML): %.html: %.txt docs.conf Makefile
	asciidoc -b html5 --conf-file=docs.conf -a stylesheet="$(CURDIR)/lib/v35.css" -a icons $<

clean:
	rm -f *.html

