# How to connect a Rust Project to the Fuzzer

The process is explained on the example of the JSON library serde_json.
The Makefile expects that the rust lib which will be linked to *serde*.so is called *serde*_adapter.

## 1. Create the adapter

The goal is to create an adapter to the functionality to be tested.
The adapter should provide an interface in the form `int rust_run(buf: *const c_char, size: size_t, out_buf: *mut *const c_char, out_size: *mut size_t);`. \
It will then be compiled to a shared library that can be linked to the fuzzer.
The last two parameters are used to return a result bytearray to the differential fuzzer.
**Important:** This memory must be freed using the `rust_free` function from the *common* crate.

### 1.1 Initialize the adapter project

Create a new rust project with `cargo new <name>_adapter --lib`. \
The adapter for `serde` will be called [`serde_adapter`](./serde_adapter).\
Add the following dependencies to the Cargo.toml:

```toml
[dependencies]
serde_json = "1.0.86" # the fuzzing target goes here
libc = "0.2.0" # provides the data types required to interact with C/C++ code
common = { path = "../common" } # provides common status codes
```

In our example case, the code is published on crates.io and we can use this dependency.

Set the project to be compiled as a shared library:

```toml
[lib]
crate-type = ["dylib"]
```

The final example Cargo.toml can be found [here](./serde_adapter/Cargo.toml).

### 1.2 Implement `rust_run` in lib.rs

Example method:

```rust
extern crate libc;
use libc::c_char;
use common::Status;

#[no_mangle]
pub extern "C" fn rust_run(
   buf: *const c_char,
   size: size_t,
   out_buf: *mut *const c_char,
   out_size: *mut size_t
) -> i32 {
    if buf == std::ptr::null()
    {
        // the provided pointer is null
        return Status::ToolchainError as i32;
    }

    // get a rust slice from the provided input data
    let data: &[u8] = unsafe { std::slice::from_raw_parts(buf as *const u8, size as usize) };

    /*
    Process data here. On error return Status::ParserError
    */

    let mut result: Vec<u8>; // contains the data returned by the function

    result.shrink_to_fit(); // shrink vec to min size
    let ptr = result.as_mut_ptr(); // get pointer on vec

    unsafe {
        // set out_buf on the serialized result
        *out_buf = ptr as *const c_char;
        *out_size = result.len() as size_t;
    }

    std::mem::forget(result); // prevent deallocation of the result

    return Status::ParserOkay as i32;
}
```

Full example implementation can also be found [here](./serde_adapter/src/lib.rs). \
That's it. You just implemented the adapter.

## 2. Compile the adapter

Libfuzzer requires SanitizerCoverage for coverage guided fuzzing. \
Luckily we can activate basically the same coverage as if we were to compile C/C++ code with clang.
To achieve this, we pass flags to the LLVM backend of rustc that activate asan and SanitizerCoverage.

```bash
RUSTFLAGS="-Zsanitizer=address -Cpasses=sancov-module -Cllvm-args=-sanitizer-coverage-level=4 -Cllvm-args=-sanitizer-coverage-inline-8bit-counters -Cllvm-args=-sanitizer-coverage-pc-table" cargo build
```

These flags (hopefully) activate the same coverage features as `-fsanitize=fuzzer` which is currently not supported by rustc.

The [Makefile in this directory](./Makefile) provides a generic target for compiling an adapter crate to a shared lib.
[generic_target.cpp](../src/generic_target.cpp) provides a modified run method for rust targets, that uses `rust_free` to free the bytearrays allocated by the rust side.
The [Makefile in src](../src/Makefile) provides a generic make target to link the adapter lib and the generic_adapter.
A rust crate in [this directory](./) named *<name>*_adapter is automatically compiled to a shared lib and linked with the generic target to a shared lib named *<name>*.so if *name* is added to `ALL_LIBS` in [this Makefile](../Makefile).
