//
//

#include "emu.h"
#include "megadriv.h"

class md_fcart_state : public md_base_state
{
public:
	md_fcart_state(const machine_config &mconfig, device_type type, const char *tag)
		: md_base_state(mconfig, type, tag)
		, m_bank(0)
		, m_lock(0)
		, m_rom(*this, "maincpu")
	{ }

	void md_fcart(machine_config &config);
	void md_fcartu(machine_config &config);

	uint16_t read(offs_t offset);
	void write_a13(offs_t offset, uint16_t data);

	void md_fcart_map(address_map &map);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	int m_bank;
	int m_lock;
	required_region_ptr<uint16_t> m_rom;
};

class md_fcart_ssf2_state : public md_fcart_state
{
public:
	md_fcart_ssf2_state(const machine_config &mconfig, device_type type, const char *tag)
		: md_fcart_state(mconfig, type, tag)
		, m_b6(6)
		, m_b7(7)
		, m_ssf(0)
	{ }

	void md_fcart_ssf2(machine_config &config);
	void md_fcart_ssf2u(machine_config &config);

	uint16_t read(offs_t offset);
	void write_a13(offs_t offset, uint16_t data);

	void md_fcart_ssf2_map(address_map &map);

protected:
	virtual void machine_start() override;
    virtual void machine_reset() override;
	int m_b6;
	int m_b7;
	int m_ssf;
};

void md_fcart_state::md_fcart_map(address_map &map)
{
	megadriv_68k_base_map(map);
	
	map(0x000000, 0x3fffff).r(FUNC(md_fcart_state::read)); // Cartridge Program ROM
	map(0xa13000, 0xa130ff).w(FUNC(md_fcart_state::write_a13));
}

void md_fcart_ssf2_state::md_fcart_ssf2_map(address_map &map)
{
	megadriv_68k_base_map(map);
	
	map(0x000000, 0x3fffff).r(FUNC(md_fcart_ssf2_state::read)); // Cartridge Program ROM
	map(0xa13000, 0xa130ff).w(FUNC(md_fcart_ssf2_state::write_a13));
}

uint16_t md_fcart_state::read(offs_t offset)
{
	if (offset < 0x400000/2)
			return m_rom[offset + m_bank];
		else
			return 0xffff;
}

void md_fcart_state::write_a13(offs_t offset, uint16_t data)
{
	//printf("data: %04x\n", data);

	if (m_lock)
		return;

	int adr = (data & 0xff) << 17;
	printf("rom adr: %07x\n", adr);
	m_bank = adr >> 1;
	m_lock = 1;
}

uint16_t md_fcart_ssf2_state::read(offs_t offset)
{
	if (m_ssf)
	{
		if (offset < 0x300000/2)
		{
			return m_rom[offset + m_bank];
		}
		else if (offset >= 0x300000/2 && offset < 0x380000/2)
		{
			return m_rom[(offset & (0x7ffff/2)) + ((0x80000/2) * m_b6) + m_bank];
		}
		else if (offset >= 0x380000/2 && offset < 0x400000/2)
		{
			return m_rom[(offset & (0x7ffff/2)) + ((0x80000/2) * m_b7) + m_bank];
		}
		else
		{
			return 0xffff;
		}
	}
	else
	{
		if (offset < 0x400000/2)
			return m_rom[offset + m_bank];
		else
			return 0xffff;
	}
}

void md_fcart_ssf2_state::write_a13(offs_t offset, uint16_t data)
{
	if (!m_ssf)
	{
		if (m_lock)
			return;

		if (offset == 0x8/2)
		{
			printf("ssf2 mode\n");
			m_ssf = 1;
		}

		md_fcart_state::write_a13(offset, data);
	}
	else
	{
		if (offset >= 0xfc/2)  // ignore f1-fb writes
		{
			offset -= 0xf0/2;

			//printf("bank: %d = %d\n", offset, data & 0xf);

			m_b6 = offset == 6 ? data & 0xf : m_b6;
			m_b7 = offset == 7 ? data & 0xf : m_b7;
		}
	}
}

