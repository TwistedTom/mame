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
      c008  w : bit 0-3 : parallel data output                                   PC side: S3-6  err, sel, pout, ack  pins 15,13,12,10  lsb?
                bit 0   : 0=mode 20, 1=mode 21 (dram mapping)
                bit 1   : 0=mode 1, 1=mode 2 (sram mapping)
				bit 4   : busy?                                                  PC side: S7 /busy  pin 11
      c009  r : busy flag, bit 7 (ep1810 version)
      c000  r : busy flag, bit 5 (fc9203 version)  <-- wrong? strobe?            PC side: C0 /str  pin 1

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
                disable cartridge mapping at bank 20-5f, a0-df         (sys mode 2,3)   <-- what is this for?
      e00d  w : enable sram page mapping at 2000-3fff & a000-bfff      (sys mode 0)          (not dsp range)
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
      70:8000 - 70:ffff rw : sram mode 1 mapping                               <-- wrong?
      30:6000 - 30:7fff rw : sram mode 2 mapping, page 0                            lo sram is 0000-7fff
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


   ------------------------------------------------------------------------------------
   file header - 512 bytes
   ------------------------------------------------------------------------------------
   #1 - from "SUPER WILDCARD DX- USERS GUIDE"
   ------------------------------------------------------------------------------------

   file format of the wild card program:
   the wild card consists of a 512 byte header and program blocks, one block is 8kbytes
   the program block is loaded into main memory from low address to high address sequentally
   the recommended file name is .smc or .swc

   content of file header:

   byte 0 - numbers of block (low byte)
   byte 1 - numbers of block (high byte)   total numbers of blocks = (byte 1 * 256) + byte 0

   byte 2 - program execution mode
            bit: 7 6 5 4 3 2 1 0
                 x               : 0 reserved
                   x             : 0=last/only file, 1=search for next file
                     x           : sram memory mapping: 0=mode 20, 1=mode 21
                       x         : program memory mapping: 0=mode 20, 1=mode 21
                         x x     : sram size: 00=256k, 01=64k, 10=16k, 11=off
                             x x : 00 reserved

   byte 3-7 - 0 reserved
   byte 8   - aa, file id
   byte 9   - bb, file id
   byte 10  - file type:
               04=program
               05=battery backup data
               08=real-time save data
   byte 11-511 -  reserved

   ------------------------------------------------------------------------------------
   #2 - Various other old sources
   ------------------------------------------------------------------------------------

   byte
    0   - low byte of 8k-bytes page counts
    1   - high byte of 8k-bytes page counts
    2   - emulation mode select
          bit 7 6 5 4 3 2 1 0
              x               : 1=run in mode 0 (jump $8000)
                x             : 0=last file of the game (multi-file loading)
                  x           : 0=mode 1, 1=mode 2 (sram mapping)
                    x         : 0=mode 20, 1=mode 21 (dram mapping)
                      x x     : 0=sram off, 1=sram 16k, 2=sram 64k, 3=sram 256k (sram size)
                          x   : 0=run in mode 3, 1=run in mode 2 (jmp reset)
                            x : 0=disable, 1=enable (external cart at bank 20-5f,a0-df for system modes 2,3)
    3-7 - reserved (should be '00')
    8   - file id code 1 (should be 'aa')
    9   - file id code 2 (should be 'bb')
    10  - check this byte if id 1 & 2 match
          02 : magic griffin game file (pc engine)
          03 : magic griffin sram data file
          04 : swc & smc game file (snes, SUPER MAGICOM)
          05 : swc & smc password, sram data, saver data file.
          06 : smd game file (megadrive)
          07 : smd sram data file
    37  - reserved (should be 00)
    11-511 - reserved (should be 00)

   ------------------------------------------------------------------------------------
   notes:

   super wild card SMS3201 with 2.8cc seems to use #1
   is #2 for older harware and/or firmware?
   sram size bits reversed
   old super soccer file (possibly recorded with old f/w) emu byte is 2d, 2.8cc is 21

   ------------------------------------------------------------------------------------

   top cart slot:
   asic has control of a13, a14, /cart, d0-7, everything else is straight through

   bios:
   27c128 16KB, a13 pin is connected to a16

   unknown writes:
   0c -> c009       could be for ep1810 version?
   10 -> c008       bit 4 ?

   cart sram check:
   checks lo at 706000 through the 2000-3fff window
   checks hi at 306000 directly

*/

// make SOURCES=src/mame/nintendo/snes.cpp,src/mame/nintendo/snes_swc.cpp REGENIE=0 -j5

#include "emu.h"
#include "snes.h"
#include "bus/snes_ctrl/ctrl.h"
#include "speaker.h"
#include "imagedev/floppy.h"
#include "machine/upd765.h"
#include "formats/swc_dsk.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#define SWC_DEBUG 1


