use crate::internal::*;
use crate::texture::*;
use crate::*;
use std::marker::PhantomData;

pub struct SpriteBatch<'a> {
    raw: *mut BLZ_SpriteBatch,
    _marker: PhantomData<&'a ()>,
    options: SpriteBatchOpts,
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct SpriteBatchOpts {
    pub max_buckets: u32,
    pub max_sprites_per_bucket: u32,
    pub flags: InitFlags,
}

bitflags! {
    pub struct InitFlags: u32
    {
        const Default = BLZ_InitFlags_DEFAULT;
        const NoBuffering = BLZ_InitFlags_NO_BUFFERING;
    }
}

impl Default for InitFlags {
    fn default() -> InitFlags {
        InitFlags::Default
    }
}

pub fn create_batch<'a>(options: SpriteBatchOpts) -> Result<SpriteBatch<'a>, String> {
    unsafe {
        let ptr = BLZ_CreateBatch(
            options.max_buckets as i32,
            options.max_sprites_per_bucket as i32,
            options.flags.bits(),
        );
        if ptr.is_null() {
            return Err(try_get_err());
        } else {
            return Ok(SpriteBatch { raw: ptr, _marker: PhantomData, options: options });
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
            wrap_result(BLZ_Draw(
                self.raw,
                texture.raw,
                position,
                srcRectangle.as_raw(),
                rotationInRadians,
                origin.as_raw(),
                scale.as_raw(),
                color.into(),
                flip as u32,
            ))
        }
    }

    pub fn lower_draw<'t>(&self, texture: &'t Texture, quad: &Quad) -> CallResult {
        unsafe { wrap_result(BLZ_LowerDraw(self.raw, texture.id, quad)) }
    }

    pub fn present(&self) -> CallResult {
        unsafe { wrap_result(BLZ_Present(self.raw)) }
    }

    pub fn get_options(&self) -> &SpriteBatchOpts {
        &self.options
    }
}
