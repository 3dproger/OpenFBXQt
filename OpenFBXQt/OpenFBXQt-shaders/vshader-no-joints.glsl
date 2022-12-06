#ifdef GL_ES
precision mediump int;
precision highp float;
#endif

uniform mat4 model_projection_matrix;

attribute vec3 a_position;
attribute vec3 a_normal;
attribute vec2 a_texcoord;

varying vec3 v_position;
varying vec3 v_normal;
varying vec2 v_texcoord;

void main()
{
    gl_Position = model_projection_matrix * vec4(a_position, 1.0);

    v_position = gl_Position.xyz;
    v_normal = vec3(model_projection_matrix * vec4(a_normal, 0.0));
    v_texcoord = a_texcoord;
}
