/**
* This work is distributed under the General Public License,
* see LICENSE for details
*
* @author Gwenna�l ARBONA
**/

/*-------------------------------------------------
	Config
/*-----------------------------------------------*/

#version 150

uniform vec3 farCorner;
uniform mat4 projection;


/*-------------------------------------------------
	Input / Output
/*-----------------------------------------------*/

in vec4 vertex;
in vec2 uv0;

out vec2 vUv0;
out vec3 vRay;


/*-------------------------------------------------
	Shader
/*-----------------------------------------------*/

void main()
{
	vUv0 = uv0;

	vec2 pos = vertex.xy;
	vec3 corner = farCorner;
	//corner.z = -corner.z;
	
	vRay.x = pos.x * corner.x;
	vRay.y = pos.y * corner.y;
	vRay.z = corner.z;

	//vRay = corner * vec3(pos, 1);
	//vRay =  vec3(pos, 0);

	gl_Position = projection * vertex;
}
