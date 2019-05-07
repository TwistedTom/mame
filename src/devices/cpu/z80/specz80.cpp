// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/***************************************************************************

    specz80.cpp

    Z80 with ZX spectrum specific support for contended memory.

***************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "z80.h"
#include "z80dasm.h"
#include "z80_common.h"
#include "specz80.h"



/* Uncomment for verbose contended memory debug trace */
//#define DEBUG_CM


/*
dd is any of the registers BC,DE,HL,SP
qq is any of the registers BC,DE,HL,AF
ss is any of the registers BC,DE,HL
ii is either of the index registers IX or IY.
ir is the IR (Interrupt and Refresh) register pair
cc is any (applicable) condition NZ,Z,NC,C,PO,PE,P,M
nn is a 16-bit number
n is an 8-bit number
b is a number from 0 to 7 (BIT/SET/RES instructions)
r and r' are any of the registers A,B,C,D,E,H,L
alo is an arithmetic or logical operation: ADD/ADC/SUB/SBC/AND/XOR/OR and CP
sro is a shift/rotate operation: RLC/RRC/RL/RR/SLA/SRA/SRL and SLL (undocumented)


Instruction	ID	Breakdown:Sinclair			Breakdown:Amstrad
-----------	----	------------------			-----------------
NOP		CM00	pc:4					pc:4
LD r,r'
alo A,r
INC/DEC r
EXX
EX AF,AF'
EX DE,HL
DAA
CPL
CCF
SCF
DI
EI
RLA
RRA
RLCA
RRCA
JP (HL)
HALT

NOPD		CM01	pc:4,pc+1:4				pc:4,pc+1:4
sro r
BIT b,r
SET b,r
RES b,r
NEG
IM 0/1/2

LD A,I		CM02	pc:4,pc+1:4,ir:1			pc:4,pc+1:5
LD A,R
LD I,A
LD R,A

INC/DEC dd	CM03	pc:4,ir:1x2				pc:6
LD SP,HL

ADD HL,dd	CM04	pc:4,ir:1x7				pc:11

ADC HL,dd	CM05	pc:4,pc+1:4,ir:1×7			pc:4,pc+1:11
SBC HL,dd

LD r,n		CM06	pc:4,pc+1:3				pc:4,pc+1:3
alo A,n

LD r,(ss)	CM07	pc:4,ss:3				pc:4,ss:3
LD (ss),r

alo A,(HL)	CM08	pc:4,hl:3				pc:4,hl:3
alo (HL)

LD r,(ii+n)	CM09	pc:4,pc+1:4,pc+2:3,pc+2:1×5,ii+n:3	pc:4,pc+1:4,pc+2:8,ii+n:3
LD (ii+n),r
alo A,(ii+n)
alo (ii+n)

BIT b,(HL)	CM10	pc:4,pc+1:4,hl:3,hl:1			pc:4,pc+1:4,hl:4

BIT b,(ii+n)	CM11	pc:4,pc+1:4,pc+2:3,pc+3:3,pc+3:1×2,	pc:4,pc+1:4,pc+2:3,pc+3:5,ii+n:4
			 ii+n:3,ii+n:1

LD dd,nn	CM12	pc:4,pc+1:3,pc+2:3			pc:4,pc+1:3,pc+2:3
JP nn
JP cc,nn

LD (HL),n	CM13	pc:4,pc+1:3,hl:3			pc:4,pc+1:3,hl:3

LD (ii+n),n	CM14	pc:4,pc+1:4,pc+2:3,pc+3:3,pc+3:1×2,	pc:4,pc+1:4,pc+2:3,pc+3:5.ii+n:3
			 ii+n:3

LD A,(nn)	CM15	pc:4,pc+1:3,pc+2:3,nn:3			pc:4,pc+1:3,pc+2:3,nn:3
LD (nn),A


LD HL,(nn)	CM16	pc:4,pc+1:3,pc+2:3,nn:3,nn+1:3		pc:4,pc+1:3,pc+2:3,nn:3,nn+1:3
LD (nn),HL

LD dd,(nn)	CM17	pc:4,pc+1:4,pc+2:3,pc+3:3,nn:3,nn+1:3	pc:4,pc+1:4,pc+2:3,pc+3:3,nn:3,nn+1:3
LD (nn),dd

INC/DEC (HL)	CM18	pc:4,hl:3,hl:1,hl(w):3			pc:4,hl:4,hl(w):3

SET b,(HL)	CM19	pc:4,pc+1:4,hl:3,hl:1,hl(w):3		pc:4,pc+1:4,hl:4,hl(w):3
RES b,(HL)
sro (HL)

INC/DEC (ii+n)	CM20	pc:4,pc+1:4,pc+2:3,pc+2:1x5,ii+n:3,	pc:4,pc+1:4,pc+2:8,ii+n:4,ii+n(w):3
			 ii+n:1,ii+n(w):3

SET b,(ii+n)	CM21	pc:4,pc+1:4,pc+2:3,pc+3:3,pc+3:1x2,	pc:4,pc+1:4,pc+2:3,pc+3:5,ii+n:4,ii+n(w):3
RES b,(ii+n)		 ii+n:3,ii+n:1,ii+n(w):3
sro (ii+n)

POP dd		CM22	pc:4,sp:3,sp+1:3			pc:4,sp:3,sp+1:3
RET

RETI		CM23	pc:4,pc+1:4,sp:3,sp+1:3			pc:4,pc+1:4,sp:3,sp+1:3
RETN

RET cc		CM24	pc:4,ir:1,[sp:3,sp+1:3]			pc:5,[sp:3,sp+1:3]

PUSH dd		CM25	pc:4,ir:1,sp-1:3,sp-2:3			pc:4,ir:1,sp-1:3,sp-2:3
RST n

CALL nn		CM26	pc:4,pc+1:3,pc+2:3,			pc:4,pc+1:3,pc+2:3,[1,sp-1:3,sp-2:3]
CALL cc,nn		 [pc+2:1,sp-1:3,sp-2:3]

JR n		CM27	pc:4,pc+1:3,[pc+1:1x5]			pc:4,pc+1:3,[5]
JR cc,n

DJNZ n		CM28	pc:4,ir:1,pc+1:3,[pc+1:1x5]		pc:5,pc+1:3,[5]

RLD		CM29	pc:4,pc+1:4,hl:3,hl:1x4,hl(w):3		pc:4,pc+1:4,hl:7,hl(w):3
RRD

IN A,(n)	CM30	pc:4,pc+1:3,IO				pc:4,pc+1:3,IO
OUT (n),A

IN r,(C)	CM31	pc:4,pc+1:4,IO				pc:4,pc+1:4,IO
OUT (C),r

EX (SP),HL	CM32	pc:4,sp:3,sp+1:3,sp+1:1,sp+1(w):3,	pc:4,sp:3,sp+1:4,sp+1(w):3,sp(w):5
			 sp(w):3,sp(w):1×2

LDI/LDIR	CM33	pc:4,pc+1:4,hl:3,de:3,de:1x2,[de:1x5]	pc:4,pc+1:4,hl:3,de:5,[5]
LDD/LDDR

CPI/CPIR	CM34	pc:4,pc+1:4,hl:3,hl:1x5,[hl:1x5]	pc:4,pc+1:4,hl:8,[5]
CPD/CPDR

INI/INIR	CM35	pc:4,pc+1:4,ir:1,IO,hl:3,[hl:1×5]	pc:4,pc+1:5,IO,hl:3,[5]
IND/INDR

OUTI/OTIR	CM36	pc:4,pc+1:4,ir:1,hl:3,IO,[bc:1×5]	pc:4,pc+1:5,hl:3,IO,[5]
OUTD/OTDR
*/


