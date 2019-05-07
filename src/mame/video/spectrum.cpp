// license:GPL-2.0+
// copyright-holders:Kevin Thacker
/***************************************************************************

  spectrum.c

  Functions to emulate the video hardware of the ZX Spectrum.

  Changes:

  DJR 08/02/00 - Added support for FLASH 1.
  DJR 16/05/00 - Support for TS2068/TC2048 hires and 64 column modes.
  DJR 19/05/00 - Speeded up Spectrum 128 screen refresh.
  DJR 23/05/00 - Preliminary support for border colour emulation.

***************************************************************************/

#include "emu.h"
#include "includes/spectrum.h"
#include "includes/spec128.h"
#include "includes/specpls3.h"
#include "cpu/z80/specz80.h"

/***************************************************************************
  Start the video hardware emulation.
***************************************************************************/
VIDEO_START_MEMBER(spectrum_state,spectrum)
{
	//The amount of tstates from the very first (top left) display pixel, to the very first screen pixel (top left).
	int beam_adjust = (SPEC_TOP_BORDER*SPEC_CYCLES_PER_LINE) + SPEC_LEFT_BORDER_CYCLES;
	device_t *const cpudevice = mconfig().root_device().subdevice("maincpu");
	
	m_frame_invert_count = 16;
	m_frame_number = 0;
	m_flash_invert = 0;

	m_previous_border_x = 0;
	m_previous_border_y = 0;
	m_screen->register_screen_bitmap(m_border_bitmap);
	m_previous_screen_x = 0;
	m_previous_screen_y = 0;
	m_screen->register_screen_bitmap(m_screen_bitmap);

	m_screen_location = m_video_ram;

	m_irq_off_timer = timer_alloc(TIMER_IRQ_OFF);

	m_tstate_info.per_line = SPEC_CYCLES_PER_LINE;
	//The SPEC_CYCLES_ULA_* defines all relate to the first 'screen' pixel (y=96,x=48). Convert them
	//so that they all relate to the first 'display' pixel (y=0, x=0)
	m_tstate_info.beam_start_screen       = SPEC_CYCLES_ULA_SCREEN       - beam_adjust;
	m_tstate_info.beam_start_border       = SPEC_CYCLES_ULA_BORDER       - beam_adjust;
	m_tstate_info.beam_start_floating_bus = SPEC_CYCLES_ULA_FLOATING_BUS - beam_adjust;
	m_tstate_info.using_raster_callback = false;
	specz80_device * specz80_cpu = dynamic_cast<specz80_device*> (cpudevice);
	if(specz80_cpu != nullptr)
	{
		m_tstate_info.using_raster_callback = specz80_cpu->get_using_raster_callback();
	}
	m_border_brush = 0;
}

VIDEO_START_MEMBER(spectrum_state,spectrum_128)
{
	//The amount of tstates from the very first (top left) display pixel, to the very first screen pixel (top left).
	int beam_adjust = (SPEC128_TOP_BORDER*SPEC128_CYCLES_PER_LINE) + SPEC128_LEFT_BORDER_CYCLES;
	device_t *const cpudevice = mconfig().root_device().subdevice("maincpu");
	
	m_frame_invert_count = 16;
	m_frame_number = 0;
	m_flash_invert = 0;

	m_previous_border_x = 0;
	m_previous_border_y = 0;
	m_screen->register_screen_bitmap(m_border_bitmap);
	m_previous_screen_x = 0;
	m_previous_screen_y = 0;
	m_screen->register_screen_bitmap(m_screen_bitmap);

	m_screen_location = m_ram->pointer() + (5 << 14);

	m_irq_off_timer = timer_alloc(TIMER_IRQ_OFF);

	m_tstate_info.per_line = SPEC128_CYCLES_PER_LINE;
	//The SPEC_CYCLES_ULA_* defines all relate to the first 'screen' pixel (y=96,x=48). Convert them
	//so that they all relate to the first 'display' pixel (y=0, x=0)
	m_tstate_info.beam_start_screen       = SPEC128_CYCLES_ULA_SCREEN       - beam_adjust;
	m_tstate_info.beam_start_border       = SPEC128_CYCLES_ULA_BORDER       - beam_adjust;
	m_tstate_info.beam_start_floating_bus = SPEC128_CYCLES_ULA_FLOATING_BUS - beam_adjust;
	m_tstate_info.using_raster_callback = false;
	specz80_device * specz80_cpu = dynamic_cast<specz80_device*> (cpudevice);
	if(specz80_cpu != nullptr)
	{
		m_tstate_info.using_raster_callback = specz80_cpu->get_using_raster_callback();
	}
	m_border_brush = 0;
}

