// license:BSD-3-Clause
// copyright-holders:David Graves, R. Belmont
/***************************************************************************

Excellent System's ES-9209B Hardware

Games supported:

   Grand Cross Pinball
   Power Flipper Pinball Shooting


Made from Raine source


Code
----

Inputs get tested at $4aca2 on


TODO
----

 - Screen flipping support
 - Modernize ES-8712 and hook up to MSM6585 and HCT157
 - Figure out which customs use D80010-D80077 and merge implementation with Aquarium
 - Is SW3 actually used?
 - Missing row scroll (column scroll?)
   Reference video showing effect: https://www.youtube.com/watch?v=zBGjncVsSf4

BGMs (controlled by OKI MSM6585 sound chip)
  MSM6585: is an upgraded MSM5205 voice synth IC.
   Improvements:
    More precise internal DA converter
    Built in low-pass filter
    Expanded sampling frequency

Stephh's notes (based on the game M68000 code and some tests) :

  - Reset the game while pressing START1 to enter the "test mode"


ES-9209B
+-----------------------------------------------+
|      M6585  U56  ES-8712                      |
| VR1 640kHz  U55           +-------+           |
|1056khz M6295              |ES 9207|           |
|                 6116      |       |  AS7C256  |
|                 6116      +-------+  AS7C256  |
|J                                     AS7C256  |
|A  MB3773                    AS7C256  AS7C256  |
|M  TSW1*               +-------+          U13* |
|M   PAL          32MHz |ES-9303|          U11  |
|A   PAL     68000P-16  +-------+               |
|       62256  4.U46      +-------+      1.U10  |
|       62256  3.U45      |ES-9208|             |
|  93C46         U44*     +-------+        U6   |
|              2.U43                            |
|SW4* SW3 SW2 SW1  14.31818MHz   5864 5864 U1   |
+-----------------------------------------------+

   CPU: TMP68HC000P-16
 Sound: OKI M6295
        OKI M6585
        Excellent ES-8712
   OSC: 32MHz, 14.31818MHz & 1056kHz, 640kHz resonators
   RAM: Sony CXK5864BSP-10L 8K x 8bit high speed CMOS SRAM
        Alliance AS7C256-20PC 32K x 8bit CMOS SRAM
        Hitachi HM6116LK-70 2K x  8bit SRAM
        Hitachi HM62256ALP-8 32K x 8bit CMOS SRAM
EEPROM: 93C46 1K Serial EEPROM
Custom: EXCELLENT SYSTEM ES-9208 347102 (QFP160)
        EXCELLENT SYSTEM LTD. ES 9207 9343 T (QFP208)
        ES-9303 EXCELLENT 9338 C001 (QFP120)

* Denotes unpopulated components

NOTE: Mask roms from Power Flipper Pinball Shooting have not been dumped, but assumed to
      be the same data.

***************************************************************************/

#include "emu.h"
#include "includes/gcpinbal.h"

#include "cpu/m68000/m68000.h"
#include "screen.h"
#include "speaker.h"


/***********************************************************
                      INTERRUPTS
***********************************************************/

void gcpinbal_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_GCPINBAL_INTERRUPT1:
		m_maincpu->set_input_line(1, HOLD_LINE);
		break;
	default:
		assert_always(false, "Unknown id in gcpinbal_state::device_timer");
	}
}

INTERRUPT_GEN_MEMBER(gcpinbal_state::gcpinbal_interrupt)
{
	/* Unsure of actual sequence */

	m_int1_timer->adjust(m_maincpu->cycles_to_attotime(500));
	device.execute().set_input_line(4, HOLD_LINE);
}


/***********************************************************
                          IOC
***********************************************************/

WRITE16_MEMBER(gcpinbal_state::d80010_w)
{
	//logerror("CPU #0 PC %06x: warning - write ioc offset %06x with %04x\n", m_maincpu->pc(), offset, data);
	COMBINE_DATA(&m_d80010_ram[offset]);
}

WRITE8_MEMBER(gcpinbal_state::d80040_w)
{
	logerror("Writing byte value %02X to offset %X\n", data, offset);
}

