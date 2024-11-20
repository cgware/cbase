CBASE_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

CBASE_SRC := $(CBASE_DIR)src/
CBASE_C := $(wildcard $(CBASE_SRC)*.c)
CBASE_H := $(wildcard $(CBASE_SRC)*.h)
CBASE_HEADERS := $(wildcard $(CBASE_DIR)include/*.h)

CBASE_OUTDIR := $(BUILDDIR)bin/$(ARCH)-$(CONFIG)/cbase
CBASE_INTDIR := $(CBASE_OUTDIR)/int/
CBASE_OBJ := $(patsubst $(CBASE_SRC)%.c,$(CBASE_INTDIR)%.o,$(CBASE_C))
CBASE_GCDA := $(patsubst %.o,%.gcda,$(CBASE_OBJ))

CBASE_INCLUDES := -I$(CBASE_DIR)include/

CBASE := $(CBASE_OUTDIR)/cbase.a

.PHONY: cbase
cbase: $(CBASE)

$(CBASE): $(CBASE_OBJ)
	@mkdir -p $(@D)
	@ar rcs $@ $(CBASE_OBJ)

$(CBASE_INTDIR)%.o: $(CBASE_SRC)%.c $(CBASE_H) $(CBASE_HEADERS)
	@mkdir -p $(@D)
	@$(TCC) -m$(BITS) -c -I$(CBASE_SRC) $(CBASE_INCLUDES) $(CFLAGS) -o $@ $<

CBASE_TEST_SRC := $(CBASE_DIR)test/
CBASE_TEST_C := $(wildcard $(CBASE_TEST_SRC)*.c)
CBASE_TEST_H := $(wildcard $(CBASE_TEST_SRC)*.h)
CBASE_TEST_HEADERS := $(CBASE_HEADERS)

CBASE_TEST_OUTDIR := $(BUILDIR)bin/$(ARCH)-$(CONFIG)/cbase_test
CBASE_TEST_INTDIR := $(CBASE_TEST_OUTDIR)/int/
CBASE_TEST_OBJ := $(patsubst $(CBASE_TEST_SRC)%.c,$(CBASE_TEST_INTDIR)%.o,$(CBASE_TEST_C))
CBASE_TEST_GCDA := $(patsubst %.o,%.gcda,$(CBASE_TEST_OBJ))

CBASE_TEST_INCLUDES :=  $(CBASE_INCLUDES)
CBASE_TEST_LIBS := $(CBASE)

CBASE_TEST := $(CBASE_TEST_OUTDIR)/cbase_test

.PHONY: cbase_test
cbase_test: $(CBASE_TEST)
	@rm -rf $(CBASE_GCDA) $(CBASE_TEST_GCDA)
	@$(CBASE_TEST)

$(CBASE_TEST): $(CBASE_TEST_OBJ) $(CBASE_TEST_LIBS)
	@mkdir -p $(@D)
	@$(TCC) -m$(BITS) $(LDFLAGS) -o $@ $(CBASE_TEST_OBJ) $(patsubst %,-L%,$(dir $(CBASE_TEST_LIBS))) $(patsubst %,-l:%,$(notdir $(CBASE_TEST_LIBS)))

$(CBASE_TEST_INTDIR)%.o: $(CBASE_TEST_SRC)%.c $(CBASE_TEST_H) $(CBASE_TEST_HEADERS)
	@mkdir -p $(@D)
	@$(TCC) -m$(BITS) -c -I$(CBASE_TEST_SRC) $(CBASE_TEST_INCLUDES) $(CFLAGS) -o $@ $<
