extern crate blaze_rs;

mod bmp;

#[cfg(test)]
mod test {
    use crate::bmp::compare;
    use blaze_rs::blend::*;
    use blaze_rs::dynamic::*;
    use blaze_rs::texture::*;
    use blaze_rs::*;
    use sdl2::sys::SDL_GL_GetProcAddress;
    use sdl2::video::{GLProfile, Window};

    const WINDOW_WIDTH: u32 = 512;
    const WINDOW_HEIGHT: u32 = 512;
    const BLACK: Color = Color { r: 0.0, g: 0.0, b: 0.0, a: 1.0 };
    const WHITE: Color = Color { r: 1.0, g: 1.0, b: 1.0, a: 1.0 };

    #[test]
    pub fn test_all() {
        /* SDL2, window and OpenGL context setup */
        let context = sdl2::init().unwrap();
        let video_sys = context.video().unwrap();
        let gl_attr = video_sys.gl_attr();
        gl_attr.set_context_profile(GLProfile::Core);
        gl_attr.set_context_version(3, 3);
        let window =
            video_sys.window("Test", WINDOW_WIDTH, WINDOW_HEIGHT).opengl().build().unwrap();
        let _ctx = window.gl_create_context().unwrap();

        /* Here is the action */
        match load(SDL_GL_GetProcAddress) {
            Ok(_) => {}
            Err(e) => panic!(e),
        }
        set_viewport(WINDOW_WIDTH, WINDOW_HEIGHT).unwrap();
        set_clear_color(BLACK);

        test_init_shutdown();
        test_draw_dynamic(&window);
        /* TODO: Implement tests from C version */
        assert!(true);
    }

    fn validate_output(name: &str, likeness: f32) {
        let path = format!("{}.bmp", name);
        let ref_path = format!("../test/refs/{}.bmp", name);
        save_screenshot(&path, SaveImageFormat::BMP, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT)
            .expect("Cannot save screenshot");
        let actual_likeness = compare(&path, &ref_path).unwrap();
        assert!(
            actual_likeness >= likeness,
            format!("expected > {}, actual is {}", likeness, actual_likeness)
        );
    }

    /* ---------------------- test_init_shutdown.c -------------------------- */
    fn test_init_shutdown() {
        {
            let options = SpriteBatchOpts {
                max_buckets: 5,
                max_sprites_per_bucket: 100,
                flags: InitFlags::Default,
            };
            let batch = create_batch(options.clone());
            assert!(batch.is_ok());
            assert!(batch.unwrap().get_options() == &options);
        }
        {
            let fail = create_batch(SpriteBatchOpts {
                max_buckets: 0,
                max_sprites_per_bucket: 0,
                flags: InitFlags::Default,
            });
            assert!(fail.is_err());
        }
    }

    /* ---------------------- test_draw_dynamic.c --------------------------- */

    const COLORS: [Color; 12] = [
        Color { r: 1.0, g: 0.0, b: 0.0, a: 1.0 },
        Color { r: 0.0, g: 1.0, b: 0.0, a: 1.0 },
        Color { r: 0.0, g: 0.0, b: 1.0, a: 1.0 },
        Color { r: 1.0, g: 1.0, b: 0.0, a: 1.0 },
        Color { r: 0.0, g: 1.0, b: 1.0, a: 1.0 },
        Color { r: 1.0, g: 0.0, b: 1.0, a: 1.0 },
        Color { r: 1.0, g: 0.0, b: 0.0, a: 0.5 },
        Color { r: 0.0, g: 1.0, b: 0.0, a: 0.5 },
        Color { r: 0.0, g: 0.0, b: 1.0, a: 0.5 },
        Color { r: 1.0, g: 1.0, b: 0.0, a: 0.5 },
        Color { r: 0.0, g: 1.0, b: 1.0, a: 0.5 },
        Color { r: 1.0, g: 0.0, b: 1.0, a: 0.5 },
    ];

    #[inline]
    fn next_line(position: &mut Vector2) {
        position.x = 20.0;
        position.y += 40.0;
    }

    #[inline]
    fn deg(rad: f32) -> f32 {
        rad * 3.14159265 / 180.0
    }

    fn draw_dynamic(batch: &SpriteBatch, tex: &Texture, start: Vector2) {
        let mut pos = start;
        /* Different rotation angles */
        for j in 0..12 {
            batch
                .draw(tex, pos, None, deg(j as f32 * 30.0), None, None, WHITE, SpriteFlip::None)
                .unwrap();
            pos.x += 40.0;
        }
        next_line(&mut pos);
        /* Different colors */
        for j in 0..12 {
            batch.draw(tex, pos, None, 0.0, None, None, COLORS[j], SpriteFlip::None).unwrap();
            pos.x += 40.0;
        }
        next_line(&mut pos);
        /* Rotate around specified origin */
        let origin = Some(Vector2 { x: 8.0, y: 8.0 });
        for j in 0..12 {
            batch
                .draw(tex, pos, None, deg(j as f32 * 30.0), origin, None, WHITE, SpriteFlip::None)
                .unwrap();
            pos.x += 40.0;
        }
        next_line(&mut pos);
        /* Draw only specified part using different scales */
        let src = Some(Rectangle { x: 4.0, y: 4.0, w: 8.0, h: 8.0 });
        for j in 0..12 {
            let scale = Some(Vector2 { x: j as f32 / 6.0, y: j as f32 / 6.0 });
            batch.draw(tex, pos, src, 0.0, None, scale, WHITE, SpriteFlip::None).unwrap();
            pos.x += 40.0;
        }
        next_line(&mut pos);
        /* Do various flips */
        let flips = [SpriteFlip::None, SpriteFlip::FlipH, SpriteFlip::FlipV, SpriteFlip::Both];
        for flip in flips.iter() {
            batch.draw(tex, pos, None, 0.0, None, None, WHITE, *flip).unwrap();
            pos.x += 40.0;
        }
    }

    pub fn test_draw_dynamic(window: &Window) {
        use blaze_rs::dynamic::*;
        use blaze_rs::texture::*;

        let batch = create_batch(SpriteBatchOpts {
            max_buckets: 2,
            max_sprites_per_bucket: 100,
            flags: InitFlags::Default,
        })
        .expect("Cannot create batch");
        let textures: Vec<_> = ["../test/test_texture.png", "../test/test_texture2.png"]
            .iter()
            .map(|path| Texture::from_file(path, ImageChannels::Auto, None, ImageFlags::None))
            .map(|result| result.expect("Cannot load texture"))
            .collect();

        set_blend_mode(NORMAL);
        set_clear_color(BLACK);
        for _ in 1..4 {
            clear();
            let mut start = Vector2 { x: 20.0, y: 20.0 };
            for texture in textures.iter() {
                draw_dynamic(&batch, texture, start);
                start.y += 240.0;
            }
            batch.present().unwrap();
            window.gl_swap_window();
        }
        validate_output("test_draw_dynamic", 0.999);
    }
}
