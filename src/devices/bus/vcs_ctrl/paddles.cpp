// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Atari Video Computer System analog paddles emulation

**********************************************************************/

#include "emu.h"
#include "paddles.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(VCS_PADDLES, vcs_paddles_device, "vcs_paddles", "Atari / CBM Dual paddles")


static INPUT_PORTS_START( vcs_paddles )
	PORT_START("JOY")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) // pin 3
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) // pin 4
	PORT_BIT( 0xf3, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("POTX")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE) PORT_PLAYER(1) PORT_SENSITIVITY(30) PORT_KEYDELTA(20) PORT_MINMAX(0, 255) PORT_REVERSE // pin 5

	PORT_START("POTY")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE) PORT_PLAYER(2) PORT_SENSITIVITY(30) PORT_KEYDELTA(20) PORT_MINMAX(0, 255) PORT_REVERSE // pin 9
INPUT_PORTS_END

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor vcs_paddles_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( vcs_paddles );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vcs_paddles_device - constructor
//-------------------------------------------------

vcs_paddles_device::vcs_paddles_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, VCS_PADDLES, tag, owner, clock),
	device_vcs_control_port_interface(mconfig, *this),
	m_joy(*this, "JOY"),
	m_potx(*this, "POTX"),
	m_poty(*this, "POTY"),
	m_reverse_players(false)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vcs_paddles_device::device_start()
{
}


//-------------------------------------------------
//  vcs_joy_r - joystick read
//-------------------------------------------------

uint8_t vcs_paddles_device::vcs_joy_r()
{
	uint8_t const data = m_joy->read();
	return m_reverse_players ? bitswap<8>(data, 7, 6, 5, 4, 2, 3, 1, 0) : data;
}


//-------------------------------------------------
//  vcs_pot_x_r - potentiometer X read
//-------------------------------------------------

uint8_t vcs_paddles_device::vcs_pot_x_r()
{
	return m_reverse_players ? m_poty->read() : m_potx->read();
}


//-------------------------------------------------
//  vcs_pot_y_r - potentiometer Y read
//-------------------------------------------------

uint8_t vcs_paddles_device::vcs_pot_y_r()
{
	return m_reverse_players ? m_potx->read() : m_poty->read();
}
