// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  slider.cpp - BGFX shader parameter slider
//
//============================================================

#include "slider.h"
#include "../frontend/mame/ui/slider.h"

#include "strformat.h"

#include <utility>

bgfx_slider::bgfx_slider(running_machine &machine, std::string &&name, float min, float def, float max, float step, slider_type type, screen_type screen, std::string format, std::string description, std::vector<std::string>& strings)
	: m_name(std::move(name))
	, m_step(step)
	, m_type(type)
	, m_screen_type(screen)
	, m_format(format)
	, m_description(description)
	, m_machine(machine)
{
	m_min = min;
	m_default = def;
	m_max = max;
	m_value = def;

	for (const std::string &string : strings)
	{
		m_strings.push_back(string);
	}

	m_slider_state = create_core_slider();
}

bgfx_slider::~bgfx_slider()
{
}

void bgfx_slider::import(float val)
{
	m_value = val;
	update(nullptr, int32_t(floor(m_value / m_step + 0.5f)));
}

std::unique_ptr<slider_state> bgfx_slider::create_core_slider()
{
	int32_t minval = int32_t(floor(m_min / m_step + 0.5f));
	int32_t defval = int32_t(floor(m_default / m_step + 0.5f));
	int32_t maxval = int32_t(floor(m_max / m_step + 0.5f));
	int32_t incval = int32_t(floor(m_step / m_step + 0.5f));

	using namespace std::placeholders;
	return std::make_unique<slider_state>(m_description, minval, defval, maxval, incval, std::bind(&bgfx_slider::update, this, _1, _2));
}

int32_t bgfx_slider::update(std::string *str, int32_t newval)
{
	switch (m_type)
	{
		case SLIDER_INT_ENUM:
		{
			if (newval != SLIDER_NOCHANGE)
			{
				m_value = float(newval);
			}
			if (str != nullptr)
			{
				*str = util::string_format(m_format, m_strings[as_int()]);
			}
			return as_int();
		}

		case SLIDER_INT:
		{
			if (newval != SLIDER_NOCHANGE)
			{
				m_value = float(newval);
			}
			if (str != nullptr)
			{
				*str = util::string_format(m_format, as_int());
			}
			return as_int();
		}

		default:
		{
			auto *val_ptr = reinterpret_cast<float *>(&m_value);
			if (newval != SLIDER_NOCHANGE)
			{
				*val_ptr = float(newval) * m_step;
			}
			if (str != nullptr)
			{
				*str = util::string_format(m_format, *val_ptr);
			}
			return int32_t(floor(*val_ptr / m_step + 0.5f));
		}
	}
	return 0;
}

size_t bgfx_slider::get_size_for_type(slider_type type)
{
	switch(type)
	{
		case SLIDER_INT_ENUM:
		case SLIDER_FLOAT:
		case SLIDER_INT:
			return sizeof(float);
		case SLIDER_COLOR:
			return sizeof(float) * 3;
		case SLIDER_VEC2:
			return sizeof(float) * 2;
		default:
			return 0;
	}
}