void md_fcart_state::machine_start()
{
	md_base_state::machine_start();
	m_vdp->stop_timers();
	save_item(NAME(m_bank));
	save_item(NAME(m_lock));
}

void md_fcart_state::machine_reset()
{
	m_bank = 0;
	m_lock = 0;
	md_base_state::machine_reset();
}

void md_fcart_ssf2_state::machine_start()
{
	md_fcart_state::machine_start();
	save_item(NAME(m_b6));
	save_item(NAME(m_b7));
	save_item(NAME(m_ssf));
}

void md_fcart_ssf2_state::machine_reset()
{
	m_b6 = 6;
	m_b7 = 7;
	m_ssf = 0;
	md_fcart_state::machine_reset();
}

void md_fcart_state::md_fcart(machine_config &config)
{
	md_pal(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &md_fcart_state::md_fcart_map);
}

void md_fcart_state::md_fcartu(machine_config &config)
{
	md_ntsc(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &md_fcart_state::md_fcart_map);
}

void md_fcart_ssf2_state::md_fcart_ssf2(machine_config &config)
{
	md_pal(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &md_fcart_ssf2_state::md_fcart_ssf2_map);
}

void md_fcart_ssf2_state::md_fcart_ssf2u(machine_config &config)
{
	md_ntsc(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &md_fcart_ssf2_state::md_fcart_ssf2_map);
}

ROM_START( md_fcart )
	ROM_REGION( 0x2000000, "maincpu", 0 )
	ROM_LOAD( "rom_1.bin", 0x0000000, 0x800000, CRC(c5c26cc6) SHA1(281159a42f796defa9808091a56710625b71e014) )
	ROM_LOAD( "rom_2.bin", 0x0800000, 0x800000, CRC(3a506fcb) SHA1(81cee2fc0ed7e6ccd850a84d4cfaa3b7f4a3cb92) )
	ROM_LOAD( "rom_3.bin", 0x1000000, 0x800000, CRC(cc819d83) SHA1(1dade629b588c36df4ffd25040b295f63decc05a) )
	ROM_LOAD( "rom_4.bin", 0x1800000, 0x800000, CRC(a7558e96) SHA1(0186366879658940c19adb78144718f01957d443) )
ROM_END

ROM_START( md_fcartu )
	ROM_REGION( 0x2000000, "maincpu", 0 )
	ROM_LOAD( "rom_1.bin", 0x0000000, 0x800000, CRC(c5c26cc6) SHA1(281159a42f796defa9808091a56710625b71e014) )
	ROM_LOAD( "rom_2.bin", 0x0800000, 0x800000, CRC(3a506fcb) SHA1(81cee2fc0ed7e6ccd850a84d4cfaa3b7f4a3cb92) )
	ROM_LOAD( "rom_3.bin", 0x1000000, 0x800000, CRC(cc819d83) SHA1(1dade629b588c36df4ffd25040b295f63decc05a) )
	ROM_LOAD( "rom_4.bin", 0x1800000, 0x800000, CRC(a7558e96) SHA1(0186366879658940c19adb78144718f01957d443) )
ROM_END

ROM_START( md_fcartj )
	ROM_REGION( 0x2000000, "maincpu", 0 )
	ROM_LOAD( "rom_1.bin", 0x0000000, 0x800000, CRC(c5c26cc6) SHA1(281159a42f796defa9808091a56710625b71e014) )
	ROM_LOAD( "rom_2.bin", 0x0800000, 0x800000, CRC(3a506fcb) SHA1(81cee2fc0ed7e6ccd850a84d4cfaa3b7f4a3cb92) )
	ROM_LOAD( "rom_3.bin", 0x1000000, 0x800000, CRC(cc819d83) SHA1(1dade629b588c36df4ffd25040b295f63decc05a) )
	ROM_LOAD( "rom_4.bin", 0x1800000, 0x800000, CRC(a7558e96) SHA1(0186366879658940c19adb78144718f01957d443) )
