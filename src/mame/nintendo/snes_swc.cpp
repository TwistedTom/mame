//
// Super Wild Card
//
/*
   1) Registers

      [floppy drive i/o]
      c000  r : input register
                bit 7 : mcs3201 irq signal
                bit 6 : drive 'index' signal (disk insert check)
      c002  w : digital output register
      c004  r : main status register       (uPD765 compatible)
      c005 rw : data register              (uPD765 compatible)
      c007  r : digital input register
      c007  w : diskette control register

      [parallel i/o]
      c008  r : bit 0-7 : parallel data input (reading clears the busy flag)
      c008  w : bit 0-3 : parallel data output
                bit 0   : 0=mode 20, 1=mode 21 (dram mapping)
                bit 1   : 0=mode 1, 1=mode 2 (sram mapping)
      c009  r : busy flag, bit 7 (ep1810 version)
      c000  r : busy flag, bit 5 (fc9203 version)

      c00a-c00f unused (mirrors c008-c009)
      c010-dfff unused (mirrors c000-c00f)

      [page select]
      e000  w : memory page 0     select 8KB (0x2000) page    +0000 1fff
      e001  w : memory page 1                                 +2000 3fff
      e002  w : memory page 2     (snes_addr & 0xff1fff)      +4000 5fff
      e003  w : memory page 3        + (page * 0x2000)        +6000 7fff

      [mode select]
      e004  w : system mode 0 (bios mode, power on default)
      e005  w : system mode 1 (play cartridge)
      e006  w : system mode 2 (cartridge emulation 1)
      e007  w : system mode 3 (cartridge emulation 2)

      [others]
      e008  w : 44256 dram type (for 2,4,6,8 mega byte dram card)
      e009  w : 441000 dram type (for 8,16,24,32 mega byte dram card)

      e00c  w : enable cartridge page mapping at 2000-3fff & a000-bfff (sys mode 0)
                disable cartridge mapping at bank 20-5f, a0-df         (sys mode 2,3)
      e00d  w : enable sram page mapping at 2000-3fff & a000-bfff      (sys mode 0)
                enable cartridge mapping at bank 20-5f, a0-df          (sys mode 2,3)

      * the bank address of the above registers is 00-7d, 80-ff
      * the above registers are available only in system mode 0 (bios mode)
      * [mode select] registers also available in system mode 2

   2) Memory Mapping

      [system mode 0]
      bb:2000 - bb:3fff rw : sram or cartridge page mapping  bb=40-7d,c0-ff
      bb:8000 - bb:9fff rw : dram page mapping               bb=00-7d,80-ff
      bb:a000 - bb:bfff rw : sram or cartridge page mapping  bb=00-7d,80-ff
      bb:c000 - bb:dfff rw : i/o registers                   bb=00-7d,80-ff
      bb:e000 - bb:ffff  r : rom page mapping                bb=00-01

      * 1 page = 8k bytes, 1 bank = 4 pages
      * bb:00-0f = 4 mega bytes
      * bb:00-1f = 8 mega bytes
      * bb:00-2f = 12 mega bytes
      * bb:00-3f = 16 mega bytes
        ...

      [system mode 1]
      bb:0000 - bb:7fff  r : cartridge mapping, bb=40-7d, c0-ff (mode 21)
      bb:8000 - bb:ffff  r : cartridge mapping, bb=00-7d, 80-ff (mode 20,21)

      [system mode 2]
      bb:0000 - bb:7fff  r : dram mapping, bb=40-70, c0-e0 (mode 21)
      bb:8000 - bb:ffff  r : dram mapping, bb=00-70, 80-e0 (mode 20,21)
      70:8000 - 70:ffff rw : sram mode 1 mapping
      30:6000 - 30:7fff rw : sram mode 2 mapping, page 0
      31:6000 - 31:7fff rw : sram mode 2 mapping, page 1
      32:6000 - 32:7fff rw : sram mode 2 mapping, page 2
      33:6000 - 33:7fff rw : sram mode 2 mapping, page 3
      bb:e004 - bb:e007  w : mode select registers, bb=00-7d, 80-ff

      [system mode 3]
      bb:0000 - bb:7fff  r: dram mapping, bb=40-6f, c0-df (mode 21)
      bb:8000 - bb:ffff  r: dram mapping, bb=00-6f, 80-df (mode 20,21)
      70:8000 - 70:ffff rw: sram mode 1 mapping
      30:6000 - 30:7fff rw: sram mode 2 mapping, page 0
      31:6000 - 31:7fff rw: sram mode 2 mapping, page 1
      32:6000 - 32:7fff rw: sram mode 2 mapping, page 2
      33:6000 - 33:7fff rw: sram mode 2 mapping, page 3

      * mode 21
        even dram bank is mapped to bb:0000-bb:7fff
        odd  dram bank is mapped to bb:8000-bb:ffff
*/

