-- Vertex
varying vec3 v_normal;

void main() {
	gl_Position = ftransform();
	v_normal = gl_NormalMatrix * gl_Normal;
}

-- Fragment
varying vec3 v_normal;

void main() {
	vec3 n = normalize(v_normal.xyz);
	if(false == gl_FrontFacing)
		n *= -1;
	float ndl = max(0, dot(n, vec3(0,0,-1)));
	gl_FragColor = vec4(ndl, ndl, ndl, 1);
}