// license:BSD-3-Clause
// copyright-holders:TwistedTom
/**********************************************************************

    Multiface 1 multipurpose interface
      Romantic Robot UK Ltd 1986

**********************************************************************/

#include "emu.h"
#include "mface1.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SPECTRUM_MFACE1, spectrum_mface1_device, "spectrum_mface1", "Multiface 1 Interface")


//-------------------------------------------------
//  ROM( mface1 )
//-------------------------------------------------

// rom infomation from:
//   https://x128.speccy.cz/multiface/multiface.htm

// Early versions had 2KB of RAM, composite video output, a basic toolkit (pokes only).
// Later versions dropped the composite video output, added an enable/disable switch to counter s/w detection in games.
// One very early version has no toolkit at all.
// One late (rare) version supports the Kempston Disc interface (available only on request from RR).

// MUxx,     -> pcb revision (silkscreen marking)
//       yy  -> rom checksum

// Press Symbol Shift + A (STOP) to see checksum, space to return

ROM_START( mface1 )
	ROM_REGION(0x2000, "rom", 0)
	ROM_DEFAULT_BIOS("v21")
	
	// early versions - composite video, no on/off switch, no toolkit
	ROM_SYSTEM_BIOS(0, "v11", "MU12, CB")  // Very early version, 2KB RAM  (page out: port 0x5F)
	ROMX_LOAD("mf1_12_cb.rom", 0x0000, 0x2000, CRC(c88fbf9f) SHA1(c3018d1b495b8bc0a135038db0987de7091c9d4c), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v12", "MU 2.0?, 23")  // 8KB RAM, pokes  (page out: port 0x5F)
	ROMX_LOAD("mf1_20_23.rom", 0x0000, 0x2000, CRC(d4ae8953) SHA1(b442eb634a72fb63f1ccbbd0021a7a581152888d), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "v13", "MU 2.0, FE")  // 8KB RAM, pokes
	ROMX_LOAD("mf1_20_fe.rom", 0x0000, 0x2000, CRC(fa1b8b0d) SHA1(20cd508b0143166558a7238c7a9ccfbe37b90b0d), ROM_BIOS(2))
	//ROM_SYSTEM_BIOS(?, "v14", "MU 2.0, 90")  // pokes only or toolkit?
	//ROMX_LOAD("mf1_20_90.rom", 0x0000, 0x2000, CRC(2eaf8e41) SHA1(?), ROM_BIOS(?))

	// later versions - 8KB RAM, no composite video, on/off switch
	ROM_SYSTEM_BIOS(3, "v21", "MU 2.1, E4")  // the most common version
	ROMX_LOAD("mf1_21_e4.rom", 0x0000, 0x2000, CRC(4b31a971) SHA1(ba28754a3cc31a4ca579829ed4310c313409cf5d), ROM_BIOS(3))
	ROM_SYSTEM_BIOS(4, "v22", "MU 2.1, E7")
	ROMX_LOAD("mf1_21_e7.rom", 0x0000, 0x2000, CRC(670f0ec2) SHA1(50fba2d628f3a2e9219f72980e4efd62fc9ec1f8), ROM_BIOS(4))
	ROM_SYSTEM_BIOS(5, "v23", "MU 2.1, 67")  // Kempston Disc support, no Betadisk 48 support
	ROMX_LOAD("mf1_21_67.rom", 0x0000, 0x2000, CRC(d720ec1b) SHA1(91a40d8f503ef825df3e2ed712897dbf4ca3671d), ROM_BIOS(5))

	//ROM_SYSTEM_BIOS(?, "v24", "?, 93 (Brazilian clone)")
	//ROMX_LOAD("mf1_bc_fe.rom", 0x0000, 0x2000, CRC(8c17113b) SHA1(?), ROM_BIOS(?))
ROM_END


//-------------------------------------------------
//  INPUT_PORTS( mface1 )
//-------------------------------------------------

static INPUT_PORTS_START( mface1 )
	PORT_START("joystick")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT) PORT_8WAY
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT)  PORT_8WAY
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN)  PORT_8WAY
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP)    PORT_8WAY
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON1)
	PORT_START("red_btn")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Red Button") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHANGED_MEMBER(DEVICE_SELF, spectrum_mface1_device, red_btn_press, 0)
