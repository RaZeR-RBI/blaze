extern crate bindgen;

use std::env;
use std::path::PathBuf;

fn main() {
    // println!("cargo:rustc-link-lib=libblaze");
    let mut project_dir = PathBuf::from(env::var("CARGO_MANIFEST_DIR").unwrap());
    project_dir.pop();
    println!("cargo:rustc-flags=-l blaze -L {}", project_dir.to_str().unwrap());

    let bindings = bindgen::Builder::default()
        .header("wrapper.h")
        .rustfmt_bindings(false)
        .generate()
        .expect("Unable to generate bindings");
    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());
    bindings
        .write_to_file(out_path.join("bindings.rs"))
        .expect("Couldn't write bindings!");
}
