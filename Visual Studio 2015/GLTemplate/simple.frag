#version 150

in vec3 fragment_normal;
in vec3 fragment_pos;

out vec4 finalColor;

uniform vec3 light_pos;

uniform sampler2D shadowmap;

//licht und inverse camera
uniform mat4 mvp_l;

//hier sit alles in bildraum
void main()
{
	//in bereich 0,0,hohe,breite
	//gl_FragCoord.xy
	//in bereich 0 bis 1
	//gl_FragCoord.z

	//auf 0-1 mappen, danach auf -1 bis 1
	//und schon haben wir kamerabildraumkoord
	vec4 fragCoord = vec4(gl_FragCoord.xy / vec2(640, 480)*2-1, gl_FragCoord.z*2-1, 1.f);
	vec4 light_fragCoord = mvp_l * fragCoord;
	light_fragCoord/=light_fragCoord.w;
	
	float shadow_z = texture(shadowmap, light_fragCoord.xy*0.5+0.5).r;
	//von -1 bis 1(viewport) auf 0 bis 1 (texturkoord) mappen
	float diff = light_fragCoord.z - (shadow_z*2-1);
	if( diff < 0.001f)
	{

		vec3 light_diff = normalize(light_pos - fragment_pos);
		float diffuse = dot(light_diff, fragment_normal);
		finalColor = vec4(vec3(diffuse), 1);
	}
	//finalColor = vec4(vec3(shadow_z), 1);
	
}