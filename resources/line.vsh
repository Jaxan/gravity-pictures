attribute vec4 position;
attribute vec4 color;

uniform mat4 modelViewProjectionMatrix;

varying vec4 varying_color;

void main() {
	gl_Position = modelViewProjectionMatrix * position;
	varying_color = color;
}
