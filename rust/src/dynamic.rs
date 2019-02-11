use crate::internal::*;

enum_from_primitive! {
    #[derive(Debug, PartialEq)]
    pub enum InitFlags
    {
        Default = BLZ_InitFlags_DEFAULT as isize,
        NoBuffering = BLZ_InitFlags_NO_BUFFERING as isize
    }
}
