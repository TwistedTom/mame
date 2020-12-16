#include "emu.h"
#include "slot.h"
#include "rom.h"

class neogeo_garouc_cart_device : public neogeo_rom_device
{
public:
	// construction/destruction
	neogeo_garouc_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint16_t clock);

	// reading and writing
	virtual uint16_t protection_r(address_space &space, offs_t offset) override;
	virtual uint16_t addon_r(offs_t offset) override;
	virtual uint32_t get_bank_base(uint16_t sel) override;

	virtual int get_fixed_bank_type() override { return 0; }

protected:
	neogeo_garouc_cart_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint16_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	uint16_t m_sma_rng;
};

// device type definition
//DECLARE_DEVICE_TYPE(NEOGEO_GAROUC_CART, neogeo_garouc_cart_device)
DEFINE_DEVICE_TYPE(NEOGEO_GAROUC_CART, neogeo_garouc_cart_device, "neocart_garouc", "Neo Geo Garouc Cart")

//-------------------------------------------------
//  neogeo_garouc_cart_device - constructor
//-------------------------------------------------

neogeo_garouc_cart_device::neogeo_garouc_cart_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint16_t clock) :
	neogeo_rom_device(mconfig, type, tag, owner, clock),
	m_sma_rng(0)
{
}

neogeo_garouc_cart_device::neogeo_garouc_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint16_t clock) :
	neogeo_garouc_cart_device(mconfig, NEOGEO_GAROUC_CART, tag, owner, clock)
{
}


//-------------------------------------------------
//  mapper specific start/reset
//-------------------------------------------------

void neogeo_garouc_cart_device::device_start()
{
	save_item(NAME(m_sma_rng));
}

void neogeo_garouc_cart_device::device_reset()
{
	m_sma_rng = 0x2345;
}

uint32_t neogeo_garouc_cart_device::get_bank_base(uint16_t sel)
{
	static const int bankoffset[64] =
	{
		0x000000, 0x100000, 0x200000, 0x300000, // 00
		0x280000, 0x380000, 0x2d0000, 0x3d0000, // 04
		0x2f0000, 0x3f0000, 0x400000, 0x500000, // 08
		0x420000, 0x520000, 0x440000, 0x540000, // 12
		0x498000, 0x598000, 0x4a0000, 0x5a0000, // 16
		0x4a8000, 0x5a8000, 0x4b0000, 0x5b0000, // 20
		0x4b8000, 0x5b8000, 0x4c0000, 0x5c0000, // 24
		0x4c8000, 0x5c8000, 0x4d0000, 0x5d0000, // 28
		0x458000, 0x558000, 0x460000, 0x560000, // 32
		0x468000, 0x568000, 0x470000, 0x570000, // 36
		0x478000, 0x578000, 0x480000, 0x580000, // 40
		0x488000, 0x588000, 0x490000, 0x590000, // 44
		0x5d0000, 0x5d8000, 0x5e0000, 0x5e8000, // 48
		0x5f0000, 0x5f8000, 0x600000, // rest not used?
	};

	// unscramble bank number
	int data =
		(BIT(sel,  5) << 0)+
		(BIT(sel,  9) << 1)+
		(BIT(sel,  7) << 2)+
		(BIT(sel,  6) << 3)+
		(BIT(sel, 14) << 4)+
		(BIT(sel, 12) << 5);

	int bankaddress = 0x100000 + bankoffset[data];

	//logerror("bankswitch: %04x -> %04x\n", sel, bankaddress);
	
	return bankaddress;
}

uint16_t neogeo_garouc_cart_device::protection_r(address_space &space, offs_t offset)
{
	logerror("protection read\n");

	return 0x9a37;
}

uint16_t neogeo_garouc_cart_device::addon_r(offs_t offset)
{
	uint16_t old = m_sma_rng;
	uint16_t newbit = ((m_sma_rng >> 2) ^
						(m_sma_rng >> 3) ^
						(m_sma_rng >> 5) ^
						(m_sma_rng >> 6) ^
						(m_sma_rng >> 7) ^
						(m_sma_rng >>11) ^
						(m_sma_rng >>12) ^
						(m_sma_rng >>15)) & 1;

	m_sma_rng = (m_sma_rng << 1) | newbit;

	logerror("rng read: %d -> %d\n", old, m_sma_rng);

	return old;
}
