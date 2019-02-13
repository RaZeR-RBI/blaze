extern crate blaze_rs;

#[cfg(test)]
mod test
{
    use blaze_rs::*;
    use sdl2::sys::SDL_GL_GetProcAddress;
    use sdl2::video::{Window, GLProfile};

    const WINDOW_WIDTH: u32 = 512;
    const WINDOW_HEIGHT: u32 = 512;
    const BLACK: Color = Color {
        r: 1.0,
        g: 1.0,
        b: 1.0,
        a: 1.0
    };

    #[test]
    pub fn test_all() {
        /* SDL2, window and OpenGL context setup */
        let context = sdl2::init().unwrap();
        let video_sys = context.video().unwrap();
        let gl_attr = video_sys.gl_attr();
        gl_attr.set_context_profile(GLProfile::Core);
        gl_attr.set_context_version(3, 3);
        let window = video_sys
            .window("Test", WINDOW_WIDTH, WINDOW_HEIGHT)
            .opengl()
            .build()
            .unwrap();
        let _ctx = window.gl_create_context().unwrap();

        /* Here is the action */
        match load(SDL_GL_GetProcAddress) {
            Ok(_) => {}
            Err(e) => panic!(e),
        }
        set_viewport(WINDOW_WIDTH, WINDOW_HEIGHT).unwrap();
        set_clear_color(BLACK);

        test_dynamic(window);
        /* TODO: Implement tests from C version */
        assert!(true);
    }

    pub fn test_dynamic(window: Window)
    {
        use blaze_rs::dynamic::*;
        clear();
        let batch = create_batch(2, 100, InitFlags::Default);
        assert!(batch.is_some());
        /* TODO: Implement test_draw_dynamic.c */
        window.gl_swap_window();
    }
}
