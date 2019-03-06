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

ROM_START( mface1 )
	ROM_REGION(0x2000, "rom", 0)
	ROM_LOAD("mf1_21_e4.rom", 0x0000, 0x2000, CRC(4b31a971) SHA1(ba28754a3cc31a4ca579829ed4310c313409cf5d))
ROM_END


//-------------------------------------------------
//  INPUT_PORTS( mface1 )
//-------------------------------------------------

static INPUT_PORTS_START( mface1 )
	PORT_START("JOY")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT) PORT_8WAY
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT)  PORT_8WAY
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN)  PORT_8WAY
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP)    PORT_8WAY
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON1)
  
  PORT_START("RED_BTN")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Red Button") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHANGED_MEMBER(DEVICE_SELF, spectrum_mface1_device, btn_press, 0)
INPUT_PORTS_END


// typedef u32 ioport_value;
// #define INPUT_CHANGED_MEMBER(name)
// void name(ioport_field &field, void *param, ioport_value oldval, ioport_value newval)

INPUT_CHANGED_MEMBER(spectrum_mface1_device::btn_press)
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
	, m_joy(*this, "JOY")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void spectrum_mface1_device::device_start()
{
  m_ram = std::make_unique<uint8_t[]>(8192);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void spectrum_mface1_device::device_reset()
{
  //void install_read_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read8_delegate rhandler
  
	io_space().install_read_handler(0x1f, 0x1f, 0, 0xff00, 0, read8_delegate(FUNC(spectrum_mface1_device::joystick_r), this));
  io_space().install_read_handler(0x9f, 0x9f, 0, 0xff00, 0, read8_delegate(FUNC(spectrum_mface1_device::page_in_r), this));
  io_space().install_write_handler(0x1f, 0x1f, 0, 0xff00, 0, write8_delegate(FUNC(spectrum_mface1_device::reset_ff), this));
  mf1_active = 0;
  rb_en = 1;
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

READ8_MEMBER(spectrum_mface1_device::joystick_r)
{
  mf1_active = 0;
  uint8_t data = space.read_byte(offset);
  logerror("io rd 0x1f: %02x  [mf1 mem paged out]\n", data);
	return m_joy->read() & 0x1f;
}

READ8_MEMBER(spectrum_mface1_device::page_in_r)
{
  // mf rom/ram page in @ 0x0000/0x20000
  mf1_active = 1;
  uint8_t data = space.read_byte(offset);
  logerror("io rd 0x9f: %02x  [mf1 mem paged in]\n", data);
  return data;
}

WRITE8_MEMBER(spectrum_mface1_device::reset_ff)
{
  logerror("io wr 0x1f: %02x  [mf1 nmi reset]\n", data);
  m_slot->nmi_w(CLEAR_LINE);
  rb_en = 1;
}

READ8_MEMBER(spectrum_mface1_device::mreq_r)
{
	uint8_t data = 0xff;
  
  if (mf1_active)
  {
    if (offset >= 0x0000 && offset < 0x2000)
      data &= m_rom->base()[offset];
    else if (offset >= 0x2000 && offset < 0x4000)
    {
      data &= m_ram[offset-0x2000];
      //logerror("mf ram r: %#04x: %02x\n", offset-0x2000, data);
    }
  }
	
  return data;
}

WRITE8_MEMBER(spectrum_mface1_device::mreq_w)
{
	if (mf1_active)
    if (offset >= 0x2000 && offset < 0x4000)
    {
      //logerror("mf ram w: %#04x: %02x\n", offset-0x2000, data);
      m_ram[offset-0x2000] = data;
    }
}

READ_LINE_MEMBER(spectrum_mface1_device::romcs)
{
	return mf1_active;
}