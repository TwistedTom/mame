//
// Super Wild Card / Super Magicom
// SNES backup copiers
//

/*
   Super Wild Card SMS3201
   -----------------------
   pcb: "Super Magicom Plus"
        "Front Fareast Co."

   FC9203 100-pin qfp asic   HG62E22S26FS (c)1992 FRONT    (Hitachi HG62E series gate array)
   27c128 16KB rom
   62256  32KB ram w/ nicd battery backup
   8/16/24/32mbit dram on plug-in daughter board
   unpopulated locations for 16mbit on-board dram (ever used? not seen any examples...)
   2x peel 18CV8
   MCCS3201FN fdc w/ 24MHz xtal
   internal 3.5" dd/hd fdd
   unpopulated location (db25) for external fdd (ever used? not seen any examples...)
   db25 parallel port (for pc comms)
   snes cart slot
   16-pin dip socket for CIC chip or jumpers to use cart instead

   dram config:
   8mbit is 2x generic 1Mx4 dram chips (814400, 514400, 44c1000 etc.)
   factory 16 and 24 mbit configs exist

   32mbit support requires:
     adding 2 (or more) dram chips
     hardware mod (track cutting, wires added etc.)
     different peels
     firmware upgrade (2.8cc?)

   FFE released instructions, firmware and peel jed files for suppliers/resellers to upgrade stock
   does factory 32mbit version exist?


   earlier hardware...


   Super Magicom MS-3201
   ---------------------
   older model
   no pcb numbers/markings
   27c64 8KB rom
   8/16mbit dram
   no provision for on-board dram (daughter board only)
   EP1810 cpld, no asic, no pals/peels
   external fdd only (2nd db25 is populated)
   suppied with "magic" 3.5" disk drive (generic 3.5" fdd in stand-alone case w/ psu)
   unpopulated location (34-pin dil) for internal fdd (but no 4-pin power header)
   unpopulated location for 16-pin dip CIC chip, jumpers on solder-side to select chip or cart
   extra 74xx174 ttl
   known firmware: official v31, various hacks
   supports only lorom games
   doesn't support games that have sram presence protection checks (doesn't disable it's sram)
   can be modded to play hirom games (unofficial mod), was there a factoy modded version released?

   odd versions:
   24mbit model: identical case, pcb is very similar to wild card "Super Magicom Plus"
   ??? model: smaller cart slot (46-pin), clone or early version? no pcb pic

   clones:
   Supercom Pro.1/2 SP-3200
   Micro Genius Future Supercom Pro.9
   Twin Supercom Pro.5 TS32  (NO DUMP)
   ...
   some clone bioses use different ports, what h/w are they for?
   UFO Super Drive Pro?  lots gals, no cpld


   later hardware...


   Super Wild Card SMS3201 "goldstar" variant
   ------------------------------------------
   externally looks same, pcb differences:
   no pcb numbers/markings?
   GM82C765B fdc
   extra 74xx129, 74xx125 ttl
   unpopulated locations for power socket and regulator (and power switch?)
   no provision for on-board dram (daughter board only)
   extra jumpers JP3-8, function unknown
   same asic and peels
   same firmware
   same hardware mod required for 32mbit


   Super Wild Card DX
   ------------------
   externally very similar, black case, pcb differences:
   no pcb numbers/markings?
   based on goldstar variant plus:
   32mbit dram
   new daughter board with 2x 4Mx4 44c4000 chips (2 more unpopulated locations, 64mbit is supported?)
   power socket and regulator populated
   larger rom (32-pin)
   gal20v8 gal (in addition to 2x peels)
   93c46 eeprom
   extra unknown function jumpers JP2-12
   same asic
   2x peels are the same?
   exclusive firmware  x.x_101494(undumped)?, x.x_110394, 1.122_111494, x.x_010496


   Super Wild Card DX2     *** TODO ***
   -------------------
   new case, black, more rounded design
   64mbit dram support
   external CDROM/HDD/Zip support
   exclusive firmware  1.106  6/8/96 youtube
   asic?
   pals/peels?

   optional "diskdual" - external case for a cdrom and/or hdd

   what else...?


   known firmware:
   * 1.6c
     1.6xc     NO DUMP     http://www.videogameobsession.com/videogame/hk/swc.htm
   * 1.8
     2.0xl     "adj" hack, just name changed?
     2.0xl     hack "fairlight trading", just name changed
     2.1b      "adj" hack, just name changed?
     2.1c      "adj" hack, just name changed?
   * 2.2cc     NO DUMP     https://www.youtube.com/watch?v=62ZlDgp3FyI , gYNHoFc7cVs
     2.2cc     hack "cyrus consoles", just name changed?
   * 2.6cc     dumped
     2.6cc-s   NO DUMP, spanish, hack?     https://www.youtube.com/watch?v=UUkRfpY9uck
     2.6f      "adj" hack, just name changed?
     2.6f      "adj" hack, alt?
     2.6?      "roc" hack, date removed, different colour scheme
     2.6?      "tris" hack, as above + name changed, "loading" string changed to "alligator"
   * 2.7cc
     2.7c      "adj" hack, just name changed
     2.7bc     NO DUMP, brazil hack?     https://www.youtube.com/watch?v=dfvmXv7naBA
   * 2.8cc     940608 dumped
     2.8cc     940608 "sprint elec" hack, just name changed
     2.8cc     940608 alt or hack?  has "Super Wild Card 2.8cc" string @ 1fc0
   * 2.8cc     940628 rts fix

   * = official ver
   need 2.0, 2.1, 2.2cc official
*/

