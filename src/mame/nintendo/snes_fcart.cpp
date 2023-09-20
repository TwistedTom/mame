//
//

#include "emu.h"
#include "snes.h"
#include "bus/snes_ctrl/ctrl.h"
#include "speaker.h"
#include "machine/nvram.h"

class snes_fcart_state : public snes_state
{
public:
	snes_fcart_state(const machine_config &mconfig, device_type type, const char *tag)
		: snes_state(mconfig, type, tag)
		, m_ctrl1(*this, "ctrl1")
		, m_ctrl2(*this, "ctrl2")
		, m_nvram(*this, "nvram")
		, m_ram(0x20000)
	{ }

	void snes_fcart(machine_config &config);
	void snes_fcart_pal(machine_config &config);
	void init_snes_fcart();
	void init_snes_fcart2();

private:
	virtual void io_read() override;
	virtual uint8_t oldjoy1_read(int latched) override;
	virtual uint8_t oldjoy2_read(int latched) override;
	virtual void write_joy_latch(uint8_t data) override;
	virtual void wrio_write(uint8_t data) override;

	virtual void machine_start() override;
	virtual void machine_reset() override;

	required_device<snes_control_port_device> m_ctrl1;
	required_device<snes_control_port_device> m_ctrl2;
	required_device<nvram_device> m_nvram;

	void snes_fcart_map(address_map &map);
	void spc_map(address_map &map);

	uint8_t snes_fcart_rom_access(uint32_t offset);
	inline uint8_t snes_fcart_rom_access_lo(uint32_t offset);
	inline uint8_t snes_fcart_rom_access_hi(uint32_t offset);
	uint8_t snes_fcart_ram_r(uint32_t offset);
	void snes_fcart_ram_w(uint32_t offset, uint8_t data);

	uint8_t snes_fcart_r_bank1(offs_t offset);
	uint8_t snes_fcart_r_bank2(offs_t offset);
	void snes_fcart_w_bank1(address_space &space, offs_t offset, uint8_t data);
	void snes_fcart_w_bank2(address_space &space, offs_t offset, uint8_t data);

	void snes_fcart_menu_wr(uint16_t adr, uint8_t data);
	inline void snes_fcart_setup_rom_map();
	inline void snes_fcart_setup_ram_map();

	uint8_t m_rom_reg = 0;
	uint8_t m_ram_reg = 0;
	uint8_t m_size_reg = 0;
	uint8_t m_mode_reg = 0;

	uint32_t m_rom_size[2] {};
	uint32_t m_ram_size = 0;

	uint8_t *m_rom;
	std::vector<uint8_t> m_ram;
	
	uint8_t m_rom_shift = 17;
};


void snes_fcart_state::snes_fcart(machine_config &config)
{
	/* basic machine hardware */
	_5A22(config, m_maincpu, MCLK_NTSC);   // Nintendo S-CPU 5A22-0x, 21.477272MHz / (6, 8, 12) = 1.79 MHz, 2.68 MHz, also 3.58 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &snes_fcart_state::snes_fcart_map);

	// runs at 24.576 MHz / 12 = 2.048 MHz
	S_SMP(config, m_soundcpu, XTAL(24'576'000) / 12);
	m_soundcpu->set_addrmap(AS_DATA, &snes_fcart_state::spc_map);
	m_soundcpu->dsp_io_read_callback().set(m_s_dsp, FUNC(s_dsp_device::dsp_io_r));
	m_soundcpu->dsp_io_write_callback().set(m_s_dsp, FUNC(s_dsp_device::dsp_io_w));

	//config.set_maximum_quantum(attotime::from_hz(48000));
	config.set_perfect_quantum(m_maincpu);

	INPUT_MERGER_ANY_HIGH(config, m_scpu_irq);
	m_scpu_irq->output_handler().set_inputline(m_maincpu, G65816_LINE_IRQ);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(DOTCLK_NTSC * 2, SNES_HTOTAL * 2, 0, SNES_SCR_WIDTH * 2, SNES_VTOTAL_NTSC, 0, SNES_SCR_HEIGHT_NTSC);
	m_screen->set_video_attributes(VIDEO_VARIABLE_WIDTH);
	m_screen->set_screen_update(FUNC(snes_state::screen_update));

	SNES_PPU(config, m_ppu, MCLK_NTSC);
	m_ppu->open_bus_callback().set(FUNC(snes_fcart_state::snes_open_bus_r));
	m_ppu->set_screen("screen");

	SNES_CONTROL_PORT(config, m_ctrl1, snes_control_port_devices, "joypad");
	//m_ctrl1->set_onscreen_callback(FUNC(snes_fcart_state::onscreen_cb));
	SNES_CONTROL_PORT(config, m_ctrl2, snes_control_port_devices, "joypad");
	//m_ctrl2->set_onscreen_callback(FUNC(snes_fcart_state::onscreen_cb));
	//m_ctrl2->set_gunlatch_callback(FUNC(snes_fcart_state::gun_latch_cb));

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	S_DSP(config, m_s_dsp, XTAL(24'576'000) / 12);
	m_s_dsp->set_addrmap(0, &snes_fcart_state::spc_map);
	m_s_dsp->add_route(0, "lspeaker", 1.00);
	m_s_dsp->add_route(1, "rspeaker", 1.00);

	NVRAM(config, m_nvram, nvram_device::DEFAULT_ALL_1);
}

