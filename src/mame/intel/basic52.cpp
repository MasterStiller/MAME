// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

        MCS BASIC 52 and MCS BASIC 31 board

        03/12/2009 Skeleton driver.

        2012-08-08 Made to work [Robbbert]

BASIC-52 is an official Intel release.

BASIC-31 (and variants) as found on the below url, are homebrews.

https://web.archive.org/web/20110908004037/http://dsaprojects.110mb.com/electronics/8031-ah/8031-bas.html


The driver is working, however there are issues with the cpu serial code.
When started, you are supposed to press Space and the system works out
the baud rate and boots up.

However, the way the cpu is written, it actually passes bytes around, so
the auto-speed detection doesn't work as intended. Also the cpu interface
is horribly outdated and needs to be brought up to date.

The optional eprom presents the ability to bypass the auto-speed problems.

Once the system starts, all input must be in uppercase. Read the manual
to discover the special features of this Basic.

Sound: BASIC-31 has sound, and BASIC-52 doesn't. The sound command is PWM.
       Example: PWM 800,200,900

****************************************************************************/

#include "emu.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/i8255.h"
#include "machine/terminal.h"
#include "sound/spkrdev.h"
#include "speaker.h"


namespace {

class basic52_state : public driver_device
{
public:
	basic52_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_terminal(*this, "terminal")
		, m_speaker(*this, "speaker")
	{ }

	void basic52(machine_config &config);
	void basic31(machine_config &config);

private:
	void machine_start() override;
	void kbd_put(u8 data);
	void port1_w(u8 data);
	uint8_t unk_r();
	uint8_t from_term();
	void io_map(address_map &map);
	void mem_map(address_map &map);
	uint8_t m_term_data = 0U;
	required_device<mcs51_cpu_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
	required_device<speaker_sound_device> m_speaker;
};


void basic52_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x1fff).rom();
}

void basic52_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff).ram();
	// 8000-9FFF is reserved for a plug-in EPROM containing BASIC programs.
	// We have used this to preset the baud rate and skip the unreliable auto-detect.
	map(0x8000, 0x8000).lr8(NAME([] () { return 0x31; }));  // we will be manually setting the baud rate
	map(0x8001, 0x8002).lr8(NAME([] () { return 0xFE; }));  // to the fastest possible
	map(0xa000, 0xa003).rw("ppi8255", FUNC(i8255_device::read), FUNC(i8255_device::write));  // PPI-8255
}

/* Input ports */
static INPUT_PORTS_START( basic52 )
INPUT_PORTS_END


uint8_t basic52_state::from_term()
{
	return m_term_data;
}

uint8_t basic52_state::unk_r()
{
	return m_term_data; // won't boot without this
}

void basic52_state::port1_w(u8 data)
{
	m_speaker->level_w(BIT(data, 2));
}


void basic52_state::kbd_put(u8 data)
{
	m_maincpu->set_input_line(MCS51_RX_LINE, ASSERT_LINE);
	m_maincpu->set_input_line(MCS51_RX_LINE, CLEAR_LINE);
	m_term_data = data;
}

void basic52_state::machine_start()
{
	save_item(NAME(m_term_data));
}

void basic52_state::basic31(machine_config &config)
{
	/* basic machine hardware */
	I8031(config, m_maincpu, XTAL(11'059'200));
	m_maincpu->set_addrmap(AS_PROGRAM, &basic52_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &basic52_state::io_map);
	m_maincpu->port_out_cb<1>().set(FUNC(basic52_state::port1_w));
	m_maincpu->port_in_cb<3>().set(FUNC(basic52_state::unk_r));
	m_maincpu->serial_tx_cb().set(m_terminal, FUNC(generic_terminal_device::write));
	m_maincpu->serial_rx_cb().set(FUNC(basic52_state::from_term));

	/* video hardware */
	GENERIC_TERMINAL(config, m_terminal, 0);
	m_terminal->set_keyboard_callback(FUNC(basic52_state::kbd_put));

	I8255(config, "ppi8255", 0);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, "speaker").add_route(ALL_OUTPUTS, "mono", 0.50);
}

void basic52_state::basic52(machine_config &config)
{
	basic31(config);
	/* basic machine hardware */
	I8052(config.replace(), m_maincpu, XTAL(11'059'200));
	m_maincpu->set_addrmap(AS_PROGRAM, &basic52_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &basic52_state::io_map);
	m_maincpu->port_in_cb<3>().set(FUNC(basic52_state::unk_r));
	m_maincpu->serial_tx_cb().set(m_terminal, FUNC(generic_terminal_device::write));
	m_maincpu->serial_rx_cb().set(FUNC(basic52_state::from_term));
}

/* ROM definition */
ROM_START( basic52 )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_SYSTEM_BIOS(0, "v11", "v 1.1")
	ROMX_LOAD( "mcs-51-11.bin",  0x0000, 0x2000, CRC(4157b22b) SHA1(bd9e6869b400cc1c9b163243be7bdcf16ce72789), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v11b", "v 1.1b")
	ROMX_LOAD( "mcs-51-11b.bin", 0x0000, 0x2000, CRC(a60383cc) SHA1(9515cc435e2ca3d3adb19631c03a62dfbeab0826), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "v131", "v 1.3.1")
	ROMX_LOAD( "mcs-51-131.bin", 0x0000, 0x2000, CRC(6a493162) SHA1(ed1079a6b4d4dbf448e15238c5a9e4dd004e401c), ROM_BIOS(2))
ROM_END

ROM_START( basic31 )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_SYSTEM_BIOS(0, "v12", "v 1.2")
	ROMX_LOAD( "mcs-51-12.bin",  0x0000, 0x2000, CRC(ee667c7c) SHA1(e69b32e69ecda2012c7113649634a3a64e984bed), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v12a", "v 1.2a")
	ROMX_LOAD( "mcs-51-12a.bin", 0x0000, 0x2000, CRC(225bb2f0) SHA1(46e97643a7a5cb4c278f9e3c73d18cd93209f8bf), ROM_BIOS(1))
ROM_END

} // anonymous namespace


/* Driver */
/*    YEAR  NAME     PARENT   COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY  FULLNAME        FLAGS */
COMP( 1985, basic52, 0,       0,      basic52, basic52, basic52_state, empty_init, "Intel", "MCS BASIC 52", MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
COMP( 1985, basic31, basic52, 0,      basic31, basic52, basic52_state, empty_init, "Intel", "MCS BASIC 31", MACHINE_SUPPORTS_SAVE )