/*
   1) Registers

      [floppy drive i/o]
      c000  r : input register
                bit 7 : mcs3201 irq signal
                bit 6 : drive 'index' signal (disk insert check)
      c002  w : digital output register
      c004  r : main status register       (uPD765 compatible)
      c005 rw : data register              (uPD765 compatible)
      c007  r : digital input register
      c007  w : diskette control register

      [parallel i/o]
      c008  r : bit 0-7 : parallel data input (reading reverses the busy flag)
      c008  w : bit 0-3 : parallel data output                                   PC side: S3-6  err, sel, pout, ack  pins 15,13,12,10
                bit 0   : 0=mode 20, 1=mode 21 (dram mapping)
                bit 1   : 0=mode 1, 1=mode 2 (sram mapping)
                bit 4   : ?
      c009  r : busy flag, bit 7 (ep1810 version)
      c000  r : busy flag, bit 5 (fc9203 version)                                PC side: C0 /str  pin 1
                                                                          also   PC side: S7 /busy  pin 11  ??
      c00a-c00f unused (mirrors c008-c009)
      c010-dfff unused (mirrors c000-c00f)

      [page select]
      e000  w : memory page 0     select 8KB (0x2000) page    +0000 1fff
      e001  w : memory page 1                                 +2000 3fff
      e002  w : memory page 2     (snes_addr & 0xff1fff)      +4000 5fff
      e003  w : memory page 3        + (page * 0x2000)        +6000 7fff

      [mode select]
      e004  w : system mode 0 (bios mode, power on default)
      e005  w : system mode 1 (play cartridge)
      e006  w : system mode 2 (cartridge emulation 1)          "memory mode"
      e007  w : system mode 3 (cartridge emulation 2)          "normal mode"

      [others]
      e008  w : 44256 dram type (for 2,4,6,8 mega byte dram card)          do these cards exist?
      e009  w : 441000 dram type (for 8,16,24,32 mega byte dram card)       even earliest magicom uses 441000

      e00c  w : enable cartridge page mapping at 2000-3fff & a000-bfff (sys mode 0)
                disable cartridge mapping at bank 20-5f, a0-df         (sys mode 2,3)   <-- what is this for?
      e00d  w : enable sram page mapping at 2000-3fff & a000-bfff      (sys mode 0)          (not dsp range)
                enable cartridge mapping at bank 20-5f, a0-df          (sys mode 2,3)

      * the bank address of the above registers is 00-7d, 80-ff
      * the above registers are available only in system mode 0 (bios mode)
      * [mode select] registers also available in system mode 2

   2) Memory Mapping

      [system mode 0]
      bb:2000 - bb:3fff rw : sram or cartridge page mapping  bb=40-7d,c0-ff
      bb:8000 - bb:9fff rw : dram page mapping               bb=00-7d,80-ff
      bb:a000 - bb:bfff rw : sram or cartridge page mapping  bb=00-7d,80-ff
      bb:c000 - bb:dfff rw : i/o registers                   bb=00-7d,80-ff
      bb:e000 - bb:ffff  r : rom page mapping                bb=00-01

      * 1 page = 8KB, 1 bank = 4 pages
      * bb:00-0f = 512KB  4mbit
      * bb:00-1f = 1MB    8mbit
      * bb:00-2f = 1.5MB 12mbit
      * bb:00-3f = 2MB   16mbit
        ...

      [system mode 1]
      bb:0000 - bb:7fff  r : cartridge mapping, bb=40-7d, c0-ff (mode 21)
      bb:8000 - bb:ffff  r : cartridge mapping, bb=00-7d, 80-ff (mode 20,21)

      [system mode 2]
      bb:0000 - bb:7fff  r : dram mapping, bb=40-6f, c0-df (mode 21)          6f/df must be wrong?  a23 not connected, so mirror would be 5f/df or 6f/ef
      bb:8000 - bb:ffff  r : dram mapping, bb=00-6f, 80-df (mode 20,21)       32mbit mod allows full map (40-7d,c0-ff:0000-7fff 00-7d,80-ff:8000-ffff)
      70:0000 - 70:7fff rw : sram mode 1 mapping                               <-- corrected
      30:6000 - 30:7fff rw : sram mode 2 mapping, page 0
      31:6000 - 31:7fff rw : sram mode 2 mapping, page 1
      32:6000 - 32:7fff rw : sram mode 2 mapping, page 2
      33:6000 - 33:7fff rw : sram mode 2 mapping, page 3
      bb:e004 - bb:e007  w : mode select registers, bb=00-7d, 80-ff

      [system mode 3]
      bb:0000 - bb:7fff  r : dram mapping, bb=40-6f, c0-df (mode 21)
      bb:8000 - bb:ffff  r : dram mapping, bb=00-6f, 80-df (mode 20,21)
      70:0000 - 70:7fff rw : sram mode 1 mapping                               <-- corrected
      30:6000 - 30:7fff rw : sram mode 2 mapping, page 0
      31:6000 - 31:7fff rw : sram mode 2 mapping, page 1
      32:6000 - 32:7fff rw : sram mode 2 mapping, page 2
      33:6000 - 33:7fff rw : sram mode 2 mapping, page 3

      * mode 21
        even dram bank is mapped to bb:0000-bb:7fff
        odd  dram bank is mapped to bb:8000-bb:ffff


   ------------------------------------------------------------------------------------
   file header - 512 bytes
   ------------------------------------------------------------------------------------
   #1 - from "SUPER WILDCARD DX- USERS GUIDE"
   ------------------------------------------------------------------------------------

   file format of the wild card program:
   the wild card consists of a 512 byte header and program blocks, one block is 8kbytes
   the program block is loaded into main memory from low address to high address sequentally
   the recommended file name is .smc or .swc

   content of file header:

   byte 0 - numbers of block (low byte)
   byte 1 - numbers of block (high byte)   total numbers of blocks = (byte 1 * 256) + byte 0

   byte 2 - program execution mode
            bit: 7 6 5 4 3 2 1 0
                 x               : 0 reserved
                   x             : 0=last/only file, 1=search for next file
                     x           : sram memory mapping: 0=mode 20, 1=mode 21
                       x         : program memory mapping: 0=mode 20, 1=mode 21
                         x x     : sram size: 00=256k, 01=64k, 10=16k, 11=off
                             x x : 00 reserved

   byte 3-7 - 0 reserved
   byte 8   - aa, file id
   byte 9   - bb, file id
   byte 10  - file type:
               04=program
               05=battery backup data
               08=real-time save data
   byte 11-511 -  reserved

   ------------------------------------------------------------------------------------
   #2 - Various other old sources
   ------------------------------------------------------------------------------------

   byte
    0   - low byte of 8k-bytes page counts
    1   - high byte of 8k-bytes page counts
    2   - emulation mode select
          bit 7 6 5 4 3 2 1 0
              x               : 1=run in mode 0 (jump $8000)
                x             : 0=last file of the game (multi-file loading)
                  x           : 0=mode 1, 1=mode 2 (sram mapping)
                    x         : 0=mode 20, 1=mode 21 (dram mapping)
                      x x     : 0=sram off, 1=sram 16k, 2=sram 64k, 3=sram 256k (sram size)
                          x   : 0=run in mode 3, 1=run in mode 2 (jmp reset)
                            x : 0=disable, 1=enable (external cart at bank 20-5f,a0-df for system modes 2,3)
    3-7 - reserved (should be '00')
    8   - file id code 1 (should be 'aa')
    9   - file id code 2 (should be 'bb')
    10  - check this byte if id 1 & 2 match
          02 : magic griffin game file (pc engine)
          03 : magic griffin sram data file
          04 : swc & smc game file (snes, SUPER MAGICOM)
          05 : swc & smc password, sram data, saver data file.
          06 : smd game file (megadrive)
          07 : smd sram data file
    37  - reserved (should be 00)
    11-511 - reserved (should be 00)

   ------------------------------------------------------------------------------------
   notes:

   assume these docs refer to >=DX models
   ucon64 seems to use doc #1, differs from what bios produces

   byte 2 "emulation mode" produced by bios (saving a cart to floppy):

              rom mbit
              |   sram kbit
              |   |    map type
              |   |    |    rom speed
              |   |    |    |
              |   |    |    |   2.8cc 28/08     2.7cc/2.6cc     2.2cc/1.8/1.6c       ucon64 --swc
   ssoccer    4   0    lo   s   2d  0010 1101   21  0010 0001   21  0010 0001        0c  0000 1100
   smw        4   16   lo   s   00  0000 0000   00  0000 0000   00  0000 0000        08  0000 1000
   sbombman   4   0    hi   s   1c  0001 1100   10  0001 0000   30  0011 0000        3c  0011 1100
   nhl95      8   64   hi   f   31  0011 0001   31  0011 0001   30  0011 0000        34  0011 0100
   congcap    4   0    lo   f   2d  0010 1101
   ffight2    10  0    lo   f   2c  0010 1100   lo sram check
   simcity    4   256  lo   s   00  0000 0000
   chaoseng   12  0    hi   f   1c  0001 1100
   zoop       4   0    hi   f   1c  0001 1100
   ssf2       32  0    hi   f   3c  0011 1100   hi sram check (and rom size/mapping check) *
   killinst   32  0    hi   f   3c  0011 1100   no sram checks (lots other copier-detect checks with anti-tamper) *
   samsho     32  0    hi   f   3c  0011 1100   hi & lo sram checks *
   simcity2k  16  256  hi   f   30  0011 0000
   dkc2       32  16   hi   f   30  0011 0000   hi sram size==0 check (no size>16 check, lots other copier-detect checks with anti-tamper) *
   smas       16  64   lo   s   00  0000 0000   lo sram size>64 check *
   demcrest   16  0    lo   f   2c  0010 1100   lo sram check (and rom size/mapping check)
   ffight3    24  0    hi   f   1c  0001 1100   hi sram check
   ironcomm   10  0    lo   f   2c  0010 1100   lo sram check
   legend     8   0    lo   f   2d  0010 1101   lo sram check
   dkc        32  16   hi   f   30  0011 0000   hi sram size>16 check *
   dkc3       32  16   hi   f   30  0011 0000   hi sram size==0 check (no size>16 check, lots other copier-detect checks, no anti-tamper?) *
   spnchout   16  64   lo   s   00  0000 0000
   zelda      8   64   lo   s   00  0000 0000
   axelay     8   0    lo   s   2d  0010 1101
   fzero      4   16   lo   s   00  0000 0000
   fatfury    12  0    lo   s   2c  0010 1100
   ffight     8   0    lo   s   2d  0010 1101
   joemac     8   0    lo   s   2d  0010 1101
   joemac3    8   0    lo   f   2d  0010 1101
   sgng       8   0    lo   s   2d  0010 1101
   sprobo     8   0    lo   s   2d  0010 1101
   tmht4      8   0    lo   s   2d  0010 1101
   smetroid   24  64   lo   f   00  0000 0000   lo sram size>64 check *
   nhl96      12  64   hi   f   30  0011 0000
   sf2t       20  0    hi   f   1c  0001 1100
   som        16  64   hi   s   30  0011 0000
   smas+w     20  64   lo   s   00  0000 0000   lo sram size>64 check *

   these need checking fully for smc:
   sbombliss        4  16   lo   f              lo sram size>16 check *
   tet2+bbliss      8  16   lo   s
   tet2+bblissLtd   8  16   lo   f              lo sram size>16 check *
   tetris3          8  16   lo   f              lo sram size>16 check *
   tetris+Dr.Mario  8  0    lo   f
   tetris2          8  0    lo   f
   tetrisattack     8  0    lo   f              copier-detect checks *
   tetbtlgaiden     8  0    lo   f
   yoshicookie      4  0    lo   s
   yoshisafari      8  0    lo   f

   findings from h/w and bios code:
   bit 7 is set to run in mode 0 (jump $8000) ^
   bit 6 is set if "another file to come" for split-file games  eg. files 1-4: 40 40 40 2c (full emu byte for last file only)
   bit 5 is hi/lo sram type select  c008:1  U13 peel pin 1  0= lo enabled / hi disabled  1= lo disabled / hi enabled
   bit 4 is hi/lo rom select  c008:0  U12 peel pin 1
   bits 3:2 used only by 2.8cc, written to c008 3:2   could be sram size that was never implemented in h/w?
   bit 1 is system mode 2/3  0=3, 1=2  written to e006 or e007
   bit 0 is set to enable cart at banks 20-5f,a0-df (for system modes 2,3)  but why?  seems to be set for all 4 and 8mbit lo games

   not possible to turn off both sram areas together (or turn on, not that there's a reason to!)
   can be a problem as some games (samsho) protection check both types
   for 32mbit hirom games (all 32mbit are hirom?) lo must be disabled (would otherwise conflict with rom), which means hi must be on which can fail protection checks!
   bios does this:
   lo        = 1 = lo dis  hi en   lo games with hi sram check will fail (do any exist?)
   lo+sram   = 0 = lo en   hi dis  ok
   hi <32m   = 0 = lo en   hi dis  hi games with lo sram check will fail (do any exist? samsho does but is 32m)
   hi =32m   = 1 = lo dis  hi en   32m hi games with hi sram check will fail
   hi+sram   = 1 = lo dis  hi en   ok  (inc. 32m+sram)

   * these games need patching to run on swc
     all games with any kind of protection will need patching for smc

   ^ after load, the normal/memory mode select option is skipped, stays in mode 0 and jumps to 8000
     (mode 0, so dram is mapped in 8000-9fff) good for homebrew etc.

   magicom doesn't use emu byte at all (0 or 0x40)

   when saving, bios splits into 4mbit chunks, if whole game fits on single disk will give "single or multi file" option
   
   largest file that can be loaded from floppy is officially 1.5MB (12mbit, 192*8KB blocks)
   bios code reads only low byte of header "8k-bytes page counts" field, stores a counter in $2f with A in 8-bit mode 
   2MB (16mbit, 256*8KB blocks) files seem to work ok but counter glitches (reads 8k-bytes page counts as 0 and rolls over)
   
   2.88MB ED floppy images work ok with gotek floppy emulator (with hxc firmware)
   unknown if a real ED drive works

   ------------------------------------------------------------------------------------

   top cart slot:
   asic has control of a13, a14, /cart, d0-7, everything else is straight through

   bios:
   swc 27c128 16KB, a13 pin is connected to a16
   smc 27c64 8KB, pin 26 (27c128 a13) goes to 74ls174 so can be latched somehow

   unknown writes:
   0c -> c009       bits 7-6 ?
   10 -> c008       bit 4 ?     these could be for ep1810 version?

   00 -> c009       when starting game after "play game" or "backup test" mode 2 or 3
   are these just 2nd part of a 16-bit write (2 byte writes) ?

   cart sram check:
   checks lo at 706000 through the 2000-3fff window
   checks hi at 306000 directly

   dram mapping:
   a23 not connected so rom size/mapping protection checks should pass (banks 80-ff == 00-7d)

   sram mapping:
   full 32KB/256kbit hi/lo sram mapping supported
   h/w doesn't clamp sram size to pass protection checks, some games need patching
   latest 2.8cc f/w reads the bits from header, sets c008 3:2, but these go nowhere on h/w, assume on the DX the larger pal handles this

   TODO:
   parallel port  how are /str (pc->swc) and /bsy (swc->pc) related?
   save states?  if playing cart pointer to m_cart is lost?
   dx/dx2 bioses
   seperate magicom?
*/

// make SOURCES=src/mame/nintendo/snes.cpp,src/mame/nintendo/snes_swc.cpp REGENIE=0 -j5