// z80
VIDEO_START_MEMBER(spectrum_state,spectrum_plus3)
{
	//The amount of tstates from the very first (top left) display pixel, to the very first screen pixel (top left).
	int beam_adjust = (SPECPLS3_TOP_BORDER*SPECPLS3_CYCLES_PER_LINE) + SPECPLS3_LEFT_BORDER_CYCLES;
	device_t *const cpudevice = mconfig().root_device().subdevice("maincpu");
	
	m_frame_invert_count = 16;
	m_frame_number = 0;
	m_flash_invert = 0;

	m_previous_border_x = 0;
	m_previous_border_y = 0;
	m_screen->register_screen_bitmap(m_border_bitmap);
	m_previous_screen_x = 0;
	m_previous_screen_y = 0;
	m_screen->register_screen_bitmap(m_screen_bitmap);

	m_screen_location = m_ram->pointer() + (5 << 14);

	m_irq_off_timer = timer_alloc(TIMER_IRQ_OFF);

	m_tstate_info.per_line = SPECPLS3_CYCLES_PER_LINE;
	//The SPEC_CYCLES_ULA_* defines all relate to the first 'screen' pixel (y=96,x=48). Convert them
	//so that they all relate to the first 'display' pixel (y=0, x=0)
	m_tstate_info.beam_start_screen       = SPECPLS3_CYCLES_ULA_SCREEN       - beam_adjust;
	m_tstate_info.beam_start_border       = SPECPLS3_CYCLES_ULA_BORDER       - beam_adjust;
	m_tstate_info.beam_start_floating_bus = SPECPLS3_CYCLES_ULA_FLOATING_BUS - beam_adjust;
	m_tstate_info.using_raster_callback = false;
	specz80_device * specz80_cpu = dynamic_cast<specz80_device*> (cpudevice);
	if(specz80_cpu != nullptr)
	{
		m_tstate_info.using_raster_callback = specz80_cpu->get_using_raster_callback();
	}
	m_border_brush = 0;  // ?
}

/* return the color to be used inverting FLASHing colors if necessary */
inline unsigned char spectrum_state::get_display_color (unsigned char color, int invert)
{
	if (invert && (color & 0x80))
		return (color & 0xc0) + ((color & 0x38) >> 3) + ((color & 0x07) << 3);
	else
		return color;
}

/* Code to change the FLASH status every 25 frames. Note this must be
   independent of frame skip etc. */
WRITE_LINE_MEMBER(spectrum_state::screen_vblank_spectrum)
{
	// rising edge
	if (state)
	{
		if(!m_tstate_info.using_raster_callback)
		{
			spectrum_UpdateBorderBitmap();
			spectrum_UpdateScreenBitmap(true);
		}

		m_frame_number++;

		if (m_frame_number >= m_frame_invert_count)
		{
			m_frame_number = 0;
			m_flash_invert = !m_flash_invert;
		}
	}
}



/***************************************************************************
  Update the spectrum screen display.

  The screen consists of 312 scanlines as follows:
  64  border lines (the last 48 are actual border lines; the others may be
                    border lines or vertical retrace)
  192 screen lines
  56  border lines

  Each screen line has 48 left border pixels, 256 screen pixels and 48 right
  border pixels.

  Each scanline takes 224 T-states divided as follows:
  128 Screen (reads a screen and ATTR byte [8 pixels] every 4 T states)
  24  Right border
  48  Horizontal retrace
  24  Left border

  The 128K Spectrums have only 63 scanlines before the TV picture (311 total)
  and take 228 T-states per scanline.

***************************************************************************/

inline void spectrum_state::spectrum_plot_pixel(bitmap_ind16 &bitmap, int x, int y, uint32_t color)
{
	bitmap.pix16(y, x) = (uint16_t)color;
}