enum ContendedMemoryID
{
	//37 different contended memory scripts (CM00 to CM36)
	CM00,CM01,CM02,CM03,CM04,CM05,CM06,CM07,CM08,CM09,
	CM10,CM11,CM12,CM13,CM14,CM15,CM16,CM17,CM18,CM19,
	CM20,CM21,CM22,CM23,CM24,CM25,CM26,CM27,CM28,CM29,
	CM30,CM31,CM32,CM33,CM34,CM35,CM36,
	OPII, //The opcode that follows the dd or fd should be treated standard opcode (cc_op[]) except it works on the IX or IY register instead.
	ILL1, //The opcode that follows the dd or fd should be treated standard opcode (cc_op[]). These are all undocumented.
	ILL2, //(Undocumented EDXX) Process these the same as CM01 (I could have put CM01 in the cc_ed_contended table instead of ILL2, but I think it was a good idea to be able to see there was a distinction).
	_CB_, //Go to the cc_cb_contended[] table
	_ED_, //Go to the cc_ed_contended[] table.
	_XY_, //Go to the cc_xy_contended[] table (for dd of fd prefixed opcodes)
	XYCB  //Use the cc_xycb_contended[] table (for ddcb or fdcb prefixed opcodes)
};


///////////////////////////////////////////////////////
// Scripts derived from the SinclairFAQ              //
// https://faqwiki.zxnet.co.uk/wiki/Contended_memory //
///////////////////////////////////////////////////////
CM_SCRIPT_DESCRIPTION cm_script_descriptions[MAX_CM_SCRIPTS] =
{
	// ID	  		Sinclair								Amstrad (NULL = same as Sinclair)
	{ /*CM00*/		"0:4",									NULL},
	{ /*CM01*/		"0:4,1:4",								NULL},
	{ /*CM02*/		"0:4,1:4,ir:1",							"0:4,1:5"},
	{ /*CM03*/		"0:4,ir:1x2",							"0:6"},
	{ /*CM04*/		"0:4,ir:1x7",							"0:11"},
	{ /*CM05*/		"0:4,1:4,ir:1x7",						"0:4,1:11"},
	{ /*CM06*/		"0:4,1:3",								NULL},
	{ /*CM07*/		"0:4,1:3",								NULL},
	{ /*CM08*/		"0:4,1:3",								NULL},
	{ /*CM09*/		"0:4,1:4,2:3,2:1x5,3:3",				"0:4,1:4,2:8,3:3"},
	{ /*CM10*/		"0:4,1:4,2:3,2:1",						"0:4,1:4,2:4"},
	{ /*CM11*/		"0:4,1:4,2:3,3:3,3:1x2,4:3,4:1",		"0:4,1:4,2:3,3:5,4:4"},
	{ /*CM12*/		"0:4,1:3,2:3",							NULL},
	{ /*CM13*/		"0:4,1:3,2:3",							NULL},
	{ /*CM14*/		"0:4,1:4,2:3,3:3,3:1x2,4:3",			"0:4,1:4,2:3,3:5,4:3"},
	{ /*CM15*/		"0:4,1:3,2:3,3:3",						NULL},
	{ /*CM16*/		"0:4,1:3,2:3,3:3,4:3",					NULL},
	{ /*CM17*/		"0:4,1:4,2:3,3:3,4:3,5:3",				NULL},
	{ /*CM18*/		"0:4,1:3,1:1,2:3",						"0:4,1:4,2:3"},
	{ /*CM19*/		"0:4,1:4,2:3,2:1,3:3",					"0:4,1:4,2:4,3:3"},
	{ /*CM20*/		"0:4,1:4,2:3,2:1x5,3:3,3:1,4:3",		"0:4,1:4,2:8,3:4,4:3"},
	{ /*CM21*/		"0:4,1:4,2:3,3:3,3:1x2,4:3,4:1,5:3",	"0:4,1:4,2:3,3:5,4:4,5:3"},
	{ /*CM22*/		"0:4,1:3,2:3",							NULL},
	{ /*CM23*/		"0:4,1:4,2:3,3:3",						NULL},
	{ /*CM24*/		"0:4,ir:1,[1:3,2:3]",					"0:5,[1:3,2:3"},
	{ /*CM25*/		"0:4,ir:1,1:3,2:3",						NULL},
	{ /*CM26*/		"0:4,1:3,2:3,[2:1,3:3,4:3]",			"0:4,1:3,2:3,[N:1,3:3,4:3]"},
	{ /*CM27*/		"0:4,1:3,[1:1x5]",						"0:4,1:3,[N:5]"},
	{ /*CM28*/		"0:4,ir:1,1:3,[1:1x5]",					"0:5,1:3,[N:5]"},
	{ /*CM29*/		"0:4,1:4,2:3,2:1x4,3:3",				"0:4,1:4,2:7,3:3"},
	{ /*CM30*/		"0:4,1:3,2:IO",							NULL},
	{ /*CM31*/		"0:4,1:4,2:IO",							NULL},
	{ /*CM32*/		"0:4,1:3,2:3,2:1,3:3,4:3,4:1x2",		"0:4,1:3,2:4,3:3,4:5"},
	{ /*CM33*/		"0:4,1:4,2:3,3:3,3:1x2,[3:1x5]",		"0:4,1:4,2:3,3:5,[N:5]"},
	{ /*CM34*/		"0:4,1:4,2:3,2:1x5,[2:1x5]",			"0:4,1:4,2:8,[N:5]"},
	{ /*CM35*/		"0:4,1:4,ir:1,2:IO,3:3,[3:1x5]",		"0:4,1:5,2:IO,3:3,[N:5]"},
	{ /*CM36*/		"0:4,1:4,ir:1,2:3,3:IO,[bc:1x5]",		"0:4,1:5,2:3,3:IO,[N:5]"}
};


static const int cc_op_contended[0x100] = {
	/*		0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F */
	/* 0 */	CM00,CM12,CM07,CM03,CM00,CM00,CM06,CM00,CM00,CM04,CM07,CM03,CM00,CM00,CM06,CM00,
	/* 1 */	CM28,CM12,CM07,CM03,CM00,CM00,CM06,CM00,CM27,CM04,CM07,CM03,CM00,CM00,CM06,CM00,
	/* 2 */	CM27,CM12,CM16,CM03,CM00,CM00,CM06,CM00,CM27,CM04,CM16,CM03,CM00,CM00,CM06,CM00,
	/* 3 */	CM27,CM12,CM15,CM03,CM18,CM18,CM13,CM00,CM27,CM04,CM15,CM03,CM00,CM00,CM06,CM00,
	/* 4 */	CM00,CM00,CM00,CM00,CM00,CM00,CM07,CM00,CM00,CM00,CM00,CM00,CM00,CM00,CM07,CM00,
	/* 5 */	CM00,CM00,CM00,CM00,CM00,CM00,CM07,CM00,CM00,CM00,CM00,CM00,CM00,CM00,CM07,CM00,
	/* 6 */	CM00,CM00,CM00,CM00,CM00,CM00,CM07,CM00,CM00,CM00,CM00,CM00,CM00,CM00,CM07,CM00,
	/* 7 */	CM07,CM07,CM07,CM07,CM07,CM07,CM00,CM07,CM00,CM00,CM00,CM00,CM00,CM00,CM07,CM00,
	/* 8 */	CM00,CM00,CM00,CM00,CM00,CM00,CM08,CM00,CM00,CM00,CM00,CM00,CM00,CM00,CM08,CM00,
	/* 9 */	CM00,CM00,CM00,CM00,CM00,CM00,CM08,CM00,CM00,CM00,CM00,CM00,CM00,CM00,CM08,CM00,
	/* A */	CM00,CM00,CM00,CM00,CM00,CM00,CM08,CM00,CM00,CM00,CM00,CM00,CM00,CM00,CM08,CM00,
	/* B */	CM00,CM00,CM00,CM00,CM00,CM00,CM08,CM00,CM00,CM00,CM00,CM00,CM00,CM00,CM08,CM00,
	/* C */	CM24,CM22,CM12,CM12,CM26,CM25,CM06,CM25,CM24,CM22,CM12,_CB_,CM26,CM26,CM06,CM25,
	/* D */	CM24,CM22,CM12,CM30,CM26,CM25,CM06,CM25,CM24,CM00,CM12,CM30,CM26,_XY_,CM06,CM25,
	/* E */	CM24,CM22,CM12,CM32,CM26,CM25,CM06,CM25,CM24,CM00,CM12,CM00,CM26,_ED_,CM06,CM25,
	/* F */	CM24,CM22,CM12,CM00,CM26,CM25,CM06,CM25,CM24,CM03,CM12,CM00,CM26,_XY_,CM06,CM25
};

