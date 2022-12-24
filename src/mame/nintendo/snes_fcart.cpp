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
		, m_rom_reg(0)
		, m_ram_reg(0)
		, m_size_reg(0)
		, m_mode_reg(0)
		, m_ram(0x20000)
	{ }

	void snes_fcart(machine_config &config);
	void snes_fcart_pal(machine_config &config);
	void init_snes_fcart();

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

	inline uint8_t snes_fcart_rom_access(uint32_t offset);
	inline uint8_t snes_fcart_rom_access_lo(uint32_t offset);
	inline uint8_t snes_fcart_rom_access_hi(uint32_t offset);
	inline uint8_t snes_fcart_ram_r(uint32_t offset);
	inline void snes_fcart_ram_w(uint32_t offset, uint8_t data);
	
	uint8_t snes_fcart_r_bank1(offs_t offset);
	uint8_t snes_fcart_r_bank2(offs_t offset);
	void snes_fcart_w_bank1(address_space &space, offs_t offset, uint8_t data);
	void snes_fcart_w_bank2(address_space &space, offs_t offset, uint8_t data);

	void snes_fcart_menu_wr(uint16_t adr, uint8_t data);

	uint8_t m_rom_reg;
	uint8_t m_ram_reg;
	uint8_t m_size_reg;
	uint8_t m_mode_reg;

	uint8_t *m_rom;
	std::vector<uint8_t> m_ram;
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
	uint8_t value = 0xff;
	uint32_t addr_h, addr_m, addr_l;

	addr_h = m_rom_reg << 17;     // rom a[24:17]
	
	addr_m = (offset & 0x30000);  // snes a[17:16]  a[15] not used
	switch (m_size_reg & 0xf)
	{
		case 3:  // 4mbit 512KB 80000  loads...
			addr_m += offset & 0xc0000;  // snes {4'd0, a[19:18]}
			break;
			
		case 4:  // 8mbit 1MB 100000  loads...
			addr_m += offset & 0x1c0000;  // snes {3'd0, a[20:18]}
			break;
			
		case 5:  // 10mbit 1.25MB 140000  ffight2
			addr_m += offset & 0x240000;                               // snes a[21], a[18]
			addr_m += (offset & 0x200000) ? 0 : (offset & 0x180000);  // snes a[20:19]
			break;
			
		case 6:  // 12mbit 1.5MB 180000  fatfury
			addr_m += offset & 0x2c0000;                               // snes a[21], a[19:18]
			addr_m += (offset & 0x200000) ? 0 : (offset & 0x100000);  // snes a[20]
			break;
			
		case 7:  // 16mbit 2MB 200000  loads...
			addr_m += offset & 0x3c0000;  // snes {2'd0, a[21:18]}
			break;
			
		case 8:  // 20mbit 2.5MB 280000  smas+w
			addr_m += offset & 0x4c0000;                               // snes a[22], a[19:18]
			addr_m += (offset & 0x400000) ? 0 : (offset & 0x300000);  // snes a[21:20]
			break;
			
		case 9:  // 24mbit 3MB 300000  nbajamte, metroid
			addr_m += offset & 0x5c0000;                               // snes a[22], a[20:18]
			addr_m += (offset & 0x400000) ? 0 : (offset & 0x200000);  // snes a[21]
			break;
			
		// case 10:  // 32mbit 4MB 400000  ?
			// addr_m += offset & 0x7c0000;  // snes {1'd0, a[22:18]}
			// break;
			
		default:
			addr_m = 0;  // menu
			break;
	}
	addr_m >>= 1;  // snes -> rom
	
	addr_l = (offset & 0x7fff);   // snes a[14:0] == rom a[14:0]

	value = m_rom[addr_h | (addr_m + addr_l)];
	
	return value;
}

inline uint8_t snes_fcart_state::snes_fcart_rom_access_hi(uint32_t offset)
{
	uint8_t value = 0xff;
	uint32_t addr_h, addr_l;

	addr_h = m_rom_reg << 17;     // rom a[24:17]
	
	addr_l = (offset & 0x18000);  // snes a[16:15]
	switch (m_size_reg & 0xf)
	{
		case 3:  // 4mbit 512KB 80000  sbomberman, zoop
			addr_l += offset & 0x60000;  // snes {4'd0, a[18:17]}
			break;
			
		case 4:  // 8mbit 1MB 100000  claymates, sbomberman2, nhl95
			addr_l += offset & 0xe0000;  // snes {3'd0, a[19:17]}
			break;
			
		// case 5:  // 10mbit 1.25MB 140000  ?
			// break;
			
		case 6:  // 12mbit 1.5MB 180000  actraiser2, chaoseng, nhl96
			addr_l += offset & 0x160000;                              // snes a[20], a[18:17]
			addr_l += (offset & 0x100000) ? 0 : (offset & 0x80000);  // snes a[19]
			break;
			
		case 7:  // 16mbit 2MB 200000  megaman7, secomana
			addr_l += offset & 0x1e0000;  // snes {2'd0, a[20:17]}
			break;
			
		case 8:  // 20mbit 2.5MB 280000  sf2t
			addr_l += offset & 0x260000;                               // snes a[21], a[18:17]
			addr_l += (offset & 0x200000) ? 0 : (offset & 0x180000);  // snes a[20:19]
			break;
			
		case 9:  // 24mbit 3MB 300000  ffight3
			addr_l += offset & 0x2e0000;                               // snes a[21], a[19:17]
			addr_l += (offset & 0x200000) ? 0 : (offset & 0x100000);  // snes a[20]
			break;
			
		case 10:  // 32mbit 4MB 400000  dkc/2/3  all 32mbit?
			addr_l += offset & 0x3e0000;  // snes {1'd0, a[21:17]}
			break;
			
		default:
			addr_l = 0;
			break;
	}
	addr_l += (offset & 0x7fff);   // snes a[14:0] == rom a[14:0]

	value = m_rom[addr_h | addr_l];
	
	return value;
}

