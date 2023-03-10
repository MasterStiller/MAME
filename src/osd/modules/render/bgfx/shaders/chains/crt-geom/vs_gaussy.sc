$input a_position, a_texcoord0, a_color0
$output v_texCoord, v_coeffs, v_coeffs2

#include "common.sh"

uniform vec4 u_tex_size0;
uniform vec4 u_inv_view_dims;
uniform vec4 u_width;
uniform vec4 u_aspect;

void main()
{
	float wid = u_width.x*u_tex_size0.y/(320.*u_aspect.y);
	v_coeffs = exp(vec4(1.,4.,9.,16.)*vec4_splat(-1.0/wid/wid));
	v_coeffs2 = exp(vec4(25.,36.,49.,64.)*vec4_splat(-1.0/wid/wid));

	// Do the standard vertex processing.
	gl_Position = mul(u_viewProj, vec4(a_position.xy, 0.0, 1.0));
#if BGFX_SHADER_LANGUAGE_HLSL && BGFX_SHADER_LANGUAGE_HLSL <= 300
	gl_Position.xy += u_inv_view_dims.xy * gl_Position.w;
#endif
	v_texCoord = a_texcoord0;
}
