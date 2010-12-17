-- Scene.Vertex
varying vec3 v_normal;
varying vec3 v_pos;

void main() {
	gl_Position = ftransform();
	v_normal = gl_NormalMatrix * gl_Normal;
	v_pos = (gl_ModelViewMatrix * gl_Vertex).xyz;
}

-- Scene.Fragment
varying vec3 v_normal;
varying vec3 v_pos;

void main() {
	vec3 n = normalize(v_normal.xyz);
	vec3 l = normalize(vec3(0,10,10) - v_pos.xyz);
	vec3 h = normalize(l + vec3(0, 0, 1));
	
	if(false == gl_FrontFacing)
		n *= -1;
		
	float ndl = max(0, dot(n, l)) * 0.8 + 0.2;
	float ndh = max(0, dot(n, h));
	ndh = pow(ndh, gl_FrontMaterial.shininess);
	
	vec4 color = gl_FrontMaterial.diffuse;
	color.xyz *= ndl;
	color.xyz += gl_FrontMaterial.specular.xyz * ndh;
	
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