#include "emu.h"
#include "snes.h"
#include "bus/snes_ctrl/ctrl.h"
#include "speaker.h"
#include "imagedev/floppy.h"
#include "machine/upd765.h"
#include "formats/swc_dsk.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "machine/nvram.h"

#define SWC_DEBUG 0


class snes_swc_state : public snes_state
{
public:
	snes_swc_state(const machine_config &mconfig, device_type type, const char *tag)
		: snes_state(mconfig, type, tag)
		, m_ctrl1(*this, "ctrl1")
		, m_ctrl2(*this, "ctrl2")
		, m_fdc(*this, "fdc")
		, m_fdd(*this, "fdd")
		, m_cartslot(*this, "cartslot")
		, m_nvram(*this, "nvram")
		, m_dram_opt(*this, "DRAM")
	{ }

	void snes_swc(machine_config &config);
	void snes_swc_pal(machine_config &config);
	void init_snes_swc();

	DECLARE_INPUT_CHANGED_MEMBER(snes_swc_dram_changed);

private:
	virtual void io_read() override;
	virtual uint8_t oldjoy1_read(int latched) override;
	virtual uint8_t oldjoy2_read(int latched) override;
	virtual void write_joy_latch(uint8_t data) override;
	virtual void wrio_write(uint8_t data) override;

	virtual void machine_start() override;
	virtual void machine_reset() override;

	required_device<snes_control_port_device> m_ctrl1;
	required_device<snes_control_port_device> m_ctrl2;
	required_device<mcs3201_device> m_fdc;
	required_device<floppy_connector> m_fdd;
	required_device<generic_slot_device> m_cartslot;
	required_device<nvram_device> m_nvram;
	required_ioport m_dram_opt;

	void snes_swc_map(address_map &map);
	void spc_map(address_map &map);

	uint8_t snes_swc_r_bank1(offs_t offset);
	uint8_t snes_swc_r_bank2(offs_t offset);
	void snes_swc_w_bank1(address_space &space, offs_t offset, uint8_t data);
	void snes_swc_w_bank2(address_space &space, offs_t offset, uint8_t data);
	inline uint8_t snes_swc_mode_0_r_bank1(offs_t offset);
	inline uint8_t snes_swc_mode_0_r_bank2(offs_t offset);
	inline void snes_swc_mode_0_w(address_space &space, offs_t offset, uint8_t data);
	inline uint8_t snes_swc_io_r(uint16_t address);
	inline void snes_swc_io_w(uint16_t address, uint8_t data);
	inline void snes_swc_misc_w(uint16_t address, uint8_t data);
	inline void snes_swc_dram_w(offs_t offset, uint8_t data);
	inline uint8_t snes_swc_dram_r(offs_t offset);
	inline void snes_swc_sram_w(uint16_t address, uint8_t data);
	inline uint8_t snes_swc_sram_r(uint16_t address);
	inline void snes_swc_cart_w(uint16_t address, uint8_t data);
	inline uint8_t snes_swc_cart_r(offs_t offset);
	inline uint8_t snes_swc_mode_1_r(offs_t offset);
	inline void snes_swc_mode_1_w(address_space &space, offs_t offset, uint8_t data);
	inline uint8_t snes_swc_mode_1_rom_access(offs_t offset);
	inline uint8_t snes_swc_mode_2_r(offs_t offset);
	inline void snes_swc_mode_2_w(address_space &space, offs_t offset, uint8_t data);
	inline uint8_t snes_swc_mode_2_rom_access(offs_t offset);
	inline void snes_swc_mode_3_w(address_space &space, offs_t offset, uint8_t data);

	static void swc_floppy_formats(format_registration &fr);
	uint8_t fdc_input_r();

	//DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);
	image_init_result cart_load(device_image_interface &image);
	//DECLARE_DEVICE_IMAGE_UNLOAD_MEMBER(cart_unload)
	void cart_unload(device_image_interface &image);
	void snes_swc_find_cart_type();
	void snes_swc_set_cart_size_mask();
	inline uint32_t snes_swc_clamp_rom_size(uint32_t address);
	void snes_swc_find_cart_sram();

	uint8_t *m_bios;
	uint8_t m_dram[0x400000];
	uint32_t m_dram_size;
	uint8_t m_sram[0x8000];

	int m_rom_map = 0;           // emu byte bit 4
	int m_sram_map = 0;          // emu byte bit 5
	int m_page = 0;
	int m_sys_mode = 0;
	int m_dram_type = 0;         // no use for this?
	int m_sram_or_cart_map = 0;  // swc sram or cart rom mapped to the 2000-3fff, a000-bfff windows
	int m_busy = 0;

	int m_cart_type = 0;
	uint32_t m_cart_size = 0;
	uint32_t m_cart_size_mask = 0;
	uint8_t *m_cart;
	uint32_t m_cart_sram_size = 0;
	std::vector<uint8_t> m_cart_sram;

	uint8_t par_debug_str = 0;
	uint8_t par_debug_data_in = 0;
	uint8_t par_debug_data_out = 0;
};

INPUT_CHANGED_MEMBER(snes_swc_state::snes_swc_dram_changed)
{
	m_dram_size = 0x100000 + (m_dram_opt->read() * 0x100000);
}

//DEVICE_IMAGE_LOAD_MEMBER(snes_swc_state::cart_load)
image_init_result snes_swc_state::cart_load(device_image_interface &image)
{
	uint32_t size = m_cartslot->common_get_size("rom");

	if (size > 0x400000)
	{
		image.seterror(image_error::INVALIDIMAGE, "Unsupported cartridge size");
		return image_init_result::FAIL;
	}

	m_cartslot->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	m_cartslot->common_load_rom(m_cartslot->get_rom_base(), size, "rom");

	m_cart_size = m_cartslot->get_rom_size();
	m_cart = m_cartslot->get_rom_base();
	snes_swc_set_cart_size_mask();
	snes_swc_find_cart_type();
	snes_swc_find_cart_sram();

	return image_init_result::PASS;
}

//DECLARE_DEVICE_IMAGE_UNLOAD_MEMBER(snes_swc_state::cart_unload)
void snes_swc_state::cart_unload(device_image_interface &image)
{
	if (m_cart_sram_size)
	{
		m_cartslot->battery_save(m_cart_sram.data(), m_cart_sram_size);
	}
}

void snes_swc_state::snes_swc_find_cart_type()
{
	int fail = 0;

	for (int i = 0x7fc0; i < (0x7fc0 + 21); i++ )
	{
		uint8_t ch = m_cart[i];
		if (!(ch >= ' ' && ch <= '~'))
		{
			fail = 1;
			break;
		}
	}

	if ((m_cart[0x7fd5] & 0xef) != 0x20)  // 20 or 30
		fail = 1;

	if (!fail)
	{
		m_cart_type = 0;
		logerror("cart is lorom\n");
		return;
	}


	fail = 0;

	for (int i = 0xffc0; i < (0xffc0 + 21); i++ )
	{
		uint8_t ch = m_cart[i];
		if (!(ch >= ' ' && ch <= '~'))
		{
			fail = 1;
			break;
		}
	}

	if ((m_cart[0xffd5] & 0xef) != 0x21)  // 21 or 31
		fail = 1;

	if (!fail)
	{
		m_cart_type = 1;
		logerror("cart is hirom\n");
		return;
	}


	logerror("unknown cart\n");
}

void snes_swc_state::snes_swc_set_cart_size_mask()
{
	uint32_t sz = m_cart_size;

	if ((sz & (sz - 1)) && (sz != 0))  // not power of 2 (more than 1 bit set)
	{
		// get top bit
		int i = 0;
		for (; sz > 1; sz >>= 1)
			i++;
		sz = 1 << i;
	}

	m_cart_size_mask = sz;

	logerror("cart size is %d bytes (0x%06x)\n", m_cart_size, m_cart_size);
	logerror("cart size mask is %d bytes (0x%06x)\n", m_cart_size_mask, m_cart_size_mask);
}

inline uint32_t snes_swc_state::snes_swc_clamp_rom_size(uint32_t address)
{
	if (m_cart_size != m_cart_size_mask)
	{
		if (!(address & m_cart_size_mask))
			address &= m_cart_size_mask - 1;
		else
			address &= m_cart_size - 1;
	}
	else
		address &= m_cart_size - 1;

	return address;
}

void snes_swc_state::snes_swc_find_cart_sram()
{
	uint32_t sz = 0;

	if (!m_cart_type)  // lo
	{
		if (m_cart[0x7fd6] == 2)
			sz = m_cart[0x7fd8];
	}
	else
	{
		if (m_cart[0xffd6] == 2)
			sz = m_cart[0xffd8];
	}

	if (sz)
	{
		sz = 1 << sz;
		logerror("cart has %dKB sram\n", sz);
	}
	else
	{
		logerror("cart doesn't have sram\n");
	}

	m_cart_sram_size = sz << 10;
	m_cart_sram_size &= 0x20000 - 1;  // clamp just in case...
	if (m_cart_sram_size)
	{
		m_cart_sram.resize(m_cart_sram_size);
		m_cartslot->battery_load(m_cart_sram.data(), m_cart_sram_size, 0xff);
	}
}


static void swc_fdd_options(device_slot_interface &device)
{
	//device.option_add("35dd", FLOPPY_35_DD);
	//device.option_add("35hd", FLOPPY_35_HD);
	device.option_add("35ed", FLOPPY_35_ED);
}

void snes_swc_state::swc_floppy_formats(format_registration &fr)
{
	fr.add(FLOPPY_SWC_FORMAT);
}

uint8_t snes_swc_state::fdc_input_r()
{
	// 6: index, 7: irq
	uint8_t data = 0;

	if (m_fdc->get_irq())
		data |= 0x80;

	int idx = m_fdd->get_device()->idx_r();
	if (!idx)
		data |= 0x40;

	return data;
}


