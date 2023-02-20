@vs default_vs
in vec2 position;
in vec2 texcoord;

out vec2 uv;

void main() {
    gl_Position = vec4(position, 0.0, 1.0);
    uv = texcoord;
}
@end

@fs default_fs
uniform sampler2D tex;
in vec2 uv;

out vec4 fragColor;

void main() {
    fragColor = texture(tex, uv);
}
@end

@program default_program default_vs default_fs
