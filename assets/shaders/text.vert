/* Shaders for text
 * OpenGL 4.6 version core
*/
#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
out vec2 TexCoord;
void main() {
	gl_Position = vec4(aPos, 1.0);
	TexCoord = aTexCoord;
}