void snes_swc_state::snes_swc(machine_config &config)
{
	/* basic machine hardware */
	_5A22(config, m_maincpu, MCLK_NTSC);   // Nintendo S-CPU 5A22-0x, 21.477272MHz / (6, 8, 12) = 1.79 MHz, 2.68 MHz, also 3.58 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &snes_swc_state::snes_swc_map);

	// runs at 24.576 MHz / 12 = 2.048 MHz
	S_SMP(config, m_soundcpu, XTAL(24'576'000) / 12);
	m_soundcpu->set_addrmap(AS_DATA, &snes_swc_state::spc_map);
	m_soundcpu->dsp_io_read_callback().set(m_s_dsp, FUNC(s_dsp_device::dsp_io_r));
	m_soundcpu->dsp_io_write_callback().set(m_s_dsp, FUNC(s_dsp_device::dsp_io_w));

	//config.set_maximum_quantum(attotime::from_hz(48000));
	config.set_perfect_quantum(m_maincpu);

	INPUT_MERGER_ANY_HIGH(config, m_scpu_irq);
	m_scpu_irq->output_handler().set_inputline(m_maincpu, G65816_LINE_IRQ);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(DOTCLK_NTSC * 2, SNES_HTOTAL * 2, 0, SNES_SCR_WIDTH * 2, SNES_VTOTAL_NTSC, 0, SNES_SCR_HEIGHT_NTSC);
	m_screen->set_video_attributes(VIDEO_VARIABLE_WIDTH);
	m_screen->set_screen_update(FUNC(snes_state::screen_update));

	SNES_PPU(config, m_ppu, MCLK_NTSC);
	m_ppu->open_bus_callback().set(FUNC(snes_swc_state::snes_open_bus_r));
	m_ppu->set_screen("screen");

	SNES_CONTROL_PORT(config, m_ctrl1, snes_control_port_devices, "joypad");
	//m_ctrl1->set_onscreen_callback(FUNC(snes_swc_state::onscreen_cb));
	SNES_CONTROL_PORT(config, m_ctrl2, snes_control_port_devices, "joypad");
	//m_ctrl2->set_onscreen_callback(FUNC(snes_swc_state::onscreen_cb));
	//m_ctrl2->set_gunlatch_callback(FUNC(snes_swc_state::gun_latch_cb));

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	S_DSP(config, m_s_dsp, XTAL(24'576'000) / 12);
	m_s_dsp->set_addrmap(0, &snes_swc_state::spc_map);
	m_s_dsp->add_route(0, "lspeaker", 1.00);
	m_s_dsp->add_route(1, "rspeaker", 1.00);

	/* swc floppy drive (drive #1) */
	MCS3201(config, m_fdc, 24_MHz_XTAL);
	m_fdc->input_handler().set(FUNC(snes_swc_state::fdc_input_r));
	FLOPPY_CONNECTOR(config, m_fdd, swc_fdd_options, "35ed", snes_swc_state::swc_floppy_formats, true).enable_sound(true);

	/* cart slot */
	GENERIC_CARTSLOT(config, m_cartslot, generic_plain_slot, "snes_swc_cart", "sfc,bin,rom");
	m_cartslot->set_device_load(FUNC(snes_swc_state::cart_load));
	m_cartslot->set_device_unload(FUNC(snes_swc_state::cart_unload));

	/* swc sram */
	NVRAM(config, m_nvram, nvram_device::DEFAULT_ALL_1);
}

void snes_swc_state::snes_swc_pal(machine_config &config)
{
	snes_swc(config);
	m_maincpu->set_clock(MCLK_PAL);
	m_screen->set_raw(DOTCLK_PAL * 2, SNES_HTOTAL * 2, 0, SNES_SCR_WIDTH * 2, SNES_VTOTAL_PAL, 0, SNES_SCR_HEIGHT_PAL);
	m_ppu->set_clock(MCLK_PAL);
}

void snes_swc_state::init_snes_swc()
{
	m_bios = memregion("bios")->base();
}

static INPUT_PORTS_START( snes_swc )
	PORT_START("DRAM")
	PORT_CONFNAME( 0x03, 0x03, "DRAM size") PORT_CHANGED_MEMBER(DEVICE_SELF, snes_swc_state, snes_swc_dram_changed, 0)
	PORT_CONFSETTING(    0x00, "8 mbit" )
	PORT_CONFSETTING(    0x01, "16 mbit" )
	PORT_CONFSETTING(    0x02, "24 mbit" )
	PORT_CONFSETTING(    0x03, "32 mbit" )
INPUT_PORTS_END

void snes_swc_state::io_read()
{
	// is automatic reading on? if so, read 16bits from oldjoy1/2
	if (SNES_CPU_REG(NMITIMEN) & 1)
	{
		uint16_t joy1 = 0, joy2 = 0, joy3 = 0, joy4 = 0;
		m_ctrl1->port_poll();
		m_ctrl2->port_poll();

		for (int i = 0; i < 16; i++)
		{
			joy1 |= ((m_ctrl1->read_pin4() & 1) << (15 - i));
			joy2 |= ((m_ctrl2->read_pin4() & 1) << (15 - i));
			joy3 |= ((m_ctrl1->read_pin5() & 1) << (15 - i));
			joy4 |= ((m_ctrl2->read_pin5() & 1) << (15 - i));
		}

		SNES_CPU_REG(JOY1L) = (joy1 & 0x00ff) >> 0;
		SNES_CPU_REG(JOY1H) = (joy1 & 0xff00) >> 8;
		SNES_CPU_REG(JOY2L) = (joy2 & 0x00ff) >> 0;
		SNES_CPU_REG(JOY2H) = (joy2 & 0xff00) >> 8;
		SNES_CPU_REG(JOY3L) = (joy3 & 0x00ff) >> 0;
		SNES_CPU_REG(JOY3H) = (joy3 & 0xff00) >> 8;
		SNES_CPU_REG(JOY4L) = (joy4 & 0x00ff) >> 0;
		SNES_CPU_REG(JOY4H) = (joy4 & 0xff00) >> 8;
	}
}

uint8_t snes_swc_state::oldjoy1_read(int latched)
{
	uint8_t ret = 0;
	ret |= m_ctrl1->read_pin4();
	ret |= (m_ctrl1->read_pin5() << 1);
	return ret;
}

uint8_t snes_swc_state::oldjoy2_read(int latched)
{
	uint8_t ret = 0;
	ret |= m_ctrl2->read_pin4();
	ret |= (m_ctrl2->read_pin5() << 1);
	return ret;
}

void snes_swc_state::write_joy_latch(uint8_t data)
{
	m_ctrl1->write_strobe(data);
	m_ctrl2->write_strobe(data);
}

void snes_swc_state::wrio_write(uint8_t data)
{
	if (!(SNES_CPU_REG(WRIO) & 0x80) && (data & 0x80))
	{
		// external latch
		m_ppu->set_latch_hv(m_ppu->current_x(), m_ppu->current_y());
	}

	m_ctrl1->write_pin6(BIT(data, 6));
	m_ctrl2->write_pin6(BIT(data, 7));
}

void snes_swc_state::machine_start()
{
	snes_state::machine_start();

	m_nvram->set_base(&m_sram[0], 0x8000);

	save_item(NAME(m_dram));
	save_item(NAME(m_dram_size));
	save_item(NAME(m_sram));
	save_item(NAME(m_rom_map));
	save_item(NAME(m_sram_map));
	save_item(NAME(m_page));
	save_item(NAME(m_sys_mode));
	save_item(NAME(m_dram_type));
	save_item(NAME(m_sram_or_cart_map));
	save_item(NAME(m_busy));
	save_item(NAME(m_cart_type));
	save_item(NAME(m_cart_size));
	save_item(NAME(m_cart_size_mask));
	save_item(NAME(m_cart_sram_size));
	save_item(NAME(m_cart_sram));
	save_item(NAME(par_debug_str));
	save_item(NAME(par_debug_data_in));
	save_item(NAME(par_debug_data_out));
}

void snes_swc_state::machine_reset()
{
	snes_state::machine_reset();

	// don't reset asic registers
	// when playing dram or cart, the reset button resets the game, doesn't return to swc bios

	m_dram_size = 0x100000 + (m_dram_opt->read() * 0x100000);
}

void snes_swc_state::snes_swc_map(address_map &map)
{
	map(0x000000, 0x7dffff).rw(FUNC(snes_swc_state::snes_swc_r_bank1), FUNC(snes_swc_state::snes_swc_w_bank1));
	map(0x7e0000, 0x7fffff).ram().share("wram");
	map(0x800000, 0xffffff).rw(FUNC(snes_swc_state::snes_swc_r_bank2), FUNC(snes_swc_state::snes_swc_w_bank2));
}

void snes_swc_state::spc_map(address_map &map)
{
	map(0x0000, 0xffff).ram().share("aram");
}


// rd  00-7d:0000-ffff
uint8_t snes_swc_state::snes_swc_r_bank1(offs_t offset)
{
	uint8_t data = 0xff;

	switch (m_sys_mode)
	{
		case 0:  // bios
			data = snes_swc_mode_0_r_bank1(offset);
			break;
		case 1:  // cart
			data = snes_swc_mode_1_r(offset);
			break;
		case 2:  // dram with mode select registers available
			data = snes_swc_mode_2_r(offset);
			break;
		case 3:  // dram
			data = snes_swc_mode_2_r(offset);
			break;
	}

	return data;
}

// rd  80-ff:0000-ffff
uint8_t snes_swc_state::snes_swc_r_bank2(offs_t offset)
{
	uint8_t data = 0xff;

	switch (m_sys_mode)
	{
		case 0:  // bios
			data = snes_swc_mode_0_r_bank2(offset);
			break;
		case 1:  // cart
			data = snes_swc_mode_1_r(offset);
			break;
		case 2:  // dram with mode select registers available
			data = snes_swc_mode_2_r(offset);
			break;
		case 3:  // dram
			data = snes_swc_mode_2_r(offset);
			break;
	}

	return data;
}

// wr  00-7d:0000-ffff
void snes_swc_state::snes_swc_w_bank1(address_space &space, offs_t offset, uint8_t data)
{
	switch (m_sys_mode)
	{
		case 0:  // bios
			snes_swc_mode_0_w(space, offset, data);
			break;
		case 1:  // cart
			snes_swc_mode_1_w(space, offset, data);
			break;
		case 2:  // dram with mode select registers available
			snes_swc_mode_2_w(space, offset, data);
			break;
		case 3:  // dram
			snes_swc_mode_3_w(space, offset, data);
			break;
	}
}

// wr  80-ff:0000-ffff
void snes_swc_state::snes_swc_w_bank2(address_space &space, offs_t offset, uint8_t data)
{
	switch (m_sys_mode)
	{
		case 0:  // bios
			snes_swc_mode_0_w(space, offset, data);
			break;
		case 1:  // cart
			snes_swc_mode_1_w(space, offset, data);
			break;
		case 2:  // dram with mode select registers available
			snes_swc_mode_2_w(space, offset, data);
			break;
		case 3:  // dram
			snes_swc_mode_3_w(space, offset, data);
			break;
	}
}


// mode 0 - bios

inline uint8_t snes_swc_state::snes_swc_mode_0_r_bank1(offs_t offset)
{
	uint8_t data = 0xff;
	uint16_t address = offset & 0xffff;

	if ((offset < 0x020000) && (address >= 0xe000))
		data = m_bios[(address & 0x1fff) | ((offset & 0x10000) >> 3)];
		// data = m_bios[(address & 0x1fff) | ((offset & 0x10000) ? 0 : 0x2000)];
	else
		data = snes_swc_r_bank2(offset);

	return data;
}

