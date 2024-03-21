# activate sanitizer coverage in rust binary https://internals.rust-lang.org/t/improving-support-for-coverage-guided-fuzzing/15070
RUSTFLAGS := -Zsanitizer=address -Cpasses=sancov-module -Cllvm-args=-sanitizer-coverage-inline-8bit-counters -Cllvm-args=-sanitizer-coverage-pc-table  -Cllvm-args=-sanitizer-coverage-level=4
# currently not required: -Cllvm-args=-sanitizer-coverage-trace-pc-guard

ABSOLUTE_RUST_DIR := $(shell pwd)/rust

$(RUST_DIR)/%_adapter/target/debug/libadapter.a: $(RUST_DIR)/%_adapter/src/*
	cd $(RUST_DIR)/$*_adapter && RUSTFLAGS="$(RUSTFLAGS)" cargo build