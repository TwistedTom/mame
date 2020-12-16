#include "emu.h"
#include "slot.h"
#include "rom.h"

class neogeo_ngmc120_cart_device : public neogeo_rom_device
{
public:
	// construction/destruction
	neogeo_ngmc120_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint16_t clock);

	// reading and writing
	virtual int get_fixed_bank_type() override { return 0; }

protected:
	neogeo_ngmc120_cart_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint16_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
};

// device type definition
//DECLARE_DEVICE_TYPE(NEOGEO_NGMC120_CART, neogeo_ngmc120_cart_device)
DEFINE_DEVICE_TYPE(NEOGEO_NGMC120_CART, neogeo_ngmc120_cart_device, "neocart_ngmc120", "Neo Geo 120-in-1 Multi-Cart")

//-------------------------------------------------
//  neogeo_ngmc120_cart_device - constructor
//-------------------------------------------------

neogeo_ngmc120_cart_device::neogeo_ngmc120_cart_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint16_t clock) :
	neogeo_rom_device(mconfig, type, tag, owner, clock)
{
}

neogeo_ngmc120_cart_device::neogeo_ngmc120_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint16_t clock) :
	neogeo_ngmc120_cart_device(mconfig, NEOGEO_NGMC120_CART, tag, owner, clock)
{
}


//-------------------------------------------------
//  mapper specific start/reset
//-------------------------------------------------

void neogeo_ngmc120_cart_device::device_start()
{
	
}

void neogeo_ngmc120_cart_device::device_reset()
{
	
}
