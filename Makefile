LDFLAGS = -L target -l$(subst lib,,libcmail$(PSUFFIX))

ifeq ($(target),win32)
	PLATFORM = win32
	PSUFFIX := -$(PLATFORM)
	PDIR := $(PLATFORM)/
	TPREFIX = x86_64-w64-mingw32
	LDFLAGS += $$($(TPREFIX)-pkg-config libcurl --libs) -lws2_32
	CC = $(TPREFIX)-cc
	AR = $(TPREFIX)-ar
else
	PLATFORM = linux
	LDFLAGS += -lcurl
endif

EXAMPLES = $(wildcard examples/*.c)
EXAMPLES_BIN = $(addprefix target/examples/$(PDIR),$(notdir $(basename $(EXAMPLES))))

FINAL_CFLAGS := -fPIC
FINAL_CFLAGS += $(CFLAGS)
ifeq ($(debug),1)
FINAL_CFLAGS += -g
else
FINAL_CFLAGS += -O3
endif

ALL = dirs .WAIT target/libcmail$(PSUFFIX).a target/cmail.h examples

ifneq ($(wildcard .makecflags),)
	ifneq ($(shell cat .makecflags),$(FINAL_CFLAGS))
	REBUILD = clean .WAIT
	endif
endif
ALL := $(REBUILD) $(ALL)

$(shell echo $(FINAL_CFLAGS) > .makecflags)

all: $(ALL)

dirs:
	@sh -c "test -d target/examples/$(PDIR) || mkdir -p target/examples/$(PDIR)"
	@sh -c "test -d build/$(PDIR) || mkdir -p build/$(PDIR)"

build/$(PDIR)cJSON.o: external/cJSON.c external/cJSON.h
	$(CC) external/cJSON.c -o build/$(PDIR)cJSON.o -c $(FINAL_CFLAGS)

target/cmail.h: src/cmail.h
	cp src/cmail.h target/cmail.h

build/$(PDIR)cmail.o: src/cmail.h src/cmail.c
	$(CC) src/cmail.c -o build/$(PDIR)cmail.o -c $(FINAL_CFLAGS)

target/libcmail$(PSUFFIX).a: build/$(PDIR)cmail.o build/$(PDIR)cJSON.o
	$(AR) cr target/libcmail$(PSUFFIX).a $^

examples: target/libcmail$(PSUFFIX).a target/cmail.h .WAIT $(EXAMPLES_BIN)

$(EXAMPLES_BIN): $(EXAMPLES)
	$(CC) examples/$(notdir $(basename $@)).c -o $@ $(FINAL_CFLAGS) -DCMAIL_BUILD -I target $(LDFLAGS)

.PHONY: clean
.IGNORE: clean

release: $(REBUILD) dirs target/release.zip target/sources.tar.gz

target/release$(PSUFFIX).zip: target/libcmail$(PSUFFIX).a target/cmail.h LICENSE README.md
	zip -quj target/release$(PSUFFIX).zip $^

target/sources.tar.gz: src/* external/* examples/* .gitignore LICENSE Makefile README.md
	tar czf target/sources.tar.gz $^

clean:
ifneq ($(wildcard build),)
	rm build -vr
endif
ifneq ($(wildcard target),)
	rm target -vr
endif

distclean: clean
ifneq ($(wildcard .makecflags),)
	rm .makecflags
endif