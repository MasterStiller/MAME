// license:BSD-3-Clause
// copyright-holders:Stefan Jokisch
/***************************************************************************

    Atari Liberator hardware

    Games supported:
        * Liberator

    Known bugs:
        * none at this time

****************************************************************************

    Liberator Memory Map (for the main set, the other one is rearranged)
     (from the schematics/manual)

    HEX        R/W   D7 D6 D5 D4 D3 D2 D1 D0  function
    ---------+-----+------------------------+------------------------
    0000             D  D  D  D  D  D  D  D   XCOORD
    0001             D  D  D  D  D  D  D  D   YCOORD
    0002             D  D  D                  BIT MODE DATA
    ---------+-----+------------------------+------------------------
    0003-033F        D  D  D  D  D  D  D  D   Working RAM
    0340-3D3F        D  D  D  D  D  D  D  D   Screen RAM
    3D40-3FFF        D  D  D  D  D  D  D  D   Working RAM
    ---------+-----+------------------------+------------------------
    4000-403F    R   D  D  D  D  D  D  D  D   EARD*  read from non-volatile memory
    ---------+-----+------------------------+------------------------
    5000         R                        D   coin AUX   (CTRLD* set low)
    5000         R                     D      coin LEFT  (CTRLD* set low)
    5000         R                  D         coin RIGHT (CTRLD* set low)
    5000         R               D            SLAM       (CTRLD* set low)
    5000         R            D               SPARE      (CTRLD* set low)
    5000         R         D                  SPARE      (CTRLD* set low)
    5000         R      D                     COCKTAIL   (CTRLD* set low)
    5000         R   D                        SELF-TEST  (CTRLD* set low)
    5000         R               D  D  D  D   HDIR   (CTRLD* set high)
    5000         R   D  D  D  D               VDIR   (CTRLD* set high)
    ---------+-----+------------------------+------------------------
    5001         R                        D   SHIELD 2
    5001         R                     D      SHIELD 1
    5001         R                  D         FIRE 2
    5001         R               D            FIRE 1
    5001         R            D               SPARE      (CTRLD* set low)
    5001         R         D                  START 2
    5001         R      D                     START 1
    5001         R   D                        VBLANK
    ---------+-----+------------------------+------------------------
    6000-600F    W               D  D  D  D   base_ram*
    6200-621F    W   D  D  D  D  D  D  D  D   COLORAM*
    6400         W                            INTACK*
    6600         W               D  D  D  D   EARCON
    6800         W   D  D  D  D  D  D  D  D   STARTLG (planet frame)
    6A00         W                            WDOG*
    ---------+-----+------------------------+------------------------
    6C00         W            D               START LED 1
    6C01         W            D               START LED 2
    6C02         W            D               TBSWP*
    6C03         W            D               SPARE
    6C04         W            D               CTRLD*
    6C05         W            D               COINCNTRR
    6C06         W            D               COINCNTRL
    6C07         W            D               PLANET
    ---------+-----+------------------------+------------------------
    6E00-6E3F    W   D  D  D  D  D  D  D  D   EARWR*
    7000-701F        D  D  D  D  D  D  D  D   IOS2* (Pokey 2)
    7800-781F        D  D  D  D  D  D  D  D   IOS1* (Pokey 1)
    8000-EFFF    R   D  D  D  D  D  D  D  D   ROM
    -----------------------------------------------------------------


     Dip switches at D4 on the PCB for play options: (IN2)

    LSB  D1   D2   D3   D4   D5   D6   MSB
    SW8  SW7  SW6  SW5  SW4  SW3  SW2  SW1    Option
    -------------------------------------------------------------------------------------
    Off  Off                                 4 ships per game   <-
    On   Off                                 5 ships per game
    Off  On                                  6 ships per game
    On   On                                  8 ships per game
    -------------------------------------------------------------------------------------
              Off  Off                       Bonus ship every 15000 points
              On   Off                       Bonus ship every 20000 points   <-
              Off  On                        Bonus ship every 25000 points
              On   On                        Bonus ship every 30000 points
    -------------------------------------------------------------------------------------
                        On   Off             Easy game play
                        Off  Off             Normal game play   <-
                        Off  On              Hard game play
    -------------------------------------------------------------------------------------
                                    X    X   Not used
    -------------------------------------------------------------------------------------


     Dip switches at A4 on the PCB for price options: (IN3)

    LSB  D1   D2   D3   D4   D5   D6   MSB
    SW8  SW7  SW6  SW5  SW4  SW3  SW2  SW1    Option
    -------------------------------------------------------------------------------------
    Off  Off                                 Free play
    On   Off                                 1 coin for 2 credits
    Off  On                                  1 coin for 1 credit   <-
    On   On                                  2 coins for 1 credit
    -------------------------------------------------------------------------------------
              Off  Off                       Right coin mech X 1   <-
              On   Off                       Right coin mech X 4
              Off  On                        Right coin mech X 5
              On   On                        Right coin mech X 6
    -------------------------------------------------------------------------------------
                        Off                  Left coin mech X 1    <-
                        On                   Left coin mech X 2
    -------------------------------------------------------------------------------------
                             Off  Off  Off   No bonus coins        <-
                             Off  On   Off   For every 4 coins inserted, game logic
                                              adds 1 more coin

                             On   On   Off   For every 4 coins inserted, game logic
                                              adds 2 more coin
                             Off  Off  On    For every 5 coins inserted, game logic
                                              adds 1 more coin
                             On   Off  On    For every 3 coins inserted, game logic
                                              adds 1 more coin
                              X   On   On    No bonus coins
    -------------------------------------------------------------------------------------
    <-  = Manufacturer's suggested settings


    Note:
    ----

    The loop at $cf60 should count down from Y=0 instead of Y=0xff.  Because of this the first
    four leftmost pixels of each row are not cleared.  This bug is masked by the visible area
    covering up the offending pixels.

******************************************************************************************/