class snes_swc_state : public snes_state
{
public:
	snes_swc_state(const machine_config &mconfig, device_type type, const char *tag)
		: snes_state(mconfig, type, tag)
		, m_ctrl1(*this, "ctrl1")
		, m_ctrl2(*this, "ctrl2")
		, m_fdc(*this, "fdc")
		, m_fdd(*this, "fdd")
		, m_cartslot(*this, "cartslot")
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
	required_device<floppy_connector> m_fdd;
	required_device<generic_slot_device> m_cartslot;

	void snes_swc_map(address_map &map);
	void spc_map(address_map &map);

	uint8_t snes_swc_r_bank1(offs_t offset);
	uint8_t snes_swc_r_bank2(offs_t offset);
	void snes_swc_w_bank1(address_space &space, offs_t offset, uint8_t data);
	void snes_swc_w_bank2(address_space &space, offs_t offset, uint8_t data);
	inline uint8_t snes_swc_mode_0_r_bank1(offs_t offset);
	inline uint8_t snes_swc_mode_0_r_bank2(offs_t offset);
	inline void snes_swc_mode_0_w(address_space &space, offs_t offset, uint8_t data);
	inline uint8_t snes_swc_io_r(uint16_t address);
	inline void snes_swc_io_w(uint16_t address, uint8_t data);
	inline void snes_swc_misc_w(uint16_t address, uint8_t data);
	inline void snes_swc_dram_w(offs_t offset, uint8_t data);
	inline uint8_t snes_swc_dram_r(offs_t offset);
	inline void snes_swc_sram_w(uint16_t address, uint8_t data);
	inline uint8_t snes_swc_sram_r(uint16_t address);
	inline void snes_swc_cart_w(uint16_t address, uint8_t data);
	inline uint8_t snes_swc_cart_r(offs_t offset);
	inline uint8_t snes_swc_mode_1_r(offs_t offset);
	inline void snes_swc_mode_1_w(address_space &space, offs_t offset, uint8_t data);
	inline uint8_t snes_swc_mode_1_rom_access(offs_t offset);

	static void swc_floppy_formats(format_registration &fr);
	uint8_t fdc_input_r();

	//DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);
	image_init_result cart_load(device_image_interface &image);
	//DECLARE_DEVICE_IMAGE_UNLOAD_MEMBER(cart_unload)
	void cart_unload(device_image_interface &image);
	void snes_swc_find_cart_type();
	void snes_swc_set_cart_size_mask();
	inline uint32_t snes_swc_clamp_rom_size(uint32_t address);
	void snes_swc_find_cart_sram();

	uint8_t *m_bios;
	uint8_t m_dram[0x400000];
	uint8_t m_sram[0x8000];
	int dram_map = 0;
	int sram_map = 0;
	int page_select = 0;
	int mode_select = 0;
	int dram_type = 0;
	int sram_or_cart_map = 0;
	int busy = 0;

	int cart_type = 0;
	uint32_t cart_size = 0;
	uint32_t cart_size_mask = 0;
	uint8_t *m_cart;
	uint32_t cart_sram_size = 0;
	std::vector<uint8_t> cart_sram;

	uint8_t par_debug_str = 0;
	uint8_t par_debug_data = 0;
};

//DEVICE_IMAGE_LOAD_MEMBER(snes_swc_state::cart_load)
image_init_result snes_swc_state::cart_load(device_image_interface &image)
{
	uint32_t size = m_cartslot->common_get_size("rom");

	if (size > 0x400000)
	{
		image.seterror(image_error::INVALIDIMAGE, "Unsupported cartridge size");
		return image_init_result::FAIL;
	}

	m_cartslot->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	m_cartslot->common_load_rom(m_cartslot->get_rom_base(), size, "rom");

	cart_size = m_cartslot->get_rom_size();
	m_cart = m_cartslot->get_rom_base();
	snes_swc_set_cart_size_mask();
	snes_swc_find_cart_type();
	snes_swc_find_cart_sram();

	return image_init_result::PASS;
}

//DECLARE_DEVICE_IMAGE_UNLOAD_MEMBER(snes_swc_state::cart_unload)
void snes_swc_state::cart_unload(device_image_interface &image)
{
	if (cart_sram_size)
	{
		m_cartslot->battery_save(cart_sram.data(), cart_sram_size);
	}
}

