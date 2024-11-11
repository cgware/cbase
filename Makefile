TOPDIR := $(CURDIR)/

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

CBASE_SRCDIR := src
CBASE_SRC := $(wildcard ${CBASE_SRCDIR}/*.c)
CBASE_INCLUDE := $(wildcard include/*.h)
CBASE_INCLUDE += $(wildcard ${CBASE_SRCDIR}/*.h)

CBASE_OUTDIR := $(TOPDIR)bin/cbase/$(ARCH)-$(CONFIG)/
CBASE_INTDIR := $(CBASE_OUTDIR)int/
CBASE_OBJ := $(patsubst %.c,$(CBASE_INTDIR)%.o,$(CBASE_SRC))
CBASE_GCDA := $(patsubst %.o,%.gcda,$(CBASE_OBJ))

CBASE_INCLUDES := -Isrc/ -Iinclude/

CBASE := cbase.a

.PHONY: cbase
cbase: $(CBASE_OUTDIR)$(CBASE)

$(CBASE_OUTDIR)$(CBASE): $(CBASE_OBJ)
	@mkdir -p $(@D)
	@ar rcs $@ $(CBASE_OBJ)

$(CBASE_INTDIR)%.o: %.c $(CBASE_INCLUDE)
	@mkdir -p $(@D)
	@$(TCC) -m$(BITS) -c $(CBASE_INCLUDES) $(CFLAGS) -o $@ $<

CBASE_TEST_SRCDIR := test
CBASE_TEST_SRC := $(wildcard ${CBASE_TEST_SRCDIR}/*.c)
CBASE_TEST_INCLUDE := $(wildcard include/*.h)
CBASE_TEST_INCLUDE += $(wildcard ${CBASE_TEST_SRCDIR}/*.h)

CBASE_TEST_OUTDIR := $(TOPDIR)bin/cbase_test/$(ARCH)-$(CONFIG)/
CBASE_TEST_INTDIR := $(CBASE_TEST_OUTDIR)int/
CBASE_TEST_OBJ := $(patsubst %.c,$(CBASE_TEST_INTDIR)%.o,$(CBASE_TEST_SRC))
CBASE_TEST_GCDA := $(patsubst %.o,%.gcda,$(CBASE_TEST_OBJ))

CBASE_TEST_INCLUDES := -Itest/ -Iinclude/
CBASE_TEST_LIBS := -L$(CBASE_OUTDIR) -l:$(CBASE)

CBASE_TEST := cbase_test

.PHONY: cbase_test
cbase_test: cbase $(CBASE_TEST_OUTDIR)$(CBASE_TEST)

$(CBASE_TEST_OUTDIR)$(CBASE_TEST): $(CBASE_TEST_OBJ)
	@mkdir -p $(@D)
	@$(TCC) -m$(BITS) $(CBASE_TEST_OBJ) $(CBASE_TEST_LIBS) $(LDFLAGS) -o $@

$(CBASE_TEST_INTDIR)%.o: %.c $(CBASE_TEST_INCLUDE)
	@mkdir -p $(@D)
	@$(TCC) -m$(BITS) -c $(CBASE_TEST_INCLUDES) $(CFLAGS) -o $@ $<

test: cbase_test
	@rm -rf $(CBASE_TEST_GCDA) $(CBASE_GCDA)
	@$(CBASE_TEST_OUTDIR)$(CBASE_TEST)

coverage: test
	@lcov -q -c -d $(TOPDIR) -o $(TOPDIR)/bin/lcov.info
	@genhtml -q $(TOPDIR)/bin/lcov.info -o $(TOPDIR)/report/coverage
ifeq ($(SHOW), true)
	@open $(TOPDIR)/report/coverage/index.html
endif
