// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

Steel Force

ELECTRONIC DEVICES 1994 Milano Italy
ECOGAMES S.L. Barcelona, Spain

driver by David Haywood
inputs etc. by stephh

----------------------------------------

68000P12 processor
15mHZ crystal next to it

2 of these:

TPC 1020AFN-084c

32MHz crystal close to this.

1 GAL
5 PROMS  (16S25H)

27c4001
u1, u27, u28, u29, u30

27c2001
u31,u32, u33, u34

27c010
u104, u105

----------------------------------------

notes:

lev 1 : 0x64 : 0000 0100 - just rte
lev 2 : 0x68 : 0000 0100 - just rte
lev 3 : 0x6c : 0000 0100 - just rte
lev 4 : 0x70 : 0000 CBD6 - vblank
lev 5 : 0x74 : 0000 0100 - just rte
lev 6 : 0x78 : 0000 0100 - just rte
lev 7 : 0x7c : 0000 0100 - just rte


  2002.02.03 : There doesn't seem to be Dip Switches
               (you make the changes in the "test mode")
               Bits 8 to 15 of IN1 seem to be unused
               The 2nd part of the "test mode" ("sound and video") is in Spanish
               Release date and manufacturers according to the title screen

 2004.xx.10 - Pierpaolo Prazzoli
 - fixed bit 4 of IN1. it is vblank and it fixed scroll issue in attract mode
 - fixed sprite glitches with visible flag
 - added rows scroll
 - added eeprom
 - fixed sound banking

TO DO :
  - unknown registers
  - clipping issues?
  - priority issues?
  - same sprites buffer used in Mighty Warriors
  - clocks don't match on the games?

*/

#include "emu.h"

#include "edevices.h"

#include "cpu/m68000/m68000.h"
#include "machine/eepromser.h"
#include "sound/okim6295.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class stlforce_state : public driver_device
{
public:
	stlforce_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_eeprom(*this, "eeprom"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void stlforce(machine_config &config);

	void program_map(address_map &map);

	required_device<cpu_device> m_maincpu;

private:
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	void eeprom_w(uint8_t data);
};

class twinbrat_state : public stlforce_state
{
public:
	twinbrat_state(const machine_config &mconfig, device_type type, const char *tag) :
		stlforce_state(mconfig, type, tag),
		m_okibank(*this, "okibank")
	{ }

	void twinbrat(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	required_memory_bank m_okibank;

	void oki_bank_w(uint8_t data);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void program_map(address_map &map);
	void oki_map(address_map &map);
};


void twinbrat_state::machine_start()
{
	m_okibank->configure_entries(0, 4, memregion("oki")->base(), 0x20000);
	m_okibank->set_entry(0);
}

void stlforce_state::eeprom_w(uint8_t data)
{
	m_eeprom->di_write(data & 0x01);
	m_eeprom->cs_write((data & 0x02) ? ASSERT_LINE : CLEAR_LINE );
	m_eeprom->clk_write((data & 0x04) ? ASSERT_LINE : CLEAR_LINE );
}

void twinbrat_state::oki_bank_w(uint8_t data)
{
	m_okibank->set_entry(data & 3);
}

void stlforce_state::program_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x100000, 0x1007ff).ram().w("video", FUNC(edevices_device::bg_videoram_w)).share("bg_videoram");
	map(0x100800, 0x100fff).ram().w("video", FUNC(edevices_device::mlow_videoram_w)).share("mlow_videoram");
	map(0x101000, 0x1017ff).ram().w("video", FUNC(edevices_device::mhigh_videoram_w)).share("mhigh_videoram");
	map(0x101800, 0x1027ff).ram().w("video", FUNC(edevices_device::tx_videoram_w)).share("tx_videoram");
	map(0x102800, 0x102fff).ram(); // unknown / RAM
	map(0x103000, 0x1033ff).ram().share("bg_scrollram");
	map(0x103400, 0x1037ff).ram().share("mlow_scrollram");
	map(0x103800, 0x103bff).ram().share("mhigh_scrollram");
	map(0x103c00, 0x103fff).ram().share("vidattrram");
	map(0x104000, 0x104fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x105000, 0x107fff).ram(); // unknown / RAM
	map(0x108000, 0x1087ff).ram().share("spriteram");
	map(0x108800, 0x108fff).ram();
	map(0x109000, 0x11ffff).ram();
	map(0x400000, 0x400001).portr("INPUT");
	map(0x400002, 0x400003).portr("SYSTEM");
	map(0x400011, 0x400011).w(FUNC(stlforce_state::eeprom_w));
	map(0x40001e, 0x40001f).w("video", FUNC(edevices_device::sprites_commands_w));
	map(0x410001, 0x410001).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
}

