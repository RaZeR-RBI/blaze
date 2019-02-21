use crate::internal::*;

enum_from_primitive! {
    #[derive(Debug, PartialEq)]
    pub enum BlendFactor
    {
        Zero = GL_ZERO as isize,
        One = GL_ONE as isize,
        SrcColor = GL_SRC_COLOR as isize,
        OneMinusSrcColor = GL_ONE_MINUS_SRC_COLOR as isize,
        DstColor = GL_DST_COLOR as isize,
        OneMinusDstColor = GL_ONE_MINUS_DST_COLOR as isize,
        SrcAlpha = GL_SRC_ALPHA as isize,
        OneMinusSrcAlpha = GL_ONE_MINUS_SRC_ALPHA as isize,
        DstAlpha = GL_DST_ALPHA as isize,
        OneMinusDstAlpha = GL_ONE_MINUS_DST_ALPHA as isize
    }
}

#[derive(Debug, PartialEq)]
pub struct BlendMode
{
    pub src: BlendFactor,
    pub dst: BlendFactor
}

pub const NORMAL: BlendMode = BlendMode {
    src: BlendFactor::SrcAlpha,
    dst: BlendFactor::OneMinusSrcAlpha
};

pub const ADDITIVE: BlendMode = BlendMode {
    src: BlendFactor::One,
    dst: BlendFactor::One
};

pub const MULTIPLY: BlendMode = BlendMode {
    src: BlendFactor::DstColor,
    dst: BlendFactor::Zero
};

impl From<BlendMode> for BLZ_BlendFunc {
    fn from(mode: BlendMode) -> BLZ_BlendFunc {
        BLZ_BlendFunc {
            source: mode.src as u32,
            destination: mode.dst as u32
        }
    }
}


pub fn set_blend_mode(mode: BlendMode) {
    unsafe { BLZ_SetBlendMode(mode.into()); }
}