static const int cc_ed_contended[0x100] = {
	/*		0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F */
	/* 0 */	ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,
	/* 1 */	ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,
	/* 2 */	ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,
	/* 3 */	ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,
	/* 4 */	CM31,CM31,CM05,CM17,CM01,CM23,CM01,CM02,CM31,CM31,CM05,CM17,CM01,CM23,CM01,CM02,
	/* 5 */	CM31,CM31,CM05,CM17,CM01,CM23,CM01,CM02,CM31,CM31,CM05,CM17,CM01,CM23,CM01,CM02,
	/* 6 */	CM31,CM31,CM05,CM17,CM01,CM23,CM01,CM29,CM31,CM31,CM05,CM17,CM01,CM23,CM01,CM29,
	/* 7 */	CM31,CM31,CM05,CM17,CM01,CM23,CM01,ILL2,CM31,CM31,CM05,CM17,CM01,CM23,CM01,ILL2,
	/* 8 */	ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,
	/* 9 */	ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,
	/* A */	CM33,CM34,CM35,CM36,ILL2,ILL2,ILL2,ILL2,CM33,CM34,CM35,CM36,ILL2,ILL2,ILL2,ILL2,
	/* B */	CM33,CM34,CM35,CM36,ILL2,ILL2,ILL2,ILL2,CM33,CM34,CM35,CM36,ILL2,ILL2,ILL2,ILL2,
	/* C */	ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,
	/* D */	ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,
	/* E */	ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,
	/* F */	ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2,ILL2
};

static const int cc_cb_contended[0x100] = {
	/*		0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F */
	/* 0 */	CM01,CM01,CM01,CM01,CM01,CM01,CM19,CM01,CM01,CM01,CM01,CM01,CM01,CM01,CM19,CM01,
	/* 1 */	CM01,CM01,CM01,CM01,CM01,CM01,CM19,CM01,CM01,CM01,CM01,CM01,CM01,CM01,CM19,CM01,
	/* 2 */	CM01,CM01,CM01,CM01,CM01,CM01,CM19,CM01,CM01,CM01,CM01,CM01,CM01,CM01,CM19,CM01,
	/* 3 */	CM01,CM01,CM01,CM01,CM01,CM01,CM19,CM01,CM01,CM01,CM01,CM01,CM01,CM01,CM19,CM01,
	/* 4 */	CM01,CM01,CM01,CM01,CM01,CM01,CM10,CM01,CM01,CM01,CM01,CM01,CM01,CM01,CM10,CM01,
	/* 5 */	CM01,CM01,CM01,CM01,CM01,CM01,CM10,CM01,CM01,CM01,CM01,CM01,CM01,CM01,CM10,CM01,
	/* 6 */	CM01,CM01,CM01,CM01,CM01,CM01,CM10,CM01,CM01,CM01,CM01,CM01,CM01,CM01,CM10,CM01,
	/* 7 */	CM01,CM01,CM01,CM01,CM01,CM01,CM10,CM01,CM01,CM01,CM01,CM01,CM01,CM01,CM10,CM01,
	/* 8 */	CM01,CM01,CM01,CM01,CM01,CM01,CM19,CM01,CM01,CM01,CM01,CM01,CM01,CM01,CM19,CM01,
	/* 9 */	CM01,CM01,CM01,CM01,CM01,CM01,CM19,CM01,CM01,CM01,CM01,CM01,CM01,CM01,CM19,CM01,
	/* A */	CM01,CM01,CM01,CM01,CM01,CM01,CM19,CM01,CM01,CM01,CM01,CM01,CM01,CM01,CM19,CM01,
	/* B */	CM01,CM01,CM01,CM01,CM01,CM01,CM19,CM01,CM01,CM01,CM01,CM01,CM01,CM01,CM19,CM01,
	/* C */	CM01,CM01,CM01,CM01,CM01,CM01,CM19,CM01,CM01,CM01,CM01,CM01,CM01,CM01,CM19,CM01,
	/* D */	CM01,CM01,CM01,CM01,CM01,CM01,CM19,CM01,CM01,CM01,CM01,CM01,CM01,CM01,CM19,CM01,
	/* E */	CM01,CM01,CM01,CM01,CM01,CM01,CM19,CM01,CM01,CM01,CM01,CM01,CM01,CM01,CM19,CM01,
	/* F */	CM01,CM01,CM01,CM01,CM01,CM01,CM19,CM01,CM01,CM01,CM01,CM01,CM01,CM01,CM19,CM01,
};


//This is for opcodes that start with dd or fd (Saves us having a cc_dd_contended table and a
//cc_fd_contended table, which would both be identical). If the byte following the
//dd/fd falls on OPII or ILL1, then the opcode is processed by cc_op_contended[] + 4 tstates for
//reading the dd/fd.
static const int cc_xy_contended[0x100] = {
	/*		0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F */
	/* 0 */	ILL1,ILL1,ILL1,ILL1,ILL1,ILL1,ILL1,ILL1,ILL1,OPII,ILL1,ILL1,ILL1,ILL1,ILL1,ILL1,
	/* 1 */	ILL1,ILL1,ILL1,ILL1,ILL1,ILL1,ILL1,ILL1,ILL1,OPII,ILL1,ILL1,ILL1,ILL1,ILL1,ILL1,
	/* 2 */	ILL1,OPII,OPII,OPII,OPII,OPII,OPII,ILL1,ILL1,OPII,OPII,OPII,OPII,OPII,OPII,ILL1,
	/* 3 */	ILL1,ILL1,ILL1,ILL1,CM20,CM20,CM14,ILL1,ILL1,OPII,ILL1,ILL1,ILL1,ILL1,ILL1,ILL1,
	/* 4 */	ILL1,ILL1,ILL1,ILL1,OPII,OPII,CM09,ILL1,ILL1,ILL1,ILL1,ILL1,OPII,OPII,CM09,ILL1,
	/* 5 */	ILL1,ILL1,ILL1,ILL1,OPII,OPII,CM09,ILL1,ILL1,ILL1,ILL1,ILL1,OPII,OPII,CM09,ILL1,
	/* 6 */	OPII,OPII,OPII,OPII,OPII,OPII,CM09,OPII,OPII,OPII,OPII,OPII,OPII,OPII,CM09,OPII,
	/* 7 */	CM09,CM09,CM09,CM09,CM09,CM09,ILL1,CM09,ILL1,ILL1,ILL1,ILL1,OPII,OPII,CM09,ILL1,
	/* 8 */	ILL1,ILL1,ILL1,ILL1,OPII,OPII,CM09,ILL1,ILL1,ILL1,ILL1,ILL1,OPII,OPII,CM09,ILL1,
	/* 9 */	ILL1,ILL1,ILL1,ILL1,OPII,OPII,CM09,ILL1,ILL1,ILL1,ILL1,ILL1,OPII,OPII,CM09,ILL1,
	/* A */	ILL1,ILL1,ILL1,ILL1,OPII,OPII,CM09,ILL1,ILL1,ILL1,ILL1,ILL1,OPII,OPII,CM09,ILL1,
	/* B */	ILL1,ILL1,ILL1,ILL1,OPII,OPII,CM09,ILL1,ILL1,ILL1,ILL1,ILL1,OPII,OPII,CM09,ILL1,
	/* C */	ILL1,ILL1,ILL1,ILL1,ILL1,ILL1,ILL1,ILL1,ILL1,ILL1,ILL1,XYCB,ILL1,ILL1,ILL1,ILL1,
	/* D */	ILL1,ILL1,ILL1,ILL1,ILL1,ILL1,ILL1,ILL1,ILL1,ILL1,ILL1,ILL1,ILL1,ILL1,ILL1,ILL1,
	/* E */	ILL1,OPII,ILL1,OPII,ILL1,OPII,ILL1,ILL1,ILL1,OPII,ILL1,ILL1,ILL1,ILL1,ILL1,ILL1,
	/* F */	ILL1,ILL1,ILL1,ILL1,ILL1,ILL1,ILL1,ILL1,ILL1,OPII,ILL1,ILL1,ILL1,ILL1,ILL1,ILL1
};


