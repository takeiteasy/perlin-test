@vs default2d_vs
in vec4 position;
in vec2 texcoord;

out vec2 uv;

void main() {
    gl_Position = vec4(position.xy, 0.0, 1.0);
    uv = texcoord;
}
@end

@fs default2d_fs
uniform sampler2D tex;
in vec2 uv;

out vec4 fragColor;

void main() {
    fragColor = texture(tex, uv);
}
@end

@program default2d_program default2d_vs default2d_fs
