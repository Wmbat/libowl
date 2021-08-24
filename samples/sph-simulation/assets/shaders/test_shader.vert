#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

layout(binding = 0) uniform camera_buffer_object
{
   mat4 proj;
   mat4 view;
}
cbo;

layout(push_constant) uniform mesh_data
{
   mat4 model;
   vec3 colour;
}
mesh;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec3 in_colour;

layout(location = 0) out vec3 frag_position;
layout(location = 1) out vec3 frag_normal;
layout(location = 2) out vec3 frag_colour;

void main()
{
   gl_Position = cbo.proj * cbo.view * mesh.model * vec4(in_position, 1.0);
   frag_position = vec3(mesh.model * vec4(in_position, 1.0));
   frag_normal = in_normal;
   frag_colour = mesh.colour;
}
