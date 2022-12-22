//
//

#include "emu.h"
#include "snes.h"
#include "bus/snes_ctrl/ctrl.h"
#include "speaker.h"

class snes_fcart_state : public snes_state
{
public:
	snes_fcart_state(const machine_config &mconfig, device_type type, const char *tag)
		: snes_state(mconfig, type, tag)
		, m_ctrl1(*this, "ctrl1")
		, m_ctrl2(*this, "ctrl2")
		, m_rom_reg(0)
		, m_ram_reg(0)
		, m_size_reg(0)
		, m_mode_reg(0)
		, m_menu(1)
	{ }

	void snes_fcart(machine_config &config);
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

	void snes_fcart_map(address_map &map);
	void spc_map(address_map &map);

	inline uint8_t snes_fcart_rom_access(uint32_t offset);
	uint8_t snes_fcart_r_bank1(offs_t offset);
	uint8_t snes_fcart_r_bank2(offs_t offset);
	void snes_fcart_w_bank1(address_space &space, offs_t offset, uint8_t data);
	void snes_fcart_w_bank2(offs_t offset, uint8_t data);

	void snes_fcart_menu_wr(uint16_t adr, uint8_t data);

	uint8_t m_rom_reg;
	uint8_t m_ram_reg;
	uint8_t m_size_reg;
	uint8_t m_mode_reg;
	uint8_t m_menu;

	uint8_t *m_rom;
	uint32_t m_rom_size;
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
}

void snes_fcart_state::init_snes_fcart()
{
	m_rom = memregion("user3")->base();
	m_rom_size = memregion("user3")->bytes();
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
}

void snes_fcart_state::machine_reset()
{
	snes_state::machine_reset();
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



inline uint8_t snes_fcart_state::snes_fcart_rom_access(uint32_t offset)
{
	uint32_t addr = offset & 0x7fff;
	
	//if (!menu)
		addr += (offset & 0xff0000) >> 1;

	return m_rom[addr];
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
		value = snes_fcart_rom_access(offset);
	}

	return value;
}

// rd  80-ff:0000-ffff
uint8_t snes_fcart_state::snes_fcart_r_bank2(offs_t offset)
{
	return 0;
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
				printf("unknown 00-3f:5000-7fff wr: %06x %02x\n", offset, data);
			}
		}
		else  // rom
		{
			printf("unknown 00-3f:8000-ffff wr: %06x %02x\n", offset, data);
		}
	}
	else  // < 0x7e0000 rom
	{
		printf("unknown 40-7d:0000-ffff wr: %06x %02x\n", offset, data);
	}
}

// wr  80-ff:0000-ffff
void snes_fcart_state::snes_fcart_w_bank2(offs_t offset, uint8_t data)
{
	
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

	m_menu = 0;
}


ROM_START( snes_fcart )
	//ROM_REGION( 0x2000000, "maincpu", 0 )
	//ROM_LOAD( "rom_1.bin", 0x0000000, 0x800000, CRC(c5c26cc6) SHA1(281159a42f796defa9808091a56710625b71e014) )
	//ROM_LOAD( "rom_2.bin", 0x0800000, 0x800000, CRC(3a506fcb) SHA1(81cee2fc0ed7e6ccd850a84d4cfaa3b7f4a3cb92) )
	//ROM_LOAD( "rom_3.bin", 0x1000000, 0x800000, CRC(cc819d83) SHA1(1dade629b588c36df4ffd25040b295f63decc05a) )
	//ROM_LOAD( "rom_4.bin", 0x1800000, 0x800000, CRC(a7558e96) SHA1(0186366879658940c19adb78144718f01957d443) )
	ROM_REGION( 0x100000, "user3", 0 )
	ROM_LOAD( "rom_1.bin", 0x00000, 0x100000, CRC(c5c26cc6) SHA1(281159a42f796defa9808091a56710625b71e014) )
ROM_END

// YEAR,NAME,PARENT,COMPAT,MACHINE,INPUT,CLASS,INIT,COMPANY,FULLNAME,FLAGS

CONS( 2022, snes_fcart, 0, 0, snes_fcart, snes_fcart, snes_fcart_state, init_snes_fcart, "bootleg", "SNES Flash Cart", 0 )