void snes_fcart_state::snes_fcart_pal(machine_config &config)
{
	snes_fcart(config);
	m_maincpu->set_clock(MCLK_PAL);

	m_screen->set_raw(DOTCLK_PAL * 2, SNES_HTOTAL * 2, 0, SNES_SCR_WIDTH * 2, SNES_VTOTAL_PAL, 0, SNES_SCR_HEIGHT_PAL);

	m_ppu->set_clock(MCLK_PAL);
}

void snes_fcart_state::init_snes_fcart()
{
	m_rom = memregion("rom")->base();

	// menu -> reset -> resets in emu mode, nmi is triggered every other time !?
	// emu mode nmi vector is fffa-b, this is ffff in rom
	m_rom[0x7fff] = 0x40;  // rti
}

void snes_fcart_state::init_snes_fcart2()
{
	init_snes_fcart();
	m_rom_shift = 18;
}

static INPUT_PORTS_START( snes_fcart )
INPUT_PORTS_END

void snes_fcart_state::io_read()
{
	// is automatic reading on? if so, read 16bits from oldjoy1/2
	if (SNES_CPU_REG(NMITIMEN) & 1)
	{
		uint16_t joy1 = 0, joy2 = 0, joy3 = 0, joy4 = 0;
		m_ctrl1->port_poll();
		m_ctrl2->port_poll();

		for (int i = 0; i < 16; i++)
		{
			joy1 |= ((m_ctrl1->read_pin4() & 1) << (15 - i));
			joy2 |= ((m_ctrl2->read_pin4() & 1) << (15 - i));
			joy3 |= ((m_ctrl1->read_pin5() & 1) << (15 - i));
			joy4 |= ((m_ctrl2->read_pin5() & 1) << (15 - i));
		}

		SNES_CPU_REG(JOY1L) = (joy1 & 0x00ff) >> 0;
		SNES_CPU_REG(JOY1H) = (joy1 & 0xff00) >> 8;
		SNES_CPU_REG(JOY2L) = (joy2 & 0x00ff) >> 0;
		SNES_CPU_REG(JOY2H) = (joy2 & 0xff00) >> 8;
		SNES_CPU_REG(JOY3L) = (joy3 & 0x00ff) >> 0;
		SNES_CPU_REG(JOY3H) = (joy3 & 0xff00) >> 8;
		SNES_CPU_REG(JOY4L) = (joy4 & 0x00ff) >> 0;
		SNES_CPU_REG(JOY4H) = (joy4 & 0xff00) >> 8;
	}
}

uint8_t snes_fcart_state::oldjoy1_read(int latched)
{
	uint8_t ret = 0;
	ret |= m_ctrl1->read_pin4();
	ret |= (m_ctrl1->read_pin5() << 1);
	return ret;
}

uint8_t snes_fcart_state::oldjoy2_read(int latched)
{
	uint8_t ret = 0;
	ret |= m_ctrl2->read_pin4();
	ret |= (m_ctrl2->read_pin5() << 1);
	return ret;
}

void snes_fcart_state::write_joy_latch(uint8_t data)
{
	m_ctrl1->write_strobe(data);
	m_ctrl2->write_strobe(data);
}