inline uint8_t snes_swc_state::snes_swc_mode_0_r_bank2(offs_t offset)
{
	uint8_t data = 0xff;
	uint16_t address = offset & 0xffff;

	if (offset < 0x400000)          // bank 00-3f
	{
		if (address < 0x2000)       // ram  0000-1fff
		{
			data = m_wram[address];
		}
		else if (address < 0x6000)  // io  2000-5fff
		{
			data = snes_r_io(address);
		}
		else if (address < 0x8000)  // hi cart sram  6000-7fff   2.8cc uses bank 30 for the check
		{
			if ((offset >= 0x300000) && (offset < 0x340000) && m_cart_type && m_cart_sram_size && m_cartslot && m_cartslot->exists())
			{
				address &= 0x1fff;
				address |= (offset & 0x30000) >> 3;  // Kaite Tsukutte Asoberu Dezaemon (Japan) has 128KB/1mbit sram (banks 30-3f)
				address &= m_cart_sram_size - 1;
				data = m_cart_sram[address];
				if (!machine().side_effects_disabled() && SWC_DEBUG)
					logerror("hi cart sram rd: %04x %02x\n", address, data);
			}
			else
				data = snes_open_bus_r();
		}
		else if (address < 0xa000)  // dram  8000-9fff
		{
			if (!machine().side_effects_disabled() && SWC_DEBUG)
				logerror("dram rd: %06x page %d\n", offset, m_page);
			data = snes_swc_dram_r(offset);
		}
		else if (address < 0xc000)  // sram or cart  a000-bfff
		{
			if (!machine().side_effects_disabled() && SWC_DEBUG)
			{
				if (m_sram_or_cart_map)
					logerror("sram rd: a000-bfff:%04x:%d\n", (address & 0x1fff), m_page);
				else
					logerror("cart rd: a000-bfff:%04x:%d -> %06x\n", (address & 0x1fff), m_page, (m_page << 13) | (offset & 0xff9fff));  // 0x008000 == 1
			}
			if (m_sram_or_cart_map)
				data = snes_swc_sram_r(address);
			else
				data = snes_swc_cart_r(offset);
		}
		else if (address < 0xe000)  // i/o regs  c000-dfff
		{
			if (!machine().side_effects_disabled() && SWC_DEBUG)
				logerror("swc i/o regs rd: %06x\n", offset);
			data = snes_swc_io_r(address);
		}
		else                        // ?  e000-ffff  (bios in rom banks 00-01 only)
		{
			if (!machine().side_effects_disabled() && SWC_DEBUG)
				logerror("? rd: %06x\n", offset);
			data = snes_open_bus_r();  // or cart if present?
		}
	}
	else  // < 0x7e0000                bank 40-7d
	{
		if (address < 0x2000)       // ?  0000-1fff
		{
			if (!machine().side_effects_disabled() && SWC_DEBUG)
				logerror("? rd: %06x\n", offset);
			data = snes_open_bus_r();
		}
		else if (address < 0x4000)  // sram or cart  2000-3fff
		{
			if (!machine().side_effects_disabled() && SWC_DEBUG)
			{
				if (m_sram_or_cart_map)
					logerror("sram rd: 2000-3fff:%04x:%d\n", (address & 0x1fff), m_page);
				else
					logerror("cart rd: 2000-3fff:%04x:%d -> %06x\n", (address & 0x1fff), m_page, (m_page << 13) | (offset & 0xff9fff));  // 0x008000 == 0
			}
			if (m_sram_or_cart_map)
				data = snes_swc_sram_r(address);
			else
				data = snes_swc_cart_r(offset);
		}
		else if (address < 0x8000)  // ?  4000-7fff
		{
			if (!machine().side_effects_disabled() && SWC_DEBUG)
				logerror("? rd: %06x\n", offset);
			data = snes_open_bus_r();
		}
		else if (address < 0xa000)  // dram  8000-9fff
		{
			if (!machine().side_effects_disabled() && SWC_DEBUG)
				logerror("dram rd: %06x page %d\n", offset, m_page);
			data = snes_swc_dram_r(offset);
		}
		else if (address < 0xc000)  // sram or cart  a000-bfff
		{
			if (!machine().side_effects_disabled() && SWC_DEBUG)
			{
				if (m_sram_or_cart_map)
					logerror("sram rd: a000-bfff:%04x:%d\n", (address & 0x1fff), m_page);
				else
					logerror("cart rd: a000-bfff:%04x:%d -> %06x\n", (address & 0x1fff), m_page, (m_page << 13) | (offset & 0xff9fff));  // 0x008000 == 1
			}
			if (m_sram_or_cart_map)
				data = snes_swc_sram_r(address);
			else
				data = snes_swc_cart_r(offset);
		}
		else if (address < 0xe000)  // i/o regs  c000-dfff
		{
			if (!machine().side_effects_disabled() && SWC_DEBUG)
				logerror("swc i/o regs rd: %06x\n", offset);
			data = snes_swc_io_r(address);
		}
		else                        // ?  e000-ffff  (bios in rom banks 00-01 only)
		{
			if (!machine().side_effects_disabled() && SWC_DEBUG)
				logerror("? rd: %06x\n", offset);
			data = snes_open_bus_r();  // or cart if present?
		}
	}

	return data;
}

inline void snes_swc_state::snes_swc_mode_0_w(address_space &space, offs_t offset, uint8_t data)
{
	uint16_t address = offset & 0xffff;

	if (offset < 0x400000)          // bank 00-3f
	{
		if (address < 0x2000)       // ram  0000-1fff
		{
			m_wram[address] = data;
		}
		else if (address < 0x6000)  // io  2000-5fff
		{
			snes_w_io(space, address, data);
		}
		else if (address < 0x8000)  // hi cart sram  6000-7fff   2.8cc uses bank 30 for the check
		{
			if ((offset >= 0x300000) && (offset < 0x340000) && m_cart_type && m_cart_sram_size && m_cartslot && m_cartslot->exists())
			{
				address &= 0x1fff;
				address |= (offset & 0x30000) >> 3;  // Kaite Tsukutte Asoberu Dezaemon (Japan) has 128KB/1mbit sram (banks 30-3f)
				address &= m_cart_sram_size - 1;
				if (SWC_DEBUG)
					logerror("hi cart sram wr: %04x %02x\n", address, data);
				m_cart_sram[address] = data;
			}
		}
		else if (address < 0xa000)  // dram  8000-9fff
		{
			if (SWC_DEBUG)
				logerror("dram wr: 8000-9fff:%04x:%d -> %06x %02x\n", (address & 0x1fff), m_page, (m_page << 13) | (offset & 0xff9fff), data);
			snes_swc_dram_w(offset, data);
		}
		else if (address < 0xc000)  // sram or cart  a000-bfff
		{
			if (SWC_DEBUG)
			{
				if (m_sram_or_cart_map)
					logerror("sram wr: a000-bfff:%04x:%d %02x\n", (address & 0x1fff), m_page, data);
				else
					logerror("cart wr: a000-bfff:%04x:%d -> %06x %02x\n", (address & 0x1fff), m_page, (m_page << 13) | (offset & 0xff9fff), data);  // 0x008000 == 1
			}
			if (m_sram_or_cart_map)
				snes_swc_sram_w(address, data);
			else
				snes_swc_cart_w(address, data);
		}
		else if (address < 0xe000)  // i/o regs  c000-dfff
		{
			if (SWC_DEBUG)
				logerror("swc i/o regs wr: %06x %02x\n", offset, data);
			snes_swc_io_w(address, data);
		}
		else                        // page select, mode select, others  e000-ffff
		{
			if (SWC_DEBUG)
				logerror("swc misc regs wr: %06x %02x\n", offset, data);
			snes_swc_misc_w(address, data);
		}
	}
	else  // < 0x7e0000                bank 40-7d
	{
		if (address < 0x2000)       // ?  0000-1fff
		{
			if (SWC_DEBUG)
				logerror("? wr: %06x %02x\n", offset, data);
		}
		else if (address < 0x4000)  // sram or cart  2000-3fff
		{
			if (SWC_DEBUG)
			{
				if (m_sram_or_cart_map)
					logerror("sram wr: 2000-3fff:%04x:%d %02x\n", (address & 0x1fff), m_page, data);
				else
					logerror("cart wr: 2000-3fff:%04x:%d -> %06x %02x\n", (address & 0x1fff), m_page, (m_page << 13) | (offset & 0xff9fff), data);  // 0x008000 == 0
			}
			if (m_sram_or_cart_map)
				snes_swc_sram_w(address, data);
			else
				snes_swc_cart_w(address, data);
		}
		else if (address < 0x8000)  // ?  4000-7fff
		{
			if (SWC_DEBUG)
				logerror("? wr: %06x %02x\n", offset, data);
		}
		else if (address < 0xa000)  // dram  8000-9fff
		{
			if (SWC_DEBUG)
				logerror("dram wr: 8000-9fff:%04x:%d -> %06x %02x\n", (address & 0x1fff), m_page, (m_page << 13) | (offset & 0xff9fff), data);
			snes_swc_dram_w(offset, data);
		}
		else if (address < 0xc000)  // sram or cart  a000-bfff
		{
			if (SWC_DEBUG)
			{
				if (m_sram_or_cart_map)
					logerror("sram wr: a000-bfff:%04x:%d %02x\n", (address & 0x1fff), m_page, data);
				else
					logerror("cart wr: a000-bfff:%04x:%d -> %06x %02x\n", (address & 0x1fff), m_page, (m_page << 13) | (offset & 0xff9fff), data);  // 0x008000 == 1
			}
			if (m_sram_or_cart_map)
				snes_swc_sram_w(address, data);
			else
				snes_swc_cart_w(address, data);
		}
		else if (address < 0xe000)  // i/o regs  c000-dfff
		{
			if (SWC_DEBUG)
				logerror("swc i/o regs wr: %06x %02x\n", offset, data);
			snes_swc_io_w(address, data);
		}
		else                        // page select, mode select, others  e000-ffff
		{
			if (SWC_DEBUG)
				logerror("swc misc regs wr: %06x %02x\n", offset, data);
			snes_swc_misc_w(address, data);
		}
	}
}