void twinbrat_state::program_map(address_map &map)
{
	stlforce_state::program_map(map);

	map(0x400012, 0x400012).w(FUNC(twinbrat_state::oki_bank_w));
}

void twinbrat_state::oki_map(address_map &map)
{
	map(0x00000, 0x1ffff).rom();
	map(0x20000, 0x3ffff).bankr(m_okibank);
}

static INPUT_PORTS_START( stlforce )
	PORT_START("INPUT")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE_NO_TOGGLE( 0x0008, IP_ACTIVE_LOW )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static const gfx_layout stlforce_bglayout =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{0,1,2,3},
	{12,8,4,0,28,24,20,16,16*32+12,16*32+8,16*32+4,16*32+0,16*32+28,16*32+24,16*32+20,16*32+16},
	{0*32,1*32,2*32,3*32,4*32,5*32,6*32,7*32,8*32,9*32,10*32,11*32,12*32,13*32,14*32,15*32},
	32*32
};

static const gfx_layout stlforce_txlayout =
{
	8,8,
	RGN_FRAC(1,4),
	4,
	{0,1,2,3},
	{12,8,4,0,28,24,20,16},
	{0*32,1*32,2*32,3*32,4*32,5*32,6*32,7*32 },
	8*32
};

static const gfx_layout stlforce_splayout =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{0x040000*3*8,0x040000*2*8,0x040000*1*8,0x040000*0*8},
	{16*8+7,16*8+6,16*8+5,16*8+4,16*8+3,16*8+2,16*8+1,16*8+0,7,6,5,4,3,2,1,0},
	{0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8,8*8,9*8,10*8,11*8,12*8,13*8,14*8,15*8},
	32*8
};

static GFXDECODE_START( gfx_stlforce )
	GFXDECODE_ENTRY( "sprites",      0, stlforce_splayout, 1024, 16  )
	GFXDECODE_ENTRY( "tiles", 0x180000, stlforce_txlayout, 384,   8  )
	GFXDECODE_ENTRY( "tiles", 0x100000, stlforce_bglayout, 256,   8  )
	GFXDECODE_ENTRY( "tiles", 0x080000, stlforce_bglayout, 128,   8  )
	GFXDECODE_ENTRY( "tiles", 0x000000, stlforce_bglayout, 0,     8  )
GFXDECODE_END


void stlforce_state::stlforce(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, XTAL(15'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &stlforce_state::program_map);
	m_maincpu->set_vblank_int("screen", FUNC(stlforce_state::irq4_line_hold));

	EEPROM_93C46_16BIT(config, "eeprom");

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(58);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	screen.set_size(64*8, 32*8);
	screen.set_visarea(8, 48*8-1-8-2, 0, 30*8-1);
	screen.set_screen_update("video", FUNC(edevices_sforce_device::draw));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_stlforce);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 0x800);

	edevices_sforce_device &video(EDEVICES_SFORCE_VID(config, "video", 0));
	video.set_bg_videoram_tag("bg_videoram");
	video.set_mlow_videoram_tag("mlow_videoram");
	video.set_mhigh_videoram_tag("mhigh_videoram");
	video.set_tx_videoram_tag("tx_videoram");
	video.set_bg_scrollram_tag("bg_scrollram");
	video.set_mlow_scrollram_tag("mlow_scrollram");
	video.set_mhigh_scrollram_tag("mhigh_scrollram");
	video.set_vidattrram_tag("vidattrram");
	video.set_spriteram_tag("spriteram");
	video.set_gfxdecode_tag("gfxdecode");
	video.set_palette_tag("palette");

	// sound hardware
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, "oki", XTAL(32'000'000) / 32, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0);
}

