-- Scene.Vertex
in vec4 i_vertex;
in vec3 i_normal;
in vec2 i_texcoord0;

out vec4 v_vertex;
out vec3 v_normal;
out vec2 v_texcoord0;

void main() {
	v_vertex = i_vertex;
	v_normal = i_normal;
	v_texcoord0 = i_texcoord0;
}

-- Scene.TessControl
layout(vertices = 3) out;

in vec4 v_vertex[];
in vec3 v_normal[];
in vec2 v_texcoord0[];

out vec4 tc_vertex[];
out vec3 tc_normal[];
out vec2 tc_texcoord0[];

#define ID gl_InvocationID

void main()
{
    tc_vertex[ID] = v_vertex[ID];
    tc_normal[ID] = v_normal[ID];
    tc_texcoord0[ID] = v_texcoord0[ID];
    
    if (ID == 0) {
        gl_TessLevelInner[0] = 2;
        gl_TessLevelOuter[0] = 1;
        gl_TessLevelOuter[1] = 1;
        gl_TessLevelOuter[2] = 1;
    }
}

-- Scene.TessEvaluation
layout(triangles, equal_spacing, ccw) in;

in vec4 tc_vertex[];
in vec3 tc_normal[];
in vec2 tc_texcoord0[];

out vec3 f_normal;
out vec3 f_pos;
out vec2 f_texcoord;

uniform mat4 u_worldViewMtx;
uniform mat4 u_worldViewProjMtx;

void main()
{
	vec4 te_vertex =
		gl_TessCoord.x * tc_vertex[0] +
		gl_TessCoord.y * tc_vertex[1] +
		gl_TessCoord.z * tc_vertex[2] ;
	
	vec3 te_normal = 
		gl_TessCoord.x * tc_normal[0] +
		gl_TessCoord.y * tc_normal[1] +
		gl_TessCoord.z * tc_normal[2] ;
		
	vec2 te_texcoord0 = 
		gl_TessCoord.x * tc_texcoord0[0] +
		gl_TessCoord.y * tc_texcoord0[1] +
		gl_TessCoord.z * tc_texcoord0[2] ;
		
	gl_Position = u_worldViewProjMtx * te_vertex;
	f_normal = (u_worldViewMtx * vec4(te_normal, 0)).xyz;
	f_pos = (u_worldViewMtx * te_vertex).xyz;
	f_texcoord = te_texcoord0;
}

-- Scene.Fragment
in vec3 f_normal;
in vec3 f_pos;
in vec2 f_texcoord;

out vec4 o_fragColor;

uniform vec4 u_matDiffuse;
uniform vec4 u_matSpecular;
uniform float u_matShininess;

void main() {
	vec3 n = normalize(f_normal.xyz);
	vec3 l = normalize(vec3(0,10,10) - f_pos.xyz);
	vec3 h = normalize(l + vec3(0, 0, 1));
	
	if(false == gl_FrontFacing)
		n *= -1;
		
	float ndl = max(0, dot(n, l)) * 0.8 + 0.2;
	float ndh = max(0, dot(n, h));
	ndh = pow(ndh, u_matShininess);
	
	vec4 color = u_matDiffuse;
	color.xyz *= ndl;
	color.xyz += u_matSpecular.xyz * ndh;
	
	o_fragColor = color;
}
