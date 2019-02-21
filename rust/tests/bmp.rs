use std::fs::File;
use std::io::{Read, Seek, SeekFrom};
use std::mem::size_of;

#[repr(C, packed)]
struct BMP_Header {
    marker_1: u8,
    marker_2: u8,
    size: u32,
    reserved_1: u16,
    reserved_2: u16,
    data_offset: u32,
}

const BMP_HEADER_SIZE: usize = size_of::<BMP_Header>();

pub fn read_bmp(path: &str) -> Result<Vec<u8>, String> {
    let mut header_data: [u8; BMP_HEADER_SIZE] = [0; BMP_HEADER_SIZE];

    let mut file = File::open(path).map_err(|e| format!("{}: {}", e.to_string(), path))?;
    file.read_exact(&mut header_data)
        .map_err(|_| format!("Unexpected EOF while reading header: {}", path))?;
    let header: BMP_Header = unsafe { std::mem::transmute(header_data) };
    if header.marker_1 != 66 || header.marker_2 != 77 {
        return Err(format!("Not a BMP file: {}", path));
    }

    let data_length = (header.size - header.data_offset) as u64;
    let mut result = Vec::<u8>::with_capacity(data_length as usize);
    file.seek(SeekFrom::Current((header.data_offset as usize - BMP_HEADER_SIZE) as i64))
        .map_err(|e| format!("Cannot seek file {}: {}", path, e.to_string()))?;
    let bytes_read = file
        .take(data_length)
        .read_to_end(&mut result)
        .map_err(|_| format!("Unexpected EOF while reading pixel data: {}", path))?;
    if bytes_read < data_length as usize {
        return Err(format!("Unexpected EOF while reading pixel data: {}", path));
    }
    Ok(result)
}

pub fn compare(output_path: &str, ref_path: &str) -> Result<f32, String> {
    let output = read_bmp(output_path)?;
    let reference = read_bmp(ref_path)?;
    if output.len() != reference.len() {
        return Err("Data size differs".to_owned());
    }
    let total = output.len();
    let mut wrong_bytes = 0usize;
    for i in 0..total {
        if output[i] != reference[i] {
            wrong_bytes += 1;
        }
    }
    Ok((total - wrong_bytes) as f32 / total as f32)
}