WRITE16_MEMBER(gcpinbal_state::d80060_w)
{
	//logerror("CPU #0 PC %06x: warning - write ioc offset %06x with %04x\n", m_maincpu->pc(), offset, data);
	COMBINE_DATA(&m_d80060_ram[offset]);
}

WRITE8_MEMBER(gcpinbal_state::bank_w)
{
	// MSM6585 bank, coin LEDs, maybe others?
	if (m_msm_bank != ((data & 0x10) >> 4))
	{
		m_msm_bank = ((data & 0x10) >> 4);
		m_essnd->set_rom_bank(m_msm_bank);
		logerror("Bankswitching ES8712 ROM %02x\n", m_msm_bank);
	}
	m_oki->set_rom_bank((data & 0x20) >> 5);

	m_bg0_gfxset = (data & 0x04) ? 0x1000 : 0;
	m_bg1_gfxset = (data & 0x08) ? 0x1000 : 0;

	m_watchdog->write_line_ck(BIT(data, 7));

//          machine().bookkeeping().coin_lockout_w(0, ~data & 0x01);
//          machine().bookkeeping().coin_lockout_w(1, ~data & 0x02);
}

WRITE8_MEMBER(gcpinbal_state::eeprom_w)
{
	// 93C46 serial EEPROM (status read at D80087)
	m_eeprom->di_write(BIT(data, 2));
	m_eeprom->clk_write(BIT(data, 1));
	m_eeprom->cs_write(BIT(data, 0));
}

WRITE8_MEMBER(gcpinbal_state::es8712_reset_w)
{
	// This probably works by resetting the ES-8712
	m_essnd->reset();
}


/***********************************************************
                     MEMORY STRUCTURES
***********************************************************/

void gcpinbal_state::gcpinbal_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom();
	map(0xc00000, 0xc03fff).rw(this, FUNC(gcpinbal_state::gcpinbal_tilemaps_word_r), FUNC(gcpinbal_state::gcpinbal_tilemaps_word_w)).share("tilemapram");
	map(0xc80000, 0xc81fff).rw(m_sprgen, FUNC(excellent_spr_device::read), FUNC(excellent_spr_device::write)).umask16(0x00ff);
	map(0xd00000, 0xd00fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0xd80010, 0xd8002f).ram().w(this, FUNC(gcpinbal_state::d80010_w)).share("d80010");
	map(0xd80040, 0xd8005b).w(this, FUNC(gcpinbal_state::d80040_w)).umask16(0x00ff);
	map(0xd80060, 0xd80077).ram().w(this, FUNC(gcpinbal_state::d80060_w)).share("d80060");
	map(0xd80080, 0xd80081).portr("DSW");
	map(0xd80084, 0xd80085).portr("IN0");
	map(0xd80086, 0xd80087).portr("IN1");
	map(0xd80088, 0xd80088).w(this, FUNC(gcpinbal_state::bank_w));
	map(0xd8008a, 0xd8008a).w(this, FUNC(gcpinbal_state::eeprom_w));
	map(0xd8008e, 0xd8008e).w(this, FUNC(gcpinbal_state::es8712_reset_w));
	map(0xd800a0, 0xd800a0).mirror(0x2).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xd800c0, 0xd800cd).w(m_essnd, FUNC(es8712_device::write)).umask16(0xff00);
	map(0xff0000, 0xffffff).ram(); /* RAM */
}



/***********************************************************
                   INPUT PORTS, DIPs
***********************************************************/

