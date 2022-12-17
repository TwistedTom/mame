// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

NEC SGP (スーパーグラフィックプロセッサ / Super Graphic Processor)

Unknown part number, used as GPU for PC88VA

TODO:
- timing details
- specifics about what exactly happens in work area when either SGP runs or is idle.
- famista: during gameplay it BITBLT same source to destination 0x00037076 
  with tp_mode = 3 and pitch = 0 (!?);
- rtype: during gameplay it does transfers with Pitch = 0xfff0, alias for negative draw?
- basic fires a VABOT on loading;

**************************************************************************************************/

#include "emu.h"
#include "pc88va_sgp.h"

#include <iostream>


#define LOG_COMMAND     (1U << 2)

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_STREAM std::cout

#include "logmacro.h"

#define LOGCOMMAND(...)       LOGMASKED(LOG_COMMAND, __VA_ARGS__)

// device type definition
DEFINE_DEVICE_TYPE(PC88VA_SGP, pc88va_sgp_device, "pc88va_sgp", "NEC PC88VA Super Graphic Processor")

pc88va_sgp_device::pc88va_sgp_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, PC88VA_SGP, tag, owner, clock),
	device_memory_interface(mconfig, *this),
	m_data(nullptr)
{
}

device_memory_interface::space_config_vector pc88va_sgp_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_DATA, &m_data_config)
	};
}


void pc88va_sgp_device::device_config_complete()
{
	m_data_config = address_space_config( "data", ENDIANNESS_LITTLE, 16, 22, 0 );
}

void pc88va_sgp_device::device_start()
{
	m_data = &space(AS_DATA);
}

void pc88va_sgp_device::device_reset()
{
	
}

/****************************************
 * I/Os
 ***************************************/

void pc88va_sgp_device::sgp_io(address_map &map)
{
	// TODO: check if readable
	map(0x00, 0x03).w(FUNC(pc88va_sgp_device::vdp_address_w));
	map(0x04, 0x04).w(FUNC(pc88va_sgp_device::control_w));
	map(0x06, 0x06).rw(FUNC(pc88va_sgp_device::status_r), FUNC(pc88va_sgp_device::trigger_w));
}

void pc88va_sgp_device::vdp_address_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_vdp_address[offset]);
}

/*
 * ---- -x-- VINTF (1) enable irq on END execution, (0) clear irq
 * ---- --x- VABOT suspend SGP execution (1)
 */
void pc88va_sgp_device::control_w(u8 data)
{
	if (data)
		popmessage("SGP warning write %02x", data);
}

/*
 * ---- ---x VBUSY (1) busy flag
 */
u8 pc88va_sgp_device::status_r()
{
	return 0;
}

/*
 * ---- ---x SGPCA start SGP execution (1)
 */
void pc88va_sgp_device::trigger_w(u8 data)
{
	// TODO: under a timer
	if (BIT(data, 0))
		start_exec();

	if (data != 1)
		LOG("Warning: SGP trigger write %02x\n", data);
}

