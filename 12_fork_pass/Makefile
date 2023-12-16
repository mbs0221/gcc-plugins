
PLUGIN=fork_pass.so
SOURCES=\
        fork_pass.cc \
		$(END)

include ../Makefile.common

.PHONY: test
test: $(PLUGIN)
	$(CCPLUGIN) -c -x c /dev/null -o /dev/null

all: hello_fork

hello_fork: hello_fork.c
	$(CCPLUGIN) -o $@ $^ -lvmpl

.PHONY: clean
clean:
	rm -f hello_fork $(PLUGIN)
