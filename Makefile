BUILDDIR := $(CURDIR)/
SRCDIR := $(CURDIR)/

TCC := $(CC)

ifeq ($(ARCH), x64)
BITS := 64
endif
ifeq ($(ARCH), x86)
BITS := 32
endif

ifeq ($(CONFIG), Debug)
CFLAGS := -Wall -Wextra -Werror -pedantic -O0 -ggdb -coverage
LDFLAGS := -coverage
endif
ifeq ($(CONFIG), Release)
CFLAGS := -Wall -Wextra -Werror -pedantic
LDFLAGS :=
endif

include $(SRCDIR)cbase.mk

.PHONY: test
test: cbase_test

.PHONY: coverage
coverage: test
	@lcov -q -c -o $(BUILDDIR)/bin/lcov.info -d $(CBASE_OUTDIR)
ifeq ($(SHOW), true)
	@genhtml -q -o $(BUILDDIR)/report/coverage $(BUILDDIR)/bin/lcov.info
	@open $(BUILDDIR)/report/coverage/index.html
endif
