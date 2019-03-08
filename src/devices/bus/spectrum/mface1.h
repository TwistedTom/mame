// license:BSD-3-Clause
// copyright-holders:TwistedTom
/**********************************************************************

    Multiface 1 multipurpose interface
      Romantic Robot UK Ltd 1986

**********************************************************************/

#ifndef MAME_BUS_SPECTRUM_MFACE1_H
#define MAME_BUS_SPECTRUM_MFACE1_H

#pragma once


#include "exp.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> spectrum_mface1_device

class spectrum_mface1_device :
	public device_t,
	public device_spectrum_expansion_interface
{
public:
	// construction/destruction
	spectrum_mface1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_INPUT_CHANGED_MEMBER(red_btn_press);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;

	virtual DECLARE_READ8_MEMBER(mreq_r) override;
	virtual DECLARE_WRITE8_MEMBER(mreq_w) override;
	virtual DECLARE_READ_LINE_MEMBER(romcs) override;

private:
	DECLARE_READ8_MEMBER(page_in_r);
	DECLARE_READ8_MEMBER(page_out_r);
	DECLARE_WRITE8_MEMBER(reset_nmi_w);
	DECLARE_READ8_MEMBER(joystick_r);
	
	required_memory_region m_rom;      // 8KB ROM
	std::unique_ptr<uint8_t[]> m_ram;  // 8KB RAM
	required_ioport m_joy;

	int mf1_active;
	int rb_en;
};


// device type definition
DECLARE_DEVICE_TYPE(SPECTRUM_MFACE1, spectrum_mface1_device)


#endif // MAME_BUS_SPECTRUM_MFACE1_H
