// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
#ifndef MAME_CPU_Z80_SPECZ80_H
#define MAME_CPU_Z80_SPECZ80_H

#pragma once

#include "z80.h"
	
#define MAX_CMSE	9	    // Maximum contended memory script elements
#define MAX_RWINFO	6	    // Maximum reads/writes per opcode
#define MAX_CM_SCRIPTS 37

enum CMSE_TYPES
{
	CMSE_TYPE_MEMORY,
	CMSE_TYPE_IO_PORT,
	CMSE_TYPE_IR_REGISTER,
	CMSE_TYPE_BC_REGISTER,
	CMSE_TYPE_UNCONTENDED
};

enum ULA_VARIANT_TYPES
{
	ULA_VARIANT_NONE,
	ULA_VARIANT_SINCLAIR,
	ULA_VARIANT_AMSTRAD
};

enum RWINFO_FLAGS
{
	RWINFO_READ      = 0x01,
	RWINFO_WRITE     = 0x02,
	RWINFO_IO_PORT   = 0x04,
	RWINFO_MEMORY    = 0x08,
	RWINFO_PROCESSED = 0x10
};

typedef struct ContendedMemoryScriptElement
{
	int	rw_ix;
	int	inst_cycles;
	int type;
	int multiplier;
	bool is_optional;
} CMSE;

typedef struct ContendedMemoryScriptBreakdown
{
	CMSE elements[MAX_CMSE];
	int number_of_elements;
	int inst_cycles_mandatory;
	int inst_cycles_optional;
	int inst_cycles_total;
} CM_SCRIPT_BREAKDOWN;

typedef struct ContendedMemoryScriptDescription
{
	const char* sinclair;
	const char* amstrad;
} CM_SCRIPT_DESCRIPTION;

typedef struct ContendedMemoryScript
{
	int id;
	const char* desc;
	CM_SCRIPT_BREAKDOWN breakdown;
} CM_SCRIPT;

typedef struct MemoryReadWriteInformation
{
	uint16_t addr;
	uint8_t val;
	uint16_t flags;
	const char *dbg;
} RWINFO;

typedef struct OpcodeHistory
{
	bool capturing;
	RWINFO rw[MAX_RWINFO];
	int rw_count;
	int tstate_start;
	uint16_t register_ir;
	uint16_t register_bc;

	int uncontended_cycles_predicted;
	int uncontended_cycles_eaten;
	bool do_optional;

	CM_SCRIPT *script;
	CM_SCRIPT_BREAKDOWN *breakdown;
	int element;
} OPCODE_HISTORY;


class specz80_device : public z80_device
{
public:
	specz80_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	void configure_contended_memory(int ula_variant, const char *ula_delay_sequence, int cycles_contention_start, int cycles_per_line, int cycles_per_frame, const char *contended_banks)
	{
		m_ula_variant = ula_variant;
		m_ula_delay_sequence = ula_delay_sequence;
		m_cycles_contention_start = cycles_contention_start;
		m_cycles_per_line = cycles_per_line;
		m_cycles_per_frame = cycles_per_frame;
		m_contended_banks = contended_banks;
	}
	auto update_raster_cb() { return m_raster_cb.bind(); }
	bool get_using_raster_callback() { return m_using_raster_callback; }
	int get_tstate_counter() { return m_tstate_counter; }
	void set_selected_bank(int bank) { m_selected_bank = bank; }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void execute_run() override;

	uint8_t in(uint16_t port) override;
	void out(uint16_t port, uint8_t value) override;
	uint8_t rm(uint16_t addr) override;
	void wm(uint16_t addr, uint8_t value) override;
	uint8_t rop() override;
	uint8_t arg() override;
	uint16_t arg16() override;
	void push(PAIR &r) override;
	void jr_cond(bool cond, uint8_t opcode) override;
	void call_cond(bool cond, uint8_t opcode) override;
	void ret_cond(bool cond, uint8_t opcode) override;
	void ldir() override;
	void cpir() override;
	void inir() override;
	void otir() override;
	void lddr() override;
	void cpdr() override;
	void indr() override;
	void otdr() override;

	virtual void eat_cycles(int type, int cycles) override;
	void capture_opcode_history_start(uint16_t reg_bc, uint16_t reg_ir);
	void capture_opcode_history_finish();
	void parse_script(const char *script_desc, CM_SCRIPT_BREAKDOWN *breakdown);
	bool find_script();
	void run_script();
	int get_ula_delay();
	int get_memory_access_delay(uint16_t pc_address);
	void store_rwinfo(uint16_t addr, uint8_t val, uint16_t flags, const char *dbg);

	int m_ula_variant;                   // ULA_VARIANT_SINCLAIR  | ULA_VARIANT_AMSTRAD
	const char*	m_ula_delay_sequence;    // "654321200"           | "10765432"
	const char*	m_contended_banks;       // "1357"                | "4567"
	int m_contended_banks_length;
	CM_SCRIPT m_scripts[MAX_CM_SCRIPTS]; // "pc:4,pc+1:4,ir:1" etc...
	int m_cycles_contention_start;
	int m_cycles_per_line;
	int m_cycles_per_frame;
	devcb_write32 m_raster_cb;           //Let the driver know a good time (in T-States) to update the video raster position.
	bool m_using_raster_callback;
	OPCODE_HISTORY m_opcode_history;     // A list of reads/writes per opcode.
	int m_tstate_counter;                // The current t-state / cpu cycle.
	int m_selected_bank;                 // What ram bank 7ffd port has selected.
};

DECLARE_DEVICE_TYPE(SPECZ80, specz80_device)

#endif // MAME_CPU_Z80_SPECZ80_H