// 00-7d,80-ff : c00x rd  fdc & parallel i/o
inline uint8_t snes_swc_state::snes_swc_io_r(uint16_t address)
{
	uint8_t data = 0;

	switch (address & 0xf)
	{
		case 0:  // fdc  input  6: index, 7: irq
			data = m_fdc->input_r();
			if (!machine().side_effects_disabled() && SWC_DEBUG)
				logerror(" fdc input reg rd: %04x %02x\n", address, data);
			// 5: parallel strobe
			data |= par_debug_str & 0x20;
			break;

		case 4:  // fdc  main status
			data = m_fdc->msr_r();
			if (!machine().side_effects_disabled() && SWC_DEBUG)
				logerror(" fdc main status reg rd: %04x %02x\n", address, data);
			break;

		case 5:  // fdc  data
			data = m_fdc->fifo_r();
			if (!machine().side_effects_disabled() && SWC_DEBUG)
				logerror(" fdc data reg rd: %04x %02x\n", address, data);
			break;

		case 7:  // fdc  digital input
			data = m_fdc->dir_r();
			if (!machine().side_effects_disabled() && SWC_DEBUG)
				logerror(" fdc digital input reg rd: %04x %02x\n", address, data);
			break;

		case 8:  // parallel in
			data = par_debug_data_in;
			m_busy ^= 1;
			if (!machine().side_effects_disabled() && SWC_DEBUG)
				logerror(" parallel data rd: %04x\n", address);
			break;

		case 9:  // busy flag (EP1810 ver)
			if (!machine().side_effects_disabled() && SWC_DEBUG)
				logerror(" [EP1810 ver] busy flag rd: %04x\n", address);
			break;

		default:
			if (!machine().side_effects_disabled() && SWC_DEBUG)
				logerror(" unknown c00x rd: %04x\n", address);
			break;
	}

	return data;
}

// 00-7d,80-ff : c00x wr  fdc & parallel i/o
inline void snes_swc_state::snes_swc_io_w(uint16_t address, uint8_t data)
{
	switch (address & 0xf)
	{
		case 2:  // fdc  digital output
			m_fdc->dor_w(data);
			if (SWC_DEBUG)
				logerror(" fdc digital output reg wr: %04x %02x\n", address, data);
			m_fdc->set_floppy((data & 0x20) ? m_fdd->get_device() : nullptr);
			m_fdd->get_device()->mon_w((data & 0x20) ? 0 : 1);
			break;

		case 5:  // fdc  data
			m_fdc->fifo_w(data);
			if (SWC_DEBUG)
				logerror(" fdc data reg wr: %04x %02x\n", address, data);
			break;

		case 7:  // fdc  disk control
			m_fdc->ccr_w(data);
			if (SWC_DEBUG)
				logerror(" fdc disk control reg wr: %04x %02x\n", address, data);
			break;

		case 8:  // parallel out & dram/sram mapping

			m_rom_map = data & 1;
			m_sram_map = data & 2;

			if (SWC_DEBUG)
			{
				logerror(" parallel data / map mode wr: %04x %02x\n", address, data);
				if (!m_rom_map)
					logerror("  lorom map set\n");
				else
					logerror("  hirom map set\n");

				if (!m_sram_map)
					logerror("  lo sram map set\n");
				else
					logerror("  hi sram map set\n");
			}

			// write parallel port
			par_debug_data_out = data & 0xf;
			//m_busy = (data >> 4) & 1;
			break;

		default:
			if (SWC_DEBUG)
				logerror(" unknown c00x i/o wr: %04x %02x\n", address, data);
			break;
	}
	// TODO: mirrors
}

// 00-7d,80-ff : e00x wr  misc  page select, mode select, others
inline void snes_swc_state::snes_swc_misc_w(uint16_t address, uint8_t data)
{
	switch (address & 0xf)  // data not used
	{
		case 0:  // page select
		case 1:
		case 2:
		case 3:
			m_page = address & 3;
			if (SWC_DEBUG)
				logerror(" page select wr: page = %d\n", m_page);
			break;

		case 4:  // system mode select
		case 5:
		case 6:
		case 7:
			m_sys_mode = address & 3;
			if (SWC_DEBUG)
				logerror(" system mode select wr: mode = %d\n", m_sys_mode);
			break;

		case 8:  // dram type
		case 9:
			m_dram_type = address & 1;
			if (SWC_DEBUG)
			{
				logerror(" dram type select wr:\n");
				if (m_dram_type)
					logerror("  441000 dram selected (8,16,24,32 MB)\n");
				else
					logerror("  44256 dram selected (2,4,6,8 MB)\n");
			}
			break;

		case 0xc:  // cart/sram mapped @ 2000-3fff & a000-bfff
		case 0xd:
			m_sram_or_cart_map = address & 1;
			if (SWC_DEBUG)
			{
				logerror(" cart/sram mapping select wr:\n");
				if (m_sram_or_cart_map)
					logerror("  enabled sram @ 2000-3fff, a000-bfff\n");
				else
					logerror("  enabled cart @ 2000-3fff, a000-bfff\n");
			}
			break;

		default:
			if (SWC_DEBUG)
				logerror(" unknown e00x misc wr: %04x %02x\n", address, data);
			break;
	}
}


inline void snes_swc_state::snes_swc_dram_w(offs_t offset, uint8_t data)
{
	uint32_t address = m_page << 13;

	// needed for mode 2 "memory mode", when it patches hirom
	if (!m_rom_map)  // lo
		address |= (offset & 0x1fff) | ((offset & 0xff0000) >> 1);
	else  // hi
		address |= (offset & 0xff9fff);  // 0x008000 == 1

	m_dram[address] = (address < m_dram_size) ? data : 0;
	if (SWC_DEBUG)
		logerror(" dram wr: %06x %02x\n", address, data);
}

inline uint8_t snes_swc_state::snes_swc_dram_r(offs_t offset)
{
	uint32_t address = m_page << 13;

	if (!m_rom_map)  // lo
		address |= (offset & 0x1fff) | ((offset & 0xff0000) >> 1);
	else  // hi
		address |= (offset & 0xff9fff);  // 0x008000 == 1

	uint8_t data = (address < m_dram_size) ? m_dram[address] : 0;
	if (!machine().side_effects_disabled() && SWC_DEBUG)
		logerror(" dram rd: %06x %02x\n", address, data);
	return data;
}


inline void snes_swc_state::snes_swc_sram_w(uint16_t address, uint8_t data)
{
	address = (address & 0x1fff) | (m_page << 13);
	m_sram[address] = data;
	if (SWC_DEBUG)
		logerror(" swc sram wr: %04x %02x\n", address, data);
}

inline uint8_t snes_swc_state::snes_swc_sram_r(uint16_t address)
{
	address = (address & 0x1fff) | (m_page << 13);
	uint8_t data = m_sram[address];
	if (!machine().side_effects_disabled() && SWC_DEBUG)
		logerror(" swc sram rd: %04x %02x\n", address, data);
	return data;
}


inline void snes_swc_state::snes_swc_cart_w(uint16_t address, uint8_t data)
{
	if (m_cartslot && m_cartslot->exists())
	{
		if (m_cart_sram_size && !m_cart_type)  // && ...
		{
			address = (address & 0x1fff) | (m_page << 13);
			address &= m_cart_sram_size - 1;
			m_cart_sram[address] = data;
			if (SWC_DEBUG)
				logerror(" lo cart sram wr: %04x %02x\n", address, data);
		}
	}
	else
	{
		if (SWC_DEBUG)
			logerror(" cart wr: no cart!\n");
	}
}

inline uint8_t snes_swc_state::snes_swc_cart_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (m_cartslot && m_cartslot->exists())
	{
		uint32_t address = (m_page << 13) | (offset & 0xff9fff);

		if (m_cart_sram_size && !m_cart_type && (address >= 0x700000) && ((address & 0xffff) < 0x8000))
		{
			// tests lo sram at 706000
			// clashes with rom in a 32mbit+sram hi cart (70-7d,f0-ff:0000-7fff)
			// 2.8cc seems to use only the >8000 area for rom size check
			address &= m_cart_sram_size - 1;
			data = m_cart_sram[address];
			if (!machine().side_effects_disabled() && SWC_DEBUG)
				logerror(" lo cart sram rd: %04x %02x\n", address, data);
		}
		else
		{
			if (!m_cart_type)  // lo
				address = (m_page << 13) | (offset & 0x1fff) | ((offset & 0xff0000) >> 1);

			address = snes_swc_clamp_rom_size(address);
			data = m_cart[address];
			if (!machine().side_effects_disabled() && SWC_DEBUG)
				logerror(" cart rom rd: %06x %02x\n", address, data);
		}
	}
	else
	{
		if (!machine().side_effects_disabled() && SWC_DEBUG)
			logerror(" cart rd: no cart!\n");
	}

	return data;
}


// mode 1 - play cart

inline uint8_t snes_swc_state::snes_swc_mode_1_r(offs_t offset)
{
	uint8_t data = 0xff;
	uint16_t address = offset & 0xffff;

	if (offset < 0x400000)          // bank 00-3f
	{
		if (address < 0x2000)       // ram  0000-1fff
		{
			data = m_wram[address];
		}
		else if (address < 0x6000)  // io  2000-5fff
		{
			data = snes_r_io(address);
		}
		else if (address < 0x8000)  //     6000-7fff
		{
			if (m_cart_type && m_cart_sram_size && (offset >= 0x300000) && (offset < 0x340000))  // hi cart sram  30-33,b0-b3:6000-7fff
			{
				address &= 0x1fff;
				address |= (offset & 0x30000) >> 3;  // Kaite Tsukutte Asoberu Dezaemon (Japan) has 128KB/1mbit sram (banks 30-3f)
				address &= m_cart_sram_size - 1;
				data = m_cart_sram[address];
				if (!machine().side_effects_disabled() && SWC_DEBUG)
					logerror("hi cart sram rd: %04x %02x\n", address, data);
			}
			else
				data = snes_open_bus_r();
		}
		else                        // 8000-ffff
		{
			data = snes_swc_mode_1_rom_access(offset);
		}
	}
	else  // < 0x7e0000                bank 40-7d
	{
		if (!m_cart_type && m_cart_sram_size && (offset >= 0x700000) && (address < 0x8000))  // lo cart sram  70-7d,f0-ff:0000-7fff
		{
			address &= m_cart_sram_size - 1;
			data = m_cart_sram[address];
			if (!machine().side_effects_disabled() && SWC_DEBUG)
				logerror("lo cart sram rd: %04x %02x\n", address, data);
		}
		else
			data = snes_swc_mode_1_rom_access(offset);
	}

	return data;
}