void snes_swc_state::snes_swc_find_cart_type()
{
	int fail = 0;

	for (int i = 0x7fc0; i < (0x7fc0 + 21); i++ )
	{
		uint8_t ch = m_cart[i];
		if (!(ch >= ' ' && ch <= '~'))
		{
			fail = 1;
			break;
		}
	}

	if ((m_cart[0x7fd5] & 0xef) != 0x20)  // 20 or 30
		fail = 1;

	if (!fail)
	{
		cart_type = 0;
		logerror("cart is lorom\n");
		return;
	}


	fail = 0;

	for (int i = 0xffc0; i < (0xffc0 + 21); i++ )
	{
		uint8_t ch = m_cart[i];
		if (!(ch >= ' ' && ch <= '~'))
		{
			fail = 1;
			break;
		}
	}

	if ((m_cart[0xffd5] & 0xef) != 0x21)  // 21 or 31
		fail = 1;

	if (!fail)
	{
		cart_type = 1;
		logerror("cart is hirom\n");
		return;
	}


	logerror("unknown cart\n");
}

void snes_swc_state::snes_swc_set_cart_size_mask()
{
	uint32_t sz = cart_size;

	if ((sz & (sz - 1)) && (sz != 0))  // not power of 2 (more than 1 bit set)
	{
		// get top bit
		int i = 0;
		for (; sz > 1; sz >>= 1)
			i++;
		sz = 1 << i;
	}

	cart_size_mask = sz;

	logerror("cart size is %d bytes (0x%06x)\n", cart_size, cart_size);
	logerror("cart size mask is %d bytes (0x%06x)\n", cart_size_mask, cart_size_mask);
}

inline uint32_t snes_swc_state::snes_swc_clamp_rom_size(uint32_t address)
{
	if (cart_size != cart_size_mask)
	{
		if (!(address & cart_size_mask))
			address &= cart_size_mask - 1;
		else
			address &= cart_size - 1;
	}
	else
		address &= cart_size - 1;

	return address;
}

void snes_swc_state::snes_swc_find_cart_sram()
{
	uint32_t sz = 0;

	if (!cart_type)  // lo
	{
		if (m_cart[0x7fd6] == 2)
			sz = m_cart[0x7fd8];
	}
	else
	{
		if (m_cart[0xffd6] == 2)
			sz = m_cart[0xffd8];
	}

	if (sz)
	{
		sz = 1 << sz;
		logerror("cart has %dKB sram\n", sz);
	}
	else
	{
		logerror("cart doesn't have sram\n");
	}

	cart_sram_size = sz << 10;
	cart_sram_size &= 0x20000 - 1;  // clamp just in case...
	if (cart_sram_size)
	{
		cart_sram.resize(cart_sram_size);
		m_cartslot->battery_load(cart_sram.data(), cart_sram_size, 0xff);
	}
}


static void swc_fdd_options(device_slot_interface &device)
{
	device.option_add("35hd", FLOPPY_35_HD);
}

void snes_swc_state::swc_floppy_formats(format_registration &fr)
{
	fr.add(FLOPPY_SWC_FORMAT);
}

uint8_t snes_swc_state::fdc_input_r()
{
	// 5: busy flag (FC9203 ver) 6: index, 7: irq
	uint8_t data = 0;

	if (m_fdc->get_irq())
		data |= 0x80;

	int idx = m_fdd->get_device()->idx_r();
	if (!idx)
		data |= 0x40;

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

	/* swc floppy drive (drive #1) */
	MCS3201(config, m_fdc, 24_MHz_XTAL);
	m_fdc->input_handler().set(FUNC(snes_swc_state::fdc_input_r));
	FLOPPY_CONNECTOR(config, m_fdd, swc_fdd_options, "35hd", snes_swc_state::swc_floppy_formats, true).enable_sound(true);

	/* cart slot */
	GENERIC_CARTSLOT(config, m_cartslot, generic_plain_slot, "snes_swc_cart", "sfc,bin,rom");
	m_cartslot->set_device_load(FUNC(snes_swc_state::cart_load));
	m_cartslot->set_device_unload(FUNC(snes_swc_state::cart_unload));
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
	save_item(NAME(dram_map));
	save_item(NAME(sram_map));
	save_item(NAME(page_select));
	save_item(NAME(mode_select));
	save_item(NAME(dram_type));
	save_item(NAME(sram_or_cart_map));
	save_item(NAME(busy));
	save_item(NAME(cart_type));
	save_item(NAME(cart_size));
	save_item(NAME(cart_size_mask));
	save_item(NAME(cart_sram_size));
	save_item(NAME(cart_sram));
	save_item(NAME(par_debug_str));
	save_item(NAME(par_debug_data));
	//save_item(NAME(m_cart));
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

	switch (mode_select)
	{
		case 0:  // bios
			value = snes_swc_mode_0_r_bank1(offset);
			break;
		case 1:  // cart
			value = snes_swc_mode_1_r(offset);
			break;
		case 2:  // dram with mode select registers available

			break;
		case 3:  // dram

			break;
	}

	return value;
}

