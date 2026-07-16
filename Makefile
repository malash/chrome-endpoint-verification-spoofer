SDK  := $(shell xcrun --sdk macosx --show-sdk-path)
FRAMEWORKS := -framework CoreFoundation -framework IOKit -framework Foundation

DYLIB    := build/spoof-lib.dylib
LAUNCHER := build/spoof
CHECK    := build/check

all: $(DYLIB) $(LAUNCHER)

$(DYLIB): src/spoof-lib.c
	@mkdir -p build
	clang -arch x86_64 -arch arm64 -arch arm64e -isysroot $(SDK) \
	  -dynamiclib $(FRAMEWORKS) $< -o $@

$(LAUNCHER): src/spoof.c
	@mkdir -p build
	clang -arch x86_64 -arch arm64 -isysroot $(SDK) $< -o $@

$(CHECK): tests/check.c
	@mkdir -p build
	clang $(FRAMEWORKS) $< -o $@

test: all $(CHECK)
	@echo "== baseline =="; $(CHECK)
	@echo "== via launcher =="; $(LAUNCHER) $(CHECK)

clean:
	rm -rf build

.PHONY: all test clean
