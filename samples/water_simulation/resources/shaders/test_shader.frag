#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 frag_position;
layout(location = 1) in vec3 frag_normal;
layout(location = 2) in vec3 frag_colour;

layout(location = 0) out vec4 outColor;

void main()
{
   vec3 norm = normalize(frag_normal);
   vec3 light_dir = normalize(vec3(50.0, 50.0, 0.0 - frag_position));
   vec3 light_col = vec3(1.0, 1.0, 1.0);

   vec3 ambient = 0.5 * light_col;
   vec3 diffuse = max(dot(norm, light_dir), 0.0) * light_col;

   outColor = vec4((ambient + diffuse) * frag_colour, 1.0F);
}