void twinbrat_state::twinbrat(machine_config &config)
{
	stlforce(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &twinbrat_state::program_map);
	m_maincpu->set_clock(XTAL(14'745'600));

	subdevice<screen_device>("screen")->set_visarea(3*8, 44*8-1, 0*8, 30*8-1);

	subdevice<edevices_sforce_device>("video")->set_spritexoffset(10);

	subdevice<okim6295_device>("oki")->set_clock(XTAL(30'000'000) / 32); // verified on 2 PCBs
	subdevice<okim6295_device>("oki")->set_addrmap(0, &twinbrat_state::oki_map);
}

ROM_START( stlforce )
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "stlforce.105", 0x00000, 0x20000, CRC(3ec804ca) SHA1(4efcf3321b7111644ac3ee0a83ad95d0571a4021) )
	ROM_LOAD16_BYTE( "stlforce.104", 0x00001, 0x20000, CRC(69b5f429) SHA1(5bd20fad91a22f4d62f85a5190d72dd824ee26a5) )

	ROM_REGION( 0x200000, "tiles", 0 ) // 16x16 bg tiles & 8x8 tx tiles merged
	ROM_LOAD16_BYTE( "stlforce.u27", 0x000001, 0x080000, CRC(c42ef365) SHA1(40e9ee29ea14b3bc2fbfa4e6acb7d680cf72f01a) )
	ROM_LOAD16_BYTE( "stlforce.u28", 0x000000, 0x080000, CRC(6a4b7c98) SHA1(004d7f3c703c6abc79286fa58a4c6793d66fca39) )
	ROM_LOAD16_BYTE( "stlforce.u29", 0x100001, 0x080000, CRC(30488f44) SHA1(af0d92d8952ce3cd893ab9569afdda12e17795e7) )
	ROM_LOAD16_BYTE( "stlforce.u30", 0x100000, 0x080000, CRC(cf19d43a) SHA1(dc04930548ac5b7e2b74c6041325eac06e773ed5) )

	ROM_REGION( 0x100000, "sprites", 0 ) // 16x16
	ROM_LOAD( "stlforce.u36", 0x00000, 0x40000, CRC(037dfa9f) SHA1(224f5cd1a95d55b065aef5c0bd03b50cabcb619b) )
	ROM_LOAD( "stlforce.u31", 0x40000, 0x40000, CRC(305a8eb5) SHA1(3a8d26f8bc4ec2e8246d1c59115e21cad876630d) )
	ROM_LOAD( "stlforce.u32", 0x80000, 0x40000, CRC(760e8601) SHA1(a61f1d8566e09ce811382c6e23f3881e6c438f15) )
	ROM_LOAD( "stlforce.u33", 0xc0000, 0x40000, CRC(19415cf3) SHA1(31490a1f3321558f82667b63f3963b2ec3fa0c59) )

	// only one bank
	ROM_REGION( 0x80000, "oki", 0 ) // samples, second half 0xff filled
	ROM_LOAD( "stlforce.u1", 0x00000, 0x80000, CRC(0a55edf1) SHA1(091f12e8110c62df22b370a2e710c930ba06e8ca) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "eeprom-stlforce.bin", 0x0000, 0x0080, CRC(3fb83951) SHA1(0cbf09751e46f100db847cf0594a4440126a7b6e) )
ROM_END



/*

Twin Brats
Elettronica Video-Games S.R.L, 1995

PCB Layout
----------

|----------------------------------------------|
|  1.BIN                                       |
|      M6295  PAL                  6116   6116 |
|                                  6116   6116 |
|   62256                PAL                   |
|   62256                                      |
|                        6116                  |
|J  6116   |---------|   6116                  |
|A  6116   |ACTEL    |                         |
|M         |A1020A   |                         |
|M         |PL84C    |                         |
|A    PAL  |         |       30MHz       11.BIN|
|          |---------|6264   PAL         10.BIN|
|     62256    62256  6264   |---------| 9.BIN |
|     2.BIN    3.BIN         |ACTEL    | 8.BIN |
|   |-----------------|      |A1020A   | 7.BIN |
|   |   MC68000P12    |      |PL84C    | 6.BIN |
| * |                 | PAL  |         | 5.BIN |
|   |-----------------| PAL  |---------| 4.BIN |
| 93C46  14.7456MHz                            |
|----------------------------------------------|
Notes:
      68000 clock : 14.7456MHz
      M6295 clock : 0.9375MHz (30/32). Sample Rate = 937500 / 132
      VSync       : 58Hz
      *           : Push button test switch

*/


