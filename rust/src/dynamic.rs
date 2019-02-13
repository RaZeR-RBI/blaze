use crate::internal::*;
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

impl Default for InitFlags {
    fn default() -> InitFlags {
        InitFlags::Default
    }
}

pub fn create_batch<'a>(
    max_buckets: u32,
    max_sprites_per_bucket: u32,
    flags: InitFlags
) -> Option<SpriteBatch<'a>> {
    unsafe {
        let ptr = BLZ_CreateBatch(
            max_buckets as i32,
            max_sprites_per_bucket as i32,
            flags.bits()
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
