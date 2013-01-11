# simple makefile wrapper to run waf
# https://ftp.samba.org/pub/unpacked/tdb/

WAF=WAF_MAKE=1 PATH=.:$$PATH waf

all:
	$(WAF) build

install:
	$(WAF) install

uninstall:
	$(WAF) uninstall

test: FORCE
	$(WAF) test $(TEST_OPTIONS)

testenv:
	$(WAF) test --testenv $(TEST_OPTIONS)

quicktest:
	$(WAF) test --quick $(TEST_OPTIONS)

dist:
	touch .tmplock
	WAFLOCK=.tmplock $(WAF) dist

distcheck:
	touch .tmplock
	WAFLOCK=.tmplock $(WAF) distcheck

clean:
	$(WAF) clean

distclean:
	$(WAF) distclean

configure:
	$(WAF) configure

reconfigure: configure
	$(WAF) reconfigure

show_waf_options:
	$(WAF) --help

# some compatibility make targets
everything: all

testsuite: all

check: test

torture: all

# this should do an install as well, once install is finished
installcheck: test

etags:
	$(WAF) etags

ctags:
	$(WAF) ctags

pydoctor:
	$(WAF) pydoctor

bin/%:: FORCE
	$(WAF) --targets=`basename $@`
FORCE:
