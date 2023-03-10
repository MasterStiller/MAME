// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  timeparameter.h - Time-based dynamic shader param
//
//============================================================

#ifndef MAME_RENDER_BGFX_TIMEPARAMETER_H
#define MAME_RENDER_BGFX_TIMEPARAMETER_H

#pragma once

#include "parameter.h"

#include <string>

class bgfx_time_parameter : public bgfx_parameter
{
public:
	bgfx_time_parameter(std::string &&name, parameter_type type, double limit);

	virtual float value() override;
	virtual void tick(double delta) override;

private:
	double m_current_time;
	double m_limit;
};

#endif // MAME_RENDER_BGFX_TIMEPARAMETER_H
