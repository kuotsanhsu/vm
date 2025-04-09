# .DELETE_ON_ERROR:
.DEFAULT_GOAL := sign
CFLAGS += -Wnullable-to-nonnull-conversion
LDFLAGS += -framework Hypervisor
.PHONY: clean force-sign sign run
clean:
	rm -f main
force-sign: CODESIGN_EXTRA_FLAGS += --force
force-sign sign: main
	codesign --sign - --entitlements entitlements.plist $(CODESIGN_EXTRA_FLAGS) $<
run: main
	$(realpath $<)