//This is for opcodes that start with ddcb or fdcb (Saves us having a cc_ddcb_contended table and a
//cc_fdcb_contended table, which would both be identical).
static const int cc_xycb_contended[0x100] = {
	/*		0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F */
	/* 0 */	CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,
	/* 1 */	CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,
	/* 2 */	CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,
	/* 3 */	CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,
	/* 4 */	CM11,CM11,CM11,CM11,CM11,CM11,CM11,CM11,CM11,CM11,CM11,CM11,CM11,CM11,CM11,CM11,
	/* 5 */	CM11,CM11,CM11,CM11,CM11,CM11,CM11,CM11,CM11,CM11,CM11,CM11,CM11,CM11,CM11,CM11,
	/* 6 */	CM11,CM11,CM11,CM11,CM11,CM11,CM11,CM11,CM11,CM11,CM11,CM11,CM11,CM11,CM11,CM11,
	/* 7 */	CM11,CM11,CM11,CM11,CM11,CM11,CM11,CM11,CM11,CM11,CM11,CM11,CM11,CM11,CM11,CM11,
	/* 8 */	CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,
	/* 9 */	CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,
	/* A */	CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,
	/* B */	CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,
	/* C */	CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,
	/* D */	CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,
	/* E */	CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,
	/* F */	CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21,CM21
};


/***************************************************************
 * Input a byte from given I/O port
 ***************************************************************/
inline uint8_t specz80_device::in(uint16_t port)
{
	// For floating bus support, the read_byte triggers the
	// 'spectrum_port_ula_r' callback which will require the tstate
	// counter to be up-to-date.
	store_rwinfo(port, -1, RWINFO_READ|RWINFO_IO_PORT, "in port");
	return m_io->read_byte(port);
}

/***************************************************************
 * Output a byte to given I/O port
 ***************************************************************/
inline void specz80_device::out(uint16_t port, uint8_t value)
{
	store_rwinfo(port, value, RWINFO_WRITE|RWINFO_IO_PORT, "out port");
	m_io->write_byte(port, value);
}

/***************************************************************
 * Read a byte from given memory location
 ***************************************************************/
inline uint8_t specz80_device::rm(uint16_t addr)
{
	uint8_t res = m_program->read_byte(addr);
	store_rwinfo(addr, res, RWINFO_READ|RWINFO_MEMORY, "rm");
	return res;
}

/***************************************************************
 * Write a byte to given memory location
 ***************************************************************/
inline void specz80_device::wm(uint16_t addr, uint8_t value)
{
	store_rwinfo(addr, value, RWINFO_WRITE|RWINFO_MEMORY, "wm");
	m_program->write_byte(addr, value);
}


/***************************************************************
 * rop() is identical to rm() except it is used for
 * reading opcodes. In case of system with memory mapped I/O,
 * this function can be used to greatly speed up emulation
 ***************************************************************/
inline uint8_t specz80_device::rop()
{
	unsigned pc = PCD;
	PC++;
	uint8_t res = m_opcodes_cache->read_byte(pc);

	m_icount -= 2;
	m_refresh_cb((m_i << 8) | (m_r2 & 0x80) | ((m_r-1) & 0x7f), 0x00, 0xff);
	m_icount += 2;

	store_rwinfo(((PAIR*)(&pc))->w.l, res, RWINFO_READ|RWINFO_MEMORY, "rop");
	return res;
}

/****************************************************************
 * arg() is identical to rop() except it is used
 * for reading opcode arguments. This difference can be used to
 * support systems that use different encoding mechanisms for
 * opcodes and opcode arguments
 ***************************************************************/
inline uint8_t specz80_device::arg()
{
	unsigned pc = PCD;
	uint8_t res;
	PC++;

	res = m_cache->read_byte(pc);
	store_rwinfo(((PAIR*)(&pc))->w.l, res, RWINFO_READ|RWINFO_MEMORY, "arg");

	return res;
}

inline uint16_t specz80_device::arg16()
{
	unsigned pc1 = PCD;
	unsigned pc2 = ((pc1+1)&0xffff);
	uint8_t res1, res2;
	PC += 2;

	res1 = m_cache->read_byte(pc1);
	store_rwinfo(((PAIR*)(&pc1))->w.l, res1, RWINFO_READ|RWINFO_MEMORY, "arg16 byte1");
	res2 = m_cache->read_byte(pc2);
	store_rwinfo(((PAIR*)(&pc2))->w.l, res2, RWINFO_READ|RWINFO_MEMORY, "arg16 byte2");

	return res1 | (res2 << 8);
}

/***************************************************************
 * PUSH
 ***************************************************************/
inline void specz80_device::push(PAIR &r)
{
	//For the benefit of contended memory, ensure these writes are
	//carried out in the order that they would be on the hardware.
	SP--;
	wm(SPD, r.b.h);
	SP--;
	wm(SPD, r.b.l);
}

/***************************************************************
 * JR_COND
 ***************************************************************/
inline void specz80_device::jr_cond(bool cond, uint8_t opcode)
{
	if (cond)
	{
		CC(ex, opcode);
		m_opcode_history.do_optional = true;
		run_script();

		jr();
	}
	else
	{
		//Need to read another byte for contended memory support.
		(int8_t)arg();
	}
}

/***************************************************************
 * CALL_COND
 ***************************************************************/
inline void specz80_device::call_cond(bool cond, uint8_t opcode)
{
	if (cond)
	{
		CC(ex, opcode);
		m_opcode_history.do_optional = true;
		run_script();

		m_ea = arg16();
		WZ = m_ea;
		push(m_pc);
		PCD = m_ea;
	}
	else
	{
		WZ = arg16(); /* implicit call PC+=2; */
	}
}

/***************************************************************
 * RET_COND
 ***************************************************************/
inline void specz80_device::ret_cond(bool cond, uint8_t opcode)
{
	if (cond)
	{
		CC(ex, opcode);
		m_opcode_history.do_optional = true;
		run_script();

		pop(m_pc);
		WZ = PC;
	}
}

/***************************************************************
 * LDIR
 ***************************************************************/
inline void specz80_device::ldir()
{
	ldi();
	if (BC != 0)
	{
		CC(ex, 0xb0);
		m_opcode_history.do_optional = true;
		run_script();

		PC -= 2;
		WZ = PC + 1;
	}
}