ROM_START( twinbrat )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "12.u105", 0x00000, 0x20000, CRC(552529b1) SHA1(bf23680335e1c5b05b80ab139609bee9f239b910) ) // higher numbers are newer??
	ROM_LOAD16_BYTE( "13.u104", 0x00001, 0x20000, CRC(9805ba90) SHA1(cdc188fa38220d18c60c9f438520ee574e6ce0f7) ) // higher numbers are newer??

	ROM_REGION( 0x200000, "tiles", 0 )
	ROM_LOAD16_BYTE( "6.bin", 0x000000, 0x80000, CRC(af10ddfd) SHA1(e5e83044f20d6cbbc1b4ef1812ac57b6dc958a8a) )
	ROM_LOAD16_BYTE( "7.bin", 0x000001, 0x80000, CRC(3696345a) SHA1(ea38be3586757527b2a1aad2e22b83937f8602da) )
	ROM_LOAD16_BYTE( "4.bin", 0x100000, 0x80000, CRC(1ae8a751) SHA1(5f30306580c6ab4af0ddbdc4519eb4e0ab9bd23a) )
	ROM_LOAD16_BYTE( "5.bin", 0x100001, 0x80000, CRC(cf235eeb) SHA1(d067e2dd4f28a8986dd76ec0eba90e1adbf5787c) )

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD( "11.bin", 0x000000, 0x40000, CRC(00eecb03) SHA1(5913da4d2ad97c1ce5e8e601a22b499cd93af744) )
	ROM_LOAD( "10.bin", 0x040000, 0x40000, CRC(7556bee9) SHA1(3fe99c7e9378791b79c43b04f5d0a36404448beb) )
	ROM_LOAD( "9.bin",  0x080000, 0x40000, CRC(13194d89) SHA1(95c35b6012f98a64630abb40fd55b24ff8a5e031) )
	ROM_LOAD( "8.bin",  0x0c0000, 0x40000, CRC(79f14528) SHA1(9c07d9a9e59f69a525bbaec05d74eb8d21bb9563) )

	ROM_REGION( 0x080000, "oki", 0 ) // Samples, 0x00000 - 0x20000 fixed, 0x20000 - 0x40000 banked
	ROM_LOAD( "1.bin", 0x00000, 0x80000, CRC(76296578) SHA1(04eca78abe60b283269464c0d12815579126ac08) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "eeprom-twinbrat.bin", 0x0000, 0x0080, CRC(9366263d) SHA1(ff5155498ed0b349ecc1ce98a39566b642201cf2) )
ROM_END

ROM_START( twinbrata )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "2.u105", 0x00000, 0x20000, CRC(33a9bb82) SHA1(0f54239397c93e264b9b211f67bf626acf1246a9) )
	ROM_LOAD16_BYTE( "3.u104", 0x00001, 0x20000, CRC(b1186a67) SHA1(502074063101885874db76ae707db1082313efcf) )

	ROM_REGION( 0x200000, "tiles", 0 )
	ROM_LOAD16_BYTE( "6.bin", 0x000000, 0x80000, CRC(af10ddfd) SHA1(e5e83044f20d6cbbc1b4ef1812ac57b6dc958a8a) )
	ROM_LOAD16_BYTE( "7.bin", 0x000001, 0x80000, CRC(3696345a) SHA1(ea38be3586757527b2a1aad2e22b83937f8602da) )
	ROM_LOAD16_BYTE( "4.bin", 0x100000, 0x80000, CRC(1ae8a751) SHA1(5f30306580c6ab4af0ddbdc4519eb4e0ab9bd23a) )
	ROM_LOAD16_BYTE( "5.bin", 0x100001, 0x80000, CRC(cf235eeb) SHA1(d067e2dd4f28a8986dd76ec0eba90e1adbf5787c) )

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD( "11.bin", 0x000000, 0x40000, CRC(00eecb03) SHA1(5913da4d2ad97c1ce5e8e601a22b499cd93af744) )
	ROM_LOAD( "10.bin", 0x040000, 0x40000, CRC(7556bee9) SHA1(3fe99c7e9378791b79c43b04f5d0a36404448beb) )
	ROM_LOAD( "9.bin",  0x080000, 0x40000, CRC(13194d89) SHA1(95c35b6012f98a64630abb40fd55b24ff8a5e031) )
	ROM_LOAD( "8.bin",  0x0c0000, 0x40000, CRC(79f14528) SHA1(9c07d9a9e59f69a525bbaec05d74eb8d21bb9563) )

	ROM_REGION( 0x080000, "oki", 0 ) // Samples, 0x00000 - 0x20000 fixed, 0x20000 - 0x40000 banked
	ROM_LOAD( "1.bin", 0x00000, 0x80000, CRC(76296578) SHA1(04eca78abe60b283269464c0d12815579126ac08) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "eeprom-twinbrat.bin", 0x0000, 0x0080, CRC(9366263d) SHA1(ff5155498ed0b349ecc1ce98a39566b642201cf2) )