void pc88va_sgp_device::start_exec()
{
	u32 vdp_pointer = (m_vdp_address[0]) | (m_vdp_address[1] << 16);
	// TODO: the SGP should go indefinitely until an END is issued
	// for now punt early until we find something that warrants a parallel execution
	// boomer is the current upper limit, uses SGP to transfer rows on vertical scrolling.
	const u32 end_pointer = vdp_pointer + 0x800;
	LOGCOMMAND("SGP: trigger start %08x\n", vdp_pointer);

	bool end_issued = false;

	while( vdp_pointer < end_pointer )
	{
		const u16 cur_opcode = m_data->read_word(vdp_pointer);
		u16 next_pc = 2;

		switch(cur_opcode)
		{
			// END
			case 0x0001:
				LOGCOMMAND("SGP: (PC=%08x) END\n", vdp_pointer);
				end_issued = true;
				break;
			// NOP
			case 0x0002:
				LOGCOMMAND("SGP: (PC=%08x) NOP\n", vdp_pointer);
				break;
			// SET WORK
			case 0x0003:
			{
				// This sets a work RAM address so that SGP can store internal data
				// This buffer needs 58 bytes, and nothing else is known other than
				// SGP needing this command issued as first before it can work with
				// any meaningful command.
				// Speculation is that assuming the host can r/w this area without
				// huge penalties it can also observe or even override the
				// SGP internals ...
				const u16 lower_offset = m_data->read_word(vdp_pointer + 2);
				const u16 upper_offset = m_data->read_word(vdp_pointer + 4);

				m_work_address = lower_offset | upper_offset << 16;
				LOGCOMMAND("SGP: (PC=%08x) SET WORK %08x\n",
					vdp_pointer,
					m_work_address
				);

				next_pc += 4;
				break;
			}
			// SET SOURCE
			case 0x0004:
			// SET DESTINATION
			case 0x0005:
			{
				const bool mode = cur_opcode == 5;
				BufferArea *ptr = mode ? &m_dst : &m_src;

				/*
				 * ---- ---- xxxx ---- start dot position
				 * ---- ---- ---- --xx SCRN_M: pixel mode
				 * ---- ---- ---- --00 1bpp
				 * ---- ---- ---- --01 4bpp
				 * ---- ---- ---- --10 8bpp
				 * ---- ---- ---- --11 RGB565
				 */
				const u16 param1 = m_data->read_word(vdp_pointer + 2);

				ptr->start_dot = (param1 & 0xf0) >> 4;
				ptr->pixel_mode = (param1 & 0x03);
				ptr->hsize = m_data->read_word(vdp_pointer + 4) & 0x0fff;
				ptr->vsize = m_data->read_word(vdp_pointer + 6) & 0x0fff;
				ptr->fb_pitch = m_data->read_word(vdp_pointer + 8) & 0xfffc;
				ptr->address = (m_data->read_word(vdp_pointer + 10) & 0xfffe)
					| (m_data->read_word(vdp_pointer + 12) << 16);

				LOGCOMMAND("SGP: (PC=%08x) SET %s %02x|H %4u|V %4u|Pitch %5u| address %08x\n"
					, vdp_pointer
					, mode ? "DESTINATION" : "SOURCE     "
					, param1
					, ptr->hsize
					, ptr->vsize
					, ptr->fb_pitch
					, ptr->address
				);

				next_pc += 12;
				break;
			}
			case 0x0006:
			{
				const u16 color_code = m_data->read_word(vdp_pointer + 2);
				LOGCOMMAND("SGP: (PC=%08x) SET COLOR %04x\n"
					, vdp_pointer
					, color_code
				);
				next_pc += 2;
				break;
			}
			// BitBLT
			case 0x0007:
			// PATBLT
			case 0x0008:
			{
				const bool cmd_mode = cur_opcode == 0x0008;
				const u16 draw_mode = m_data->read_word(vdp_pointer + 2);

				LOGCOMMAND("SGP: (PC=%08x) %s %04x\n"
					, vdp_pointer
					, cmd_mode ? "PATBLT" : "BITBLT"
					, draw_mode
				);
				cmd_blit(draw_mode, cmd_mode);

				next_pc += 2;
				break;
			}
			// LINE
			case 0x0009:
			{
				// mostly same as above
				const u16 draw_mode = m_data->read_word(vdp_pointer + 2);
				// documentation omits there's an extra parameter here
				// cfr. animefrm, hardcoded to 2 there
				const u16 unk_param = m_data->read_word(vdp_pointer + 4);
				// in pixels
				const u16 h_size = m_data->read_word(vdp_pointer + 6);
				const u16 v_size = m_data->read_word(vdp_pointer + 8);
				const u16 fb_pitch = m_data->read_word(vdp_pointer + 10) & 0xfffc;
				const u32 src_address = (m_data->read_word(vdp_pointer + 12) & 0xfffe)
					| (m_data->read_word(vdp_pointer + 14) << 16);

				// Note: start dot position and pixel mode set by SET SOURCE
				LOGCOMMAND("SGP: (PC=%08x) LINE %04x %04x %04x %04x %04x %08x\n"
					, vdp_pointer
					, draw_mode
					, unk_param
					, h_size
					, v_size
					, fb_pitch
					, src_address
				);
				next_pc += 14;
				break;
			}
			// CLS
			case 0x000a:
			{
				const u32 src_address = (m_data->read_word(vdp_pointer + 2) & 0xfffe)
					| (m_data->read_word(vdp_pointer + 4) << 16);
				const u32 word_size = (m_data->read_word(vdp_pointer + 6))
					| (m_data->read_word(vdp_pointer + 8) << 16);

				LOGCOMMAND("SGP: (PC=%08x) CLS %08x %08x\n"
					, vdp_pointer
					, src_address
					, word_size
				);
				next_pc += 8;
				break;
			}
			// SCAN RIGHT
			case 0x000b:
			// SCAN LEFT
			case 0x000c:
			{
				// This uses the destination block data to find a specific pixel
				// thru the SET COLOR command.
				// It updates the horizontal size of destination if the color is found,
				// returns 0 if the pixel is at origin, doesn't update if not found.
				const u8 mode = cur_opcode == 0x000c;
				LOGCOMMAND("SGP: (PC=%08x) %s\n"
					, vdp_pointer
					, mode ? "SCAN LEFT" : "SCAN RIGHT"
				);
				break;
			}
			default:
				LOGCOMMAND("SGP: (PC=%08x) %04x???\n", vdp_pointer, cur_opcode);
		}

		if (end_issued == true)
			break;

		vdp_pointer += next_pc;
	}

	if (vdp_pointer >= end_pointer)
		LOG("Warning: execution punt without an END issued\n");
}

