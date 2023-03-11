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
	
	// additional formats working on real hw
	
	{   /* 1476K 3 1/2 inch high density - gaps unverified */
		floppy_image::FF_35,  floppy_image::DSHD, floppy_image::MFM,
		1000, 18, 82, 2, 512, {}, 1, {}, 80, 50, 22, 108
	},
	{   /* 1640K 3 1/2 inch high density - gaps unverified */
		floppy_image::FF_35,  floppy_image::DSHD, floppy_image::MFM,
		1000, 20, 82, 2, 512, {}, 1, {}, 80, 50, 22, 42
	},
	{   /* 1680K 3 1/2 inch high density - gaps unverified */
		floppy_image::FF_35,  floppy_image::DSHD, floppy_image::MFM,
		1000, 21, 80, 2, 512, {}, 1, {}, 80, 50, 22, 14
	},
	{   /* 1722K 3 1/2 inch high density - gaps unverified */
		floppy_image::FF_35,  floppy_image::DSHD, floppy_image::MFM,
		1000, 21, 82, 2, 512, {}, 1, {}, 80, 50, 22, 14
	},
	{   /* 2880K 3 1/2 inch extended density - gaps unverified */
		floppy_image::FF_35,  floppy_image::DSED, floppy_image::MFM,
		500, 36, 80, 2, 512, {}, 1, {}, 80, 50, 41, 80
	},  // not working  read err
	{}
	
	/* some other formats tested on real hw (.hfe only)
	
	1743K   HD  21  83   2  512   ok
	1764K   HD  21  84   2  512   ok
	1785K   HD  21  85   2  512   ok
	2500K   DD  10  255  2  512   ng
	3380K   HD  26  128  2  512   ok
	4500K   HD  18  255  2  512   ng
	5350K   HD  21  255  2  512   ng
	6780K   HD  27  255  2  512   ng
	
	swc doesn't seem like 255 track formats...
	tested with gotek hxc fw ver. 5.1.2.1a
	TODO: add .hfe support
	
	*/
};

const swc_format FLOPPY_SWC_FORMAT;