/***************************************************************
 * CPIR
 ***************************************************************/
inline void specz80_device::cpir()
{
	cpi();
	if (BC != 0 && !(F & ZF))
	{
		CC(ex, 0xb1);
		m_opcode_history.do_optional = true;
		run_script();

		PC -= 2;
		WZ = PC + 1;
	}
}

/***************************************************************
 * INIR
 ***************************************************************/
inline void specz80_device::inir()
{
	ini();
	if (B != 0)
	{
		CC(ex, 0xb2);
		m_opcode_history.do_optional = true;
		run_script();

		PC -= 2;
	}
}

/***************************************************************
 * OTIR
 ***************************************************************/
inline void specz80_device::otir()
{
	outi();
	if (B != 0)
	{
		CC(ex, 0xb3);
		m_opcode_history.do_optional = true;
		run_script();

		PC -= 2;
	}
}

/***************************************************************
 * LDDR
 ***************************************************************/
inline void specz80_device::lddr()
{
	ldd();
	if (BC != 0)
	{
		CC(ex, 0xb8);
		m_opcode_history.do_optional = true;
		run_script();

		PC -= 2;
		WZ = PC + 1;
	}
}

/***************************************************************
 * CPDR
 ***************************************************************/
inline void specz80_device::cpdr()
{
	cpd();
	if (BC != 0 && !(F & ZF))
	{
		CC(ex, 0xb9);
		m_opcode_history.do_optional = true;
		run_script();

		PC -= 2;
		WZ = PC + 1;
	}
}

/***************************************************************
 * INDR
 ***************************************************************/
inline void specz80_device::indr()
{
	ind();
	if (B != 0)
	{
		CC(ex, 0xba);
		m_opcode_history.do_optional = true;
		run_script();

		PC -= 2;
	}
}

/***************************************************************
 * OTDR
 ***************************************************************/
inline void specz80_device::otdr()
{
	outd();
	if (B != 0)
	{
		CC(ex, 0xbb);
		m_opcode_history.do_optional = true;
		run_script();

		PC -= 2;
	}
}


/****************************************************************************
 * Processor initialization
 ****************************************************************************/
void specz80_device::device_start()
{
	const char *default_pattern = "00000000";
	int i;

	z80_device::device_start();

	for(i=0; i<MAX_CM_SCRIPTS; i++)
	{
		//Create 'sinclair' variant breakdown tables by default.
		m_scripts[i].id = i;
		m_scripts[i].desc = cm_script_descriptions[i].sinclair;

		if((m_ula_variant == ULA_VARIANT_AMSTRAD) && (cm_script_descriptions[i].amstrad != NULL))
			m_scripts[i].desc = cm_script_descriptions[i].amstrad;

		parse_script(m_scripts[i].desc, &(m_scripts[i].breakdown));
	}

	//Find out if the driver wants to use the raster callback before it has been 'resolved'.
	//Note. Once it is resolved, isnull() will always be false, thus it will not be
	//possible to tell if  MCFG_Z80_CFG_CONTENDED_MEMORY set it to DEVCB_NOOP or not.
	if(!m_raster_cb.isnull())
	{
		m_using_raster_callback = true;
	}
	else
	{
		//Either it has been set to DEVCB_NOOP or it has not been set at all.
		m_using_raster_callback = false;
	}
	m_raster_cb.resolve_safe();


	//Check the ula_delay_sequence.... (Set it to "00000000" if "" has been set)
	if(m_ula_delay_sequence == NULL) m_ula_delay_sequence = default_pattern;
	if(strlen(m_ula_delay_sequence) == 0) m_ula_delay_sequence = default_pattern;
	if(strlen(m_ula_delay_sequence) != 8)
	{
		logerror("ULA delay sequence length must be 8 numeric digits in length (%s)\n", m_ula_delay_sequence);
		assert_always(false, "Bad ULA delay sequence length");
	}
	for(i=0; i<8; i++)
	{
		if((m_ula_delay_sequence[i] < '0') || (m_ula_delay_sequence[i] > '9'))
		{
			logerror("Bad character '%c' in ULA delay sequence (%s)\n", m_ula_delay_sequence[i], m_ula_delay_sequence);
			assert_always(false, "Bad character in ULA delay sequence");
		}
	}

	//Check contended banks...
	if(m_contended_banks == NULL) m_contended_banks = "";
	m_contended_banks_length = strlen(m_contended_banks);
	for(i=0; i<m_contended_banks_length; i++)
	{
		if((m_contended_banks[i] < '0') || (m_contended_banks[i] > '9'))
		{
			logerror("Bad character '%c' in contended banks (%s)\n", m_contended_banks[i], m_contended_banks);
			assert_always(false, "Bad character in contended banks string");
		}
	}

	memset(&m_opcode_history, 0, sizeof(OPCODE_HISTORY));
	m_opcode_history.capturing = false;
	m_tstate_counter = 0;
	m_selected_bank = 0;

#ifdef DEBUG_CM
	printf("m_ula_variant = %d\n", m_ula_variant);
	printf("m_ula_delay_sequence = \"%s\"\n", m_ula_delay_sequence);
	printf("m_contended_banks = \"%s\"\n",m_contended_banks);
	printf("m_contended_banks_length = %d\n", m_contended_banks_length);
	printf("m_cycles_contention_start = %d\n",m_cycles_contention_start);
	printf("m_cycles_per_line = %d\n",m_cycles_per_line);
	printf("m_cycles_per_frame = %d\n",m_cycles_per_frame);
	printf("m_using_raster_callback = %d\n", m_using_raster_callback);
#endif
}


/****************************************************************************
 * Execute 'cycles' T-states.
 ****************************************************************************/
void specz80_device::execute_run()
{
	do
	{
		if (m_wait_state)
		{
			// stalled
			m_icount = 0;
			return;
		}

		// check for interrupts before each instruction
		if (m_nmi_pending)
			take_nmi();
		else if (m_irq_state != CLEAR_LINE && m_iff1 && !m_after_ei)
			take_interrupt();

		m_after_ei = false;
		m_after_ldair = false;

		PRVPC = PCD;
#ifdef DEBUG_CM
		printf("execute_run - TState=%d, LastOpcodeTstates=%d PC=%04X\n", m_tstate_counter, m_tstate_counter-m_opcode_history.tstate_start, PCD);
#endif
		debugger_instruction_hook(PCD);
		m_r++;

		if(m_ula_variant == ULA_VARIANT_NONE)
		{
			EXEC(op,rop());
		}
		else
		{
			capture_opcode_history_start((uint16_t)BC,(uint16_t)(m_i << 8));
			EXEC(op,rop());
			capture_opcode_history_finish();
		}

		// Elimiates sprite flicker on various games (E.g. Marauder and
		// Stormlord) and makes Firefly playable.
		m_raster_cb(m_tstate_counter);

	} while (m_icount > 0);
}


/**************************************************************************
 * Generic set_info
 **************************************************************************/
specz80_device::specz80_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	z80_device(mconfig, SPECZ80, tag, owner, clock),
	m_raster_cb(*this)
{
	m_ula_variant = ULA_VARIANT_NONE;
	m_ula_delay_sequence = "";
	m_contended_banks = "";
	m_using_raster_callback = false;
}

DEFINE_DEVICE_TYPE(SPECZ80, specz80_device, "Zilog specz80", "Zilog Z80 with contended memory support")


/**************************************************************************
 * Contended Memory Functions
 **************************************************************************/