/****************************************
 * Commands
 ***************************************/


/*
 * ---x ---- ---- ---- SF (0) shift source according to destination position
 * ---- x--- ---- ---- VD vertical transfer direction (1) negative (0) positive
 * ---- -x-- ---- ---- HD horizontal transfer direction (1) negative (0) positive
 * ---- --xx ---- ---- TP/MOD
 * ---- --00 ---- ---- transfer source as-is
 * ---- --01 ---- ---- do not transfer if source is 0 (transparent pen)
 * ---- --10 ---- ---- transfer only if destination is 0
 * ---- --11 ---- ---- <undocumented>
 * ---- ---- ---- xxxx LOGICAL OP
 * ---- ---- ---- 0000 0
 * ---- ---- ---- 0001 Src AND Dst
 * ---- ---- ---- 0010 /Src AND Dst
 * ---- ---- ---- 0011 NOP
 * ---- ---- ---- 0100 Src AND /Dst
 * ---- ---- ---- 0101 Src
 * ---- ---- ---- 0110 Src XOR Dst
 * ---- ---- ---- 0111 Src OR Dst
 * ---- ---- ---- 1000 /(Src OR Dst)
 * ---- ---- ---- 1001 /(Src XOR Dst)
 * ---- ---- ---- 1010 /Src
 * ---- ---- ---- 1011 /Src OR Dst
 * ---- ---- ---- 1100 /Dst
 * ---- ---- ---- 1101 Src OR /Dst
 * ---- ---- ---- 1110 /(Src AND Dst)
 * ---- ---- ---- 1111 1
 *
 * PATBLT is identical to BITBLT except it repeats source copy 
 * if it exceeds the clipping range.
 */
void pc88va_sgp_device::cmd_blit(u16 draw_mode, bool is_patblt)
{
	const u8 logical_op = draw_mode & 0xf;
	const u8 tp_mod = (draw_mode >> 8) & 0x3;

	// TODO: boomer title screen
	if (is_patblt == true)
	{
		LOG("PATBLT\n");
	//	return;
	}

	// ballbrkr: 6
	if (logical_op != 5)
	{
		LOG("BITBLT logical_op == %d\n", logical_op);
		return;
	}

	if (tp_mod > 1)
	{
		LOG("BITBLT tp_mod == %d\n", tp_mod);
		return;
	}

	if (m_src.hsize != m_dst.hsize || m_src.vsize != m_dst.vsize)
	{
		LOG("BITBLT non-even sizes (%d x %d) x (%d x %d)\n", m_src.hsize, m_src.vsize, m_dst.hsize, m_dst.vsize);
		return;
	}

	if (m_src.pixel_mode == 0 || m_src.pixel_mode == 3 || m_src.pixel_mode != m_dst.pixel_mode)
	{
		LOG("BITBLT pixel mode %d x %d\n", m_src.pixel_mode, m_dst.pixel_mode);
		return;
	}

	for (int yi = 0; yi < m_src.vsize; yi ++)
	{
		u32 src_address = m_src.address + (yi * m_src.fb_pitch);
		u32 dst_address = m_dst.address + (yi * m_dst.fb_pitch);

		for (int xi = 0; xi < (m_src.hsize >> 2); xi ++)
		{
			// TODO: not very efficient, we need a cleaner per-pixel RMW phase
			const u16 src_dot = m_data->read_word(src_address);
			const u16 dst_dot = m_data->read_word(dst_address);
			u16 result = 0;

			for (int pixi = 0; pixi < 4; pixi ++)
			{
				u8 cur_pixel = (src_dot & 0xf);
				if (cur_pixel || tp_mod == 0)
					result |= (src_dot & 0xf) << (pixi * 4);
				else
					result |= (dst_dot & 0xf) << (pixi * 4);
			}

			m_data->write_word(dst_address, result);
			src_address += 2;
			dst_address += 2;
		}
	}
}
