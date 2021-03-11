#include "emu.h"
#include "slot.h"
#include "rom.h"
#include "cpu/mcs51/mcs51.h"

class neogeo_ngmc161_cart_device : public neogeo_rom_device
{
public:
	neogeo_ngmc161_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint16_t clock);

	virtual uint16_t ram_r(offs_t offset) override;
	virtual void ram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;

	virtual int get_fixed_bank_type() override { return 0; }

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	
private:
	required_device<i87c52_device> m_mcu;
	void mcu_map(address_map &map);
	void mcu_w(offs_t offset, u8 data);
	u8 mcu_r(offs_t offset);
	void mcu_p3_w(u8 data);
	uint16_t m_mcu_latch;
	uint16_t m_cpld_latch;
};

DEFINE_DEVICE_TYPE(NEOGEO_NGMC161_CART, neogeo_ngmc161_cart_device, "neocart_ngmc161", "Neo Geo 161-in-1 Multi-Cart")


neogeo_ngmc161_cart_device::neogeo_ngmc161_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint16_t clock) :
	neogeo_rom_device(mconfig, NEOGEO_NGMC161_CART, tag, owner, clock),
	m_mcu(*this, "mcu")
{}

void neogeo_ngmc161_cart_device::device_start()
{
	
}

void neogeo_ngmc161_cart_device::device_reset()
{
	m_mcu_latch = 0xffff;
	m_cpld_latch = 0xffff;
}

void neogeo_ngmc161_cart_device::device_add_mconfig(machine_config &config)
{
	I87C52(config, m_mcu, XTAL(11'059'200));
	m_mcu->set_addrmap(AS_IO, &neogeo_ngmc161_cart_device::mcu_map);
	m_mcu->port_out_cb<3>().set(FUNC(neogeo_ngmc161_cart_device::mcu_p3_w));
}

void neogeo_ngmc161_cart_device::mcu_map(address_map &map)
{
	map(0x0000, 0x0001).rw(FUNC(neogeo_ngmc161_cart_device::mcu_r), FUNC(neogeo_ngmc161_cart_device::mcu_w));
}

void neogeo_ngmc161_cart_device::mcu_w(offs_t offset, u8 data)
{
	logerror("mcu wr:  %02x %02x\n", offset, data);
	
	if (offset == 0)
		m_cpld_latch = (m_cpld_latch & 0xff00) | data;
	else
		m_cpld_latch = (m_cpld_latch & 0xff) | ((uint16_t)data << 8);
}

u8 neogeo_ngmc161_cart_device::mcu_r(offs_t offset)
{
	u8 data;
	
	if (offset == 0)
		data = (u8)m_mcu_latch;
	else
		data = (u8)(m_mcu_latch >> 8);
	
	logerror("mcu rd:  %02x %02x\n", offset, data);
	
	return data;
}

void neogeo_ngmc161_cart_device::ram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	switch (offset * 2)
	{
	case 0x00:  // 2fff00 rom switch
		logerror("2fff00 wr:  %04x\n", data);
		break;
	case 0xe0:  // 2fffe0 mcu
		logerror("2fffe0 wr:  %04x\n", data);
		m_mcu->set_input_line(MCS51_INT0_LINE, HOLD_LINE);
		m_mcu_latch = data;
		break;
	case 0xf0:  // 2ffff0 ?
		logerror("2ffff0 wr:  %04x\n", data);
		break;
	case 0xf2:  // 2ffff2 ?
		logerror("2ffff2 wr:  %04x\n", data);
		break;
	case 0xf4:  // 2ffff4 ?
		logerror("2ffff4 wr:  %04x\n", data);
		break;
	case 0xf6:  // 2ffff6 unlock
		logerror("2ffff6 wr:  %04x\n", data);
		break;
	default:
		logerror("unknown wr:  2fff%02x %04x\n", offset * 2, data);
	}
}

uint16_t neogeo_ngmc161_cart_device::ram_r(offs_t offset)
{
	logerror("2fffe0 rd:  %04x\n", m_cpld_latch);
	
	return m_cpld_latch;
}

void neogeo_ngmc161_cart_device::mcu_p3_w(u8 data)
{
	logerror("mcu p3 wr:  %02x\n", data);
	
	// P3.3 is cpld reset active low
	if (!(data & 0x08))
	{
		m_mcu_latch = 0;
		m_cpld_latch = 0;
	}
}

ROM_START(ngmc161)
	ROM_REGION( 0x2000, "mcu", 0 )
	ROM_LOAD( "mcu.bin", 0x0000, 0x2000, CRC(fd607982) SHA1(9a21e3ac084207b5ac88bcdf4b7ed4b4007798be) )
ROM_END

const tiny_rom_entry *neogeo_ngmc161_cart_device::device_rom_region() const
{
	return ROM_NAME(ngmc161);
}