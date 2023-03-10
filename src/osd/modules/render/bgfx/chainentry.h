// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  chainentry.h - BGFX shader post-processing node
//
//  Represents a single entry in a list of post-processing
//  passes to be applied to a screen quad or buffer.
//
//============================================================

#pragma once

#ifndef DRAWBGFX_CHAIN_ENTRY
#define DRAWBGFX_CHAIN_ENTRY

#include <bgfx/bgfx.h>

#include <string>
#include <vector>

#include "inputpair.h"
#include "targetmanager.h"
#include "chainmanager.h"

class bgfx_effect;
class bgfx_target;
class bgfx_entry_uniform;
class bgfx_suppressor;
class clear_state;
class texture_manager;
class target_manager;

class bgfx_chain_entry
{
public:
	bgfx_chain_entry(std::string name, bgfx_effect* effect, clear_state* clear, std::vector<bgfx_suppressor*> suppressors, std::vector<bgfx_input_pair*> inputs,
		std::vector<bgfx_entry_uniform*> uniforms, target_manager& targets, std::string output, bool apply_tint);
	~bgfx_chain_entry();

	void submit(int view, chain_manager::screen_prim &prim, texture_manager& textures, uint16_t screen_count, uint16_t screen_width, uint16_t screen_height,
		float screen_scale_x, float screen_scale_y, float screen_offset_x, float screen_offset_y, uint32_t rotation_type, bool swap_xy, int32_t screen);

	// Getters
	std::string name() const { return m_name; }
	std::vector<bgfx_input_pair*>& inputs() { return m_inputs; }
	bool skip();

private:
	void setup_auto_uniforms(chain_manager::screen_prim &prim, texture_manager& textures, uint16_t screen_count, uint16_t view_width, uint16_t view_height,
		uint16_t screen_width, uint16_t screen_height, float screen_scale_x, float screen_scale_y, float screen_offset_x, float screen_offset_y,
		uint32_t rotation_type, bool swap_xy, int32_t screen);
	void setup_viewsize_uniforms(uint16_t view_width, uint16_t view_height);
	void setup_screensize_uniforms(texture_manager& textures, uint16_t screen_width, uint16_t screen_height, int32_t screen);
	void setup_screenscale_uniforms(float screen_scale_x, float screen_scale_y);
	void setup_screenoffset_uniforms(float screen_offset_x, float screen_offset_y);
	void setup_screencount_uniforms(uint16_t screen_count);
	void setup_sourcesize_uniform(chain_manager::screen_prim &prim) const;
	void setup_targetsize_uniform(int32_t screen) const;
	void setup_targetscale_uniform(int32_t screen) const;
	void setup_rotationtype_uniform(uint32_t rotation_type) const;
	void setup_swapxy_uniform(bool swap_xy) const;
	void setup_quaddims_uniform(chain_manager::screen_prim &prim) const;
	void setup_screenindex_uniform(int32_t screen) const;

	bool setup_view(texture_manager& textures, int view, uint16_t screen_width, uint16_t screen_height, int32_t screen,
		uint16_t &out_view_width, uint16_t &out_view_height) const;
	void put_screen_buffer(uint16_t screen_width, uint16_t screen_height, uint32_t screen_tint, bgfx::TransientVertexBuffer* buffer) const;

	std::string                         m_name;
	bgfx_effect*                        m_effect;
	clear_state*                        m_clear;
	std::vector<bgfx_suppressor*>       m_suppressors;
	std::vector<bgfx_input_pair*>       m_inputs;
	std::vector<bgfx_entry_uniform*>    m_uniforms;
	target_manager&                     m_targets;
	std::string                         m_output;
	bool                                m_apply_tint;
};

#endif // __DRAWBGFX_CHAIN_ENTRY__
