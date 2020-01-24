#version 450
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(set = 0, binding = 0) uniform uniform_buffer {
	mat4 m_matrix;
};

layout(location = 2) out VS_OUT {
	vec3 color;
} vs_out;

void main() {
	vs_out.color = color;
	gl_Position = m_matrix*vec4(position,1.0f);
}