// make SOURCES=src/mame/nintendo/snes.cpp,src/mame/nintendo/snes_swc.cpp REGENIE=0 -j5

#include "emu.h"
#include "snes.h"
#include "bus/snes_ctrl/ctrl.h"
#include "speaker.h"
#include "imagedev/floppy.h"
#include "machine/upd765.h"
#include "formats/pc_dsk.h"


class snes_swc_state : public snes_state
{
public:
	snes_swc_state(const machine_config &mconfig, device_type type, const char *tag)
		: snes_state(mconfig, type, tag)
		, m_ctrl1(*this, "ctrl1")
		, m_ctrl2(*this, "ctrl2")
		, m_fdc(*this, "fdc")
		, m_dram(0x400000)
		, m_sram(0x40000)
	{ }

	void snes_swc(machine_config &config);
	void snes_swc_pal(machine_config &config);
	void init_snes_swc();

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
	required_device<mcs3201_device> m_fdc;

	void snes_swc_map(address_map &map);
	void spc_map(address_map &map);

	uint8_t snes_swc_r_bank1(offs_t offset);
	uint8_t snes_swc_r_bank2(offs_t offset);
	void snes_swc_w_bank1(address_space &space, offs_t offset, uint8_t data);
	void snes_swc_w_bank2(address_space &space, offs_t offset, uint8_t data);
	uint8_t snes_swc_io_r(uint16_t address);
	void snes_swc_io_w(uint16_t address, uint8_t data);
	void snes_swc_misc_w(uint16_t address, uint8_t data);
	
	uint8_t fdc_input_r();

	uint8_t *m_bios;
	uint8_t *m_cart;
	std::vector<uint8_t> m_dram;
	std::vector<uint8_t> m_sram;
	int page_select = 0;
	int mode_select = 0;
	int dram_type = 0;
	int other_map = 0;
	int busy = 0;
};


static void swc_floppy_formats(format_registration &fr)
{
	fr.add_pc_formats();
}

static void fdd(device_slot_interface &device)
{
	device.option_add("35hd", FLOPPY_35_HD);
	device.option_add("35dd", FLOPPY_35_DD);
}

uint8_t snes_swc_state::fdc_input_r()
{
	// 5: busy flag (FC9203 ver) 6: index, 7: irq
	uint8_t data = 0;
	
	if (m_fdc->get_irq())
		data |= 0x80;		// polarity?
	
	//if ( index? )
		//data |= 0x40;
	
	if (busy)
		data |= 0x20;		// parallel port transfer started?
	
	return data;
}

