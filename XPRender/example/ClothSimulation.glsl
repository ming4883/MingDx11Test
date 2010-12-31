-- Scene.Vertex
varying vec3 v_normal;
varying vec3 v_pos;

uniform mat4 u_worldViewMtx;
uniform mat4 u_worldViewProjMtx;

void main() {
	gl_Position = u_worldViewProjMtx * gl_Vertex;
	v_normal = u_worldViewMtx * vec4(gl_Normal, 0);
	v_pos = (u_worldViewMtx * gl_Vertex).xyz;
}

-- Scene.Fragment
varying vec3 v_normal;
varying vec3 v_pos;

uniform vec4 u_matDiffuse;
uniform vec4 u_matSpecular;
uniform float u_matShininess;

void main() {
	vec3 n = normalize(v_normal.xyz);
	vec3 l = normalize(vec3(0,10,10) - v_pos.xyz);
	vec3 h = normalize(l + vec3(0, 0, 1));
	
	if(false == gl_FrontFacing)
		n *= -1;
		
	float ndl = max(0, dot(n, l)) * 0.8 + 0.2;
	float ndh = max(0, dot(n, h));
	ndh = pow(ndh, u_matShininess);
	
	vec4 color = u_matDiffuse;
	color.xyz *= ndl;
	color.xyz += u_matSpecular.xyz * ndh;
	
	gl_FragColor = color;
}

-- UI.Vertex
varying vec4 v_texcoord;

void main() {
	gl_Position = ftransform();
	v_texcoord = gl_MultiTexCoord0;
}

-- UI.Fragment
uniform sampler2D u_tex;

varying vec4 v_texcoord;

void main() {
	gl_FragColor = tex2D(u_tex, v_texcoord.xy);
}