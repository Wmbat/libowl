#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

layout(binding = 0) uniform camera_buffer_object
{
   mat4 model;
   mat4 view;
   mat4 proj;
} cbo;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_colour;

layout(location = 0) out vec3 frag_colour;

void main()
{
   //gl_Position = cbo.proj * cbo.view * cbo.model * vec4(in_position, 1.0);
   gl_Position = vec4(in_position, 1.0);
   frag_colour = in_colour;
}
