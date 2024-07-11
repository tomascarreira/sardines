# Important stuff to not forget

## PPU

1. The PPU starts rendering immediately after power-on or reset, but ignores writes to most registers (specifically $2000, $2001, $2005 and $2006) until reaching the pre-render scanline of the next frame; more specifically, for around 29658 NTSC CPU cycles or 33132 PAL CPU cycles, assuming the CPU and PPU are reset at the same time. See PPU power up state and Init code for details. 

2. Ppu latch: Reading a nominally "write-only" register returns the latch's current value, as do the unused bits of PPUSTATUS. This value begins to decay after a frame or so, faster once the PPU has warmed up, and it is likely that values with alternating bit patterns (such as $55 or $AA) will decay faster.[2]

3. Reading palette RAM

Later PPUs added an unreliable feature for reading palette data from $3F00-$3FFF. These reads work differently than standard VRAM reads, as palette RAM is a separate memory space internal to the PPU that is overlaid onto the PPU address space. The referenced 6-bit palette data is returned immediately instead of going to the internal read buffer, and hence no priming read is required. Simultaneously, the PPU also performs a normal read from PPU memory at the specified address, "underneath" the palette data, and the result of this read goes into the read buffer as normal. The old contents of the read buffer are discarded when reading palettes, but by changing the address to point outside palette RAM and performing one read, the contents of this shadowed memory (usually mirrored nametables) can be accessed. On PPUs that do not support reading palette RAM, this memory range behaves the same as the rest of PPU memory.

This feature is supported by the 2C02G, 2C02H, and PAL PPUs. The byte returned when reading palettes contains PPU open bus in the top 2 bits, and the value is returned after it is modified by greyscale mode, which clears the bottom 4 bits if enabled. Unfortunately, on some consoles, palette reads can be corrupted on one of the 4 CPU/PPU alignments relative to the master clock. This corruption depends on when the PPU /CS signal that indicates register access is deasserted, which varies by console. Combined with this feature not being present in all PPUs, developers should not rely on reading from palette RAM. 

4. Read conflict with DPCM samples

If currently playing DPCM samples, there is a chance that an interruption from the APU's sample fetch will cause an extra read cycle if it happened at the same time as an instruction that reads $2007. This will cause an extra increment and a byte to be skipped over, resulting in the wrong data being read. See: APU DMC 

5. (writing to $2000) If the PPU is currently in vertical blank, and the PPUSTATUS ($2002) vblank flag is still set (1), changing the NMI flag in bit 7 of $2000 from 0 to 1 will immediately generate an NMI. This can result in graphical errors (most likely a misplaced scroll) if the NMI routine is executed too late in the blanking period to finish on time. To avoid this problem, it is prudent to read $2002 immediately to clear the vblank flag before writing $2000 to enable NMI. 

6. (writing to $2000) Master/slave mode and EXT pins; Bit 0 race condition -> check wiki

7. OAMADDR precautions

On the 2C02G, writes to OAMADDR corrupt OAM. The exact corruption isn't fully described, but this usually seems to copy sprites 8 and 9 (address $20) over the 8-byte row at the target address. The source address for this copy seems to come from the previous value on the CPU BUS (most often $20 from the $2003 operand).[3][4] There may be other possible behaviors as well. This can then be worked around by writing all 256 bytes of OAM, though due to the limited time before OAM decay will begin this should normally be done through OAMDMA.

It is also the case that if OAMADDR is not less than eight when rendering starts, the eight bytes starting at OAMADDR & 0xF8 are copied to the first eight bytes of OAM; it seems likely that this is related. On the Dendy, the latter bug is required for 2C02 compatibility.

It is known that in the 2C03, 2C04, 2C05[5], and 2C07, OAMADDR works as intended. It is not known whether this bug is present in all revisions of the 2C02. 

8. Typically, this register is written to during vertical blanking to make the next frame start rendering from the desired location, but it can also be modified during rendering in order to split the screen. Changes made to the vertical scroll during rendering will only take effect on the next frame. Together with the nametable bits in PPUCTRL, the scroll can be thought of as 9 bits per component, and PPUCTRL must be updated along with PPUSCROLL to fully specify the scroll position. 
9.
