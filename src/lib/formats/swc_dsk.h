// license:BSD-3-Clause
// copyright-holders:
/*********************************************************************

    formats/swc_dsk.h

    Super Wild Card disk images

*********************************************************************/
#ifndef MAME_FORMATS_SWC_DSK_H
#define MAME_FORMATS_SWC_DSK_H

#pragma once

#include "flopimg.h"
#include "upd765_dsk.h"

class swc_format : public upd765_format
{
public:
	swc_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

private:
	static const format formats[];
};

extern const swc_format FLOPPY_SWC_FORMAT;

#endif // MAME_FORMATS_SWC_DSK_H