void specz80_device::capture_opcode_history_start(uint16_t reg_bc, uint16_t reg_ir)
{
	m_opcode_history.rw_count = 0;
	m_opcode_history.tstate_start = m_tstate_counter;
	m_opcode_history.register_bc = reg_bc;
	m_opcode_history.register_ir = reg_ir;
	m_opcode_history.uncontended_cycles_predicted = 0;
	m_opcode_history.uncontended_cycles_eaten = 0;

	//We'll find out sometime during the opcode's execution whether to process the
	//optional script elements or not.
	m_opcode_history.do_optional = false; //Just process mandatory script elements (for now).

	//Reset the script position
	m_opcode_history.script = NULL;
	m_opcode_history.breakdown = NULL;
	m_opcode_history.element = 0;

	//And we are good to go!
	m_opcode_history.capturing = true;
}

void specz80_device::capture_opcode_history_finish()
{
	int i;
	bool success = true;

	//Stops us capturing rwinfo when we are processing interrupts
	m_opcode_history.capturing = false;

	if (m_opcode_history.uncontended_cycles_predicted != m_opcode_history.uncontended_cycles_eaten)
	{
		logerror("Wrong amount of uncontended cycles eaten (predicted=%d eaten=%d)\n", m_opcode_history.uncontended_cycles_predicted, m_opcode_history.uncontended_cycles_eaten);
		success = false;
	}

	//All the reads and writes we did for the last instruction should have been
	//processed for contention.
	for(i=0; i<m_opcode_history.rw_count; i++)
	{
		if(!(m_opcode_history.rw[i].flags & RWINFO_PROCESSED))
		{
			logerror("RWINFO %d not processed for contention\n", i);
			success = false;
			break;
		}
	}

	if(!success)
	{
		if(m_opcode_history.script == NULL)
		{
			logerror("Contended Memory Script Unknown\n");
		}
		else
		{
			logerror("Contended Memory Script CM%02d breakdown=%s\n", m_opcode_history.script->id, m_opcode_history.script->desc);
		}

		logerror("Last Opcode History:\n");
		for(i=0; i<m_opcode_history.rw_count; i++)
		{
			logerror(" [%d] addr=0x%04X val=0x%02X (%s, %s, Processed=%s dbg=%s)\n",
					i,
					m_opcode_history.rw[i].addr,
					m_opcode_history.rw[i].val,
					m_opcode_history.rw[i].flags & RWINFO_READ     ? "Read" : "Write",
					m_opcode_history.rw[i].flags & RWINFO_IO_PORT   ? "IO"  : "Addr",
					m_opcode_history.rw[i].flags & RWINFO_PROCESSED ? "Y"   : "N",
					m_opcode_history.rw[i].dbg);
		}
		assert_always(false, "Failed to process opcode history");
	}
}


bool specz80_device::find_script()
{
	if(m_opcode_history.script == NULL)
	{
		RWINFO *rw = m_opcode_history.rw;
		int contended_type = -1;

		if(m_opcode_history.rw_count <= 0) return false;
		contended_type = cc_op_contended[rw[0].val];

		if (contended_type == _CB_)
		{
			//CB** opcodes...
			if(m_opcode_history.rw_count <= 1) return false;
			contended_type = cc_cb_contended[rw[1].val];
			m_opcode_history.script = &m_scripts[contended_type];
			m_opcode_history.breakdown = &(m_opcode_history.script->breakdown);
		}
		else if(contended_type == _ED_)
		{
			//ED** opcodes...
			if(m_opcode_history.rw_count <= 1) return false;
			contended_type = cc_ed_contended[rw[1].val];
			if(contended_type == ILL2) contended_type = CM01; //Illegal_2 ED** opcodes can be processed using the CM01 script
			m_opcode_history.script = &m_scripts[contended_type];
			m_opcode_history.breakdown = &(m_opcode_history.script->breakdown);
		}
		else if(contended_type == _XY_)
		{
			//DD** or FD** opcodes...
			if(m_opcode_history.rw_count <= 1) return false;
			contended_type = cc_xy_contended[rw[1].val];

			if(contended_type == XYCB)
			{
				 //DDCB**** opcodes or FDCB**** opcodes
				if(m_opcode_history.rw_count <= 3) return false;
				contended_type = cc_xycb_contended[rw[3].val];
				m_opcode_history.script = &m_scripts[contended_type];
				m_opcode_history.breakdown = &(m_opcode_history.script->breakdown);
			}
			else if((contended_type == OPII) || (contended_type == ILL1))
			{
				//Run one of the bog standard opcodes, but with an IX/IY shift.
				eat_cycles(CYCLES_CONTENDED, get_memory_access_delay(rw[0].addr));
				eat_cycles(CYCLES_UNCONTENDED, 4);
				rw[0] = rw[1];
				m_opcode_history.rw_count--;
				return find_script();
			}
			else
			{
				m_opcode_history.script = &(m_scripts[contended_type]);
				m_opcode_history.breakdown = &(m_opcode_history.script->breakdown);
			}
		}
		else
		{
			m_opcode_history.script = &(m_scripts[contended_type]);
			m_opcode_history.breakdown = &(m_opcode_history.script->breakdown);
			if((rw[0].val == 0xCD) || (rw[0].val == 0x18))
			{
				//0xCD ("call **", CM26) and 0x18 ("jz *", CM27) are non-optional
				//but they all its optional elements need processing
				m_opcode_history.do_optional = true;
			}
		}
	}

	return true; //Good to go!
}


