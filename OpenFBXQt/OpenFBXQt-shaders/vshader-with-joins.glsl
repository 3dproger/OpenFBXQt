#ifdef GL_ES
precision mediump int;
precision highp float;
#endif

uniform mat4 model_projection_matrix;

attribute vec3 a_position;
attribute vec3 a_normal;
attribute vec2 a_texcoord;

attribute vec4 a_joint_weights;
attribute vec4 a_joint_indices;

const int MAX_JOINTS = 100; // do not use more than 50 to avoid problems on some mobile devices https://www.gitmemory.com/issue/mgsx-dev/gdx-gltf/7/562368545
uniform mat4 joints[MAX_JOINTS];

varying vec3 v_position;
varying vec3 v_normal;
varying vec2 v_texcoord;

void main()
{
    v_texcoord = a_texcoord;

    mat4 skinningMatrix = joints[int(a_joint_indices[0])] * a_joint_weights[0];
    skinningMatrix     += joints[int(a_joint_indices[1])] * a_joint_weights[1];
    skinningMatrix     += joints[int(a_joint_indices[2])] * a_joint_weights[2];
    skinningMatrix     += joints[int(a_joint_indices[3])] * a_joint_weights[3];

    vec4 skinned_position = skinningMatrix * vec4(a_position, 1.0);

    gl_Position = model_projection_matrix * skinned_position;

    v_position = gl_Position.xyz;
    v_normal = vec3(model_projection_matrix * vec4(a_normal, 0.0));
    v_texcoord = a_texcoord;
}
