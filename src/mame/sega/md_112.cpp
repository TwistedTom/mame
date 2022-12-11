// 
// 

#include "emu.h"
#include "megadriv.h"

class md_112_state : public md_base_state
{
public:
	md_112_state(const machine_config &mconfig, device_type type, const char *tag)
		: md_base_state(mconfig, type, tag)
		, m_bank(0)
		, m_rom(*this, "maincpu")
	{ }
	
	void md_112(machine_config &config);

	uint16_t read(offs_t offset);
	void write_a13(offs_t offset, uint16_t data);

	void md_112_map(address_map &map);

protected:
    virtual void machine_start() override;
	virtual void machine_reset() override;
	int m_bank;

private:
	required_region_ptr<uint16_t> m_rom;
};

void md_112_state::md_112_map(address_map &map)
{
	megadriv_68k_base_map(map);
	
	map(0x000000, 0x3fffff).r(FUNC(md_112_state::read)); // Cartridge Program ROM
	map(0xa13000, 0xa130ff).w(FUNC(md_112_state::write_a13));
}

uint16_t md_112_state::read(offs_t offset)
{
	return m_rom[offset + m_bank];
}

void md_112_state::write_a13(offs_t offset, uint16_t data)
{
	int ofs = offset << 17;
	int d = (data & 0x380) << 17;
	int adr = ofs | d;
	printf("rom adr: %07x\n", adr);
	m_bank = adr >> 1;
}

void md_112_state::machine_start()
{
	md_base_state::machine_start();
	m_vdp->stop_timers();
	save_item(NAME(m_bank));
}

void md_112_state::machine_reset()
{
	m_bank = 0;
	md_base_state::machine_reset();
}

void md_112_state::md_112(machine_config &config)
{
	md_ntsc(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &md_112_state::md_112_map);
}

ROM_START( md_112 )
	ROM_REGION( 0x8000000, "maincpu", 0 )
	ROM_LOAD( "dump3.bin", 0x000000, 0x8000000, CRC(03dd5568) SHA1(55be186c98a6f7570373618dfdc192542542a200) )
ROM_END

// YEAR,NAME,PARENT,COMPAT,MACHINE,INPUT,CLASS,INIT,COMPANY,FULLNAME,FLAGS

CONS( 2018, md_112, 0, 0, md_112, md_common, md_112_state, init_megadriv, "bootleg", "Retroad 112 in 1", 0 )
