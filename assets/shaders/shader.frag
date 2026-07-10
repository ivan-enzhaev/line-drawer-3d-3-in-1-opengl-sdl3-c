// The #version and precision qualifiers are injected by createShaderProgram

uniform vec3 uColor;
out vec4 fragColor;

void main()
{
    fragColor = vec4(uColor, 1.0);
}