void snes_fcart_state::wrio_write(uint8_t data)
{
	if (!(SNES_CPU_REG(WRIO) & 0x80) && (data & 0x80))
	{
		// external latch
		m_ppu->set_latch_hv(m_ppu->current_x(), m_ppu->current_y());
	}

	m_ctrl1->write_pin6(BIT(data, 6));
	m_ctrl2->write_pin6(BIT(data, 7));
}

void snes_fcart_state::machine_start()
{
	snes_state::machine_start();

	m_nvram->set_base(m_ram.data(), m_ram.size());

	save_item(NAME(m_rom_reg));
	save_item(NAME(m_ram_reg));
	save_item(NAME(m_size_reg));
	save_item(NAME(m_mode_reg));
	save_item(NAME(m_rom_size));
	save_item(NAME(m_ram_size));
	save_item(NAME(m_ram));
}

void snes_fcart_state::machine_reset()
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);

	snes_state::machine_reset();

	m_rom_reg = 0;
	m_ram_reg = 0;
	m_size_reg = 0;
	m_mode_reg = 0;

	m_maincpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
}

void snes_fcart_state::snes_fcart_map(address_map &map)
{
	map(0x000000, 0x7dffff).rw(FUNC(snes_fcart_state::snes_fcart_r_bank1), FUNC(snes_fcart_state::snes_fcart_w_bank1));
	map(0x7e0000, 0x7fffff).ram().share("wram");
	map(0x800000, 0xffffff).rw(FUNC(snes_fcart_state::snes_fcart_r_bank2), FUNC(snes_fcart_state::snes_fcart_w_bank2));
}

void snes_fcart_state::spc_map(address_map &map)
{
	map(0x0000, 0xffff).ram().share("aram");
}


inline uint8_t snes_fcart_state::snes_fcart_rom_access_lo(uint32_t offset)
{
	uint32_t addr = (offset & 0xff0000) >> 1;

	if (m_rom_size[0] != m_rom_size[1])
	{
		if (!(addr & m_rom_size[1]))
			addr &= m_rom_size[1] - 1;
		else
			addr &= m_rom_size[0] - 1;
	}
	else
		addr &= m_rom_size[0] - 1;

	addr |= offset & 0x7fff;

	return m_rom[(m_rom_reg << m_rom_shift) | addr];
}

inline uint8_t snes_fcart_state::snes_fcart_rom_access_hi(uint32_t offset)
{
	if (m_rom_size[0] != m_rom_size[1])
	{
		if (!(offset & m_rom_size[1]))
			offset &= m_rom_size[1] - 1;
		else
			offset &= m_rom_size[0] - 1;
	}
	else
		offset &= m_rom_size[0] - 1;

	return m_rom[(m_rom_reg << m_rom_shift) | offset];
}

uint8_t snes_fcart_state::snes_fcart_rom_access(uint32_t offset)
{
	uint8_t value = 0xff;

	if (m_mode_reg & 1)  // hi
	{
		value = snes_fcart_rom_access_hi(offset);
	}
	else  // lo
	{
		value = snes_fcart_rom_access_lo(offset);
	}

	return value;
}


uint8_t snes_fcart_state::snes_fcart_ram_r(uint32_t offset)
{
	if ((m_mode_reg & 1) && (m_ram_size == 0x8000))  // hi 256kbit
		offset = (offset & 0x1fff) | ((offset & 0x30000) >> 3);
	else
		offset &= m_ram_size - 1;

	return m_ram[(m_ram_reg << 11) | offset];
}

void snes_fcart_state::snes_fcart_ram_w(uint32_t offset, uint8_t data)
{
	if ((m_mode_reg & 1) && (m_ram_size == 0x8000))  // hi 256kbit
		offset = (offset & 0x1fff) | ((offset & 0x30000) >> 3);
	else
		offset &= m_ram_size - 1;

	m_ram[(m_ram_reg << 11) | offset] = data;
}


