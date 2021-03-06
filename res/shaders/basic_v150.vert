in vec3 vertex;
in vec4 color;
in vec3 texCoard;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec4 postColor;
out vec3 postTexCoard;
out vec4 postVertex;

void main() {
	postColor = color;
	postTexCoard = texCoard;
	postVertex = (projection * view * model) * vec4(vertex, 1.0);
	gl_Position = postVertex;
}