#include "emu.h"
#include "liberatr.h"
#include "machine/rescap.h"
#include "speaker.h"

#define MASTER_CLOCK 20000000 /* 20Mhz Main Clock Xtal */


void liberatr_state::machine_start()
{
	save_item(NAME(m_earom_data));
	save_item(NAME(m_earom_control));

	save_item(NAME(m_trackball_offset));
	save_item(NAME(m_ctrld));
	save_item(NAME(m_videoram));
}


void liberatr_state::machine_reset()
{
	// reset the control latch on the EAROM
	m_earom->set_control(0, 1, 1, 0);
	m_earom->set_clk(0);
}



/*************************************
 *
 *  Output ports
 *
 *************************************/

void liberatr_state::output_latch_w(offs_t offset, uint8_t data)
{
	m_outlatch->write_bit(offset, BIT(data, 4));
}


WRITE_LINE_MEMBER(liberatr_state::coin_counter_left_w)
{
	machine().bookkeeping().coin_counter_w(0, state);
}


WRITE_LINE_MEMBER(liberatr_state::coin_counter_right_w)
{
	machine().bookkeeping().coin_counter_w(1, state);
}



/*************************************
 *
 *  Input ports
 *
 *************************************/

WRITE_LINE_MEMBER(liberatr_state::trackball_reset_w)
{
	/* on the rising edge of /ctrld, the /ld signal on the LS191 is released and the value of the switches */
	/* input becomes the starting point for the trackball counters */
	if (!m_ctrld && state)
	{
		uint8_t trackball = ioport("FAKE")->read();
		uint8_t switches = ioport("IN0")->read();
		m_trackball_offset = ((trackball & 0xf0) - (switches & 0xf0)) | ((trackball - switches) & 0x0f);
	}
	m_ctrld = state;
}


uint8_t liberatr_state::port0_r()
{
	/* if ctrld is high, the /ld signal on the LS191 is NOT set, meaning that the trackball is counting */
	if (m_ctrld)
	{
		uint8_t trackball = ioport("FAKE")->read();
		return ((trackball & 0xf0) - (m_trackball_offset & 0xf0)) | ((trackball - m_trackball_offset) & 0x0f);
	}

	/* otherwise, the LS191 is simply passing through the raw switch inputs */
	else
		return ioport("IN0")->read();
}



/*************************************
 *
 *  Early raster EAROM interface
 *
 *************************************/