inline void snes_swc_state::snes_swc_mode_1_w(address_space &space, offs_t offset, uint8_t data)
{
	uint16_t address = offset & 0xffff;

	if (offset < 0x400000)          // bank 00-3f
	{
		if (address < 0x2000)       // ram  0000-1fff
		{
			m_wram[address] = data;
		}
		else if (address < 0x6000)  // io  2000-5fff
		{
			snes_w_io(space, address, data);
		}
		else                        // 6000-ffff
		{
			if (m_cart_type && m_cart_sram_size && (offset >= 0x300000) && (offset < 0x340000) && (address < 0x8000))  // hi cart sram  30-33,b0-b3:6000-7fff
			{
				address &= 0x1fff;
				address |= (offset & 0x30000) >> 3;  // Kaite Tsukutte Asoberu Dezaemon (Japan) has 128KB/1mbit sram (banks 30-3f)
				address &= m_cart_sram_size - 1;
				if (SWC_DEBUG)
					logerror("hi cart sram wr: %04x %02x\n", address, data);
				m_cart_sram[address] = data;
			}
		}
	}
	else  // < 0x7e0000                bank 40-7d
	{
		if (!m_cart_type && m_cart_sram_size && (offset >= 0x700000) && (address < 0x8000))  // lo cart sram  70-7d,f0-ff:0000-7fff
		{
			address &= m_cart_sram_size - 1;
			if (SWC_DEBUG)
				logerror("lo cart sram wr: %04x %02x\n", address, data);
			m_cart_sram[address] = data;
		}
	}
}

inline uint8_t snes_swc_state::snes_swc_mode_1_rom_access(offs_t offset)
{
	uint8_t data = 0xff;

	if (m_cartslot && m_cartslot->exists())
	{
		uint32_t address;

		if (!m_cart_type)  // lo
		{
			address = (offset & 0x7fff) | ((offset & 0xff0000) >> 1);
		}
		else  // hi
		{
			address = offset;
		}

		data = m_cart[snes_swc_clamp_rom_size(address)];
	}

	return data;
}


// mode 2 - play dram (with mode select registers enabled)

inline uint8_t snes_swc_state::snes_swc_mode_2_r(offs_t offset)
{
	uint8_t data = 0xff;
	uint16_t address = offset & 0xffff;

	if (offset < 0x400000)          // bank 00-3f
	{
		if (address < 0x2000)       // ram  0000-1fff
		{
			data = m_wram[address];
		}
		else if (address < 0x6000)  // io  2000-5fff
		{
			data = snes_r_io(address);
		}
		else if (address < 0x8000)  //     6000-7fff
		{
			if (m_sram_map && (offset >= 0x300000) && (offset < 0x340000))  // hi cart sram  30-33,b0-b3:6000-7fff
			{
				address &= 0x1fff;
				address |= (offset & 0x30000) >> 3;  // swc has 32KB, can't do 128KB !
				data = m_sram[address];
				if (!machine().side_effects_disabled() && SWC_DEBUG)
					logerror("hi swc sram rd: %04x %02x\n", address, data);
			}
			else
				data = snes_open_bus_r();
		}
		else                        // 8000-ffff
		{
			data = snes_swc_mode_2_rom_access(offset);
		}
	}
	else  // < 0x7e0000                bank 40-7d
	{
		if (!m_sram_map && (offset >= 0x700000) && (address < 0x8000))  // lo cart sram  70-7d,f0-ff:0000-7fff
		{
			address &= 0x8000 - 1;  // swc has 32KB
			data = m_sram[address];
			if (!machine().side_effects_disabled() && SWC_DEBUG)
				logerror("lo swc sram rd: %04x %02x\n", address, data);
		}
		else
			data = snes_swc_mode_2_rom_access(offset);
	}

	return data;
}

inline void snes_swc_state::snes_swc_mode_2_w(address_space &space, offs_t offset, uint8_t data)
{
	uint16_t address = offset & 0xffff;

	if (address >= 0xe000)  // e000-ffff
	{
		if (SWC_DEBUG)
			logerror("swc mode 2 misc regs wr: %06x %02x\n", offset, data);

		switch (address & 0xf)  // data not used
		{
			case 4:  // system mode select
			case 5:
			case 6:
			case 7:
				m_sys_mode = address & 3;
				if (SWC_DEBUG)
					logerror(" system mode select wr: mode = %d\n", m_sys_mode);
				break;
			default:
				if (SWC_DEBUG)
					logerror(" not a system mode select address...\n");
				break;
		}
	}
	else
		snes_swc_mode_3_w(space, offset, data);
}

inline uint8_t snes_swc_state::snes_swc_mode_2_rom_access(offs_t offset)
{
	uint8_t data = 0xff;
	uint32_t address;

	if (!m_rom_map)  // lo
	{
		address = (offset & 0x7fff) | ((offset & 0xff0000) >> 1);
	}
	else  // hi
	{
		address = offset;
	}

	data = ((address & (0x400000 - 1)) < m_dram_size) ? m_dram[address & (0x400000 - 1)] : 0;

	return data;
}


// mode 3 - play dram

inline void snes_swc_state::snes_swc_mode_3_w(address_space &space, offs_t offset, uint8_t data)
{
	uint16_t address = offset & 0xffff;

	if (offset < 0x400000)          // bank 00-3f
	{
		if (address < 0x2000)       // ram  0000-1fff
		{
			m_wram[address] = data;
		}
		else if (address < 0x6000)  // io  2000-5fff
		{
			snes_w_io(space, address, data);
		}
		else                        // 6000-ffff
		{
			if (m_sram_map && (offset >= 0x300000) && (offset < 0x340000) && (address < 0x8000))  // hi cart sram  30-33,b0-b3:6000-7fff
			{
				address &= 0x1fff;
				address |= (offset & 0x30000) >> 3;  // swc has 32KB, can't do 128KB !
				if (SWC_DEBUG)
					logerror("hi swc sram wr: %04x %02x\n", address, data);
				m_sram[address] = data;
			}
		}
	}
	else  // < 0x7e0000                bank 40-7d
	{
		if (!m_sram_map && (offset >= 0x700000) && (address < 0x8000))  // lo cart sram  70-7d,f0-ff:0000-7fff
		{
			address &= 0x8000 - 1;  // swc has 32KB
			if (SWC_DEBUG)
				logerror("lo swc sram wr: %04x %02x\n", address, data);
			m_sram[address] = data;
		}
	}
}


ROM_START( snes_swc )
	ROM_REGION( 0x4000, "bios", 0 )

	// 2.8
	ROM_SYSTEM_BIOS( 0, "28a", "v2.8cc 94-06-28" )  // 1994 JSI FRONT FAREAST CO. VER 2.8CC
	ROMX_LOAD( "swc_28cc_940628.bin", 0x0000, 0x4000, CRC(6e14fce2) SHA1(05b69eb087531e488e8a7ece9437982b4e335e18), ROM_BIOS(0))  // Super Wild Card V2.8CC 06-28-94 BIOS [!].smc

	ROM_SYSTEM_BIOS( 1, "28b", "v2.8cc 94-06-08" )  // 1994 JSI FRONT FAREAST CO. VER 2.8CC
	ROMX_LOAD( "swc_28cc_940608.bin", 0x0000, 0x4000, CRC(feddeabc) SHA1(b857c1427dfdd6877d3e989f0513445027db66f5), ROM_BIOS(1))  // Super Wild Card V2.8CC 06-08-94 BIOS [!].smc

	ROM_SYSTEM_BIOS( 2, "28c", "v2.8cc 94-06-08 alt" )  // 1994 JSI FRONT FAREAST CO. VER 2.8CC
	ROMX_LOAD( "swc_28cc_940608_a.bin", 0x0000, 0x4000, CRC(1086cb9d) SHA1(6fc8ca97b624bd0df3450724fd7828943c263251), ROM_BIOS(2))  // Super Wild Card V2.8CC 06-08-94 BIOS [h1].smc

	ROM_SYSTEM_BIOS( 3, "28d", "v2.8cc 94-06-08 hack" )  // 1994 FFE SPRINT ELEC 1994. VER 2.8CC
	ROMX_LOAD( "swc_28cc_940608_h.bin", 0x0000, 0x4000, CRC(7dba7a90) SHA1(80f7d879123d157986c80ed5159c0c203c421d78), ROM_BIOS(3))  // Super Wild Card V2.8CC 06-08-94 BIOS [h2].smc

	// 2.7
	ROM_SYSTEM_BIOS( 4, "27a", "v2.7cc 93-12-07" )  // 1993 JSI FRONT FAREAST CO. VER 2.7CC
	ROMX_LOAD( "swc_27cc_931207.bin", 0x0000, 0x4000, CRC(164c9643) SHA1(1e11bd728fb7bb9458f9215b3dd2bdc2eda0ab56), ROM_BIOS(4))  // swc2.7.bin

	ROM_SYSTEM_BIOS( 5, "27b", "v2.7c 93-12-07 hack" )  // 1993 JSI FRONT FAREAST CO. V2.7C/ADJ
	ROMX_LOAD( "swc_27c_931207_h.bin", 0x0000, 0x4000, CRC(da06c27b) SHA1(af9179205cc233ad2e390e9b5a6fcf29c0564648), ROM_BIOS(5))  // Super Wild Card V2.7CC BIOS.smc

	// 2.6
	ROM_SYSTEM_BIOS( 6, "26a", "v2.6cc 93-07-17" )  // 1993 JSI FRONT FAREAST CO. VER 2.6CC
	ROMX_LOAD( "swc_26cc_930717.bin", 0x0000, 0x4000, CRC(b5875ac1) SHA1(051396e8d5b30d3aed765170c6146df957fcc886), ROM_BIOS(6))  // Super Wild Card V2.6C BIOS.smc

	ROM_SYSTEM_BIOS( 7, "26b", "v2.6f 93-07-17 hack 1" )  // 7-93 JSI FRONT FAREAST CO. V2.6F ADJ
	ROMX_LOAD( "swc_26f_930717_h1.bin", 0x0000, 0x4000, CRC(f2339837) SHA1(c3f4fe9ce3d78d199227f38339101727371cedfa), ROM_BIOS(7))  // Super Wild Card V2.6F BIOS.smc

	ROM_SYSTEM_BIOS( 8, "26c", "v2.6f 93-07-17 hack 2" )  // 7-93 JSI FRONT FAREAST CO. V2.6F ADJ
	ROMX_LOAD( "swc_26f_930717_h2.bin", 0x0000, 0x4000, CRC(71db04ce) SHA1(0f1918d0f210e61fe9050472d627af3a30a14b43), ROM_BIOS(8))  // Super Wild Card V2.6FX BIOS.smc

	ROM_SYSTEM_BIOS( 9, "26d", "v2.6 hack 1" )  // 94 TRSI
	ROMX_LOAD( "swc_26_h1.bin", 0x0000, 0x4000, CRC(9ff3d39c) SHA1(dfd3c04d1689bb71f288ffd5097c92fb4be0887b), ROM_BIOS(9))  // Super Wild Card V2.6 BIOS (TRSI Hack).smc

	ROM_SYSTEM_BIOS( 10, "26e", "v2.6 hack 2" )  // 1994 ROC
	ROMX_LOAD( "swc_26_h2.bin", 0x0000, 0x4000, CRC(e2d3093c) SHA1(784dc127563c0dd8517d2150fbbaaaa39e273498), ROM_BIOS(10))  // Super Wild Card V2.6 BIOS [h1].smc

	// 2.2
	ROM_SYSTEM_BIOS( 11, "22a", "v2.2cc 93-05-03 hack" )  // 1993 JSI CYRUS CONSOLES 93 VER 2.2CC
	ROMX_LOAD( "swc_22cc_930503_h.bin", 0x0000, 0x4000, CRC(3f1a6265) SHA1(2defacf474e0ece35e041612ebe9068f198fc567), ROM_BIOS(11))  // Super Wild Card V2.2C BIOS.smc

	// 2.1
	ROM_SYSTEM_BIOS( 12, "21a", "v2.1c 93-04-28 hack" )  // 1993 JSI FRONT FAREAST CO. V2.1C ADJ
	ROMX_LOAD( "swc_21c_930428_h.bin", 0x0000, 0x4000, CRC(4e3e713a) SHA1(547118a6bc065922211bf8b04ab56c5cb23bc178), ROM_BIOS(12))  // Super Wild Card V2.1C BIOS.smc

	ROM_SYSTEM_BIOS( 13, "21b", "v2.1b 93-04-28 hack" )  // 1993 JSI FRONT FAREAST CO. V2.1B ADJ
	ROMX_LOAD( "swc_21b_930428_h.bin", 0x0000, 0x4000, CRC(38c44f50) SHA1(dc1bcae475080f99bc42fd722bd961e3bf3d6a40), ROM_BIOS(13))  // Super Wild Card V2.1B BIOS.smc

	// 2.0
	ROM_SYSTEM_BIOS( 14, "20a", "v2.0xl 93-04-12 hack 1" )  // 4-93 JSI FRONT FAREAST CO. VER 2.0 VER2.0XL *ADJ*
	ROMX_LOAD( "swc_20xl_930412_h1.bin", 0x0000, 0x4000, CRC(d39ddb2e) SHA1(22a19d25edf1bdc5aee9c0109e5020e1de0872ec), ROM_BIOS(14))  // Super Wild Card V2.0XL BIOS.smc

	ROM_SYSTEM_BIOS( 15, "20b", "v2.0xl 93-04-12 hack 2" )  // 4-93 JSI FAIRLIGHT TRADING VER 2.0 VER2.0XL *ADJ*
	ROMX_LOAD( "swc_20xl_930412_h2.bin", 0x0000, 0x4000, CRC(8ca0c1cf) SHA1(efece5761bc416cbe31e4acd3d9f789b2a252bcb), ROM_BIOS(15))  // Super Wild Card V2.0XL BIOS [h1].smc

	// 1.8
	ROM_SYSTEM_BIOS( 16, "18a", "v1.8 93-02-19" )  // 1993 JSI FRONT FAREAST CO. VER 1.8
	ROMX_LOAD( "swc_18_930219.bin", 0x0000, 0x4000, CRC(84ed2837) SHA1(9584a011561a7c0b8b0ef5190b841ec46d0cae64), ROM_BIOS(16))  // Super Wild Card V1.8 BIOS.smc

	// 1.6
	ROM_SYSTEM_BIOS( 17, "16a", "v1.6c 93-01-29" )  // 1993 JSI FRONT FAREAST CO. VER 1.6C
	ROMX_LOAD( "swc_16a_930129.bin", 0x0000, 0x4000, CRC(add03d8f) SHA1(41ae4a37518e4daa8a56c38659ba33c3e6199c88), ROM_BIOS(17))  // Super Wild Card V1.6 BIOS.smc

