use crate::internal::*;
use crate::texture::*;
use crate::*;
use std::marker::PhantomData;

pub struct SpriteBatch<'a> {
    raw: *mut BLZ_SpriteBatch,
    _marker: PhantomData<&'a ()>,
}

bitflags! {
    pub struct InitFlags: u32
    {
        const Default = BLZ_InitFlags_DEFAULT;
        const NoBuffering = BLZ_InitFlags_NO_BUFFERING;
    }
}

enum_from_primitive! {
    #[derive(Debug, PartialEq)]
    pub enum SpriteFlip
    {
        None = BLZ_SpriteFlip_NONE as isize,
        FlipH = BLZ_SpriteFlip_FLIP_H as isize,
        FlipV = BLZ_SpriteFlip_FLIP_V as isize,
        Both = BLZ_SpriteFlip_BOTH as isize
    }
}

impl Default for InitFlags {
    fn default() -> InitFlags {
        InitFlags::Default
    }
}

pub fn create_batch<'a>(
    max_buckets: u32,
    max_sprites_per_bucket: u32,
    flags: InitFlags,
) -> Option<SpriteBatch<'a>> {
    unsafe {
        let ptr = BLZ_CreateBatch(
            max_buckets as i32,
            max_sprites_per_bucket as i32,
            flags.bits(),
        );
        if ptr.is_null() {
            return None;
        } else {
            return Some(SpriteBatch {
                raw: ptr,
                _marker: PhantomData,
            });
        }
    }
}

impl<'a> Drop for SpriteBatch<'a> {
    fn drop(&mut self) {
        unsafe {
            BLZ_FreeBatch(self.raw);
        }
    }
}

impl<'s> SpriteBatch<'s> {
    pub fn draw<'t>(
        &self,
        texture: &'t Texture,
        position: Vector2,
        srcRectangle: Option<Rectangle>,
        rotationInRadians: f32,
        origin: Option<Vector2>,
        scale: Option<Vector2>,
        color: Color,
        flip: SpriteFlip,
    ) -> CallResult {
        unsafe {
            wrap_result(
                BLZ_Draw(
                    self.raw,
                    texture.raw,
                    position,
                    srcRectangle.as_raw(),
                    rotationInRadians,
                    origin.as_raw(),
                    scale.as_raw(),
                    color.into(),
                    flip as u32
                )
            )
        }
    }
}
