attribute  vec4 vPosition;
attribute  vec3 vNormal;
attribute  vec2 vTexCoord;

varying vec4 color;
varying vec2 texCoord;

uniform vec4 ambient_product_1;
uniform vec4 diffuse_product_1;
uniform vec4 specular_product_1;

uniform int point_1;
uniform vec4 light_pos_1;
vec4 light_pos;

uniform float Shininess;

float L;
vec3 E;
vec3 H;
vec3 N;

vec4 ambient;
vec4 diffuse;
vec4 specular;

float lambert;
float phong;
vec4  global_ambient;

float attenuation;

vec4 color_1;

uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

void main() 
{

// Transform vertex  position into eye coordinates
	vec3 mvPos = (View * Model * vPosition).xyz;

	// Transform vertex normal into eye coordinates
	
	N = normalize( Model * vec4(vNormal, 0.0) ).xyz;

	light_pos = View * light_pos_1; 

	if (point_1==0){color_1 = vec4(0.0, 0.0, 0.0, 1.0);}
	else{

		//Transform light position into eye coordinates
		L = length( light_pos.xyz - mvPos.xyz );
	
		E = normalize( -mvPos );
		H = normalize( normalize( light_pos.xyz - mvPos.xyz ) + E );

		//set attenuation factor
		attenuation = 1.0/(0.5*L);

		// Compute illumination values
		ambient = ambient_product_1;

		lambert = max( dot(normalize( light_pos.xyz - mvPos.xyz ), N), 0.0 );
		diffuse = lambert * diffuse_product_1;

		phong = pow( max(dot(N, H), 0.0), Shininess );
		specular = phong * specular_product_1;

		if( dot(normalize( light_pos.xyz - mvPos.xyz ), N) < 0.0 ) 
			{specular = vec4(0.0, 0.0, 0.0, 1.0);}

		color_1 = (ambient + diffuse + specular)*attenuation;
	}
	
	global_ambient = vec4( 0.1, 0.1, 0.1, 1.0 );

	gl_Position = Projection * View * Model * vPosition;
	color = (color_1 + global_ambient);
	color.a = 1.0;
	texCoord    = vTexCoord;
}
