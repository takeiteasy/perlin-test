@ctype mat4 Mat4x4
@ctype vec4 Vec4

@vs default3d_vs
in vec3 position;
in vec2 texcoord;

out vec3 FragPos;
out vec2 TexCoord;

uniform vs_params {
    mat4 model;
    mat4 view;
    mat4 projection;
};

void main() {
    gl_Position = projection * view * model * vec4(position, 1.0);
    TexCoord = vec2(texcoord.x, -texcoord.y);
}
@end

@fs default3d_fs
uniform sampler2D tex;
in vec3 FragPos;
in vec2 TexCoord;

out vec4 fragColor;

void main() {
    fragColor = texture(tex, TexCoord);
}
@end

@program default3d_program default3d_vs default3d_fs
