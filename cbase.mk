CBASE_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

CBASE_SRC := $(wildcard $(CBASE_DIR)src/*.c)
CBASE_INC := $(wildcard $(CBASE_DIR)src/*.h)
CBASE_INCLUDE := $(wildcard $(CBASE_DIR)include/*.h)

CBASE_OUTDIR := $(BUILDDIR)bin/cbase/$(ARCH)-$(CONFIG)
CBASE_INTDIR := $(CBASE_OUTDIR)/int/
CBASE_OBJ := $(patsubst $(CBASE_DIR)%.c,$(CBASE_INTDIR)%.o,$(CBASE_SRC))
CBASE_GCDA := $(patsubst %.o,%.gcda,$(CBASE_OBJ))

CBASE_INCLUDES := -I$(CBASE_DIR)src/ -I$(CBASE_DIR)include/

CBASE_NAME := cbase.a
CBASE := $(CBASE_OUTDIR)/$(CBASE_NAME)

.PHONY: cbase
cbase: $(CBASE)

$(CBASE): $(CBASE_OBJ)
	@mkdir -p $(@D)
	@ar rcs $@ $(CBASE_OBJ)

$(CBASE_INTDIR)%.o: $(CBASE_DIR)%.c $(CBASE_INC) $(CBASE_INCLUDE)
	@mkdir -p $(@D)
	@$(TCC) -m$(BITS) -c $(CBASE_INCLUDES) $(CFLAGS) -o $@ $<

CBASE_TEST_SRC := $(wildcard $(CBASE_DIR)test/*.c)
CBASE_TEST_INC := $(wildcard $(CBASE_DIR)test/*.h)
CBASE_TEST_INCLUDE := $(CBASE_INCLUDE)

CBASE_TEST_OUTDIR := $(BUILDIR)bin/cbase_test/$(ARCH)-$(CONFIG)
CBASE_TEST_INTDIR := $(CBASE_TEST_OUTDIR)/int/
CBASE_TEST_OBJ := $(patsubst $(CBASE_DIR)%.c,$(CBASE_TEST_INTDIR)%.o,$(CBASE_TEST_SRC))
CBASE_TEST_GCDA := $(patsubst %.o,%.gcda,$(CBASE_TEST_OBJ))

CBASE_TEST_INCLUDES := -I$(CBASE_DIR)test/ -I$(CBASE_DIR)include/
CBASE_TEST_LIBS := -L$(CBASE_OUTDIR) -l:$(CBASE_NAME)

CBASE_TEST := $(CBASE_TEST_OUTDIR)/cbase_test

.PHONY: cbase_test
cbase_test: $(CBASE_TEST)
	@rm -rf $(CBASE_GCDA) $(CBASE_TEST_GCDA)
	@$(CBASE_TEST)

$(CBASE_TEST): $(CBASE) $(CBASE_TEST_OBJ)
	@mkdir -p $(@D)
	@$(TCC) -m$(BITS) $(LDFLAGS) -o $@ $(CBASE_TEST_OBJ) $(CBASE_TEST_LIBS)

$(CBASE_TEST_INTDIR)%.o: $(CBASE_DIR)%.c $(CBASE_TEST_INC) $(CBASE_TEST_INCLUDE)
	@mkdir -p $(@D)
	@$(TCC) -m$(BITS) -c $(CBASE_TEST_INCLUDES) $(CFLAGS) -o $@ $<