// rd  00-7d:0000-ffff
uint8_t snes_fcart_state::snes_fcart_r_bank1(offs_t offset)
{
	uint8_t value = 0xff;
	uint16_t address = offset & 0xffff;

	if (offset < 0x400000)
	{
		if (address < 0x8000)
		{
			if (address < 0x2000)  // ram
			{
				value = m_wram[address];
			}
			else if (address < 0x5000)  // io
			{
				value = snes_r_io(address);
			}
			else  // < 8000
			{
				if ((m_size_reg & 0xf0) && (offset >= 0x200000) && (address >= 0x6000) && (m_mode_reg & 1))  // hi ram 20-3f,a0-bf:6000-7fff
					value = snes_fcart_ram_r(offset);
				else
					value = snes_open_bus_r();
			}
		}
		else  // rom
		{
			value = snes_fcart_rom_access(offset);
		}
	}
	else  // < 0x7e0000 rom
	{
		if ((m_size_reg & 0xf0) && (offset >= 0x700000) && (address < 0x8000) && !(m_mode_reg & 1))  // lo ram 70-7d,f0-ff:0000-7fff
			value = snes_fcart_ram_r(offset);
		else
			value = snes_fcart_rom_access(offset);
	}

	return value;
}

// rd  80-ff:0000-ffff
uint8_t snes_fcart_state::snes_fcart_r_bank2(offs_t offset)
{
	return snes_fcart_r_bank1(offset);
}


// wr  00-7d:0000-ffff
void snes_fcart_state::snes_fcart_w_bank1(address_space &space, offs_t offset, uint8_t data)
{
	uint16_t address = offset & 0xffff;

	if (offset < 0x400000)
	{
		if (address < 0x8000)
		{
			if (address < 0x2000)  // ram
			{
				m_wram[address] = data;
			}
			else if (address < 0x5000)  // io
			{
				snes_w_io(space, address, data);
			}
			else if (offset < 0x10000 && address < 0x6000)  // 00:5000-5fff
			{
				snes_fcart_menu_wr(address, data);
			}
			else  // < 8000
			{
				if ((m_size_reg & 0xf0) && (offset >= 0x200000) && (address >= 0x6000) && (m_mode_reg & 1))  // hi ram 20-3f:6000-7fff
					snes_fcart_ram_w(offset, data);
				else
					logerror("unknown wr 00-3f:5000-7fff: %06x %02x\n", offset, data);
			}
		}
		else  // rom
		{
			logerror("unknown wr 00-3f:8000-ffff: %06x %02x\n", offset, data);
		}
	}
	else  // < 0x7e0000 rom
	{
		if ((m_size_reg & 0xf0) && (offset >= 0x700000) && (address < 0x8000) && !(m_mode_reg & 1))  // lo ram 70-7d:0000-7fff
			snes_fcart_ram_w(offset, data);
		else
			logerror("unknown wr 40-7d:0000-ffff: %06x %02x\n", offset, data);
	}
}

// wr  80-ff:0000-ffff
void snes_fcart_state::snes_fcart_w_bank2(address_space &space, offs_t offset, uint8_t data)
{
	uint16_t address = offset & 0xffff;

	if (offset < 0x400000)
	{
		if (address < 0x8000)
		{
			if (address < 0x2000)  // ram
			{
				m_wram[address] = data;
			}
			else if (address < 0x5000)  // io
			{
				snes_w_io(space, address, data);
			}
			else  // < 8000
			{
				if ((m_size_reg & 0xf0) && (offset >= 0x200000) && (address >= 0x6000) && (m_mode_reg & 1))  // hi ram a0-bf:6000-7fff
					snes_fcart_ram_w(offset, data);
				else
					logerror("unknown wr 80-bf:5000-7fff: %06x %02x\n", offset | 0x800000, data);
			}
		}
		else  // rom
		{
			logerror("unknown wr 80-bf:8000-ffff: %06x %02x\n", offset | 0x800000, data);
		}
	}
	else  // < 0x7e0000 rom
	{
		if ((m_size_reg & 0xf0) && (offset >= 0x700000) && (address < 0x8000) && !(m_mode_reg & 1))  // lo ram f0-ff:0000-7fff
			snes_fcart_ram_w(offset, data);
		else
			logerror("unknown wr c0-ff:0000-ffff: %06x %02x\n", offset | 0x800000, data);
	}
}


void snes_fcart_state::snes_fcart_menu_wr(uint16_t address, uint8_t data)
{
	switch (address & 0xc00)
	{
	case 0:     // mode
		m_mode_reg = data & 0xf;
		break;
	case 0x400: // size
		m_size_reg = data;
		snes_fcart_setup_rom_map();
		snes_fcart_setup_ram_map();
		break;
	case 0x800: // ram
		m_ram_reg = data & 0x3f;
		break;
	case 0xc00: // rom
		m_rom_reg = data;
		break;
	}

	printf("menu wr: %04x %02x\n", address, data);
}