void specz80_device::run_script()
{
	if(!m_opcode_history.capturing) return;

	if(!find_script())
	{
		//Stop here. We don't know what script this opcode uses (yet).
		return;
	}

	for(; m_opcode_history.element < m_opcode_history.breakdown->number_of_elements; m_opcode_history.element++)
	{
		CMSE *se = &(m_opcode_history.breakdown->elements[m_opcode_history.element]);
		RWINFO *rw;
		int mult;

		if(se->rw_ix >= m_opcode_history.rw_count)
		{
			//Stop here. We don't have the rwinfo stored that will be required to process this script element (yet).
			return;
		}

		//Check if this is an optional element.
		if(se->is_optional && !m_opcode_history.do_optional)
		{
			//Stop here. This is an optional script element and we are not sure (yet)
	                //if this opcode will need to go down the optional path.
			return;
		}


		if(se->type == CMSE_TYPE_IO_PORT) //***Process PORT contention***
		{
			bool high_byte = false;
			bool low_bit = false;

			rw = &(m_opcode_history.rw[se->rw_ix]);
			rw->flags |= RWINFO_PROCESSED;

			if(m_ula_variant == ULA_VARIANT_AMSTRAD)
			{
				//For +2a/+3, just eat 4 uncontended cycles for IO operations.
				//Note. According to https://faqwiki.zxnet.co.uk/wiki/Contended_I/O
				//On the +2a/+3 Spectrum, the ULA applies contention only when the
				//Z80's MREQ line is active, which it is not during an I/O operation.
#ifdef DEBUG_CM
				printf("  IO  TState=%d ULA=%d - Addr=%04X Val=%02X (C:0, N:4)\n",
					m_tstate_counter,
					get_ula_delay(),
					m_opcode_history.rw[se->rw_ix].addr,
					m_opcode_history.rw[se->rw_ix].val);
#endif //DEBUG_CM
				eat_cycles(CYCLES_UNCONTENDED, 4);
			}
			else //ULA_VARIANT_SINCLAIR
			{
				/*
				High byte   |         |
				in 40 - 7F? | Low bit | Contention pattern
				------------+---------+-------------------
				No          |  Reset  | N:1, C:3
				No          |   Set   | N:4
				Yes         |  Reset  | C:1, C:3
				Yes         |   Set   | C:1, C:1, C:1, C:1
				*/
				if ((rw->addr >= 0x4000) && (rw->addr <= 0x7fff))
				{
					high_byte  = true;
				}

				if(rw->addr & 0x0001)
				{
					low_bit = true;
				}

#ifdef DEBUG_CM
				printf("  IO  TState=%d ULA=%d - Addr=%04X (HiByte=%d LoBit=%d)\n",
						m_tstate_counter,
						get_ula_delay(),
						rw->addr,
						high_byte,
						low_bit);
#endif //DEBUG_CM

				if((high_byte == false) && (low_bit == false))
				{
					//N:1, C:3
					eat_cycles(CYCLES_UNCONTENDED, 1);
					eat_cycles(CYCLES_CONTENDED, get_ula_delay());
					eat_cycles(CYCLES_UNCONTENDED, 3);
				}
				else if((high_byte == false) && (low_bit == true))
				{
					//N:4
					eat_cycles(CYCLES_UNCONTENDED, 4);
				}
				else if((high_byte == true) && (low_bit == false))
				{
					//C:1, C:3
					eat_cycles(CYCLES_CONTENDED, get_ula_delay());
					eat_cycles(CYCLES_UNCONTENDED, 1);
					eat_cycles(CYCLES_CONTENDED, get_ula_delay());
					eat_cycles(CYCLES_UNCONTENDED, 3);
				}
				else //((high_byte == true) && (low_bit == true))
				{
					//C:1, C:1, C:1, C:1
					eat_cycles(CYCLES_CONTENDED, get_ula_delay());
					eat_cycles(CYCLES_UNCONTENDED, 1);
					eat_cycles(CYCLES_CONTENDED, get_ula_delay());
					eat_cycles(CYCLES_UNCONTENDED, 1);
					eat_cycles(CYCLES_CONTENDED, get_ula_delay());
					eat_cycles(CYCLES_UNCONTENDED, 1);
					eat_cycles(CYCLES_CONTENDED, get_ula_delay());
					eat_cycles(CYCLES_UNCONTENDED, 1);
				}
			}
		}
		else if(se->type == CMSE_TYPE_MEMORY) //***Process Memory Address Contention***
		{
			rw = &(m_opcode_history.rw[se->rw_ix]);
			rw->flags |= RWINFO_PROCESSED;

			for(mult=0;mult<se->multiplier;mult++)
			{
#ifdef DEBUG_CM
				printf("  MEM TState=%d ULA=%d - Addr=%04X Val=%02X (C:%d, N:%d)\n",
					m_tstate_counter,
					get_ula_delay(),
					m_opcode_history.rw[se->rw_ix].addr,
					m_opcode_history.rw[se->rw_ix].val,
					get_memory_access_delay(m_opcode_history.rw[se->rw_ix].addr),
					se->inst_cycles);
#endif //DEBUG_CM

				eat_cycles(CYCLES_CONTENDED, get_memory_access_delay(m_opcode_history.rw[se->rw_ix].addr));
				eat_cycles(CYCLES_UNCONTENDED, se->inst_cycles);
			}
		}
		else if(se->type == CMSE_TYPE_IR_REGISTER) //***Process IR Register Pair contention***
		{
			for(mult=0;mult<se->multiplier;mult++)
			{
#ifdef DEBUG_CM
				printf("  IR  TState=%d ULA=%d - Addr=%04X        (C:%d, N:%d)\n",
					m_tstate_counter,
					get_ula_delay(),
					m_opcode_history.register_ir,
					get_memory_access_delay(m_opcode_history.register_ir),
					se->inst_cycles);
#endif //DEBUG_CM

				eat_cycles(CYCLES_CONTENDED, get_memory_access_delay(m_opcode_history.register_ir));
				eat_cycles(CYCLES_UNCONTENDED, se->inst_cycles);
			}
		}
		else if(se->type == CMSE_TYPE_BC_REGISTER) //***Process BC Register Pair contention***
		{
			for(mult=0;mult<se->multiplier;mult++)
			{
#ifdef DEBUG_CM
				printf("  BC  TState=%d ULA=%d - Addr=%04X        (C:%d, N:%d)\n",
					m_tstate_counter,
					get_ula_delay(),
					m_opcode_history.register_bc,
					get_memory_access_delay(m_opcode_history.register_bc),
					se->inst_cycles);
#endif //DEBUG_CM

				eat_cycles(CYCLES_CONTENDED, get_memory_access_delay(m_opcode_history.register_bc));
				eat_cycles(CYCLES_UNCONTENDED, se->inst_cycles);
			}
		}
		else if(se->type == CMSE_TYPE_UNCONTENDED) //***Process uncontended cycles***
		{
			for(mult=0;mult<se->multiplier;mult++)
			{
#ifdef DEBUG_CM
				printf("  N   TState=%d ULA=%d -                       (N:%d)\n",
					m_tstate_counter,
					get_ula_delay(),
					se->inst_cycles);
#endif //DEBUG_CM
				eat_cycles(CYCLES_UNCONTENDED, se->inst_cycles);
			}
		}
		else
		{
			logerror("Unknown CMSE type 0x%X\n", se->type);
			assert_always(false, "Uknown element type in contented memory script");
		}
	}
}