ROM_END

#define rom_snes_swc_pal rom_snes_swc

ROM_START( snes_smc )
	ROM_REGION( 0x2000, "bios", 0 )

	// v31
	ROM_SYSTEM_BIOS( 0, "smca", "v31 92-xx-xx" )  // SUPER MAGICOM V31   1992 JSI   FRONT FAREAST COCCL H.K.   *** SUPER MAGICOM V3 *** COPYRIGHT 1992 BY JSI, FRONT FAREAST CO. ALL RIGHTS RES
	ROMX_LOAD( "smc_31.bin", 0x0000, 0x2000, CRC(a0fa40d3) SHA1(2570d247af30606e9a8b4c241b147859f5f79c90), ROM_BIOS(0))  // smc.bin, Super_Magicom_V31_ROM.bin, Super Magicom V31 BIOS.smc

	// v1h
	ROM_SYSTEM_BIOS( 1, "smcb", "v1h 91-12-23" )  // SUPER MAGICOM V1H   1991 JSI FRONT FAREAST CO. CCL H.K.   FRONT FAREAST COCCL H.K.   *** SUPER MAGICOM V-1T *** COPYRIGHT 1991 BY JSI, FRONT FAREAST CO. ALL RIGHTS RESERVED 10/20/91,12/23/91
	ROMX_LOAD( "smc_1h.bin", 0x0000, 0x2000, CRC(c7763b63) SHA1(03257beedc96e909813a03a687929e2f927bf64e), ROM_BIOS(1))  // Super Magicom V1H BIOS.smc

	// spcom2
	ROM_SYSTEM_BIOS( 2, "smcc", "Supercom Pro.2 92-06-21" )  // SUPERCOM PRO.2   1992 CCL   FRONT FAREAST COCCL H.K.   *** SUPERCOM PRO.2 *** COPYRIGHT 1992 BY JSI, CCL H.K. ALL RIGHTS RESERVED 6/21/92
	ROMX_LOAD( "smc_spcom2.bin", 0x0000, 0x2000, CRC(63199502) SHA1(2fa73958bceee5096e943262b854a3ea433bd16e), ROM_BIOS(2))  // spcom2.bin, SPCROM2.BIN

	// micro genius future supercom pro 9
	ROM_SYSTEM_BIOS( 3, "smcd", "microgenius" )  // 1992/09/01   FUTURE PRO.9   -- MICRO GENIUS --1993-
	ROMX_LOAD( "microgenius.bin", 0x0000, 0x2000, CRC(bbda5744) SHA1(fa9ea5d80317089f318e0b930b2af4435edbc838), ROM_BIOS(3))  // MICROGENIUS_CLEAN.smc

	// soft upgrades
	// HighRom.Dos     HIGHROM DOS FIX BY ARAGORN
	// MMLOADER.SMC    Mystic_Merlin FIG Loader v1  (for "Twin Supercom TS32 Pro.5"(NO DUMP) or "Micro Genius Future Supercom Pro.9")


	// these don't work - different ports  TODO...

	// SMCROM V3.1, fixed for the 'Fighter pro only' games Done by Carnivore/BeerMacht on 24-Jan-93 using BeerMon V0.44
	ROM_SYSTEM_BIOS( 4, "smce", "v31 Carnivore/BeerMacht 93-01-24" )  // HYPER MAGICOM V31   1992 JSI   CARNIVORE 24-JAN-93   *** SUPER MAGICOM V3 *** COPYRIGHT 1992 BY JSI, FRONT FAREAST CO. ALL RIGHTS RES
	ROMX_LOAD( "smchirom.bin", 0x0000, 0x2000, CRC(e5090801) SHA1(af3683c182ea9ae924c5528c3ee6315bc53f6e4d), ROM_BIOS(4))  // SMChiROM.BIN

	// above, reversed back to source
	ROM_SYSTEM_BIOS( 5, "smcf", "v31 xx-xx-xx" )  // HYPER MAGICOM V31   1992 JSI   FRONT FAREAST COCCL H.K.   *** SUPER MAGICOM V3 *** COPYRIGHT 1992 BY JSI, FRONT FAREAST CO. ALL RIGHTS RES
	ROMX_LOAD( "smchirom_unmod.bin", 0x0000, 0x2000, CRC(8c5f29c4) SHA1(244a6e38ce43da459b3fa83780521b6cea607cb3), ROM_BIOS(5))  // SMChiROM_unmod.BIN

	// SUPER MAGICOM DRIVEROM V3.1, fixed for the 'Fighter pro only' games Done by SDI on 31-Jan-93 using BeerMon V0.44
	ROM_SYSTEM_BIOS( 6, "smcg", "v31 SDI 93-01-31" )  // HIGHROM DOS FIX BY   TMS/SDI   *** SUPER MAGICOM V3 *** COPYRIGHT 1992 BY TMS, FRONT FAREAST CO. ALL RIGHTS RES
	ROMX_LOAD( "rom-smc_hir.bin", 0x0000, 0x2000, CRC(467f317a) SHA1(7d0bdf28db2d5a2cc5a168c53afad32f9bf3747e), ROM_BIOS(6))  // ROM-SMC.HIR

	// soft upgrades
	// Super Magicom V3 BIOS.bin    (add header from HighRom.Dos)  doesn't work - different ports         SUPER MAGICOM V3  ©92 TLPI!
	// Super Magicom V3H BIOS.bin   (add header from HighRom.Dos)  works                                  SUPER MAGICOM V3H ©92 TLPI!

ROM_END

#define rom_snes_smc_pal rom_snes_smc

//   YEAR,   NAME,      PARENT, COMPAT, MACHINE,      INPUT,     CLASS,          INIT,          COMPANY,         FULLNAME,              FLAGS

CONS( 1993, snes_swc,     0,        0, snes_swc,     snes_swc, snes_swc_state, init_snes_swc, "Front Fareast", "Super Wild Card SMS3201 (ntsc)", 0 )
CONS( 1993, snes_swc_pal, snes_swc, 0, snes_swc_pal, snes_swc, snes_swc_state, init_snes_swc, "Front Fareast", "Super Wild Card SMS3201 (pal)",  0 )

CONS( 1991, snes_smc,     0,        0, snes_swc,     snes_swc, snes_swc_state, init_snes_swc, "Front Fareast", "Super Magicom MS-3201 (ntsc)", 0 )
CONS( 1991, snes_smc_pal, snes_smc, 0, snes_swc_pal, snes_swc, snes_swc_state, init_snes_swc, "Front Fareast", "Super Magicom MS-3201 (pal)",  0 )
