#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

#[macro_use]
extern crate bitflags;
extern crate bytes;
#[macro_use]
extern crate enum_primitive;

pub mod blend;
pub mod dynamic;
pub mod texture;

mod internal;

use crate::internal::wrap_result;
pub use internal::BLZ_Rectangle as Rectangle;
pub use internal::BLZ_SpriteQuad as Quad;
pub use internal::BLZ_Vector2 as Vector2;
pub use internal::BLZ_Vector4 as Vector4;
pub use internal::BLZ_Vertex as Vertex;

use internal::*;
use std::ffi::*;
use std::os::raw::*;
use std::string::*;

#[derive(Debug, Copy, Clone)]
pub struct Color {
    pub r: f32,
    pub g: f32,
    pub b: f32,
    pub a: f32,
}

impl From<Vector4> for Color {
    fn from(vector: Vector4) -> Self {
        Color { r: vector.x, g: vector.y, b: vector.z, a: vector.w }
    }
}

impl From<Color> for Vector4 {
    fn from(color: Color) -> Self {
        Vector4 { x: color.r, y: color.g, z: color.b, w: color.a }
    }
}

enum_from_primitive! {
    #[derive(Debug, Copy, Clone)]
    pub enum SpriteFlip
    {
        None = BLZ_SpriteFlip_NONE as isize,
        FlipH = BLZ_SpriteFlip_FLIP_H as isize,
        FlipV = BLZ_SpriteFlip_FLIP_V as isize,
        Both = BLZ_SpriteFlip_BOTH as isize
    }
}

pub type GLProcLoader = unsafe extern "C" fn(name: *const c_char) -> *mut c_void;
pub type CallResult = Result<(), String>;

pub fn get_last_error() -> Option<String> {
    unsafe {
        let ptr = BLZ_GetLastError().as_ref();
        ptr.map(|val| CStr::from_ptr(val).to_str().unwrap().to_owned())
    }
}

pub fn load(loader: GLProcLoader) -> CallResult {
    unsafe {
        match BLZ_Load(Some(loader)) {
            x if x > 0 => Ok(()),
            _ => panic!(get_last_error().unwrap_or("Unknown error".to_string())),
        }
    }
}

pub fn set_viewport(width: u32, height: u32) -> CallResult {
    unsafe { wrap_result(BLZ_SetViewport(width as i32, height as i32)) }
}

pub fn set_clear_color(color: Color) {
    unsafe {
        BLZ_SetClearColor(color.into());
    }
}

pub fn clear() {
    unsafe {
        BLZ_Clear();
    }
}