ROM_END

ROM_START( md_fcart_ssf2 )
	ROM_REGION( 0x2000000, "maincpu", 0 )
	ROM_LOAD( "rom_1.bin", 0x0000000, 0x800000, CRC(f9667ad6) SHA1(193fb026e0d379e5f1a2fc0f16e0fe97c544b5bd) )
	ROM_LOAD( "rom_2.bin", 0x0800000, 0x800000, CRC(c91882dd) SHA1(aad9cf75983cb8187d3d3e67a2798a679b0d742e) )
	ROM_LOAD( "rom_3.bin", 0x1000000, 0x800000, CRC(038ccfae) SHA1(9b81578f192b1cb26839c1503122b0fcfc99366f) )
ROM_END

ROM_START( md_fcart_ssf2u )
	ROM_REGION( 0x2000000, "maincpu", 0 )
	ROM_LOAD( "rom_1.bin", 0x0000000, 0x800000, CRC(f9667ad6) SHA1(193fb026e0d379e5f1a2fc0f16e0fe97c544b5bd) )
	ROM_LOAD( "rom_2.bin", 0x0800000, 0x800000, CRC(c91882dd) SHA1(aad9cf75983cb8187d3d3e67a2798a679b0d742e) )
	ROM_LOAD( "rom_3.bin", 0x1000000, 0x800000, CRC(038ccfae) SHA1(9b81578f192b1cb26839c1503122b0fcfc99366f) )
ROM_END

ROM_START( md_fcart_ssf2j )
	ROM_REGION( 0x2000000, "maincpu", 0 )
	ROM_LOAD( "rom_1.bin", 0x0000000, 0x800000, CRC(f9667ad6) SHA1(193fb026e0d379e5f1a2fc0f16e0fe97c544b5bd) )
	ROM_LOAD( "rom_2.bin", 0x0800000, 0x800000, CRC(c91882dd) SHA1(aad9cf75983cb8187d3d3e67a2798a679b0d742e) )
	ROM_LOAD( "rom_3.bin", 0x1000000, 0x800000, CRC(038ccfae) SHA1(9b81578f192b1cb26839c1503122b0fcfc99366f) )
ROM_END

// YEAR,NAME,PARENT,COMPAT,MACHINE,INPUT,CLASS,INIT,COMPANY,FULLNAME,FLAGS

CONS( 2022, md_fcart,         0, 0, md_fcart,  md_common, md_fcart_state, init_megadrie, "bootleg", "MD Flash Cart (pal)",    0 )
CONS( 2022, md_fcartu, md_fcart, 0, md_fcartu, md_common, md_fcart_state, init_megadriv, "bootleg", "MD Flash Cart (ntsc u)", 0 )
CONS( 2022, md_fcartj, md_fcart, 0, md_fcartu, md_common, md_fcart_state, init_megadrij, "bootleg", "MD Flash Cart (ntsc j)", 0 )

CONS( 2022, md_fcart_ssf2,              0, 0, md_fcart_ssf2,  md_common, md_fcart_ssf2_state, init_megadrie, "bootleg", "MD Flash Cart ssf2 (pal)",    0 )
CONS( 2022, md_fcart_ssf2u, md_fcart_ssf2, 0, md_fcart_ssf2u, md_common, md_fcart_ssf2_state, init_megadriv, "bootleg", "MD Flash Cart ssf2 (ntsc u)", 0 )
CONS( 2022, md_fcart_ssf2j, md_fcart_ssf2, 0, md_fcart_ssf2u, md_common, md_fcart_ssf2_state, init_megadrij, "bootleg", "MD Flash Cart ssf2 (ntsc j)", 0 )
