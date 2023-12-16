
PLUGIN=my_sfi_pass.so
SOURCES=\
        my_sfi_pass.cc \
		$(END)

include ../Makefile.common

.PHONY: test
test: $(PLUGIN)
	$(CCPLUGIN) -c -o /dev/null test.c
