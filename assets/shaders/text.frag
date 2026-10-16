/* Frag shaders for text 
 * 
*/
#version 460 core
out vec4 FragColor;
in vec2 TexCoord;
uniform sampler2D textTexture;
uniform vec3 textColor; /* return here from C++ code. */

void main() {
	/* using red channel how alpha (GL_RED) */
	float alpha = texture(textTexture, TexCoord).r;
	if (alpha < 0.1) {
		discard; /* If a pixel is completely transparent, then we don't use it. */
	}
	FragColor = vec4(textColor, alpha);
}