INPUT_PORTS_END


INPUT_CHANGED_MEMBER(spectrum_mface1_device::red_btn_press)
{
	if (rb_en)
	{
		mf1_active = 1;
		rb_en = 0;
		m_slot->nmi_w(ASSERT_LINE);
	}
}


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor spectrum_mface1_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( mface1 );
}


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *spectrum_mface1_device::device_rom_region() const
{
	return ROM_NAME( mface1 );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  spectrum_mface1_device - constructor
//-------------------------------------------------

spectrum_mface1_device::spectrum_mface1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SPECTRUM_MFACE1, tag, owner, clock)
	, device_spectrum_expansion_interface(mconfig, *this)
	, m_rom(*this, "rom")
	, m_joy(*this, "joystick")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void spectrum_mface1_device::device_start()
{
	if (m_system_bios == 1)
		m_ram = std::make_unique<uint8_t[]>(2048);  // very early model has only 2KB ram
	else
		m_ram = std::make_unique<uint8_t[]>(8192);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void spectrum_mface1_device::device_reset()
{
	io_space().install_read_handler(0x9f, 0x9f, 0, 0xff00, 0, read8_delegate(FUNC(spectrum_mface1_device::page_in_r), this));
	
	if (m_system_bios == 1 || m_system_bios == 2)
	{
		// early model
		io_space().install_read_handler(0x5f, 0x5f, 0, 0xff00, 0, read8_delegate(FUNC(spectrum_mface1_device::page_out_r), this));
		io_space().install_write_handler(0x5f, 0x5f, 0, 0xff00, 0, write8_delegate(FUNC(spectrum_mface1_device::reset_nmi_w), this));
		io_space().install_read_handler(0x1f, 0x1f, 0, 0xff00, 0, read8_delegate(FUNC(spectrum_mface1_device::joystick_r), this));
	}
	else
	{
		// later model
		io_space().install_read_handler(0x1f, 0x1f, 0, 0xff00, 0, read8_delegate(FUNC(spectrum_mface1_device::page_out_r), this));
		io_space().install_write_handler(0x1f, 0x1f, 0, 0xff00, 0, write8_delegate(FUNC(spectrum_mface1_device::reset_nmi_w), this));
	}
	
	mf1_active = 0;
	rb_en = 1;
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

/*
		io map
		------
		
		early model         bios 1,2
		----------------------------
		pg in    rd 0x9f
		pg out   rd 0x5f
		nmi rst  wr 0x5f
		js       rd 0x1f
		
		later model         bios >= 3
		-----------------------------
		pg in    rd 0x9f
		pg out   rd 0x1f
		nmi rst  wr 0x1f
		js       rd 0x1f
		
		rom maps to 0x0000
		ram maps to 0x2000
*/


READ8_MEMBER(spectrum_mface1_device::page_in_r)
{
	mf1_active = 1;
	return 0;          // should be floating bus output?
}

READ8_MEMBER(spectrum_mface1_device::page_out_r)
{
	mf1_active = 0;
	
	uint8_t data = 0;
	
	if (m_system_bios > 2)
		data = m_joy->read() & 0x1f;  // joystick read for later models
	
	return data;
}

WRITE8_MEMBER(spectrum_mface1_device::reset_nmi_w)
{
	m_slot->nmi_w(CLEAR_LINE);
	rb_en = 1;
}

// early model only
READ8_MEMBER(spectrum_mface1_device::joystick_r)
{
	return m_joy->read() & 0x1f;
}


READ8_MEMBER(spectrum_mface1_device::mreq_r)
{
	uint8_t data = 0xff;
	
	if (mf1_active)
	{
		if (offset >= 0x0000 && offset < 0x2000)
			data &= m_rom->base()[offset];
		else if (offset >= 0x2000 && offset < 0x4000)
			data &= m_ram[offset-0x2000];
	}

	return data;
}

WRITE8_MEMBER(spectrum_mface1_device::mreq_w)
{
	if (mf1_active)
		if (offset >= 0x2000 && offset < 0x4000)
			m_ram[offset-0x2000] = data;
}

READ_LINE_MEMBER(spectrum_mface1_device::romcs)
{
	return mf1_active;
}