// rd  80-ff:0000-ffff
uint8_t snes_swc_state::snes_swc_r_bank2(offs_t offset)
{
	uint8_t value = 0xff;

	switch (mode_select)
	{
		case 0:  // bios
			value = snes_swc_mode_0_r_bank2(offset);
			break;
		case 1:  // cart
			value = snes_swc_mode_1_r(offset);
			break;
		case 2:  // dram with mode select registers available

			break;
		case 3:  // dram

			break;
	}

	return value;
}

// wr  00-7d:0000-ffff
void snes_swc_state::snes_swc_w_bank1(address_space &space, offs_t offset, uint8_t data)
{
	switch (mode_select)
	{
		case 0:  // bios
			snes_swc_mode_0_w(space, offset, data);
			break;
		case 1:  // cart
			snes_swc_mode_1_w(space, offset, data);
			break;
		case 2:  // dram with mode select registers available

			break;
		case 3:  // dram

			break;
	}
}

// wr  80-ff:0000-ffff
void snes_swc_state::snes_swc_w_bank2(address_space &space, offs_t offset, uint8_t data)
{
	switch (mode_select)
	{
		case 0:  // bios
			snes_swc_mode_0_w(space, offset, data);
			break;
		case 1:  // cart
			snes_swc_mode_1_w(space, offset, data);
			break;
		case 2:  // dram with mode select registers available

			break;
		case 3:  // dram

			break;
	}
}


// mode 0 - bios

inline uint8_t snes_swc_state::snes_swc_mode_0_r_bank1(offs_t offset)
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

inline uint8_t snes_swc_state::snes_swc_mode_0_r_bank2(offs_t offset)
{
	uint8_t value = 0xff;
	uint16_t address = offset & 0xffff;

	if (offset < 0x400000)           // bank 00-3f
	{
		if (address < 0x2000)        // ram  0000-1fff
		{
			value = m_wram[address];
		}
		else if (address < 0x6000)  // io  2000-5fff
		{
			value = snes_r_io(address);
		}
		else if (address < 0x8000)  // hi sram  6000-7fff   what banks?  2.8cc uses 30 for the check
		{
			if (cart_type && cart_sram_size && m_cartslot && m_cartslot->exists())
			{
				address &= cart_sram_size - 1;
				value = cart_sram[address];
				if (!machine().side_effects_disabled() && SWC_DEBUG)
					logerror("hi cart sram rd: %04x %02x\n", address, value);
			}
		}
		else if (address < 0xa000)  // dram  8000-9fff
		{
			if (!machine().side_effects_disabled() && SWC_DEBUG)
				logerror("dram rd: %06x page %d\n", offset, page_select);
			value = snes_swc_dram_r(offset);
		}
		else if (address < 0xc000)  // sram or cart  a000-bfff
		{
			if (!machine().side_effects_disabled() && SWC_DEBUG)
			{
				if (sram_or_cart_map)
					logerror("sram rd: a000-bfff:%04x -> %06x\n", (address & 0x1fff), (page_select << 13) | (offset & 0xff9fff));
				else
					logerror("cart rd: a000-bfff:%04x -> %06x\n", (address & 0x1fff), (page_select << 13) | (offset & 0xff9fff));
			}
			if (sram_or_cart_map)
				value = snes_swc_sram_r(address);
			else
				value = snes_swc_cart_r(offset);
		}
		else if (address < 0xe000)  // i/o regs  c000-dfff
		{
			if (!machine().side_effects_disabled() && SWC_DEBUG)
				logerror("swc i/o regs rd: %06x\n", offset);
			value = snes_swc_io_r(address);
		}
		else                        // ?  e000-ffff  (bios in rom banks 00-01 only)
		{
			if (!machine().side_effects_disabled() && SWC_DEBUG)
				logerror("? rd: %06x\n", offset);
			value = snes_open_bus_r();
		}
	}
	else  // < 0x7e0000                bank 40-7d
	{
		if (address < 0x2000)       // ?  0000-1fff
		{
			if (!machine().side_effects_disabled() && SWC_DEBUG)
				logerror("? rd: %06x\n", offset);
			value = snes_open_bus_r();
		}
		else if (address < 0x4000)  // sram or cart  2000-3fff
		{
			if (!machine().side_effects_disabled() && SWC_DEBUG)
			{
				if (sram_or_cart_map)
					logerror("sram rd: 2000-3fff:%04x -> %06x\n", (address & 0x1fff), (page_select << 13) | (offset & 0xff9fff));
				else
					logerror("cart rd: 2000-3fff:%04x -> %06x\n", (address & 0x1fff), (page_select << 13) | (offset & 0xff9fff));
			}
			if (sram_or_cart_map)
				value = snes_swc_sram_r(address);
			else
				value = snes_swc_cart_r(offset);
		}
		else if (address < 0x8000)  // ?  4000-7fff
		{
			if (!machine().side_effects_disabled() && SWC_DEBUG)
				logerror("? rd: %06x\n", offset);
			value = snes_open_bus_r();
		}
		else if (address < 0xa000)  // dram  8000-9fff
		{
			if (!machine().side_effects_disabled() && SWC_DEBUG)
				logerror("dram rd: %06x page %d\n", offset, page_select);
			value = snes_swc_dram_r(offset);
		}
		else if (address < 0xc000)  // sram or cart  a000-bfff
		{
			if (!machine().side_effects_disabled() && SWC_DEBUG)
			{
				if (sram_or_cart_map)
					logerror("sram rd: a000-bfff:%04x -> %06x\n", (address & 0x1fff), (page_select << 13) | (offset & 0xff9fff));
				else
					logerror("cart rd: a000-bfff:%04x -> %06x\n", (address & 0x1fff), (page_select << 13) | (offset & 0xff9fff));
			}
			if (sram_or_cart_map)
				value = snes_swc_sram_r(address);
			else
				value = snes_swc_cart_r(offset);
		}
		else if (address < 0xe000)  // i/o regs  c000-dfff
		{
			if (!machine().side_effects_disabled() && SWC_DEBUG)
				logerror("swc i/o regs rd: %06x\n", offset);
			value = snes_swc_io_r(address);
		}
		else                         // ?  e000-ffff  (bios in rom banks 00-01 only)
		{
			if (!machine().side_effects_disabled() && SWC_DEBUG)
				logerror("? rd: %06x\n", offset);
			value = snes_open_bus_r();
		}
	}

	return value;
}

