#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

#[allow(dead_code)]
mod internal {
    include!(concat!(env!("OUT_DIR"), "/bindings.rs"));
}

use internal::*;
use std::ffi::*;
use std::os::raw::*;
use std::string::*;

pub type GLProcLoader = unsafe extern "C" fn(name: *const c_char) -> *mut c_void;

pub fn get_last_error() -> Option<String>
{
    /* TODO FIXME Test runner fails for some reason
    unsafe
    {
        let ptr = BLZ_GetLastError().as_ref();
        ptr.map(|val| CStr::from_ptr(val).to_str().unwrap().to_owned())
    }
    */
    panic!("Not implemented");
}

pub fn load(loader: GLProcLoader) -> Result<(), String>
{
    unsafe {
        match BLZ_Load(Some(loader)) {
            x if x > 0 => Ok(()),
            _ => panic!(get_last_error())
        }
    }
}

pub fn set_viewport(width: u32, height: u32)
{
    unsafe {
        BLZ_SetViewport(width as i32, height as i32);
    }
}

#[cfg(test)]
mod tests {
    use crate::*;
    use sdl2::video::GLProfile;
    use sdl2::sys::SDL_GL_GetProcAddress;

    const WINDOW_WIDTH: u32 = 512;
    const WINDOW_HEIGHT: u32 = 512;

    #[test]
    pub fn test_all()
    {
        let context = sdl2::init().unwrap();
        let video_sys = context.video().unwrap();
        let gl_attr = video_sys.gl_attr();
        gl_attr.set_context_profile(GLProfile::Core);
        gl_attr.set_context_version(3, 3);
        let window = video_sys.window("Test", WINDOW_WIDTH, WINDOW_HEIGHT)
            .opengl()
            .build()
            .unwrap();
        let _ctx = window.gl_create_context().unwrap();
        match load(SDL_GL_GetProcAddress) {
            Ok(_) => {},
            Err(e) => panic!(e)
        }
        set_viewport(WINDOW_WIDTH, WINDOW_HEIGHT);

        assert!(true);
    }
}