void specz80_device::parse_script(const char *script, CM_SCRIPT_BREAKDOWN *breakdown)
{
	enum CMSE_STATE
	{
		CMSE_GET_INDEX,
		CMSE_SAVED_INDEX,
		CMSE_GET_CYCLES,
		CMSE_SAVED_CYCLES,
		CMSE_GET_MULTIPLIER,
		CMSE_SAVED_MULTIPLIER,
	};
	int state = CMSE_GET_INDEX;
	bool finished = false;
	bool is_optional = false;
	int pos = 0;
	int ch = '?';
	int ch2= '?';
	int *cycles_mandatory = &(breakdown->inst_cycles_mandatory);
	int *cycles_optional = &(breakdown->inst_cycles_optional);
	int  *count = &(breakdown->number_of_elements);
	CMSE *element = &(breakdown->elements[0]);


	memset(breakdown, 0, sizeof(CM_SCRIPT_BREAKDOWN));

	while (!finished)
	{
		ch  = script[pos];
		if(ch == '\0') ch2 = '\0';
		else ch2 = script[pos+1];

		if(ch == ' ')
		{
			//ignore
		}
		else if((ch == '\0') || (ch == ',') || (ch == ']'))
		{
			//Save the last entry and move onto the next
			if((state == CMSE_SAVED_CYCLES) || (state == CMSE_SAVED_MULTIPLIER))
			{
				int total_cycles = element[*count].inst_cycles * element[*count].multiplier;

				if(is_optional == false)
				{
					*cycles_mandatory += total_cycles;
				}
				else
				{
					*cycles_optional += total_cycles;
				}

				(*count)++; //Save the last entry
				state = CMSE_GET_INDEX; //onto the next
			}

			if (ch == ']') is_optional = false;
			else if (ch == '\0') finished = true;
		}
		else if(ch == '[')
		{
			is_optional = true;
		}
		else if(ch == ':')
		{
			state = CMSE_GET_CYCLES;
		}
		else if(ch == 'x')
		{
			state = CMSE_GET_MULTIPLIER;
		}
		else if((ch == 'I') && (ch2 == 'O'))
		{
			pos++;
			if(state == CMSE_GET_CYCLES)
			{
				//It takes a minimum of 4 Tstates for the Z80 to read a value
				//from an I/O port, or write a value to a port. As is the case with
				//memory access, this can be lengthened by the ULA.
				element[*count].inst_cycles = 4;
				element[*count].type = CMSE_TYPE_IO_PORT;
				state = CMSE_SAVED_CYCLES;
			}
			else break; //Failure!
		}
		else if((ch == 'i') && (ch2 == 'r'))
		{
			pos++;
			if(state == CMSE_GET_INDEX)
			{
				if( *count >= MAX_CMSE)
				{
					assert_always(false, "Too many script elements (Parsing 'ir')");
				}

				element[*count].rw_ix = -1;
				element[*count].is_optional = is_optional;
				element[*count].multiplier = 1;
				element[*count].type = CMSE_TYPE_IR_REGISTER;
				state = CMSE_SAVED_INDEX;
			}
			else break; //Failure!
		}
		else if((ch == 'b') && (ch2 == 'c'))
		{
			pos++;
			if(state == CMSE_GET_INDEX)
			{
				if( *count >= MAX_CMSE)
				{
					assert_always(false, "Too many script elements (Parsing 'bc')");
				}

				element[*count].rw_ix = -1;
				element[*count].is_optional = is_optional;
				element[*count].multiplier = 1;
				element[*count].type = CMSE_TYPE_BC_REGISTER;
				state = CMSE_SAVED_INDEX;
			}
			else break; //Failure!
		}
		else if(ch == 'N')
		{
			if(state == CMSE_GET_INDEX)
			{
				if( *count >= MAX_CMSE)
				{
					assert_always(false, "Too many script elements (Parsing 'N')");
				}

				element[*count].rw_ix = -1;
				element[*count].is_optional = is_optional;
				element[*count].multiplier = 1;
				element[*count].type = CMSE_TYPE_UNCONTENDED;
				state = CMSE_SAVED_INDEX;
			}
			else break; //Failure!
		}
		else
		{
			if((ch >= '0') && (ch<='9'))
			{
				if(state == CMSE_GET_INDEX)
				{
					if( *count >= MAX_CMSE)
					{
						assert_always(false, "Too many script elements (Parsing '0' to '9')");
					}

					element[*count].rw_ix = ch-'0';
					element[*count].is_optional = is_optional;
					element[*count].multiplier = 1;
					element[*count].type = CMSE_TYPE_MEMORY;
					state = CMSE_SAVED_INDEX;
				}
				else if(state == CMSE_GET_CYCLES)
				{
					element[*count].inst_cycles = ch-'0';

					//Might be greater than 10 so lets have a cheeky peak forward.
					ch = script[pos+1];
					if((ch>=0x30) && (ch<=0x39))
					{
						//Yup, good job we had a peak!
						pos++;
						element[*count].inst_cycles *= 10;
						element[*count].inst_cycles += ch-'0';
					}
					state = CMSE_SAVED_CYCLES;
				}
				else if(state == CMSE_GET_MULTIPLIER)
				{
					element[*count].multiplier = ch-'0';
					state = CMSE_SAVED_MULTIPLIER;
				}
				else
				{
					break; //Failure!
				}
			}
			else
			{
				break; //Failure!
			}
		}

		pos++;
	};

	//If we broke out the loop before finishing, something went wrong
	if(!finished)
	{
		logerror("Unexpected char '%c' (In script '%s')\n", ch, script);
		assert_always(false, "Unexpected character in script");
	}

	breakdown->inst_cycles_total = *cycles_mandatory + *cycles_optional;
}


int specz80_device::get_ula_delay()
{
	if(m_tstate_counter >= m_cycles_contention_start)
	{
		int base = m_tstate_counter - m_cycles_contention_start;
		int y_pos = base/m_cycles_per_line;
		int x_pos = base%m_cycles_per_line;
		if ((y_pos < 192) && (x_pos < 128))
		{
			return m_ula_delay_sequence[x_pos%8]-'0';
		}
	}

	return 0;
}


int specz80_device::get_memory_access_delay(uint16_t pc_address)
{
	if((pc_address >= 0x4000) && (pc_address <=0x7fff))
	{
		return get_ula_delay();
	}
	else if((pc_address >= 0xc000) && (pc_address <=0xffff))
	{
		if(m_selected_bank > 0)
		{
			int i;
			for(i=0; i<m_contended_banks_length; i++)
			{
				if(m_selected_bank == (m_contended_banks[i]-'0'))
				{
					return get_ula_delay();
				}
			}
		}
	}

	return 0;
}


void specz80_device::eat_cycles(int type, int cycles)
{
	// When this function is called and type CYCLES_CONTENDED or CYCLES_UNCONTENDED is set,
	// it means the run_script function is processing the opcode history and that cycles are
	// to be eaten immediately. Note. Only ever occurs when ULA_VARIANT_SINCLAIR or
	// ULA_VARIANT_AMSTRAD is set.
	//
	// When this function is called and type is CYCLES_ISR, it means opcodes during an ISR
	// have been processed. As memory contention processing is not required during ISRs, the
	// predicted cycles are eaten immediately.
	//
	// When this function is called and type is CYCLES_EXEC, it means the EXEC() macro has
	// been called. If ULA_VARIANT_NONE is set, as memory contention processing is not
	// required, the predicted cycles are eaten immediately. If ULA_VARIANT_SINCLAIR or
	// ULA_VARIANT_AMSTRAD is set, as memory contention processing IS required, then the
	// predicted cycles are not eaten. Instead, it will be left to the run_script() function
	// to eat cycles (CYCLES_CONTENDED or CYCLES_UNCONTENDED) based on the opcode's contended
	// memory script. Note. Once all the history for an opcode has been captured, the total
	// amount of CYCLES_UNCONTENDED should match up with the total amount of CYCLES_PREDICTED.
	//
	// Based on the above, this boils down to some simple logic to work out when to eat
	// and when not to...
	if((type == CYCLES_EXEC) && (m_ula_variant != ULA_VARIANT_NONE))
	{
		m_opcode_history.uncontended_cycles_predicted += cycles;
		return;
	}
	else if(type == CYCLES_UNCONTENDED)
	{
		m_opcode_history.uncontended_cycles_eaten += cycles;
	}

	m_icount -= cycles;
	m_tstate_counter += cycles;
	if(m_tstate_counter >= m_cycles_per_frame) m_tstate_counter -= m_cycles_per_frame;
}


void specz80_device::store_rwinfo(uint16_t addr, uint8_t val, uint16_t flags, const char *dbg)
{
	RWINFO *rw;

	if(!m_opcode_history.capturing) return;

	if(m_opcode_history.rw_count >= MAX_RWINFO)
	{
		logerror("RWINFO overflow. No room for addr=0x%04X val=0x%02X flags=0x%X (%s) tstate=%d\n", addr, val, flags, dbg, m_tstate_counter);
		assert_always(false, "Opcode history list is full");
	}

	//Save the new rwinfo
	rw = &(m_opcode_history.rw[m_opcode_history.rw_count]);
	rw->addr = addr;
	rw->val = val;
	rw->flags = flags;
	rw->dbg = dbg;
	m_opcode_history.rw_count++;

	run_script();

	//Originally, the intention was to update the screen raster each time
	//any tstates were eaten (I.e. call the raster_cb from inside the
	//eat_cycles function) which worked well but wasn't efficient.
	//So below is the opimisation, it is based on the fact that the screen
	//raster only needs to be updated immediately *before* a border or screen colour
	//attribute write occurs.
	if(rw->flags & RWINFO_WRITE)
	{
		if(rw->flags & RWINFO_IO_PORT)
		{
			if((addr & 0xff) == 0xfe) //Border change
			{
				m_raster_cb(m_tstate_counter);
			}
		}
		else if(rw->flags & RWINFO_MEMORY)
		{
			// Screen or attribute change (48K and 128K)
			if((addr >= 0x4000) && (addr <= 0x5AFF))
				m_raster_cb(m_tstate_counter);
			// Screen or attribute change (128K models - bank 5)
			else if( (m_selected_bank == 5) && ((addr >= 0xC000) && (addr <= 0xDAFF)) )
				m_raster_cb(m_tstate_counter);
		}
	}
}