static INPUT_PORTS_START( gcpinbal )
	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0004, "300k" )
	PORT_DIPSETTING(      0x0008, "500k" )
	PORT_DIPSETTING(      0x000c, "1000k" )
	PORT_DIPSETTING(      0x0000, DEF_STR( None ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW2:5")   // Confirmed via manual - code at 0x000508
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unused ) )   PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:7")   // Confirmed via manual - code at 0x00b6d0, 0x00b7e4, 0x00bae4
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unused ) )   PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0700, 0x0700, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(      0x0200, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0100, "2 Coins/1 Credit 3/2 4/3 6/5" )
	PORT_DIPSETTING(      0x0700, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0500, "1 Coin/1 Credit 2/3" )
	PORT_DIPSETTING(      0x0000, "1 Coin/1 Credit 5/6" )
	PORT_DIPSETTING(      0x0600, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x3800, 0x3800, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(      0x1000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0800, "2 Coins/1 Credit 3/2 4/3 6/5" )
	PORT_DIPSETTING(      0x3800, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x2800, "1 Coin/1 Credit 2/3" )
	PORT_DIPSETTING(      0x0000, "1 Coin/1 Credit 5/6" )
	PORT_DIPSETTING(      0x3000, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0xc000, 0xc000, DEF_STR( Lives ) )    PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0xc000, "3" )
	PORT_DIPSETTING(      0x8000, "4" )
	PORT_DIPSETTING(      0x4000, "5" )

	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Item Right") PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Flipper 1 Right") PORT_PLAYER(1)   // Inner flipper right
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Flipper 2 Right") PORT_PLAYER(1)   // Outer flipper right
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("Tilt Right") PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Item Left") PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Flipper 1 Left") PORT_PLAYER(1)   // Inner flipper left
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Flipper 2 Left") PORT_PLAYER(1)   // Outer flipper left
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("Tilt Left") PORT_PLAYER(1)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END



/**************************************************************
                       GFX DECODING
**************************************************************/

static const gfx_layout charlayout =
{
	16,16,  /* 16*16 characters */
	RGN_FRAC(1,1),
	4,  /* 4 bits per pixel */
	{ STEP4(0,1) },
	{ STEP16(0,4) },
	{ STEP16(0,4*16) },
	16*16*4   /* every sprite takes 128 consecutive bytes */
};

static const gfx_layout char_8x8_layout =
{
	8,8,    /* 8*8 characters */
	RGN_FRAC(1,1),
	4,  /* 4 bits per pixel */
	{ STEP4(0,1) },
	{ STEP8(0,4) },
	{ STEP8(0,4*8) },
	8*8*4    /* every sprite takes 32 consecutive bytes */
};

static const gfx_layout tilelayout =
{
	16,16,  /* 16*16 sprites */
	RGN_FRAC(1,1),
	4,  /* 4 bits per pixel */
//  { 16, 48, 0, 32 },
	{ 48, 16, 32, 0 },
	{ STEP16(0,1) },
	{ STEP16(0,16*4) },
	16*16*4   /* every sprite takes 128 consecutive bytes */
};

static GFXDECODE_START( gfx_gcpinbal )
	GFXDECODE_ENTRY( "sprite", 0, tilelayout,       0, 256 )  // sprites & playfield
	GFXDECODE_ENTRY( "bg0",    0, charlayout,       0, 256 )  // sprites & playfield
	GFXDECODE_ENTRY( "fg0",    0, char_8x8_layout,  0, 256 )  // sprites & playfield
GFXDECODE_END


/***********************************************************
                        MACHINE DRIVERS
***********************************************************/

void gcpinbal_state::machine_start()
{
	m_int1_timer = timer_alloc(TIMER_GCPINBAL_INTERRUPT1);

	save_item(NAME(m_scrollx));
	save_item(NAME(m_scrolly));
	save_item(NAME(m_bg0_gfxset));
	save_item(NAME(m_bg1_gfxset));
	save_item(NAME(m_msm_bank));
}

void gcpinbal_state::machine_reset()
{
	int i;

	for (i = 0; i < 3; i++)
	{
		m_scrollx[i] = 0;
		m_scrolly[i] = 0;
	}

	m_bg0_gfxset = 0;
	m_bg1_gfxset = 0;
	m_msm_bank = 0;
}

