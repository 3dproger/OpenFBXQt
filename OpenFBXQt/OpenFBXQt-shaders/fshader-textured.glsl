#ifdef GL_ES
precision mediump int;
precision highp float;
#endif

uniform vec3 projection_pos;
uniform sampler2D texture;

varying vec3 v_position;
varying vec3 v_normal;
varying vec2 v_texcoord;

const vec3 light_position = vec3(0.0, 100.0, 0.0);
const vec3 ambient_color = vec3(1.0, 1.0, 1.0);
const float ambient_strength = 0.05;
const float specular_strength = 1.0;

vec4 calc_spectacular(const vec4 color)
{
    vec3 normal = normalize(v_normal);

    float spec = pow(max(dot(normalize(projection_pos - v_position), reflect(-normalize(light_position - v_position), normal)), 0.0), 5.0);
    vec3 specular = spec * ambient_color;

    return vec4(color.xyz * 0.5 + specular * specular_strength + ambient_color * ambient_strength, color.w);
}

void main()
{
    gl_FragColor = calc_spectacular(texture2D(texture, v_texcoord));

    if (gl_FragColor.a < 0.5)
    {
        discard;
    }
}