inline uint8_t snes_fcart_state::snes_fcart_rom_access(uint32_t offset)
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


inline uint8_t snes_fcart_state::snes_fcart_ram_r(uint32_t offset)
{
	uint8_t value = 0xff;
	uint32_t addr_h, addr_l;

	addr_h = m_ram_reg << 11;  // ram a[16:11]
	addr_l = offset & 0x7ff;   // snes a[10:0]
	
	switch ((m_size_reg & 0xf0) >> 4)
	{
		case 1:  // 16kbit 2KB 800
			addr_l += 0;
			break;
		case 2:  // 64kbit 8KB 2000
			addr_l += offset & 0x1800;  // snes {4'd0, a[12:11]}
			break;
		case 3:  // 256kbit 32KB 8000
			if (m_mode_reg & 1)  // hi  simcity2k
			{
				addr_l += offset & 0x1800;  // snes {2'd0, a[17:16], a[12:11]}
				addr_l += (offset & 0x3000) >> 3;
			}
			else  // lo  simcity
				addr_l += offset & 0x7800;  // snes {2'd0, a[14:11]}
			break;
	}
	
	value = m_ram[addr_h | addr_l];
	
	return value;
}

inline void snes_fcart_state::snes_fcart_ram_w(uint32_t offset, uint8_t data)
{
	uint32_t addr_h, addr_l;

	addr_h = m_ram_reg << 11;  // ram a[16:11]
	addr_l = offset & 0x7ff;   // snes a[10:0]
	
	switch ((m_size_reg & 0xf0) >> 4)
	{
		case 1:  // 16kbit 2KB 800
			addr_l += 0;
			break;
		case 2:  // 64kbit 8KB 2000
			addr_l += offset & 0x1800;  // snes {4'd0, a[12:11]}
			break;
		case 3:  // 256kbit 32KB 8000
			if (m_mode_reg & 1)  // hi  simcity2k
			{
				addr_l += offset & 0x1800;  // snes {2'd0, a[17:16], a[12:11]}
				addr_l += (offset & 0x3000) >> 3;
			}
			else  // lo  simcity
				addr_l += offset & 0x7800;  // snes {2'd0, a[14:11]}
			break;
	}
	
	m_ram[addr_h | addr_l] = data;
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
				printf("menu wr: %04x %02x\n", address, data);
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
					logerror("unknown wr 80-bf:5000-7fff: %06x %02x\n", offset, data);
			}
		}
		else  // rom
		{
			logerror("unknown wr 80-bf:8000-ffff: %06x %02x\n", offset, data);
		}
	}
	else  // < 0x7e0000 rom
	{
		if ((m_size_reg & 0xf0) && (offset >= 0x700000) && (address < 0x8000) && !(m_mode_reg & 1))  // lo ram f0-ff:0000-7fff
			snes_fcart_ram_w(offset, data);
		else
			logerror("unknown wr c0-ff:0000-ffff: %06x %02x\n", offset, data);
	}
}


void snes_fcart_state::snes_fcart_menu_wr(uint16_t adr, uint8_t data)
{
	switch (adr & 0xc00)
	{
	case 0:     // mode
		m_mode_reg = data & 7;
		break;
	case 0x400: // size
		m_size_reg = data;
		break;
	case 0x800: // ram
		m_ram_reg = data & 0x3f;
		break;
	case 0xc00: // rom
		m_rom_reg = data;
		break;
	}
}


ROM_START( snes_fcart )
	ROM_REGION( 0x2000000, "rom", 0 )
	ROM_LOAD( "rom_1.bin", 0x0000000, 0x800000, CRC(a01430c1) SHA1(8b333c420395b3961ca5481f83f0bf5ea74f88da) )
	ROM_LOAD( "rom_2.bin", 0x0800000, 0x800000, CRC(1e00015f) SHA1(a24a88e56b19ca07d9585d5853629eb2949ddda4) )
	ROM_LOAD( "rom_3.bin", 0x1000000, 0x800000, CRC(4cf11888) SHA1(1bde0d75f74ef10cb4cb8f62bd02781afe2f02c9) )
	ROM_LOAD( "rom_4.bin", 0x1800000, 0x800000, CRC(79d5398d) SHA1(58f21062e120fd87e902182a3096ac38b3377821) )
ROM_END

#define rom_snes_fcart_pal rom_snes_fcart

//   YEAR,   NAME,         PARENT, COMPAT, MACHINE,        INPUT,       CLASS,            INIT,            COMPANY,    FULLNAME,            FLAGS

CONS( 2022, snes_fcart,     0,          0, snes_fcart,     snes_fcart, snes_fcart_state, init_snes_fcart, "bootleg", "SNES Flash Cart (ntsc)", 0 )
CONS( 2022, snes_fcart_pal, snes_fcart, 0, snes_fcart_pal, snes_fcart, snes_fcart_state, init_snes_fcart, "bootleg", "SNES Flash Cart (pal)", 0 )
