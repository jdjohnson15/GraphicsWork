attribute  vec4 vPosition;
attribute  vec3 vNormal;
varying vec4 color;

uniform vec4 ambient_product_1;
uniform vec4 diffuse_product_1;
uniform vec4 specular_product_1;
uniform vec4 ambient_product_2;
uniform vec4 diffuse_product_2;
uniform vec4 specular_product_2;
uniform vec4 ambient_product_dir;
uniform vec4 diffuse_product_dir;
uniform vec4 specular_product_dir;

uniform float Shininess;

uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

uniform vec4 light_pos_1;
uniform vec4 light_pos_2;
uniform vec4 light_pos_dir;

float L;
vec3 E;
vec3 H;

vec4 ambient;
vec4 diffuse;
vec4 specular;

float lambert;
float phong;
vec4  global_ambient;

float attenuation;

vec4 color_1;
vec4 color_2;
vec4 color_3;



void main()
{
	
	// Transform vertex  position into eye coordinates
	vec3 mvPos = (View * Model * vPosition).xyz;

	// Transform vertex normal into eye coordinates
	//vec3 N = normalize( View * Model * vec4(vNormal, 0.0) ).xyz;
	vec3 N = normalize(vNormal);
	

//-----------------------------------------------------------------------//
//light 1
	//Transform light position into eye coordinates
	L = length( light_pos_1.xyz - mvPos.xyz );
	
	E = normalize( -mvPos );
	H = normalize( normalize( light_pos_1.xyz - mvPos.xyz ) + E );

	//set attenuation factor
	attenuation = 1.0/(1.5*L);

	// Compute illumination values
	ambient = ambient_product_1;

	lambert = max( dot(normalize( light_pos_1.xyz - mvPos.xyz ), N), 0.0 );
	diffuse = lambert * diffuse_product_1;

	phong = pow( max(dot(N, H), 0.0), Shininess );
	specular = phong * specular_product_1;

	if( dot(normalize( light_pos_1.xyz - mvPos.xyz ), N) < 0.0 ) 
		{specular = vec4(0.0, 0.0, 0.0, 1.0);}

	color_1 = (ambient + diffuse + specular)*attenuation;

//light 2
	//Transform light position into eye coordinates
	L = length( light_pos_2.xyz - mvPos.xyz );
	
	E = normalize( -mvPos );
	H = normalize( normalize( light_pos_2.xyz - mvPos.xyz ) + E );
	
	//set attenuation factor
	attenuation = 1.0/(1.5*L);
	 
	// Compute illumination values
	ambient = ambient_product_2;

	lambert = max( dot(normalize( light_pos_2.xyz - mvPos.xyz ), N), 0.0 );
	diffuse = lambert * diffuse_product_2;

	phong = pow( max(dot(N, H), 0.0), Shininess );
	specular = phong * specular_product_2;

	if( dot(normalize( light_pos_2.xyz - mvPos.xyz ), N) < 0.0 ) 
		{specular = vec4(0.0, 0.0, 0.0, 1.0);}
	
	color_2 = ( ambient + diffuse + specular )*attenuation;

//directional light
	//Transform light position into eye coordinates

	L = length( light_pos_dir.xyz - mvPos.xyz );

	E = normalize( -mvPos );
	H = normalize( normalize( light_pos_1.xyz - mvPos.xyz ) + E );

	// Compute illumination values
	ambient = ambient_product_dir;

	lambert = max( dot(normalize( light_pos_dir.xyz - mvPos.xyz ), N), 0.0 );
	diffuse = lambert * diffuse_product_dir;

	phong = pow( max(dot(N, H), 0.0), Shininess );
	specular = phong * specular_product_dir;

	if( dot(normalize( light_pos_dir.xyz - mvPos.xyz ), N) < 0.0 ) 
		{specular = vec4(0.0, 0.0, 0.0, 1.0);}
	
	color_3 = (ambient + diffuse + specular);

	color.a = 1.0; 


//-------------------------------------------------------------------------//
	
	global_ambient = vec4( 0.1, 0.1, 0.1, 1.0 );

	gl_Position = Projection * View * Model * vPosition;

	color = (color_1 + color_2 + color_3 + global_ambient);

}







