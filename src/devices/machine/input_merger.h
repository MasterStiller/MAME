// license:BSD-3-Clause
// copyright-holders:Dirk Best, Vas Crabb
/***************************************************************************

    Input Merger

***************************************************************************/

#ifndef MAME_MACHINE_INPUT_MERGER_H
#define MAME_MACHINE_INPUT_MERGER_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class input_merger_device : public device_t
{
public:
	// configuration
	auto output_handler() { return m_output_handler.bind(); }
	auto &initial_state(u32 val) { m_initval = val; return *this; } // initial input pin(s) state, be wary about the unused pins

	// input lines
	template <unsigned Bit> DECLARE_WRITE_LINE_MEMBER(in_w) { static_assert(Bit < 32, "invalid bit"); m_update_state.synchronize((Bit << 1) | (state ? 1U : 0U)); }
	template <unsigned Bit> void in_set(u8 data) { in_w<Bit>(1); }
	template <unsigned Bit> void in_clear(u8 data) { in_w<Bit>(0); }

protected:
	// constructor/destructor
	input_merger_device(
			machine_config const &mconfig,
			device_type type,
			char const *tag,
			device_t *owner,
			uint32_t clock,
			u32 initval,
			u32 xorval,
			int active);
	virtual ~input_merger_device() override;

	// device-level overrides
	virtual void device_start() override;

	TIMER_CALLBACK_MEMBER(update_state);

	devcb_write_line m_output_handler;

	u32 m_initval;
	u32 const m_xorval;
	int const m_active;
	u32 m_state;
	transient_timer_factory m_update_state;
};


class input_merger_any_high_device : public input_merger_device
{
public:
	input_merger_any_high_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};


class input_merger_all_high_device : public input_merger_device
{
public:
	input_merger_all_high_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};


class input_merger_any_low_device : public input_merger_device
{
public:
	input_merger_any_low_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};


class input_merger_all_low_device : public input_merger_device
{
public:
	input_merger_all_low_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};


// device type definition
DECLARE_DEVICE_TYPE(INPUT_MERGER_ANY_HIGH, input_merger_any_high_device)
DECLARE_DEVICE_TYPE(INPUT_MERGER_ALL_HIGH, input_merger_all_high_device)
DECLARE_DEVICE_TYPE(INPUT_MERGER_ANY_LOW,  input_merger_any_low_device)
DECLARE_DEVICE_TYPE(INPUT_MERGER_ALL_LOW,  input_merger_all_low_device)

#endif // MAME_MACHINE_INPUT_MERGER_H