void snes_swc_state::snes_swc(machine_config &config)
{
	/* basic machine hardware */
	_5A22(config, m_maincpu, MCLK_NTSC);   // Nintendo S-CPU 5A22-0x, 21.477272MHz / (6, 8, 12) = 1.79 MHz, 2.68 MHz, also 3.58 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &snes_swc_state::snes_swc_map);

	// runs at 24.576 MHz / 12 = 2.048 MHz
	S_SMP(config, m_soundcpu, XTAL(24'576'000) / 12);
	m_soundcpu->set_addrmap(AS_DATA, &snes_swc_state::spc_map);
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
	m_ppu->open_bus_callback().set(FUNC(snes_swc_state::snes_open_bus_r));
	m_ppu->set_screen("screen");

	SNES_CONTROL_PORT(config, m_ctrl1, snes_control_port_devices, "joypad");
	//m_ctrl1->set_onscreen_callback(FUNC(snes_swc_state::onscreen_cb));
	SNES_CONTROL_PORT(config, m_ctrl2, snes_control_port_devices, "joypad");
	//m_ctrl2->set_onscreen_callback(FUNC(snes_swc_state::onscreen_cb));
	//m_ctrl2->set_gunlatch_callback(FUNC(snes_swc_state::gun_latch_cb));

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	S_DSP(config, m_s_dsp, XTAL(24'576'000) / 12);
	m_s_dsp->set_addrmap(0, &snes_swc_state::spc_map);
	m_s_dsp->add_route(0, "lspeaker", 1.00);
	m_s_dsp->add_route(1, "rspeaker", 1.00);
	
	/* swc floppy drive */
	MCS3201(config, m_fdc, 24_MHz_XTAL);
	m_fdc->input_handler().set(FUNC(snes_swc_state::fdc_input_r));
	FLOPPY_CONNECTOR(config, "fdc:0", fdd, "35hd", swc_floppy_formats);
}

void snes_swc_state::snes_swc_pal(machine_config &config)
{
	snes_swc(config);
	m_maincpu->set_clock(MCLK_PAL);
	m_screen->set_raw(DOTCLK_PAL * 2, SNES_HTOTAL * 2, 0, SNES_SCR_WIDTH * 2, SNES_VTOTAL_PAL, 0, SNES_SCR_HEIGHT_PAL);
	m_ppu->set_clock(MCLK_PAL);
}

void snes_swc_state::init_snes_swc()
{
	m_bios = memregion("bios")->base();
	m_cart = memregion("cart")->base();
}

static INPUT_PORTS_START( snes_swc )
INPUT_PORTS_END

void snes_swc_state::io_read()
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

uint8_t snes_swc_state::oldjoy1_read(int latched)
{
	uint8_t ret = 0;
	ret |= m_ctrl1->read_pin4();
	ret |= (m_ctrl1->read_pin5() << 1);
	return ret;
}

uint8_t snes_swc_state::oldjoy2_read(int latched)
{
	uint8_t ret = 0;
	ret |= m_ctrl2->read_pin4();
	ret |= (m_ctrl2->read_pin5() << 1);
	return ret;
}

void snes_swc_state::write_joy_latch(uint8_t data)
{
	m_ctrl1->write_strobe(data);
	m_ctrl2->write_strobe(data);
}

void snes_swc_state::wrio_write(uint8_t data)
{
	if (!(SNES_CPU_REG(WRIO) & 0x80) && (data & 0x80))
	{
		// external latch
		m_ppu->set_latch_hv(m_ppu->current_x(), m_ppu->current_y());
	}

	m_ctrl1->write_pin6(BIT(data, 6));
	m_ctrl2->write_pin6(BIT(data, 7));
}

void snes_swc_state::machine_start()
{
	snes_state::machine_start();
	
	save_item(NAME(m_dram));
	save_item(NAME(m_sram));
	save_item(NAME(page_select));
	save_item(NAME(mode_select));
	save_item(NAME(dram_type));
	save_item(NAME(other_map));
	save_item(NAME(busy));
}

void snes_swc_state::machine_reset()
{
	snes_state::machine_reset();
}

void snes_swc_state::snes_swc_map(address_map &map)
{
	map(0x000000, 0x7dffff).rw(FUNC(snes_swc_state::snes_swc_r_bank1), FUNC(snes_swc_state::snes_swc_w_bank1));
	map(0x7e0000, 0x7fffff).ram().share("wram");
	map(0x800000, 0xffffff).rw(FUNC(snes_swc_state::snes_swc_r_bank2), FUNC(snes_swc_state::snes_swc_w_bank2));
}

void snes_swc_state::spc_map(address_map &map)
{
	map(0x0000, 0xffff).ram().share("aram");
}


