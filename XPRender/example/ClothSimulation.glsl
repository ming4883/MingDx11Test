-- Vertex

void main() {
	gl_Position = ftransform();
}

-- Fragment

void main() {
	gl_FragColor = vec4(1,1,0,1);
}