// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  effect.cpp - BGFX shader material to be applied to a mesh
//
//============================================================

#include "effect.h"

#include "uniform.h"

#include "modules/osdmodule.h"
#include "osdcore.h"

bgfx_effect::bgfx_effect(std::string name, uint64_t state, bgfx::ShaderHandle vertex_shader, bgfx::ShaderHandle fragment_shader, std::vector<bgfx_uniform*> uniforms)
	: m_name(name)
	, m_state(state)
{
	m_program_handle = bgfx::createProgram(vertex_shader, fragment_shader, false);

	for (int i = 0; i < uniforms.size(); i++)
	{
		if (m_uniforms[uniforms[i]->name()] != nullptr)
		{
			osd_printf_verbose("Uniform %s appears to be duplicated in one or more effects, please double-check the effect JSON files.\n", uniforms[i]->name());
			delete uniforms[i];
			continue;
		}
		uniforms[i]->create();
		m_uniforms[uniforms[i]->name()] = uniforms[i];
	}
}

bgfx_effect::~bgfx_effect()
{
	for (std::pair<std::string, bgfx_uniform*> uniform : m_uniforms)
	{
		delete uniform.second;
	}
	m_uniforms.clear();
	bgfx::destroy(m_program_handle);
}

void bgfx_effect::submit(int view, uint64_t blend)
{
	for (std::pair<std::string, bgfx_uniform*> uniform_pair : m_uniforms)
	{
		(uniform_pair.second)->upload();
	}

	const uint64_t final_state = (blend != ~0ULL) ? ((m_state & ~BGFX_STATE_BLEND_MASK) | blend) : m_state;

	bgfx::setState(final_state);
	bgfx::submit(view, m_program_handle);
}

bgfx_uniform* bgfx_effect::uniform(std::string name)
{
	auto iter = m_uniforms.find(name);
	return iter != m_uniforms.end() ? iter->second : nullptr;
}
