//
// Atmospheric scattering fragment shader
//
// Author: Sean O'Neil
//
// Copyright (c) 2004 Sean O'Neil
//

uniform vec3 light_pos;
uniform float g;
uniform float g2;

varying vec3 direction;


void main (void)
{
	float fCos = dot(light_pos, direction) / length(direction);
	float fMiePhase = 1.5 * ((1.0 - g2) / (2.0 + g2)) * (1.0 + fCos*fCos) / pow(1.0 + g2 - 2.0*g*fCos, 1.5);
    gl_FragColor = gl_Color + fMiePhase * gl_SecondaryColor;
	gl_FragColor.a = gl_FragColor.b;
}