// rd  00-7d:0000-ffff
uint8_t snes_swc_state::snes_swc_r_bank1(offs_t offset)
{
	uint8_t value = 0xff;
	uint16_t address = offset & 0xffff;

	if ((offset < 0x020000) && (address >= 0xe000))
		value = m_bios[(address & 0x1fff) | ((offset & 0x10000) >> 3)];
		// value = m_bios[(address & 0x1fff) | ((offset & 0x10000) ? 0 : 0x2000)];
	else
		value = snes_swc_r_bank2(offset);

	return value;
}

// rd  80-ff:0000-ffff
uint8_t snes_swc_state::snes_swc_r_bank2(offs_t offset)
{
	uint8_t value = 0xff;
	uint16_t address = offset & 0xffff;

	if (offset < 0x400000)           // bank 00-3f
	{
		if (address < 0x2000)        // ram  0000-1fff
		{
			value = m_wram[address];
		}
		else if (address < 0x8000)  // io  2000-7fff
		{
			value = snes_r_io(address);
		}
		else if (address < 0xa000)  // dram  8000-9fff
		{
			if (!machine().side_effects_disabled())
				logerror("dram rd: %06x page %d\n", offset, page_select);
		}
		else if (address < 0xc000)  // sram or cart  a000-bfff
		{
			if (!machine().side_effects_disabled())
			{
				if (other_map)
					logerror("sram rd: %06x page %d\n", offset, page_select);
				else
					logerror("cart rd: %06x page %d\n", offset, page_select);
			}
		}
		else if (address < 0xe000)  // i/o regs  c000-dfff
		{
			if (!machine().side_effects_disabled())
				logerror("swc i/o regs rd: %06x\n", offset);
			value = snes_swc_io_r(address);
		}
		else                        // ?  e000-ffff  (bios in rom banks 00-01 only)
		{
			if (!machine().side_effects_disabled())
				logerror("? rd: %06x\n", offset);
			value = snes_open_bus_r();
		}
	}
	else  // < 0x7e0000                bank 40-7d
	{
		if (address < 0x2000)       // ?  0000-1fff
		{
			if (!machine().side_effects_disabled())
				logerror("? rd: %06x\n", offset);
			value = snes_open_bus_r();
		}
		else if (address < 0x4000)  // sram or cart  2000-3fff
		{
			if (!machine().side_effects_disabled())
			{
				if (other_map)
					logerror("sram rd: %06x page %d\n", offset, page_select);
				else
					logerror("cart rd: %06x page %d\n", offset, page_select);
			}
		}
		else if (address < 0x8000)  // ?  4000-7fff
		{
			if (!machine().side_effects_disabled())
				logerror("? rd: %06x\n", offset);
			value = snes_open_bus_r();
		}
		else if (address < 0xa000)  // dram  8000-9fff
		{
			if (!machine().side_effects_disabled())
				logerror("dram rd: %06x page %d\n", offset, page_select);
		}
		else if (address < 0xc000)  // sram or cart  a000-bfff
		{
			if (!machine().side_effects_disabled())
			{
				if (other_map)
					logerror("sram rd: %06x page %d\n", offset, page_select);
				else
					logerror("cart rd: %06x page %d\n", offset, page_select);
			}
		}
		else if (address < 0xe000)  // i/o regs  c000-dfff
		{
			if (!machine().side_effects_disabled())
				logerror("swc i/o regs rd: %06x\n", offset);
			value = snes_swc_io_r(address);
		}
		else                         // ?  e000-ffff  (bios in rom banks 00-01 only)
		{
			if (!machine().side_effects_disabled())
				logerror("? rd: %06x\n", offset);
			value = snes_open_bus_r();
		}
	}

	return value;
}