ROM_END

ROM_START( twinbratb )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "2.bin", 0x00000, 0x20000, CRC(5e75f568) SHA1(f42d2a73d737e6b01dd049eea2a10fc8c8096d8f) )
	ROM_LOAD16_BYTE( "3.bin", 0x00001, 0x20000, CRC(0e3fa9b0) SHA1(0148cc616eac84dc16415e1557ec6040d14392d4) )

	ROM_REGION( 0x200000, "tiles", 0 )
	ROM_LOAD16_BYTE( "6.bin", 0x000000, 0x80000, CRC(af10ddfd) SHA1(e5e83044f20d6cbbc1b4ef1812ac57b6dc958a8a) )
	ROM_LOAD16_BYTE( "7.bin", 0x000001, 0x80000, CRC(3696345a) SHA1(ea38be3586757527b2a1aad2e22b83937f8602da) )
	ROM_LOAD16_BYTE( "4.bin", 0x100000, 0x80000, CRC(1ae8a751) SHA1(5f30306580c6ab4af0ddbdc4519eb4e0ab9bd23a) )
	ROM_LOAD16_BYTE( "5.bin", 0x100001, 0x80000, CRC(cf235eeb) SHA1(d067e2dd4f28a8986dd76ec0eba90e1adbf5787c) )

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD( "11.bin", 0x000000, 0x40000, CRC(00eecb03) SHA1(5913da4d2ad97c1ce5e8e601a22b499cd93af744) )
	ROM_LOAD( "10.bin", 0x040000, 0x40000, CRC(7556bee9) SHA1(3fe99c7e9378791b79c43b04f5d0a36404448beb) )
	ROM_LOAD( "9.bin",  0x080000, 0x40000, CRC(13194d89) SHA1(95c35b6012f98a64630abb40fd55b24ff8a5e031) )
	ROM_LOAD( "8.bin",  0x0c0000, 0x40000, CRC(79f14528) SHA1(9c07d9a9e59f69a525bbaec05d74eb8d21bb9563) )

	ROM_REGION( 0x080000, "oki", 0 ) // Samples, 0x00000 - 0x20000 fixed, 0x20000 - 0x40000 banked
	ROM_LOAD( "1.bin", 0x00000, 0x80000, CRC(76296578) SHA1(04eca78abe60b283269464c0d12815579126ac08) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "eeprom-twinbrat.bin", 0x0000, 0x0080, CRC(9366263d) SHA1(ff5155498ed0b349ecc1ce98a39566b642201cf2) )
ROM_END

} // anonymous namespace


GAME( 1994, stlforce,  0,        stlforce, stlforce, stlforce_state, empty_init, ROT0, "Electronic Devices Italy / Ecogames S.L. Spain", "Steel Force", MACHINE_SUPPORTS_SAVE )

GAME( 1995, twinbrat,  0,        twinbrat, stlforce, twinbrat_state, empty_init, ROT0, "Elettronica Video-Games S.R.L.", "Twin Brats (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1995, twinbrata, twinbrat, twinbrat, stlforce, twinbrat_state, empty_init, ROT0, "Elettronica Video-Games S.R.L.", "Twin Brats (set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1995, twinbratb, twinbrat, twinbrat, stlforce, twinbrat_state, empty_init, ROT0, "Elettronica Video-Games S.R.L.", "Twin Brats (set 3)", MACHINE_SUPPORTS_SAVE )
