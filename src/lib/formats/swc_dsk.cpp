// license:BSD-3-Clause
// copyright-holders:
/*********************************************************************

    formats/swc_dsk.c

    Super Wild Card disk images

*********************************************************************/

#include "formats/swc_dsk.h"

#include "ioprocs.h"

#include <cstring>


swc_format::swc_format() : upd765_format(formats)
{
}

const char *swc_format::name() const
{
	return "swc";
}

const char *swc_format::description() const
{
	return "Super Wild Card floppy disk image";
}

const char *swc_format::extensions() const
{
	return "img";
}

const swc_format::format swc_format::formats[] = {
	{   /*  720K 3 1/2 inch double density */
		floppy_image::FF_35,  floppy_image::DSDD, floppy_image::MFM,
		2000,  9, 80, 2, 512, {}, 1, {}, 80, 50, 22, 80
	},
	{   /*  800K 3 1/2 inch double density - gaps unverified */
		floppy_image::FF_35,  floppy_image::DSDD, floppy_image::MFM,
		2000, 10, 80, 2, 512, {}, 1, {}, 80, 50, 22, 36  // 46
	},
	{   /* 1440K 3 1/2 inch high density */
		floppy_image::FF_35,  floppy_image::DSHD, floppy_image::MFM,
		1000, 18, 80, 2, 512, {}, 1, {}, 80, 50, 22, 108
	},
	{   /* 1600K 3 1/2 inch high density - gaps unverified */
		floppy_image::FF_35,  floppy_image::DSHD, floppy_image::MFM,
		1000, 20, 80, 2, 512, {}, 1, {}, 80, 50, 22, 42
	},
	{}
};

const swc_format FLOPPY_SWC_FORMAT;
