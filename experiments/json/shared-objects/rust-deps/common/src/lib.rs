extern crate libc;
use libc::{c_char,size_t};

#[derive(Copy, Clone)]
pub enum Status {
    ParserOkay = 0,
    ParserError = 1,
    ToolchainError = -1,
}

#[no_mangle]
/// This is intended for the C code to call for deallocating the
/// Rust-allocated char array.
unsafe extern "C" fn rust_free(ptr: *mut c_char, len: size_t) {
    if ptr == std::ptr::null_mut() {
        return;
    }
    let ptr = ptr as *mut u8;
    let len = len as usize;
    drop(Vec::from_raw_parts(ptr, len, len));
}