MACHINE_CONFIG_START(gcpinbal_state::gcpinbal)

	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", M68000, 32_MHz_XTAL/2) /* 16 MHz */
	MCFG_DEVICE_PROGRAM_MAP(gcpinbal_map)
	MCFG_DEVICE_VBLANK_INT_DRIVER("screen", gcpinbal_state,  gcpinbal_interrupt)

	MCFG_EEPROM_SERIAL_93C46_ADD("eeprom")

	MCFG_DEVICE_ADD("watchdog", MB3773, 0)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0)  /* frames per second, vblank duration */)
	MCFG_SCREEN_SIZE(40*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(gcpinbal_state, screen_update_gcpinbal)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_DEVICE_ADD("gfxdecode", GFXDECODE, "palette", gfx_gcpinbal)
	MCFG_PALETTE_ADD("palette", 4096)
	MCFG_PALETTE_FORMAT(RRRRGGGGBBBBRGBx)

	MCFG_DEVICE_ADD("spritegen", EXCELLENT_SPRITE, 0)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	MCFG_DEVICE_ADD("oki", OKIM6295, 1.056_MHz_XTAL, okim6295_device::PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)

	MCFG_ES8712_ADD("essnd", 0)
	MCFG_ES8712_RESET_HANDLER(INPUTLINE("maincpu", 3))
	MCFG_ES8712_MSM_WRITE_CALLBACK(WRITE8("msm", msm6585_device, data_w))
	MCFG_ES8712_MSM_TAG("msm")

	MCFG_DEVICE_ADD("msm", MSM6585, 640_kHz_XTAL)
	MCFG_MSM6585_VCK_CALLBACK(WRITELINE("essnd", es8712_device, msm_int))
	MCFG_MSM6585_PRESCALER_SELECTOR(S40)         /* 16 kHz */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END



/***************************************************************************
                                  DRIVERS
***************************************************************************/

ROM_START( pwrflip ) /* Updated version of Grand Cross Pinball or semi-sequel? */
	ROM_REGION( 0x200000, "maincpu", 0 )  /* 512k for 68000 program */
	ROM_LOAD16_WORD_SWAP( "p.f_1.33.u43",  0x000000, 0x80000, CRC(d760c987) SHA1(9200604377542193afc866c84733f2d3b5aa1c80) ) /* hand written labels on genuine EXCELLENT labels */
	ROM_FILL            ( 0x80000,  0x080000, 0x00 ) /* unpopulated 27C4096 socket at U44 */
	ROM_LOAD16_WORD_SWAP( "p.f.u45",       0x100000, 0x80000, CRC(6ad1a457) SHA1(8746c38efa05e3318e9b1a371470d149803fb6bb) )
	ROM_LOAD16_WORD_SWAP( "p.f.u46",       0x180000, 0x80000, CRC(e0f3a1b4) SHA1(761dddf374a92c1a1e4a211ead215d5be461a082) )

	ROM_REGION( 0x200000, "bg0", 0 )  /* BG0 (16 x 16) */
	ROM_LOAD16_WORD_SWAP( "u1",      0x000000, 0x100000, CRC(afa459bb) SHA1(7a7c64bcb80d71b8cf3fdd3209ef109997b6417c) ) /* 23C8000 MASK ROMs */
	ROM_LOAD16_WORD_SWAP( "u6",      0x100000, 0x100000, CRC(c3f024e5) SHA1(d197e2b715b154fc64ff9a61f8c6df111d6fd446) )

	ROM_REGION( 0x020000, "fg0", 0 )  /* FG0 (8 x 8) */
	ROM_LOAD16_WORD_SWAP( "p.f.u10",   0x000000, 0x020000, CRC(50e34549) SHA1(ca1808513ff3feb8bcd34d9aafd7b374e4244732) )

	ROM_REGION( 0x200000, "sprite", 0 )  /* Sprites (16 x 16) */
	ROM_LOAD16_WORD_SWAP( "u13",     0x000000, 0x200000, CRC(62f3952f) SHA1(7dc9ccb753d46b6aaa791bcbf6e18e6d872f6b79) ) /* 23C16000 MASK ROM */

	ROM_REGION( 0x080000, "oki", 0 )   /* M6295 acc to Raine */
	ROM_LOAD( "u55",   0x000000, 0x080000, CRC(b3063351) SHA1(825e63e8a824d67d235178897528e5b0b41e4485) ) /* OKI M534001B MASK ROM */

	ROM_REGION( 0x200000, "essnd", 0 )   /* M6585 acc to Raine but should be for ES-8712??? */
	ROM_LOAD( "u56",   0x000000, 0x200000, CRC(092b2c0f) SHA1(2ec1904e473ddddb50dbeaa0b561642064d45336) ) /* 23C16000 MASK ROM */

	ROM_REGION( 0x000400, "plds", 0 ) /* 2x TIBPAL16L8-15CN */
	ROM_LOAD( "a.u72", 0x000, 0x104, NO_DUMP )
	ROM_LOAD( "b.u73", 0x200, 0x104, NO_DUMP )
ROM_END

ROM_START( gcpinbal )
	ROM_REGION( 0x200000, "maincpu", 0 )  /* 512k for 68000 program */
	ROM_LOAD16_WORD_SWAP( "2_excellent.u43",  0x000000, 0x80000, CRC(d174bd7f) SHA1(0e6c17265e1400de941e3e2ca3be835aaaff6695) ) /* Red line across label */
	ROM_FILL            ( 0x80000,  0x080000, 0x00 ) /* unpopulated 27C4096 socket at U44 */
	ROM_LOAD16_WORD_SWAP( "3_excellent.u45",  0x100000, 0x80000, CRC(0511ad56) SHA1(e0602ece514126ce719ebc9de6649ebe907be904) )
	ROM_LOAD16_WORD_SWAP( "4_excellent.u46",  0x180000, 0x80000, CRC(e0f3a1b4) SHA1(761dddf374a92c1a1e4a211ead215d5be461a082) )

	ROM_REGION( 0x200000, "bg0", 0 )  /* BG0 (16 x 16) */
	ROM_LOAD16_WORD_SWAP( "u1",      0x000000, 0x100000, CRC(afa459bb) SHA1(7a7c64bcb80d71b8cf3fdd3209ef109997b6417c) ) /* 23C8000 MASK ROMs */
	ROM_LOAD16_WORD_SWAP( "u6",      0x100000, 0x100000, CRC(c3f024e5) SHA1(d197e2b715b154fc64ff9a61f8c6df111d6fd446) )

	ROM_REGION( 0x020000, "fg0", 0 )  /* FG0 (8 x 8) */
	ROM_LOAD16_WORD_SWAP( "1_excellent.u10",   0x000000, 0x020000, CRC(79321550) SHA1(61f1b772ed8cf95bfee9df8394b0c3ff727e8702) )

	ROM_REGION( 0x200000, "sprite", 0 )  /* Sprites (16 x 16) */
	ROM_LOAD16_WORD_SWAP( "u13",     0x000000, 0x200000, CRC(62f3952f) SHA1(7dc9ccb753d46b6aaa791bcbf6e18e6d872f6b79) ) /* 23C16000 MASK ROM */

	ROM_REGION( 0x080000, "oki", 0 )   /* M6295 acc to Raine */
	ROM_LOAD( "u55",   0x000000, 0x080000, CRC(b3063351) SHA1(825e63e8a824d67d235178897528e5b0b41e4485) ) /* OKI M534001B MASK ROM */

	ROM_REGION( 0x200000, "essnd", 0 )   /* M6585 acc to Raine but should be for ES-8712??? */
	ROM_LOAD( "u56",   0x000000, 0x200000, CRC(092b2c0f) SHA1(2ec1904e473ddddb50dbeaa0b561642064d45336) ) /* 23C16000 MASK ROM */

	ROM_REGION( 0x000400, "plds", 0 ) /* 2x TIBPAL16L8-15CN */
	ROM_LOAD( "a.u72", 0x000, 0x104, NO_DUMP )
	ROM_LOAD( "b.u73", 0x200, 0x104, NO_DUMP )
ROM_END


GAME( 1994, pwrflip,  0, gcpinbal, gcpinbal, gcpinbal_state, empty_init, ROT270, "Excellent System", "Power Flipper Pinball Shooting v1.33", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1994, gcpinbal, 0, gcpinbal, gcpinbal, gcpinbal_state, empty_init, ROT270, "Excellent System", "Grand Cross v1.02F",                   MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
