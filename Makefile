GCC := gcc
OPT := -g0 -O3 -z relro -z noexecstack -z now -fPIE -fstack-protector-strong -Wall -Wextra
LDFLAGS :=
TARGET := shell

SRCDIR := src
INCDIR := $(SRCDIR)/../include
BUILDDIR := build
SRC_FILES := $(wildcard $(SRCDIR)/*.c)
OBJ_FILES := $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(SRC_FILES))
GCCFLAGS := $(OPT) -I$(INCDIR) -I$(INCDIR)/internal -I..
TESTBUILDDIR := $(BUILDDIR)/test
TESTDIR := test
TEST_SUBDIRS := $(wildcard $(TESTDIR)/*)
TEST_FILES := $(wildcard $(TESTDIR)/*.c)
TEST_OBJ_FILES := $(patsubst $(TESTDIR)/%/%.c,$(TESTBUILDDIR)/%.o,$(TEST_FILES))

$(BUILDDIR)/$(TARGET): $(OBJ_FILES)
	$(GCC) $(GCCFLAGS) $^ -o $@ -lreadline
	rm -rf ./build/*.o

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	$(GCC) $(GCCFLAGS) -c $< -o $@ -lreadline

define test-dir-rule
SUBDIR_OBJ := $$(patsubst $(1)/%.c,$(TESTBUILDDIR)/$(notdir $(1))/%.o,$$(wildcard $(1)/*.c))

$(TESTBUILDDIR)/$(notdir $(1)).exe: $$(SUBDIR_OBJ)
	$(GCC) $(GCCFLAGS) $$^ -o $$@

$$(SUBDIR_OBJ): $(TESTBUILDDIR)/$(notdir $(1))/%.o: $(1)/%.c
	@if not exist "$(TESTBUILDDIR)/$(notdir $(1))" mkdir "$(TESTBUILDDIR)/$(notdir $(1))"
	$(GCC) $(GCCFLAGS) -c $$< -o $$@
endef

$(foreach subdir,$(TEST_SUBDIRS),$(eval $(call test-dir-rule,$(subdir))))
test: $(patsubst $(TESTDIR)/%,$(TESTBUILDDIR)/%,$(TEST_SUBDIRS))
	# for dir in build/test/*/ do rm -rf "$$dir" done;
	echo skip

clean:
	rm -rf ./build/**/*
	rm -rf ./build/*