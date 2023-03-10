// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************

 A7800 CPUWIZ's homebrew boards (MegaCart+ and VersaBoard)

 Here we emulate the base configurations of these two boards:

 MegaCart+ = up to 512K (31 banks at $8000, 1 at $C000) of ROM and 2 x 16K RAM @ $4000
 VersaBoard = up to 256K of ROM and 2 x 16K RAM

 Plus, for the moment, a VersaBoard with POKEY mapped at 0x0450 and support for 144K ROM,
 since a few demo homebrew programs seems to use this to combine compatibility with
 XBoarD & XM expansions

 Note that the VersaBoard can be configured to work with different banking hardware
 e.g. with SG 9bank games or with SG + RAM (so to allow reproduction of games which
 could have worked on old carts without sacrifying original carts), but games running
 on those "standard" variants can be emulated with the standard code from rom.c ;-)


 TO DO:
 - investigate whether the POKEY detection routines in homebrew do fail due to emulation
   issues or not

***********************************************************************************************************/


#include "emu.h"
#include "cpuwiz.h"
#include "speaker.h"


//-------------------------------------------------
//  constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(A78_ROM_VERSABOARD, a78_versaboard_device, "a78_versaboard", "Atari 7800 VersaBoard Cart")
DEFINE_DEVICE_TYPE(A78_ROM_MEGACART, a78_megacart_device, "a78_megacart", "Atari 7800 MegaCart+")

DEFINE_DEVICE_TYPE(A78_ROM_P450_VB, a78_rom_p450_vb_device, "a78_versapokey", "Atari 7800 VersaBoard + POKEY @ 0x450 Cart")


a78_versaboard_device::a78_versaboard_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: a78_rom_sg_device(mconfig, type, tag, owner, clock), m_ram_bank(0)
{
}

a78_versaboard_device::a78_versaboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a78_versaboard_device(mconfig, A78_ROM_VERSABOARD, tag, owner, clock)
{
}


a78_megacart_device::a78_megacart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a78_versaboard_device(mconfig, A78_ROM_MEGACART, tag, owner, clock)
{
}


a78_rom_p450_vb_device::a78_rom_p450_vb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a78_versaboard_device(mconfig, A78_ROM_P450_VB, tag, owner, clock)
	, m_pokey450(*this, "pokey450")
{
}


void a78_versaboard_device::device_start()
{
	save_item(NAME(m_bank));
	save_item(NAME(m_ram_bank));
}

void a78_versaboard_device::device_reset()
{
	m_bank = 0;
	m_ram_bank = 0;
}


// VersaBoard

uint8_t a78_versaboard_device::read_40xx(offs_t offset)
{
	if (offset < 0x4000)
		return m_ram[offset + (m_ram_bank * 0x4000)];
	else if (offset < 0x8000)
		return m_rom[(offset & 0x3fff) + (m_bank * 0x4000)];
	else
		return m_rom[(offset & 0x3fff) + (m_bank_mask * 0x4000)];   // last bank
}

void a78_versaboard_device::write_40xx(offs_t offset, uint8_t data)
{
	if (offset < 0x4000)
		m_ram[offset + (m_ram_bank * 0x4000)] = data;
	else if (offset < 0x8000)
	{
		// hardware allows up to 256K ROM
		m_bank = (data & 0x0f) & m_bank_mask;
		m_ram_bank = BIT(data, 5);
	}
}


// MegaCart+

void a78_megacart_device::write_40xx(offs_t offset, uint8_t data)
{
	if (offset < 0x4000)
		m_ram[offset + (m_ram_bank * 0x4000)] = data;
	else if (offset < 0x8000)
	{
		// hardware allows up to 512K ROM
		m_bank = (data & 0x1f) & m_bank_mask;
		m_ram_bank = BIT(data, 5);
	}
}


// VersaBoard + POKEY @ 0x0450

void a78_rom_p450_vb_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "addon").front_center();

	POKEY(config, m_pokey450, DERIVED_CLOCK(1, 1)).add_route(ALL_OUTPUTS, "addon", 1.00);
}