uint32_t spectrum_state::screen_update_spectrum(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	static const rectangle rect(SPEC_LEFT_BORDER, SPEC_LEFT_BORDER + SPEC_DISPLAY_XSIZE - 1, SPEC_TOP_BORDER, SPEC_TOP_BORDER + SPEC_DISPLAY_YSIZE - 1);

	if (m_border_bitmap.valid())
		copyscrollbitmap(bitmap, m_border_bitmap, 0, nullptr, 0, nullptr, cliprect);

	if(!m_tstate_info.using_raster_callback)
	{
		spectrum_UpdateScreenBitmap();
	}
	
	if (m_screen_bitmap.valid())
		copyscrollbitmap(bitmap, m_screen_bitmap, 0, nullptr, 0, nullptr, rect);

#if 0
	// note, don't update borders in here, this can time travel w/regards to other timers and may end up giving you
	// screen positions earlier than the last write handler gave you

	/* for now do a full-refresh */
	int x, y, b, scrx, scry;
	unsigned short ink, pap;
	unsigned char *attr, *scr;
	//  int full_refresh = 1;

	scr=m_screen_location;

	for (y=0; y<192; y++)
	{
		scrx=SPEC_LEFT_BORDER;
		scry=((y&7) * 8) + ((y&0x38)>>3) + (y&0xC0);
		attr=m_screen_location + ((scry>>3)*32) + 0x1800;

		for (x=0;x<32;x++)
		{
			/* Get ink and paper colour with bright */
			if (m_flash_invert && (*attr & 0x80))
			{
				ink=((*attr)>>3) & 0x0f;
				pap=((*attr) & 0x07) + (((*attr)>>3) & 0x08);
			}
			else
			{
				ink=((*attr) & 0x07) + (((*attr)>>3) & 0x08);
				pap=((*attr)>>3) & 0x0f;
			}

			for (b=0x80;b!=0;b>>=1)
			{
				if (*scr&b)
					spectrum_plot_pixel(bitmap,scrx++,SPEC_TOP_BORDER+scry,ink);
				else
					spectrum_plot_pixel(bitmap,scrx++,SPEC_TOP_BORDER+scry,pap);
			}

			scr++;
			attr++;
		}
	}
#endif

	return 0;
}


static constexpr rgb_t spectrum_pens[16] = {
	{ 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0xbf },
	{ 0xbf, 0x00, 0x00 },
	{ 0xbf, 0x00, 0xbf },
	{ 0x00, 0xbf, 0x00 },
	{ 0x00, 0xbf, 0xbf },
	{ 0xbf, 0xbf, 0x00 },
	{ 0xbf, 0xbf, 0xbf },
	{ 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0xff },
	{ 0xff, 0x00, 0x00 },
	{ 0xff, 0x00, 0xff },
	{ 0x00, 0xff, 0x00 },
	{ 0x00, 0xff, 0xff },
	{ 0xff, 0xff, 0x00 },
	{ 0xff, 0xff, 0xff }
};
// Initialise the palette
void spectrum_state::spectrum_palette(palette_device &palette) const
{
	palette.set_pen_colors(0, spectrum_pens);
}


void spectrum_state::spectrum_GetBeamPosition(int beam_start, unsigned int *x, unsigned int *y)
{
	if(!m_tstate_info.using_raster_callback)
	{
		*x = m_screen->hpos();
		*y = m_screen->vpos();
		return;
	}
	else
	{
		if( m_tstate_info.counter < beam_start)
		{
			*x = 0; *y = 0;
			return;
		}

		//Ensure y is always 0 to 296 (inclusive)
		*y = ( m_tstate_info.counter - beam_start)/m_tstate_info.per_line;
		if (*y >= SPEC_SCREEN_HEIGHT)
		{
			*x = 0; *y = 0;
			return;
		}

		//Ensure x is always 0 to 352 (inclusive)
		*x = (( m_tstate_info.counter - beam_start)%m_tstate_info.per_line)*2; //Times 2 because its 2 pixels for every tstate.
		if (*x >= SPEC_SCREEN_WIDTH)
		{
			(*y)++;
			*x = 0;
			if(*y>= SPEC_SCREEN_HEIGHT) *y = 0;
		}
		return;
	}
}