inline void snes_swc_state::snes_swc_mode_0_w(address_space &space, offs_t offset, uint8_t data)
{
	uint16_t address = offset & 0xffff;

	if (offset < 0x400000)           // bank 00-3f
	{
		if (address < 0x2000)        // ram  0000-1fff
		{
			m_wram[address] = data;
		}
		else if (address < 0x6000)  // io  2000-5fff
		{
			snes_w_io(space, address, data);
		}
		else if (address < 0x8000)  // hi sram  6000-7fff   what banks?  2.8cc uses 30 for the check
		{
			if (cart_type && cart_sram_size && m_cartslot && m_cartslot->exists())
			{
				address &= cart_sram_size - 1;
				cart_sram[address] = data;
				if (!machine().side_effects_disabled() && SWC_DEBUG)
					logerror("hi cart sram wr: %04x %02x\n", address, data);
			}
		}
		else if (address < 0xa000)  // dram  8000-9fff
		{
			if (SWC_DEBUG)
				logerror("dram wr: %06x page %d %02x\n", offset, page_select, data);
			snes_swc_dram_w(offset, data);
		}
		else if (address < 0xc000)  // sram or cart  a000-bfff
		{
			if (SWC_DEBUG)
			{
				if (sram_or_cart_map)
					logerror("sram rd: a000-bfff:%04x -> %06x %02x\n", (address & 0x1fff), (page_select << 13) | (offset & 0xff9fff), data);
				else
					logerror("cart rd: a000-bfff:%04x -> %06x %02x\n", (address & 0x1fff), (page_select << 13) | (offset & 0xff9fff), data);
			}
			if (sram_or_cart_map)
				snes_swc_sram_w(address, data);
			else
				snes_swc_cart_w(address, data);
		}
		else if (address < 0xe000)  // i/o regs  c000-dfff
		{
			if (SWC_DEBUG)
				logerror("swc i/o regs wr: %06x %02x\n", offset, data);
			snes_swc_io_w(address, data);
		}
		else                         // page select, mode select, others  e000-ffff
		{
			if (SWC_DEBUG)
				logerror("swc misc regs wr: %06x %02x\n", offset, data);
			snes_swc_misc_w(address, data);
		}
	}
	else  // < 0x7e0000                 bank 40-7d
	{
		if (address < 0x2000)        // ?  0000-1fff
		{
			if (SWC_DEBUG)
				logerror("? wr: %06x %02x\n", offset, data);
		}
		else if (address < 0x4000)  // sram or cart  2000-3fff
		{
			if (SWC_DEBUG)
			{
				if (sram_or_cart_map)
					logerror("sram rd: 2000-3fff:%04x -> %06x %02x\n", (address & 0x1fff), (page_select << 13) | (offset & 0xff9fff), data);
				else
					logerror("cart rd: 2000-3fff:%04x -> %06x %02x\n", (address & 0x1fff), (page_select << 13) | (offset & 0xff9fff), data);
			}
			if (sram_or_cart_map)
				snes_swc_sram_w(address, data);
			else
				snes_swc_cart_w(address, data);
		}
		else if (address < 0x8000)  // ?  4000-7fff
		{
			if (SWC_DEBUG)
				logerror("? wr: %06x %02x\n", offset, data);
		}
		else if (address < 0xa000)  // dram  8000-9fff
		{
			if (SWC_DEBUG)
				logerror("dram wr: %06x page %d %02x\n", offset, page_select, data);
			snes_swc_dram_w(offset, data);
		}
		else if (address < 0xc000)  // sram or cart  a000-bfff
		{
			if (SWC_DEBUG)
			{
				if (sram_or_cart_map)
					logerror("sram rd: a000-bfff:%04x -> %06x %02x\n", (address & 0x1fff), (page_select << 13) | (offset & 0xff9fff), data);
				else
					logerror("cart rd: a000-bfff:%04x -> %06x %02x\n", (address & 0x1fff), (page_select << 13) | (offset & 0xff9fff), data);
			}
			if (sram_or_cart_map)
				snes_swc_sram_w(address, data);
			else
				snes_swc_cart_w(address, data);
		}
		else if (address < 0xe000)  // i/o regs  c000-dfff
		{
			if (SWC_DEBUG)
				logerror("swc i/o regs wr: %06x %02x\n", offset, data);
			snes_swc_io_w(address, data);
		}
		else                        // page select, mode select, others  e000-ffff
		{
			if (SWC_DEBUG)
				logerror("swc misc regs wr: %06x %02x\n", offset, data);
			snes_swc_misc_w(address, data);
		}
	}
}


