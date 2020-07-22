// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    SAMDAC Stereo DAC for SAM Coupe

***************************************************************************/

#include "emu.h"
#include "samdac.h"
#include "sound/volt_reg.h"
#include "speaker.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(CENTRONICS_SAMDAC, centronics_samdac_device, "centronics_samdac", "SAMDAC")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void centronics_samdac_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	DAC_8BIT_R2R(config, m_dac[0], 0).add_route(ALL_OUTPUTS, "lspeaker", 0.5);
	DAC_8BIT_R2R(config, m_dac[1], 0).add_route(ALL_OUTPUTS, "rspeaker", 0.5);

	voltage_regulator_device &vref(VOLTAGE_REGULATOR(config, "vref"));
	vref.add_route(0, m_dac[0], 1.0, DAC_VREF_POS_INPUT);
	vref.add_route(0, m_dac[0], -1.0, DAC_VREF_NEG_INPUT);
	vref.add_route(0, m_dac[1], 1.0, DAC_VREF_POS_INPUT);
	vref.add_route(0, m_dac[1], -1.0, DAC_VREF_NEG_INPUT);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  centronics_samdac_device - constructor
//-------------------------------------------------

centronics_samdac_device::centronics_samdac_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, CENTRONICS_SAMDAC, tag, owner, clock),
	device_centronics_peripheral_interface(mconfig, *this),
	m_dac(*this, "dac%u", 0U),
	m_strobe(0),
	m_data{ 0x00, 0x00 }
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void centronics_samdac_device::device_start()
{
	// register for savestates
	save_item(NAME(m_strobe));
	save_item(NAME(m_data), 2);
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

WRITE_LINE_MEMBER( centronics_samdac_device::input_strobe )
{
	// raising edge, write to left channel
	if (m_strobe == 0 && state == 1)
		m_dac[0]->data_w(m_data[0]);
	
	// failing edge, write to right channel
	if (m_strobe == 1 && state == 0)
		m_dac[1]->data_w(m_data[1]);

	m_strobe = state;
}

void centronics_samdac_device::update_data(int bit, int state)
{
	if (state)
	{
		m_data[0] |= (1 << bit);
		m_data[1] |= (1 << bit);
	}
	else
	{
		m_data[0] &= ~(1 << bit);
		m_data[1] &= ~(1 << bit);
	}
}