uint8_t liberatr_state::earom_r()
{
	// return data latched from previous clock
	return m_earom->data();
}


void liberatr_state::earom_w(offs_t offset, uint8_t data)
{
	// remember the value written
	m_earom_data = data;

	// output latch only enabled if control bit 2 is set
	if (m_earom_control & 4)
		m_earom->set_data(m_earom_data);

	// always latch the address
	m_earom->set_address(offset);
}


void liberatr_state::earom_control_w(uint8_t data)
{
	// remember the control state
	m_earom_control = data;

	// ensure ouput data is put on data lines prior to updating controls
	if (m_earom_control & 4)
		m_earom->set_data(m_earom_data);

	// set the control lines; /CS2 is always held low
	m_earom->set_control(data & 8, 1, ~data & 4, data & 2);
	m_earom->set_clk(data & 1);
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

void liberatr_state::liberatr_map(address_map &map)
{
	map(0x0000, 0x0000).ram().share("xcoord");
	map(0x0001, 0x0001).ram().share("ycoord");
	map(0x0002, 0x0002).rw(FUNC(liberatr_state::bitmap_xy_r), FUNC(liberatr_state::bitmap_xy_w));
	map(0x0003, 0x3fff).ram().w(FUNC(liberatr_state::bitmap_w)).share("bitmapram");
	map(0x4000, 0x403f).r(FUNC(liberatr_state::earom_r));
	map(0x5000, 0x5000).r(FUNC(liberatr_state::port0_r));
	map(0x5001, 0x5001).portr("IN1");
	map(0x6000, 0x600f).nopr().writeonly().share("base_ram");
	map(0x6200, 0x621f).nopr().writeonly().share("colorram");
	map(0x6400, 0x6400).nopw();
	map(0x6600, 0x6600).w(FUNC(liberatr_state::earom_control_w));
	map(0x6800, 0x6800).writeonly().share("planet_frame");
	map(0x6a00, 0x6a00).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x6c00, 0x6c07).w(FUNC(liberatr_state::output_latch_w));
	map(0x6e00, 0x6e3f).nopr().w(FUNC(liberatr_state::earom_w));
	map(0x7000, 0x701f).rw("pokey2", FUNC(pokey_device::read), FUNC(pokey_device::write));
	map(0x7800, 0x781f).rw("pokey1", FUNC(pokey_device::read), FUNC(pokey_device::write));
	map(0x8000, 0xefff).rom();
	map(0xfffa, 0xffff).rom();
}



/*************************************
 *
 *  Alternate main CPU memory handlers
 *
 *************************************/

