varying  vec4 color;

uniform mat4 newColor;
uniform mat4 Model;

void main() 
{ 
    gl_FragColor = newColor*color;
} 
