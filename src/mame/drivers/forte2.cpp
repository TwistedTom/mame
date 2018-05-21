// license:BSD-3-Clause
// copyright-holders:hap, Mariusz Wojcieszek
/* Brazilian bootleg board from 1989. Forte II Games, Industria Brasileira.
MAME driver by Mariusz Wojcieszek & hap, based on
information from Alexandre Souza (a.k.a. "Tabajara").

Hardware is based on MSX1, excluding i8255 PPI:
 64KB RAM, largely unused
 64KB EPROM (2764-15, contains hacked BIOS and game ROM)
 Z80 @ 3.58MHz
 GI AY-3-8910
 TI TMS9928A
 (no dipswitches)

Games:
Pesadelo (means 'nightmare' in Portuguese), 1989 bootleg of Knightmare (Majou
Densetsu in Japan) (C) 1986 Konami, originally released exclusively on MSX.
This arcade conversion has been made a bit harder, eg. bonus power-ups deplete
three times quicker, and the game starts at a later, more difficult level.
A precise translation of the Brazilian Portuguese text displayed
upon inserting a coin is:

  NIGHTMARE DIFFICULTY-LEVEL 2 DOES NOT ACCUMULATE
  CREDITS , ONLY INSERT A NEW
  COIN AFTER THE END OF THE GAME
  IN ORDER TO START THE GAME PRESS
  THE FIRE BUTTON.

               GOOD LUCK!

If the coin detector is activated for a few seconds, an error message
meaning STUCK COIN shows up blinking and beeping:

               FICHA PRESA

According to Alexandre, there are more games for this board, but not
found/dumped yet. */

#include "emu.h"
#include "cpu/z80/z80.h"
#include "video/tms9928a.h"
#include "sound/ay8910.h"
#include "screen.h"
#include "speaker.h"


class forte2_state : public driver_device
{
public:
	forte2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void init_pesadelo();
	void pesadelo(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	void io_mem(address_map &map);
	void program_mem(address_map &map);

	DECLARE_READ8_MEMBER(forte2_ay8910_read_input);
	DECLARE_WRITE8_MEMBER(forte2_ay8910_set_input_mask);

private:
	required_device<cpu_device> m_maincpu;

	uint8_t m_input_mask;
};



void forte2_state::program_mem(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xffff).ram();
}

void forte2_state::io_mem(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x98, 0x98).rw("tms9928a", FUNC(tms9928a_device::vram_read), FUNC(tms9928a_device::vram_write));
	map(0x99, 0x99).rw("tms9928a", FUNC(tms9928a_device::register_read), FUNC(tms9928a_device::register_write));
	map(0xa0, 0xa1).w("aysnd", FUNC(ay8910_device::address_data_w));
	map(0xa2, 0xa2).r("aysnd", FUNC(ay8910_device::data_r));
//  AM_RANGE(0xa8, 0xa8) AM_RAM // Ports a8-ab are originally for communicating with the i8255 PPI on MSX.
//  AM_RANGE(0xa9, 0xab) AM_NOP // Since this arcade board doesn't have one, those ports should be unmapped.
}

static INPUT_PORTS_START( pesadelo )
	PORT_START("IN0")
	PORT_BIT (0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)
	PORT_BIT (0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)
	PORT_BIT (0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)
	PORT_BIT (0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT)
	PORT_BIT (0x10, IP_ACTIVE_LOW, IPT_BUTTON1)
	PORT_BIT (0x20, IP_ACTIVE_LOW, IPT_START1)
	PORT_BIT (0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT (0x80, IP_ACTIVE_LOW, IPT_COIN1)
INPUT_PORTS_END


READ8_MEMBER(forte2_state::forte2_ay8910_read_input)
{
	return ioport("IN0")->read() | (m_input_mask & 0x3f);
}

WRITE8_MEMBER(forte2_state::forte2_ay8910_set_input_mask)
{
	/* PSG reg 15, writes 0 at coin insert, 0xff at boot and game over */
	m_input_mask = data;
}

void forte2_state::machine_reset()
{
	m_input_mask = 0xff;
}

void forte2_state::machine_start()
{
	/* register for save states */
	save_item(NAME(m_input_mask));
}


MACHINE_CONFIG_START(forte2_state::pesadelo)

	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", Z80, XTAL(3'579'545))
	MCFG_DEVICE_PROGRAM_MAP(program_mem)
	MCFG_DEVICE_IO_MAP(io_mem)

	/* video hardware */
	MCFG_DEVICE_ADD("tms9928a", TMS9928A, XTAL(10'738'635)/2)
	MCFG_TMS9928A_VRAM_SIZE(0x4000)
	MCFG_TMS9928A_OUT_INT_LINE_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	MCFG_TMS9928A_SCREEN_ADD_NTSC("screen")
	MCFG_SCREEN_UPDATE_DEVICE("tms9928a", tms9928a_device, screen_update)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	MCFG_DEVICE_ADD("aysnd", AY8910, XTAL(3'579'545)/2)
	MCFG_AY8910_PORT_A_READ_CB(READ8(*this, forte2_state, forte2_ay8910_read_input))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(*this, forte2_state, forte2_ay8910_set_input_mask))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END

void forte2_state::init_pesadelo()
{
	uint8_t *mem = memregion("maincpu")->base();
	int memsize = memregion("maincpu")->bytes();

	// data swap
	for (int i = 0; i < memsize; i++)
	{
		mem[i] = bitswap<8>(mem[i],3,5,6,7,0,4,2,1);
	}

	// address line swap
	std::vector<uint8_t> buf(&mem[0], &mem[memsize]);

	for (int i = 0; i < memsize; i++)
	{
		mem[bitswap<16>(i,11,9,8,13,14,15,12,7,6,5,4,3,2,1,0,10)] = buf[i];
	}
}

ROM_START( pesadelo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr2764.15", 0x00000, 0x10000, CRC(1ae2f724) SHA1(12880dd7ad82acf04861843fb9d4f0f926d18f6b) )
ROM_END

GAME( 1989, pesadelo, 0, pesadelo, pesadelo, forte2_state, init_pesadelo, ROT0, "bootleg (Forte II Games) / Konami", "Pesadelo (bootleg of Knightmare on MSX)", MACHINE_SUPPORTS_SAVE )
