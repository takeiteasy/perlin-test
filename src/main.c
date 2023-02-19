#define SOKOL_IMPL
#include "sokol_gfx.h"
#include "sokol_app.h"
#include "sokol_glue.h"

static struct {
    sg_pass_action pass_action;
} state;

void init(void) {
    sg_setup(&(sg_desc){
        .context = sapp_sgcontext()
    });
    
    state.pass_action = (sg_pass_action) {
        .colors[0] = { .action=SG_ACTION_CLEAR, .value={0.f, 0.f, 0.f, 1.f} }
    };
}

void frame(void) {
    const float t = (float)(sapp_frame_duration() * 60.0);
    
    sg_begin_default_pass(&state.pass_action, sapp_width(), sapp_height());
    sg_end_pass();
    sg_commit();
}

void input(const sapp_event *e) {
}

void cleanup(void) {
    sg_shutdown();
}

sapp_desc sokol_main(int argc, char* argv[]) {
    (void)argc; (void)argv;
    return (sapp_desc){
        .init_cb = init,
        .frame_cb = frame,
        .event_cb = input,
        .cleanup_cb = cleanup,
        .width = 800,
        .height = 600,
        .sample_count = 4,
        .gl_force_gles2 = true,
        .window_title = "sokol",
        .icon.sokol_default = true,
    };
}
