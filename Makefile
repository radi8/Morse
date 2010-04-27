TOPDIR := $(shell pwd)

SUBDIRS += test_morse
SUBDIRS += test_teach

MAKEFILES = $(foreach dir,$(SUBDIRS),$(dir)/Makefile)
all clean: $(MAKEFILES)
	for dir in $(SUBDIRS); do make -C $(TOPDIR)/$$dir $@; done

$(MAKEFILES):
	@for dir in $(SUBDIRS); do cd $(TOPDIR)/$$dir; qmake-qt4; done