// 00-7d,80-ff : c00x rd  fdc & parallel i/o
inline uint8_t snes_swc_state::snes_swc_io_r(uint16_t address)
{
	uint8_t data = 0;

	switch (address & 0xf)
	{
		case 0:  // fdc  input  5: busy flag (FC9203 ver) 6: index, 7: irq
			data = m_fdc->input_r();
			if (!machine().side_effects_disabled() && SWC_DEBUG)
				logerror(" fdc input reg rd: %04x %02x\n", address, data);
			data |= par_debug_str;
			break;

		case 4:  // fdc  main status
			data = m_fdc->msr_r();
			if (!machine().side_effects_disabled() && SWC_DEBUG)
				logerror(" fdc main status reg rd: %04x %02x\n", address, data);
			break;

		case 5:  // fdc  data
			data = m_fdc->fifo_r();
			if (!machine().side_effects_disabled() && SWC_DEBUG)
				logerror(" fdc data reg rd: %04x %02x\n", address, data);
			break;

		case 7:  // fdc  digital input
			data = m_fdc->dir_r();
			if (!machine().side_effects_disabled() && SWC_DEBUG)
				logerror(" fdc digital input reg rd: %04x %02x\n", address, data);
			break;

		case 8:  // parallel in
			busy = 0;
			// read parallel port
			if (!machine().side_effects_disabled() && SWC_DEBUG)
				logerror(" parallel data rd: %04x\n", address);
			data = par_debug_data;
			break;

		case 9:  // busy flag (EP1810 ver)
			if (!machine().side_effects_disabled() && SWC_DEBUG)
				logerror(" [EP1810 ver] busy flag rd: %04x\n", address);
			break;

		default:
			if (!machine().side_effects_disabled() && SWC_DEBUG)
				logerror(" unknown c00x rd: %04x\n", address);
			break;
	}

	return data;
}

// 00-7d,80-ff : c00x wr  fdc & parallel i/o
inline void snes_swc_state::snes_swc_io_w(uint16_t address, uint8_t data)
{
	switch (address & 0xf)
	{
		case 2:  // fdc  digital output
			m_fdc->dor_w(data);
			if (SWC_DEBUG)
				logerror(" fdc digital output reg wr: %04x %02x\n", address, data);
			m_fdc->set_floppy((data & 0x20) ? m_fdd->get_device() : nullptr);
			m_fdd->get_device()->mon_w((data & 0x20) ? 0 : 1);
			break;

		case 5:  // fdc  data
			m_fdc->fifo_w(data);
			if (SWC_DEBUG)
				logerror(" fdc data reg wr: %04x %02x\n", address, data);
			break;

		case 7:  // fdc  disk control
			m_fdc->ccr_w(data);
			if (SWC_DEBUG)
				logerror(" fdc disk control reg wr: %04x %02x\n", address, data);
			break;

		case 8:  // parallel out & dram/sram mapping

			dram_map = data & 1;
			sram_map = data & 2;

			if (SWC_DEBUG)
			{
				logerror(" parallel data / map mode wr: %04x %02x\n", address, data);

				if (dram_map)
					logerror("  dram mapping set to mode 21 (hi)\n");
				else
					logerror("  dram mapping set to mode 20 (lo)\n");

				if (sram_map)
					logerror("  sram mapping set to mode 2 (hi)\n");
				else
					logerror("  sram mapping set to mode 1 (lo)\n");
			}

			// write parallel port
			break;

		default:
			if (SWC_DEBUG)
				logerror(" unknown c00x i/o wr: %04x %02x\n", address, data);
			break;
	}
	// TODO: mirrors
}

