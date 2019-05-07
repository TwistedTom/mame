// license:GPL-2.0+
// copyright-holders:Kevin Thacker
/*****************************************************************************
 *
 * includes/spec128.h
 *
 ****************************************************************************/

#ifndef MAME_INCLUDES_SPEC128_H
#define MAME_INCLUDES_SPEC128_H

/* 128K machines take an extra 4 cycles per scan line - add this to retrace */
#define SPEC128_UNSEEN_LINES    15
#define SPEC128_RETRACE_CYCLES  52
#define SPEC128_CYCLES_PER_LINE 228

//z80
#define SPEC128_UNSEEN_LINES            15
#define SPEC128_RETRACE_CYCLES          52
#define SPEC128_CYCLES_PER_LINE         228
#define SPEC128_TOP_BORDER              SPEC_TOP_BORDER
#define SPEC128_LEFT_BORDER_CYCLES      SPEC_LEFT_BORDER_CYCLES

//The SPEC128_CYCLES_ULA_* values can be tweaked by running ulatest3, btime, stime and
//timingtest(Patrik Rak) on an actual ZX spectrum, then comparing the results after
//running the same test programs on mame.
//The SPEC128_CYCLES_ULA_* values set here were based on an actual ZX Spectrum 128K +2.
// Note. "FSP" stands for First Screen Pixel. This is the pixel at position y=96 x=48
//       (I.e. The pixel just under the top border and just after the left border).
#define SPEC128_CYCLES_ULA_CONTENTION   14361 /* T-State at which IO and memory contention begins. */
#define SPEC128_CYCLES_ULA_SCREEN       14364 /* T-State when the FSP shows screen data stored at 4000H. */
#define SPEC128_CYCLES_ULA_BORDER       14368 /* T-State when the FSP shows 'port 0xfe' border data (Note. It is not possible to show border data at the FSP, but if it were then this T-State is when it would happen). */
#define SPEC128_CYCLES_ULA_FLOATING_BUS 14363 /* T-State when the colour attributes of the FSP can be read from the floating bus. */
#define SPEC128_CYCLES_PER_FRAME        70908 /* 228 x 312 */
// z80

#endif // MAME_INCLUDES_SPEC128_H
