ALL = dirs target/cmail.h

LDFLAGS = -L target -l$(subst lib,,libcmail$(PSUFFIX))

ifeq ($(target),win32)
	PLATFORM = win32
	PSUFFIX := -$(PLATFORM)
	PDIR := $(PLATFORM)/
	TPREFIX = x86_64-w64-mingw32
	LDFLAGS += $$($(TPREFIX)-pkg-config libcurl --libs)
	CC = $(TPREFIX)-cc
	AR = $(TPREFIX)-ar
else
	PLATFORM = linux
	LDFLAGS += -lcurl
endif

EXAMPLES = $(wildcard examples/*.c)
EXAMPLES_BIN = $(addprefix target/examples/$(PDIR),$(notdir $(basename $(EXAMPLES))))

ALL += target/libcmail$(PSUFFIX).a
ALL += examples

all: $(ALL)

dirs:
ifeq ($(wildcard target/examples/$(PDIR).),)
	mkdir -p target/examples/$(PDIR)
endif
ifeq ($(wildcard build/$(PDIR).),)
	mkdir -p build/$(PDIR)
endif

FINAL_CFLAGS := -fPIC -O3
FINAL_CFLAGS += $(CFLAGS)

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
.SUFFIXES: .c

release: dirs target/release.zip target/sources.tar.gz

target/release$(PSUFFIX).zip: target/libcmail$(PSUFFIX).a target/cmail.h LICENSE README.md
	zip -quj target/release$(PSUFFIX).zip $^

target/sources.tar.gz: src/* external/* examples/* .gitignore LICENSE Makefile README.md
	tar czf target/sources.tar.gz $^

clean:
	rm build -vr
	rm target -vr