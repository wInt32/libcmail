# warn about outdated make version
MIN_MAKEVERSION = 4.3
ifneq ($(MIN_MAKEVERSION),$(firstword $(sort $(MAKE_VERSION) $(MIN_MAKEVERSION))))
$(warning GNU Make versions older than $(MIN_MAKEVERSION) are not supported (you are running GNU make $(MAKE_VERSION)).)
endif

# we don't want implicit rules
MAKEFLAGS += -r

# detect platform
ifeq ($(OS),Windows_NT)
$(warning building on Windows is not currently supported)
else
    ifeq ($(target),)
    PLATFORM := $(shell sh -c "uname -s 2>/dev/null || echo unknown" | tr '[:upper:]' '[:lower:]')
    endif
endif

# Linux specific config
ifeq ($(target),linux)
endif

# Windows specific config
ifeq ($(target),win32)
TOOLCHAIN_PREFIX ?= x86_64-w64-mingw32-
PLATFORM_BIN_EXT = .exe
PLATFORM = win32
PLATFORM_LDFLAGS = -lws2_32
endif

# toolchain
CC := $(TOOLCHAIN_PREFIX)cc
AR := $(TOOLCHAIN_PREFIX)ar
PKG_CONFIG := $(TOOLCHAIN_PREFIX)pkg-config

FINAL_CFLAGS := -fPIC
FINAL_CFLAGS += $(CFLAGS)

# debug/release mode
ifeq ($(debug),1)
FINAL_CFLAGS += -g
else
FINAL_CFLAGS += -O3
endif

ifneq ($(PLATFORM),)
PLATFORM_DIRNAME := $(addsuffix /,$(PLATFORM))
PLATFORM_SUFFIX := $(addprefix -,$(PLATFORM))
endif

# create necessary folders
$(shell sh -c "test -d target/$(PLATFORM_DIRNAME)examples/ || mkdir -p target/$(PLATFORM_DIRNAME)examples/")
$(shell sh -c "test -d build/$(PLATFORM_DIRNAME) || mkdir -p build/$(PLATFORM_DIRNAME)")

EXAMPLES := $(basename $(notdir $(wildcard examples/*.c)))
EXAMPLES_C := $(addsuffix .c,$(addprefix examples/,$(EXAMPLES)))
EXAMPLES_BIN := $(addsuffix $(PLATFORM_BIN_EXT),$(addprefix target/$(PLATFORM_DIRNAME)examples/,$(EXAMPLES)))

RELEASE_ZIP := target/release$(PLATFORM_SUFFIX).zip
LIBCMAIL_A := target/$(PLATFORM_DIRNAME)libcmail.a
CMAILH := target/$(PLATFORM_DIRNAME)cmail.h

SRC := $(wildcard src/* external/*)
SRC_C := $(wildcard src/*.c external/*.c)
SRC_H := $(wildcard src/*.h external/*.h)
BUILD_O := $(addprefix build/$(PLATFORM_DIRNAME),$(addsuffix .o,$(basename $(notdir $(SRC_C)))))

EXAMPLES_LDFLAGS := -L target/$(PLATFORM_DIRNAME) -lcmail $(shell $(PKG_CONFIG) libcurl --libs)
ifneq ($(PLATFORM_LDFLAGS),)
EXAMPLES_LDFLAGS += $(PLATFORM_LDFLAGS)
endif

_C_CFG := $(PLATFORM) $(CC) $(FINAL_CFLAGS) $(EXAMPLES_LDFLAGS)
ifeq ($(debug),1)
_C_CFG += debug
all: examples
else
_C_CFG += release
all: release
endif

# if the configuration changed, update source files' timestamps to rebuild
_PREV_CFG = $(file < .makecfg)
ifneq ($(_PREV_CFG),$(_C_CFG))
$(shell find examples external src -mindepth 1 -name *.[c\|h] -exec touch -c {} +)
endif
$(file > .makecfg,$(_C_CFG))

release: target/sources.tar.gz $(RELEASE_ZIP)

target/sources.tar.gz: $(SRC) $(wildcard examples/*.c) .gitignore LICENSE Makefile README.md
	tar czf target/sources.tar.gz $^

$(RELEASE_ZIP): $(LIBCMAIL_A) $(CMAILH) LICENSE README.md
ifeq ($(debug),1)
	$(error can't build release zip in debug mode)
endif
	zip -quj $(RELEASE_ZIP) $^

$(CMAILH): src/cmail.h
	cp src/cmail.h $(CMAILH)

$(LIBCMAIL_A): $(BUILD_O)
	$(AR) cr $(LIBCMAIL_A) $^

$(BUILD_O): $(SRC_C)
	$(CC) $< -o $@ -c $(FINAL_CFLAGS)

$(SRC_C): $(SRC_H)

examples: $(EXAMPLES_BIN)

$(EXAMPLES_BIN): $(EXAMPLES_C)
    ifneq ($(shell $(PKG_CONFIG) libcurl --exists && echo libcurl),libcurl)
	$(error libcurl not found by pkg-config! Use your distro's package manager to install it)
    endif
	$(CC) $< -o $@ $(FINAL_CFLAGS) -DCMAIL_BUILD -I target/$(PLATFORM_DIRNAME) $(EXAMPLES_LDFLAGS)

$(EXAMPLES_BIN): .EXTRA_PREREQS = $(LIBCMAIL_A) $(CMAILH) examples/credentials.txt

# Ensure credentials.txt exists
examples/credentials.txt:
    ifeq ($(wildcard examples/credentials.txt),)
	$(warning To build the examples, you must supply examples/credentials.txt as follows:)
	$(warning const char *client_id = "your_google_oauth2_client_id";)
	$(warning const char *client_secret = "your_google_oauth2_client_secret";)
	$(warning const char *user_email_address = "your_google_email_address";)
	$(warning Support for non-google email servers will be added soon.)
	$(error Missing examples/credentials.txt)
    endif
	touch $@

.PHONY: clean
.IGNORE: clean

clean:
ifneq ($(wildcard build),)
	rm build -r
endif
ifneq ($(wildcard target),)
	rm target -r
endif

distclean: clean
ifneq ($(wildcard .makecfg),)
	rm .makecfg
endif