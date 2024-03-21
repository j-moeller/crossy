extern crate libc;
use libc::{c_char,size_t};
use serde_json::Value; // library to be tested
use common::Status;

#[no_mangle]
pub extern "C" fn rust_run(
    buf: *const c_char,
    size: size_t,
    out_buf: *mut *const c_char,
    out_size: *mut size_t,
) -> i32 {
    if buf == std::ptr::null()
        || out_buf == std::ptr::null_mut()
        || out_size == std::ptr::null_mut()
    {
        // one of the provided pointers is null
        return Status::ToolchainError as i32;
    }

    // get a rust slice from the provided input data
    let data: &[u8] = unsafe { std::slice::from_raw_parts(buf as *const u8, size as usize) };

    // ---------- JSON lib specific part starts here ----------

    // deserialize to JSON value
    let json: Value = match serde_json::from_slice(data) {
        Ok(v) => v,
        Err(_error) => return Status::ParserError as i32,
    };

    // serialize back to JSON value
    let mut serialized: Vec<u8> = match serde_json::to_vec(&json) {
        Ok(v) => v,
        Err(_error) => return Status::ParserError as i32,
    };

    // ---------- JSON lib specific part ends here ----------

    serialized.shrink_to_fit(); // shrink vec to min size
    let ptr = serialized.as_mut_ptr(); // get pointer on vec

    unsafe {
        // set out_buf on the serialized result
        *out_buf = ptr as *const c_char;
        *out_size = serialized.len() as size_t;
    }

    std::mem::forget(serialized); // prevent deallocation of the result
    return Status::ParserOkay as i32;
}