// wr  00-7d:0000-ffff
void snes_swc_state::snes_swc_w_bank1(address_space &space, offs_t offset, uint8_t data)
{
	uint16_t address = offset & 0xffff;

	if (offset < 0x400000)           // bank 00-3f
	{
		if (address < 0x2000)        // ram  0000-1fff
		{
			m_wram[address] = data;
		}
		else if (address < 0x8000)  // io  2000-7fff
		{
			snes_w_io(space, address, data);
		}
		else if (address < 0xa000)  // dram  8000-9fff
		{
			logerror("dram wr: %06x page %d %02x\n", offset, page_select, data);
		}
		else if (address < 0xc000)  // sram or cart  a000-bfff
		{
			if (other_map)
				logerror("sram wr: %06x page %d %02x\n", offset, page_select, data);
			else
				logerror("cart wr: %06x page %d %02x\n", offset, page_select, data);
		}
		else if (address < 0xe000)  // i/o regs  c000-dfff
		{
			logerror("swc i/o regs wr: %06x %02x\n", offset, data);
			snes_swc_io_w(address, data);
		}
		else                         // page select, mode select, others  e000-ffff
		{
			logerror("swc misc regs wr: %06x %02x\n", offset, data);
			snes_swc_misc_w(address, data);
		}
	}
	else  // < 0x7e0000                 bank 40-7d
	{
		if (address < 0x2000)        // ?  0000-1fff
		{
			logerror("? wr: %06x %02x\n", offset, data);
		}
		else if (address < 0x4000)  // sram or cart  2000-3fff
		{
			if (other_map)
				logerror("sram wr: %06x page %d %02x\n", offset, page_select, data);
			else
				logerror("cart wr: %06x page %d %02x\n", offset, page_select, data);
		}
		else if (address < 0x8000)  // ?  4000-7fff
		{
			logerror("? wr: %06x %02x\n", offset, data);
		}
		else if (address < 0xa000)  // dram  8000-9fff
		{
			logerror("dram wr: %06x page %d %02x\n", offset, page_select, data);
		}
		else if (address < 0xc000)  // sram or cart  a000-bfff
		{
			if (other_map)
				logerror("sram wr: %06x page %d %02x\n", offset, page_select, data);
			else
				logerror("cart wr: %06x page %d %02x\n", offset, page_select, data);
		}
		else if (address < 0xe000)  // i/o regs  c000-dfff
		{
			logerror("swc i/o regs wr: %06x %02x\n", offset, data);
			snes_swc_io_w(address, data);
		}
		else                        // page select, mode select, others  e000-ffff
		{
			logerror("swc misc regs wr: %06x %02x\n", offset, data);
			snes_swc_misc_w(address, data);
		}
	}
}

// wr  80-ff:0000-ffff
void snes_swc_state::snes_swc_w_bank2(address_space &space, offs_t offset, uint8_t data)
{
	snes_swc_w_bank1(space, offset, data);
}


// 00-7d,80-ff : c00x rd  fdc & parallel i/o
uint8_t snes_swc_state::snes_swc_io_r(uint16_t address)
{
	uint8_t data = 0;
	
	switch (address & 0xf)
	{
		case 0:  // fdc  input  5: busy flag (FC9203 ver) 6: index, 7: irq
			data = m_fdc->input_r();
			if (!machine().side_effects_disabled())
				logerror(" fdc input reg rd: %04x %02x\n", address, data);
			break;
			
		case 4:  // fdc  main status
			data = m_fdc->msr_r();
			if (!machine().side_effects_disabled())
				logerror(" fdc main status reg rd: %04x %02x\n", address, data);
			break;
			
		case 5:  // fdc  data
			data = m_fdc->fifo_r();
			if (!machine().side_effects_disabled())
				logerror(" fdc data reg rd: %04x %02x\n", address, data);
			break;
			
		case 7:  // fdc  digital input
			data = m_fdc->dir_r();
			if (!machine().side_effects_disabled())
				logerror(" fdc digital input reg rd: %04x %02x\n", address, data);
			break;
			
		case 8:  // parallel in
			busy = 0;
			if (!machine().side_effects_disabled())
				logerror(" parallel data rd: %04x\n", address);
			break;
			
		case 9:  // busy flag (EP1810 ver)
			if (!machine().side_effects_disabled())
				logerror(" [EP1810 ver] busy flag rd: %04x\n", address);
			break;
			
		default:
			if (!machine().side_effects_disabled())
				logerror(" unknown c00x rd: %04x\n", address);
			break;
	}

	return data;
}