void spectrum_state::spectrum_UpdateScreenBitmap(bool eof)
{
	unsigned int x,y;
	
	int width = m_screen_bitmap.width();
	int height = m_screen_bitmap.height();
	
	spectrum_GetBeamPosition(m_tstate_info.beam_start_screen, &x, &y);

	if ((m_previous_screen_x == x) && (m_previous_screen_y == y) && !eof)
		return;

	if (m_screen_bitmap.valid())
	{
		//printf("update screen from x=%d,y=%d to x=%d,y=%d\n", m_previous_screen_x, m_previous_screen_y, x, y);
		
		do
		{
			uint16_t scrx = m_previous_screen_x - SPEC_LEFT_BORDER;
			uint16_t scry = m_previous_screen_y - SPEC_TOP_BORDER;

			if (scrx < SPEC_DISPLAY_XSIZE && scry < SPEC_DISPLAY_YSIZE)
			{
				// This can/must be optimised
				// On an actual ZX spectrum, you can not change the screen/attribute
				// data when the ULA is processing an 8-pixel 'chunk'. This code ensures
				// that whatever scr/attr is shown on pixel 0, is also shown on pixels 1 to 7.
				if ((scrx & 7) == 0) {
					uint16_t *bm = &m_screen_bitmap.pix16(m_previous_screen_y, m_previous_screen_x);
					uint8_t attr = *(m_screen_location + ((scry & 0xF8) << 2) + (scrx >> 3) + 0x1800);
					uint8_t scr = *(m_screen_location + ((scry & 7) << 8) + ((scry & 0x38) << 2) + ((scry & 0xC0) << 5) + (scrx >> 3));
					uint16_t ink = (attr & 0x07) + ((attr >> 3) & 0x08);
					uint16_t pap = (attr >> 3) & 0x0f;

					//printf("spectrum_UpdateScreenBitmap - y=%d x=%d attr=%x (ink=%x pap=%x)\n", scry, scrx, attr, ink, pap);
					
					if (m_flash_invert && (attr & 0x80))
						scr = ~scr;

					for (uint8_t b = 0x80; b != 0; b >>= 1)
						*bm++ = (scr & b) ? ink : pap;
				}
			}

			m_previous_screen_x += 1;

			if (m_previous_screen_x >= width)
			{
				m_previous_screen_x = 0;
				m_previous_screen_y += 1;

				if (m_previous_screen_y >= height)
				{
					m_previous_screen_y = 0;
				}
			}
		} while (!((m_previous_screen_x == x) && (m_previous_screen_y == y)));
	}
}

/* The code below is just a per-pixel 'partial update' for the border */

void spectrum_state::spectrum_UpdateBorderBitmap()
{
	unsigned int x,y;
	int width = m_border_bitmap.width();
	int height = m_border_bitmap.height();

	spectrum_GetBeamPosition(m_tstate_info.beam_start_border, &x, &y);

	if(m_tstate_info.using_raster_callback)
	{
		//This if statement returns if the raster hans't moved
		//since our last visit (I.e. There is nothing to render).
		//Doing this check is worthwhile if the raster callback is
		//implemented. If the raster callback is not implemented, then
		//this check can not be performed as the original code relied
		//on the border being totally refreshed every frame.
		if ((m_previous_border_x == x) && (m_previous_border_y == y))
			return;
	}

	if (m_border_bitmap.valid())
	{
		uint16_t new_border_brush = m_port_fe_data & 0x07;
		
		//printf("update border from x=%d,y=%d to x=%d,y=%d border=0x%X\n", m_previous_border_x, m_previous_border_y, x, y, border);
		
		do
		{
			//m_border_bitmap.pix16(m_previous_border_y, m_previous_border_x) = border;
			// On an actual ZX spectrum, you can not change the border colour during.
			// an 8-pixel 'chunk'. This code ensures that whatever colour is shown
			// on pixel 0, is also shown on pixels 1 to 7.
			if ((m_previous_border_x & 0x07) == 0x00) 
			{
				uint16_t *bm = &m_border_bitmap.pix16(m_previous_border_y, m_previous_border_x);

				for (uint8_t b = 0x80; b != 0; b >>= 1)
					*bm++ = new_border_brush;

			}
			
			m_previous_border_x += 1;

			if (m_previous_border_x >= width)
			{
				m_previous_border_x = 0;
				m_previous_border_y += 1;

				if (m_previous_border_y >= height)
				{
					m_previous_border_y = 0;
				}
			}
		}
		while (!((m_previous_border_x == x) && (m_previous_border_y == y)));
	}
	else
	{
		// no border bitmap allocated? fatalerror?
	}
}
