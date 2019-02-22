#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]
#![allow(dead_code)]

use crate::{get_last_error, CallResult};
use std::os::raw::c_int;

include!(concat!(env!("OUT_DIR"), "/bindings.rs"));

#[inline]
pub fn wrap_result(return_code: c_int) -> CallResult {
    if return_code > 0 {
        return Ok(());
    }
    Err(get_last_error().unwrap_or("Unknown error".to_string()))
}

#[inline]
pub fn try_get_err() -> String {
    get_last_error().unwrap_or("Unknown error".to_owned())
}

pub trait AsRaw<T> {
    unsafe fn as_raw_mut(&self) -> *mut T;
    unsafe fn as_raw(&self) -> *const T;
}

impl<T> AsRaw<T> for Option<T> {
    unsafe fn as_raw_mut(&self) -> *mut T {
        self.as_raw() as *mut T
    }

    unsafe fn as_raw(&self) -> *const T {
        self.as_ref().map_or(std::ptr::null(), |x| x)
    }
}