// 00-7d,80-ff : c00x wr  fdc & parallel i/o
void snes_swc_state::snes_swc_io_w(uint16_t address, uint8_t data)
{
	switch (address & 0xf)
	{
		case 2:  // fdc  digital output
			m_fdc->dor_w(data);
			logerror(" fdc digital output reg wr: %04x %02x\n", address, data);
			break;
			
		case 5:  // fdc  data
			m_fdc->fifo_w(data);
			logerror(" fdc data reg wr: %04x %02x\n", address, data);
			break;
			
		case 7:  // fdc  disk control
			m_fdc->ccr_w(data);
			logerror(" fdc disk control reg wr: %04x %02x\n", address, data);
			break;
			
		case 8:  // parallel out & dram/sram mapping
			logerror(" parallel data / map mode wr: %04x %02x\n", address, data);

			if (data & 1)
				logerror("  dram mapping set to mode 21 (hi)\n");
			else
				logerror("  dram mapping set to mode 20 (lo)\n");

			if (data & 2)
				logerror("  sram mapping set to mode 2 (hi)\n");
			else
				logerror("  sram mapping set to mode 1 (lo)\n");

			break;
			
		default:
			logerror(" unknown c00x i/o wr: %04x %02x\n", address, data);
			break;
	}
	// TODO: mirrors
}

// 00-7d,80-ff : e00x wr  misc  page select, mode select, others
void snes_swc_state::snes_swc_misc_w(uint16_t address, uint8_t data)
{
	switch (address & 0xf)  // data not used
	{
		case 0:  // page select
		case 1:
		case 2:
		case 3:
			page_select = address & 3;
			logerror(" page select wr: page = %d\n", page_select);
			break;

		case 4:  // system mode select
		case 5:
		case 6:
		case 7:
			mode_select = address & 3;
			logerror(" system mode select wr: mode = %d\n", mode_select);
			break;

		case 8:  // dram type
		case 9:
			dram_type = address & 1;
			logerror(" dram type select wr:\n");
			if (dram_type)
				logerror("  441000 dram selected (8,16,24,32 MB)\n");
			else
				logerror("  44256 dram selected (2,4,6,8 MB)\n");
			break;

		case 0xc:  // cart/sram mapped @ a000-bfff
		case 0xd:
			other_map = address & 1;
			logerror(" cart/sram mapping select wr:\n");
			if (other_map)
				logerror("  enabled sram @ a000-bfff\n");
			else
				logerror("  enabled cart @ a000-bfff\n");
			break;

		default:
			logerror(" unknown e00x misc wr: %04x %02x\n", address, data);
			break;
	}
}


ROM_START( snes_swc )
	ROM_REGION( 0x4000, "bios", 0 )
	ROM_LOAD( "swc_28cc_280694.bin", 0x0000, 0x4000, CRC(6e14fce2) SHA1(05b69eb087531e488e8a7ece9437982b4e335e18) )

	ROM_REGION( 0x80000, "cart", 0 )
	ROM_LOAD( "super_soccer_u.bin", 0x00000, 0x80000, CRC(4fd164d8) SHA1(47f401cf0b5845e129b636d381233c23ba74cef6) )
ROM_END

#define rom_snes_swc_pal rom_snes_swc

//   YEAR,   NAME,      PARENT, COMPAT, MACHINE,      INPUT,     CLASS,          INIT,          COMPANY,         FULLNAME,              FLAGS

CONS( 2022, snes_swc,     0,        0, snes_swc,     snes_swc, snes_swc_state, init_snes_swc, "Front Fareast", "Super Wild Card (ntsc)", 0 )
CONS( 2022, snes_swc_pal, snes_swc, 0, snes_swc_pal, snes_swc, snes_swc_state, init_snes_swc, "Front Fareast", "Super Wild Card (pal)",  0 )
