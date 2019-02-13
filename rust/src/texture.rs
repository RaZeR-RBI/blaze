use crate::internal::*;
use crate::*;
use bytes::*;
use std::ffi::*;
use std::marker::PhantomData;

pub struct Texture<'a> {
    raw: *mut BLZ_Texture,
    _marker: PhantomData<&'a ()>,
}

enum_from_primitive! {
    #[derive(Debug, PartialEq)]
    pub enum ImageChannels
    {
        Auto = BLZ_ImageChannels_AUTO as isize,
        Grayscale = BLZ_ImageChannels_GRAYSCALE as isize,
        GrayscaleAlpha = BLZ_ImageChannels_GRAYSCALE_ALPHA as isize,
        RGB = BLZ_ImageChannels_RGB as isize,
        RGBA = BLZ_ImageChannels_RGBA as isize
    }
}

bitflags! {
   pub struct ImageFlags: u32 {
        const PowerOfTwo  = BLZ_ImageFlags_POWER_OF_TWO;
        const Mipmaps = BLZ_ImageFlags_MIPMAPS;
        const Repeats = BLZ_ImageFlags_TEXTURE_REPEATS;
        const MultiplyAlpha = BLZ_ImageFlags_MULTIPLY_ALPHA;
        const InvertY = BLZ_ImageFlags_INVERT_Y;
        const CompressToDXT = BLZ_ImageFlags_COMPRESS_TO_DXT;
        const DDSLoadDirect = BLZ_ImageFlags_DDS_LOAD_DIRECT;
        const NTSCSafeRGB = BLZ_ImageFlags_NTSC_SAFE_RGB;
        const CoCgY = BLZ_ImageFlags_CoCg_Y;
        const TextureRectangle = BLZ_ImageFlags_TEXTURE_RECTANGLE;
   }
}

enum_from_primitive! {
    #[derive(Debug, PartialEq)]
    pub enum SaveImageFormat
    {
        TGA = BLZ_SaveImageFormat_TGA as isize,
        BMP = BLZ_SaveImageFormat_BMP as isize,
        DDS = BLZ_SaveImageFormat_DDS as isize
    }
}

impl<'a> Drop for Texture<'a> {
    fn drop(&mut self) {
        unsafe {
            BLZ_FreeTexture(self.raw);
        }
    }
}

unsafe fn from_ptr<'a>(ptr: *mut BLZ_Texture) -> Result<Texture<'a>, String> {
    if ptr.is_null() {
        Err(get_last_error().unwrap_or("Unknown error".to_owned()))
    } else {
        Ok(Texture {
            raw: ptr,
            _marker: PhantomData,
        })
    }
}

unsafe fn path_to_ptr(path: String) -> Result<*const c_char, NulError> {
    CString::new(path).map(|s| s.as_ptr())
}

pub fn from_memory<'a>(
    bytes: &Bytes,
    channels: ImageChannels,
    texture_id: u32,
    flags: ImageFlags,
) -> Result<Texture<'a>, String> {
    unsafe {
        let buf_ptr = bytes.as_ptr();
        let ptr = BLZ_LoadTextureFromMemory(
            buf_ptr,
            bytes.len() as i32,
            channels as u32,
            texture_id,
            flags.bits,
        );
        from_ptr(ptr)
    }
}

pub fn from_file<'a>(
    path: &str,
    channels: ImageChannels,
    texture_id: u32,
    flags: ImageFlags,
) -> Result<Texture<'a>, String> {
    unsafe {
        let path_ptr = path_to_ptr(path.to_owned()).map_err(|_| "Path cannot be null".to_owned());
        if let Ok(p) = path_ptr {
            return from_ptr(BLZ_LoadTextureFromFile(
                p,
                channels as u32,
                texture_id,
                flags.bits,
            ));
        } else {
            return Err("Invalid path".to_owned());
        }
    }
}