// 00-7d,80-ff : e00x wr  misc  page select, mode select, others
inline void snes_swc_state::snes_swc_misc_w(uint16_t address, uint8_t data)
{
	switch (address & 0xf)  // data not used
	{
		case 0:  // page select
		case 1:
		case 2:
		case 3:
			page_select = address & 3;
			if (SWC_DEBUG)
				logerror(" page select wr: page = %d\n", page_select);
			break;

		case 4:  // system mode select
		case 5:
		case 6:
		case 7:
			mode_select = address & 3;
			if (SWC_DEBUG)
				logerror(" system mode select wr: mode = %d\n", mode_select);
			break;

		case 8:  // dram type
		case 9:
			dram_type = address & 1;
			if (SWC_DEBUG)
			{
				logerror(" dram type select wr:\n");
				if (dram_type)
					logerror("  441000 dram selected (8,16,24,32 MB)\n");
				else
					logerror("  44256 dram selected (2,4,6,8 MB)\n");
			}
			break;

		case 0xc:  // cart/sram mapped @ a000-bfff
		case 0xd:
			sram_or_cart_map = address & 1;
			if (SWC_DEBUG)
			{
				logerror(" cart/sram mapping select wr:\n");
				if (sram_or_cart_map)
					logerror("  enabled sram @ a000-bfff\n");
				else
					logerror("  enabled cart @ a000-bfff\n");
			}
			break;

		default:
			if (SWC_DEBUG)
				logerror(" unknown e00x misc wr: %04x %02x\n", address, data);
			break;
	}
}


inline void snes_swc_state::snes_swc_dram_w(offs_t offset, uint8_t data)
{
	// TODO: fix
	uint32_t address = (offset & 0x1fff) | (page_select << 13) | ((offset & 0xff0000) >> 1);
	m_dram[address] = data;
	if (SWC_DEBUG)
		logerror(" dram wr: %06x %02x\n", address, data);
}

inline uint8_t snes_swc_state::snes_swc_dram_r(offs_t offset)
{
	// TODO: fix
	uint32_t address = (offset & 0x1fff) | (page_select << 13) | ((offset & 0xff0000) >> 1);
	uint8_t data = m_dram[address];
	if (!machine().side_effects_disabled() && SWC_DEBUG)
		logerror(" dram rd: %06x %02x\n", address, data);
	return data;
}


inline void snes_swc_state::snes_swc_sram_w(uint16_t address, uint8_t data)
{
	address = (address & 0x1fff) | (page_select << 13);
	m_sram[address] = data;
	if (SWC_DEBUG)
		logerror(" swc sram wr: %04x %02x\n", address, data);
}

inline uint8_t snes_swc_state::snes_swc_sram_r(uint16_t address)
{
	address = (address & 0x1fff) | (page_select << 13);
	uint8_t data = m_sram[address];
	if (!machine().side_effects_disabled() && SWC_DEBUG)
		logerror(" swc sram rd: %04x %02x\n", address, data);
	return data;
}


inline void snes_swc_state::snes_swc_cart_w(uint16_t address, uint8_t data)
{
	if (m_cartslot && m_cartslot->exists())
	{
		if (cart_sram_size && !cart_type)  // && ...
		{
			address = (address & 0x1fff) | (page_select << 13);
			address &= cart_sram_size - 1;
			cart_sram[address] = data;
			if (SWC_DEBUG)
				logerror(" lo cart sram wr: %04x %02x\n", address, data);
		}
	}
	else
	{
		if (SWC_DEBUG)
			logerror(" cart wr: no cart!\n");
	}
}

inline uint8_t snes_swc_state::snes_swc_cart_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (m_cartslot && m_cartslot->exists())
	{
		uint32_t address = (page_select << 13) | (offset & 0xff9fff);

		if (cart_sram_size && !cart_type && (address >= 0x700000) && ((address & 0xffff) < 0x8000))
		{
			// tests lo sram at 706000
			// clashes with rom in a 32mbit+sram hi cart (70-7d,f0-ff:0000-7fff)
			// 2.8cc seems to use only the >8000 area for rom size check
			address &= cart_sram_size - 1;
			data = cart_sram[address];
			if (!machine().side_effects_disabled() && SWC_DEBUG)
				logerror(" lo cart sram rd: %04x %02x\n", address, data);
		}
		else
		{
			if (!cart_type)  // lo
				address = (page_select << 13) | (offset & 0x1fff) | ((offset & 0xff0000) >> 1);

			address = snes_swc_clamp_rom_size(address);
			data = m_cart[address];
			if (!machine().side_effects_disabled() && SWC_DEBUG)
				logerror(" cart rom rd: %06x %02x\n", address, data);
		}
	}
	else
	{
		if (!machine().side_effects_disabled() && SWC_DEBUG)
			logerror(" cart rd: no cart!\n");
	}

	return data;
}


