// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  effect.h - BGFX shader material to be applied to a mesh
//
//============================================================

#pragma once

#ifndef __DRAWBGFX_EFFECT__
#define __DRAWBGFX_EFFECT__

#include <bgfx/bgfx.h>

#include <vector>
#include <map>
#include <string>

class bgfx_uniform;

class bgfx_effect
{
public:
	bgfx_effect(std::string name, uint64_t state, bgfx::ShaderHandle vertex_shader, bgfx::ShaderHandle fragment_shader, std::vector<bgfx_uniform*> uniforms);
	~bgfx_effect();

	void submit(int view, uint64_t blend = ~0ULL);
	bgfx_uniform *uniform(std::string name);
	bool is_valid() { return m_program_handle.idx != bgfx::kInvalidHandle; }

private:
	std::string                          m_name;
	uint64_t                             m_state;
	bgfx::ProgramHandle                  m_program_handle;
	std::map<std::string, bgfx_uniform*> m_uniforms;
};

#endif // __DRAWBGFX_EFFECT__
