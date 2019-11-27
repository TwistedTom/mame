// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Speedcom 56K V.90 external modem using Conexant
    (formerly Rockwell) chipset.

****************************************************************************/

#include "emu.h"
#include "cpu/m6502/r65c19.h"

class vd56sp_state : public driver_device
{
public:
	vd56sp_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void vd56sp(machine_config &mconfig);

private:
	void mem_map(address_map &map);

	required_device<r65c19_device> m_maincpu;
};


void vd56sp_state::mem_map(address_map &map)
{
	map(0x0040, 0x03ff).ram();
	map(0x0800, 0x1fff).rom().region("firmware", 0x20800);
	map(0x8000, 0xffff).rom().region("firmware", 0x28000);
}


static INPUT_PORTS_START(vd56sp)
INPUT_PORTS_END

void vd56sp_state::vd56sp(machine_config &config)
{
	R65C19(config, m_maincpu, 8'000'000); // FIXME: actually L2800-38 (XTAL not readable)
	m_maincpu->set_addrmap(AS_PROGRAM, &vd56sp_state::mem_map);

	// Modem IC: Conexant R6764-61
}


ROM_START(vd56sp)
	ROM_REGION(0x40000, "firmware", 0)
	ROM_LOAD("vd56sp_v2.2_8904005.u10", 0x00000, 0x40000, CRC(23ddae13) SHA1(7a194f681389c2923ea6848b3a25f26c532a3200))
ROM_END


SYST(199?, vd56sp, 0, 0, vd56sp, vd56sp, vd56sp_state, empty_init, "Pro-Nets Technology", "Speedcom VD56SP", MACHINE_IS_SKELETON)