// mode 1 - play cart

inline uint8_t snes_swc_state::snes_swc_mode_1_r(offs_t offset)
{
	uint8_t value = 0xff;
	uint16_t address = offset & 0xffff;

	if (offset < 0x400000)           // bank 00-3f
	{
		if (address < 0x2000)        // ram  0000-1fff
		{
			value = m_wram[address];
		}
		else if (address < 0x6000)  // io  2000-5fff
		{
			value = snes_r_io(address);
		}
		else if (address < 0x8000)  // io  6000-7fff
		{
			if (cart_type && cart_sram_size && (offset >= 0x200000))  // hi cart sram  20-3f,a0-bf:6000-7fff
			{
				value = cart_sram[address & (cart_sram_size - 1)];  // TODO: 32 & 128KB
				if (!machine().side_effects_disabled() && SWC_DEBUG)
					logerror("hi cart sram rd: %06x %02x\n", offset, value);
			}
		}
		else                         // 8000-ffff
		{
			value = snes_swc_mode_1_rom_access(offset);
		}
	}
	else  // < 0x7e0000                bank 40-7d
	{
		if (!cart_type && cart_sram_size && (offset >= 0x700000) && (address < 0x8000))  // lo cart sram  70-7d,f0-ff:0000-7fff
		{
			value = cart_sram[address & (cart_sram_size - 1)];
			if (!machine().side_effects_disabled() && SWC_DEBUG)
				logerror("lo cart sram rd: %06x %02x\n", offset, value);
		}
		else
			value = snes_swc_mode_1_rom_access(offset);
	}

	return value;
}

inline void snes_swc_state::snes_swc_mode_1_w(address_space &space, offs_t offset, uint8_t data)
{
	uint16_t address = offset & 0xffff;

	if (offset < 0x400000)           // bank 00-3f
	{
		if (address < 0x2000)        // ram  0000-1fff
		{
			m_wram[address] = data;
		}
		else if (address < 0x6000)  // io  2000-5fff
		{
			snes_w_io(space, address, data);
		}
		else                         // 6000-ffff
		{
			if (cart_type && cart_sram_size && (offset >= 0x200000) && (address < 0x8000))  // hi cart sram  20-3f,a0-bf:6000-7fff
			{
				logerror("hi cart sram wr: %06x %02x\n", offset, data);
				cart_sram[address & (cart_sram_size - 1)] = data;  // TODO: 32 & 128KB
			}
		}
	}
	else  // < 0x7e0000                 bank 40-7d
	{
		if (!cart_type && cart_sram_size && (offset >= 0x700000) && (address < 0x8000))  // lo cart sram  70-7d,f0-ff:0000-7fff
		{
			logerror("lo cart sram wr: %06x %02x\n", offset, data);
			cart_sram[address & (cart_sram_size - 1)] = data;
		}
	}
}

inline uint8_t snes_swc_state::snes_swc_mode_1_rom_access(offs_t offset)
{
	uint8_t data = 0xff;

	if (m_cartslot && m_cartslot->exists())
	{
		uint32_t address;

		if (!cart_type)  // lo
		{
			address = (offset & 0x7fff) | ((offset & 0xff0000) >> 1);
		}
		else  // hi
		{
			address = offset;
		}

		data = m_cart[snes_swc_clamp_rom_size(address)];
	}

	return data;
}


ROM_START( snes_swc )
	ROM_REGION( 0x4000, "bios", 0 )
	ROM_LOAD( "swc_28cc_280694.bin", 0x0000, 0x4000, CRC(6e14fce2) SHA1(05b69eb087531e488e8a7ece9437982b4e335e18) )
ROM_END

#define rom_snes_swc_pal rom_snes_swc

//   YEAR,   NAME,      PARENT, COMPAT, MACHINE,      INPUT,     CLASS,          INIT,          COMPANY,         FULLNAME,              FLAGS

CONS( 2022, snes_swc,     0,        0, snes_swc,     snes_swc, snes_swc_state, init_snes_swc, "Front Fareast", "Super Wild Card (ntsc)", 0 )
CONS( 2022, snes_swc_pal, snes_swc, 0, snes_swc_pal, snes_swc, snes_swc_state, init_snes_swc, "Front Fareast", "Super Wild Card (pal)",  0 )