inline void snes_fcart_state::snes_fcart_setup_rom_map()
{
	#define ROM_MBIT(i) (m_rom_size[0] = ((i) * 0x20000))

	switch (m_size_reg & 0xf)
	{
		case 1: ROM_MBIT(1); break;
		case 2: ROM_MBIT(2); break;
		case 3: ROM_MBIT(4); break;
		case 4: ROM_MBIT(8); break;
		case 5: ROM_MBIT(10); break;
		case 6: ROM_MBIT(12); break;
		case 7: ROM_MBIT(16); break;
		case 8: ROM_MBIT(20); break;
		case 9: ROM_MBIT(24); break;
		case 10: ROM_MBIT(32); break;
		default: ROM_MBIT(0); break;
	}

	#undef ROM_MBIT

	uint32_t sz = m_rom_size[0];

	if ((sz & (sz - 1)) && (sz != 0))  // not power of 2 (more than 1 bit set)
	{
		// get top bit
		int i = 0;
		for (; sz > 1; sz >>= 1)
			i++;
		sz = 1 << i;
	}

	m_rom_size[1] = sz;
}

inline void snes_fcart_state::snes_fcart_setup_ram_map()
{
	#define RAM_KBIT(i) (m_ram_size = ((i) * 0x80))

	switch ((m_size_reg & 0xf0) >> 4)
	{
		case 1: RAM_KBIT(16); break;
		case 2: RAM_KBIT(64); break;
		case 3: RAM_KBIT(256); break;
		default: RAM_KBIT(0); break;
	}

	#undef RAM_KBIT
}


ROM_START( snes_fcart )
	ROM_REGION( 0x2000000, "rom", 0 )
	ROM_LOAD( "rom_1.bin", 0x0000000, 0x800000, CRC(a01430c1) SHA1(8b333c420395b3961ca5481f83f0bf5ea74f88da) )
	ROM_LOAD( "rom_2.bin", 0x0800000, 0x800000, CRC(1e00015f) SHA1(a24a88e56b19ca07d9585d5853629eb2949ddda4) )
	ROM_LOAD( "rom_3.bin", 0x1000000, 0x800000, CRC(4cf11888) SHA1(1bde0d75f74ef10cb4cb8f62bd02781afe2f02c9) )
	ROM_LOAD( "rom_4.bin", 0x1800000, 0x800000, CRC(79d5398d) SHA1(58f21062e120fd87e902182a3096ac38b3377821) )
ROM_END

#define rom_snes_fcart_pal rom_snes_fcart

ROM_START( snes_fcart2 )
	ROM_REGION( 0x4000000, "rom", 0 )
	ROM_LOAD( "rom.bin", 0x0000000, 0x4000000, CRC(bccd4d79) SHA1(302bfd7ed7dd96495d758bd2e35b3d3aa0db3876) )
ROM_END

#define rom_snes_fcart2_pal rom_snes_fcart2

//   YEAR,   NAME,         PARENT, COMPAT, MACHINE,        INPUT,       CLASS,            INIT,            COMPANY,    FULLNAME,            FLAGS

CONS( 2022, snes_fcart,     0,          0, snes_fcart,     snes_fcart, snes_fcart_state, init_snes_fcart, "bootleg", "SNES Flash Cart (ntsc)", MACHINE_SUPPORTS_SAVE )
CONS( 2022, snes_fcart_pal, snes_fcart, 0, snes_fcart_pal, snes_fcart, snes_fcart_state, init_snes_fcart, "bootleg", "SNES Flash Cart (pal)", MACHINE_SUPPORTS_SAVE )

CONS( 2022, snes_fcart2,     0,           0, snes_fcart,     snes_fcart, snes_fcart_state, init_snes_fcart2, "bootleg", "SNES Flash Cart v2 (ntsc)", MACHINE_SUPPORTS_SAVE )
CONS( 2022, snes_fcart2_pal, snes_fcart2, 0, snes_fcart_pal, snes_fcart, snes_fcart_state, init_snes_fcart2, "bootleg", "SNES Flash Cart v2 (pal)", MACHINE_SUPPORTS_SAVE )
