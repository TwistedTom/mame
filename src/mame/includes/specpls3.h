// license:GPL-2.0+
// copyright-holders:Kevin Thacker
/*****************************************************************************
 *
 * includes/specpls3.h
 *
 ****************************************************************************/

#ifndef MAME_INCLUDES_SPECPLS3_H
#define MAME_INCLUDES_SPECPLS3_H

INPUT_PORTS_EXTERN( spec_plus );

// z80
#define SPECPLS3_TOP_BORDER              SPEC128_TOP_BORDER
#define SPECPLS3_CYCLES_PER_LINE         SPEC128_CYCLES_PER_LINE
#define SPECPLS3_LEFT_BORDER_CYCLES      SPEC128_LEFT_BORDER_CYCLES

//The SPECPLS3_CYCLES_ULA_* values can be tweaked by running ulatest3, btime, stime and
//timingtest(Patrik Rak) on an actual ZX spectrum, then comparing the results after
//running the same test programs on mame.
//The SPECPLS3_CYCLES_ULA_* values set here were based on results from the fuse emulator.
//TODO. Confirm values based on an actual ZX Spectrum +3/+2a.
// Note. "FSP" stands for First Screen Pixel. This is the pixel at position y=96 x=48
//       (I.e. The pixel just under the top border and just after the left border).
#define SPECPLS3_CYCLES_ULA_CONTENTION   14360 /* T-State at which IO and memory contention begins. */
#define SPECPLS3_CYCLES_ULA_SCREEN       14365 /* T-State when the FSP shows screen data stored at 4000H. */
#define SPECPLS3_CYCLES_ULA_BORDER       14370 /* T-State when the FSP shows 'port 0xfe' border data (Note. It is not possible to show border data at the FSP, but if it were then this T-State is when it would happen). */
#define SPECPLS3_CYCLES_ULA_FLOATING_BUS 14362 /* T-State when the colour attributes of the FSP can be read from the floating bus. */
#define SPECPLS3_CYCLES_PER_FRAME        70908 /* 228 x 312 */
// z80

#endif // MAME_INCLUDES_SPECPLS3_H