void liberatr_state::liberat2_map(address_map &map)
{
	map(0x0000, 0x0000).ram().share("xcoord");
	map(0x0001, 0x0001).ram().share("ycoord");
	map(0x0002, 0x0002).rw(FUNC(liberatr_state::bitmap_xy_r), FUNC(liberatr_state::bitmap_xy_w));
	map(0x0003, 0x3fff).ram().w(FUNC(liberatr_state::bitmap_w)).share("bitmapram");
	map(0x4000, 0x4000).r(FUNC(liberatr_state::port0_r));
	map(0x4001, 0x4001).portr("IN1");
	map(0x4002, 0x400f).nopr();
	map(0x4000, 0x400f).writeonly().share("base_ram");
	map(0x4200, 0x421f).nopr().writeonly().share("colorram");
	map(0x4400, 0x4400).nopw();
	map(0x4600, 0x4600).w(FUNC(liberatr_state::earom_control_w));
	map(0x4800, 0x483f).r(FUNC(liberatr_state::earom_r));
	map(0x4800, 0x4800).writeonly().share("planet_frame");
	map(0x4a00, 0x4a00).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x4c00, 0x4c07).w(FUNC(liberatr_state::output_latch_w));
	map(0x4e00, 0x4e3f).nopr().w(FUNC(liberatr_state::earom_w));
	map(0x5000, 0x501f).rw("pokey2", FUNC(pokey_device::read), FUNC(pokey_device::write));
	map(0x5800, 0x581f).rw("pokey1", FUNC(pokey_device::read), FUNC(pokey_device::write));
	//map(0x6000, 0x601f).w(FUNC(liberatr_state::pokey1_w)); /* bug ??? */
	map(0x6000, 0xbfff).rom();
	map(0xfffa, 0xffff).rom();
}



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( liberatr )
	PORT_START("IN0")           /* IN0 - $5000 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("IN1")           /* IN1 - $5001 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH,IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("DSW1")          /* IN2  -  Game Option switches DSW @ D4 on PCB */
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x02, "6" )
	PORT_DIPSETTING(    0x03, "8" )
	PORT_DIPNAME( 0x0C, 0x04, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "15000" )
	PORT_DIPSETTING(    0x04, "20000" )
	PORT_DIPSETTING(    0x08, "25000" )
	PORT_DIPSETTING(    0x0C, "30000" )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x30, "???" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")          /* IN3  -  Pricing Option switches DSW @ A4 on PCB */
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0c, 0x00, "Right Coin" )
	PORT_DIPSETTING (   0x00, "*1" )
	PORT_DIPSETTING (   0x04, "*4" )
	PORT_DIPSETTING (   0x08, "*5" )
	PORT_DIPSETTING (   0x0c, "*6" )
	PORT_DIPNAME( 0x10, 0x00, "Left Coin" )
	PORT_DIPSETTING (   0x00, "*1" )
	PORT_DIPSETTING (   0x10, "*2" )
	/* TODO: verify the following settings */
	PORT_DIPNAME( 0xe0, 0x00, "Bonus Coins" )
	PORT_DIPSETTING (   0x00, DEF_STR( None ) )
	PORT_DIPSETTING (   0x80, "1 each 5" )
	PORT_DIPSETTING (   0x40, "1 each 4 (+Demo)" )
	PORT_DIPSETTING (   0xa0, "1 each 3" )
	PORT_DIPSETTING (   0x60, "2 each 4 (+Demo)" )
	PORT_DIPSETTING (   0x20, "1 each 2" )
	PORT_DIPSETTING (   0xc0, "Freeze Mode" )
	PORT_DIPSETTING (   0xe0, "Freeze Mode" )

	PORT_START("FAKE")          /* IN4 - FAKE - overlaps IN0 in the HW */
	PORT_BIT( 0x0f, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(30) PORT_KEYDELTA(10)
	PORT_BIT( 0xf0, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(30) PORT_KEYDELTA(10)
INPUT_PORTS_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void liberatr_state::liberatr(machine_config &config)
{
	/* basic machine hardware */
	m6502_device &maincpu(M6502(config, "maincpu", MASTER_CLOCK/16)); /* 1.25Mhz divided from 20Mhz master clock */
	maincpu.set_addrmap(AS_PROGRAM, &liberatr_state::liberatr_map);
	maincpu.set_periodic_int(FUNC(liberatr_state::irq0_line_hold), attotime::from_hz(4*60));

	ER2055(config, m_earom);

	LS259(config, m_outlatch);
	m_outlatch->q_out_cb<0>().set_output("led0").invert(); // START LED1
	m_outlatch->q_out_cb<1>().set_output("led1").invert(); // START LED2
	m_outlatch->q_out_cb<2>().set_nop(); // TBSWP
	m_outlatch->q_out_cb<3>().set_nop(); // SPARE
	m_outlatch->q_out_cb<4>().set(FUNC(liberatr_state::trackball_reset_w)); // CTRLD
	m_outlatch->q_out_cb<5>().set(FUNC(liberatr_state::coin_counter_right_w));
	m_outlatch->q_out_cb<6>().set(FUNC(liberatr_state::coin_counter_left_w));
	m_outlatch->q_out_cb<7>().set([this] (int state) { m_planet_select = state; });

	WATCHDOG_TIMER(config, "watchdog");

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	m_screen->set_screen_update(FUNC(liberatr_state::screen_update));
	m_screen->set_size(256,256);
	m_screen->set_visarea(8, 247, 13, 244);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	pokey_device &pokey1(POKEY(config, "pokey1", MASTER_CLOCK/16)); /* 1.25Mhz from Phi2 signal from 6502 */
	pokey1.allpot_r().set_ioport("DSW2");
	pokey1.set_output_opamp_low_pass(RES_K(4.7), CAP_U(0.01), 5.0);
	pokey1.add_route(ALL_OUTPUTS, "mono", 0.50);

	pokey_device &pokey2(POKEY(config, "pokey2", MASTER_CLOCK/16)); /* 1.25Mhz from Phi2 signal from 6502 */
	pokey2.set_output_opamp_low_pass(RES_K(4.7), CAP_U(0.01), 5.0);
	pokey2.allpot_r().set_ioport("DSW1");
	pokey2.add_route(ALL_OUTPUTS, "mono", 0.50);
}

void liberatr_state::liberat2(machine_config &config)
{
	liberatr(config);

	/* basic machine hardware */
	subdevice<m6502_device>("maincpu")->set_addrmap(AS_PROGRAM, &liberatr_state::liberat2_map);
}



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( liberatr )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64k for code and data  */
	ROM_LOAD( "136012.206",   0x8000, 0x1000, CRC(1a0cb4a0) SHA1(595828a07af729a84aab4e0b51e873046b56b419) )
	ROM_LOAD( "136012.205",   0x9000, 0x1000, CRC(2f071920) SHA1(8764f3e78451c4968bffb7c7f72d1ed862f4b185) )
	ROM_LOAD( "136012.204",   0xa000, 0x1000, CRC(bcc91827) SHA1(3bfbe1f1db58437ccd204a857e58695f56819649) )
	ROM_LOAD( "136012.203",   0xb000, 0x1000, CRC(b558c3d4) SHA1(0b09786d696e91b12435a9e76b127c004c32e59a) )
	ROM_LOAD( "136012.202",   0xc000, 0x1000, CRC(569ba7ea) SHA1(4812b255886204192ab999d1370550d48438ea81) )
	ROM_LOAD( "136012.201",   0xd000, 0x1000, CRC(d12cd6d0) SHA1(94474429cbcdbb406eb045152fb158e2a23cd26d) )
	ROM_LOAD( "136012.200",   0xe000, 0x1000, CRC(1e98d21a) SHA1(92c7cc033c78ae0ce8127d49debe62263404feb1) )
	ROM_RELOAD(               0xf000, 0x1000 )      /* for interrupt/reset vectors  */

	ROM_REGION( 0x4000, "gfx1", 0 ) /* planet image, used at runtime */
	ROM_LOAD( "136012.110",   0x0000, 0x1000, CRC(6eb11221) SHA1(355b71812a18cbb2ee4dc20b3622fca1c96e4570) )
	ROM_LOAD( "136012.107",   0x1000, 0x1000, CRC(8a616a63) SHA1(76794cc4e11048bb6f2628bd8b84c9a7e2e82551) )
	ROM_LOAD( "136012.108",   0x2000, 0x1000, CRC(3f8e4cf6) SHA1(a9d0feb0892f343687e00b96f05adb423ee4d659) )
	ROM_LOAD( "136012.109",   0x3000, 0x1000, CRC(dda0c0ef) SHA1(6e547c07c1abd17383a4389b0b4ced442ed65ce7) )

	ROM_REGION( 0x100, "user1", 0 ) /* latitude scaler */
	ROM_LOAD_NIB_LOW ( "136012.123",   0x0000, 0x0100, CRC(b8c806e0) SHA1(19b1b9796e1e9a42899a92ec53288d17d5d15fb3) )
	ROM_LOAD_NIB_HIGH( "136012.124",   0x0000, 0x0100, CRC(e51ec78f) SHA1(224237370c418361a00d62a77d39fa494e7d8831) )

	ROM_REGION( 0x100, "user2", 0 ) /* longitude scaler */
	ROM_LOAD_NIB_LOW ( "136012.125",   0x0000, 0x0100, CRC(52ac8dd9) SHA1(125d54b562d079b974f2562e71ab7c7a0b97e709) )
	ROM_LOAD_NIB_HIGH( "136012.126",   0x0000, 0x0100, CRC(2e670aa6) SHA1(a6bcc49d0948d2dfe497c5e3ad4a834fa78f779a) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "136012.021",   0x0000, 0x0100, CRC(ffdcd7bc) SHA1(2ce733203d628e299ec4fb93db8be1598b49142c) )    /* write protect PROM */
	ROM_LOAD( "136012.022",   0x0100, 0x0100, CRC(3353edce) SHA1(915308b11096fc1d02acf9b4af806a2a935dd748) )    /* sync PROM */
ROM_END


ROM_START( liberatr2 )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64k for code and data  */
	ROM_LOAD( "l6.bin",       0x6000, 0x1000, CRC(78093d06) SHA1(0f6ca01e27b32aae384a6ab67a6f14eedd3f1d9c) )
	ROM_LOAD( "l5.bin",       0x7000, 0x1000, CRC(988db636) SHA1(8fdd07b397d4bef108aafb10c06c2fd53fc1f99a) )
	ROM_LOAD( "l4.bin",       0x8000, 0x1000, CRC(ec114540) SHA1(eb35510b59f5e9624c3d94fb16dacb4968349030) )
	ROM_LOAD( "l3.bin",       0x9000, 0x1000, CRC(184c751f) SHA1(e020d2943be89f244c1aeeb34a28b7aa7dbc1454) )
	ROM_LOAD( "l2.bin",       0xa000, 0x1000, CRC(c3f61f88) SHA1(a56ce094fe7374d3ac341d5eb9e06df083e16b1f) )
	ROM_LOAD( "l1.bin",       0xb000, 0x1000, CRC(ef6e9f9e) SHA1(b1f7cc9e0a2ea08ec89428ad31161ac81e7faaaf) )
	ROM_RELOAD(               0xf000, 0x1000 )      /* for interrupt/reset vectors  */

	ROM_REGION( 0x4000, "gfx1", 0 ) /* planet image, used at runtime */
	ROM_LOAD( "136012.110",   0x0000, 0x1000, CRC(6eb11221) SHA1(355b71812a18cbb2ee4dc20b3622fca1c96e4570) )
	ROM_LOAD( "136012.107",   0x1000, 0x1000, CRC(8a616a63) SHA1(76794cc4e11048bb6f2628bd8b84c9a7e2e82551) )
	ROM_LOAD( "136012.108",   0x2000, 0x1000, CRC(3f8e4cf6) SHA1(a9d0feb0892f343687e00b96f05adb423ee4d659) )
	ROM_LOAD( "136012.109",   0x3000, 0x1000, CRC(dda0c0ef) SHA1(6e547c07c1abd17383a4389b0b4ced442ed65ce7) )

	ROM_REGION( 0x100, "user1", 0 ) /* latitude scaler */
	ROM_LOAD_NIB_LOW ( "136012.123",   0x0000, 0x0100, CRC(b8c806e0) SHA1(19b1b9796e1e9a42899a92ec53288d17d5d15fb3) )
	ROM_LOAD_NIB_HIGH( "136012.124",   0x0000, 0x0100, CRC(e51ec78f) SHA1(224237370c418361a00d62a77d39fa494e7d8831) )

	ROM_REGION( 0x100, "user2", 0 ) /* longitude scaler */
	ROM_LOAD_NIB_LOW ( "136012.125",   0x0000, 0x0100, CRC(52ac8dd9) SHA1(125d54b562d079b974f2562e71ab7c7a0b97e709) )
	ROM_LOAD_NIB_HIGH( "136012.126",   0x0000, 0x0100, CRC(2e670aa6) SHA1(a6bcc49d0948d2dfe497c5e3ad4a834fa78f779a) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "136012.021",   0x0000, 0x0100, CRC(ffdcd7bc) SHA1(2ce733203d628e299ec4fb93db8be1598b49142c) )    /* write protect PROM */
	ROM_LOAD( "136012.022",   0x0100, 0x0100, CRC(3353edce) SHA1(915308b11096fc1d02acf9b4af806a2a935dd748) )    /* sync PROM */
ROM_END



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1982, liberatr, 0,        liberatr, liberatr, liberatr_state, empty_init, ROT0, "Atari", "Liberator (set 1)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1982, liberatr2,liberatr, liberat2, liberatr, liberatr_state, empty_init, ROT0, "Atari", "Liberator (set 2)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
