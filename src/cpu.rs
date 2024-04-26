use crate::bus::Bus;

#[derive(Debug)]
pub struct Cpu {
    a: u8,
    x: u8,
    y: u8,
    s: u8,
    p: StatusFlag,
    pc: u16,
    instr_state: InstrState,
}

impl Cpu {
    pub fn new() -> Self {
        Cpu {
            a: 0,
            x: 0,
            y: 0,
            s: 0,
            p: StatusFlag::power_on(),
            pc: 1,
            instr_state: InstrState {
                instr: Instr::Brk,
                addr_mode: AddrMode::Implied,
                cycle: 2,
                fetch_opcode: false,
                saved_byte: 0,
                saved_addr: 0,
                page_crossed: false,
            },
        }
    }

    pub fn cycle(&mut self, bus: &mut Bus) {
        if self.instr_state.fetch_opcode {
            self.instr_state.fetch_opcode = false;
            let opcode = self.fetch_opcode(bus);

            let (instr, addr_mode) = decode(opcode);
            println!(
                "DEBUG: decoded {:x?} -> instr: {:?}, addr_mode: {:?}",
                opcode, instr, addr_mode
            );
            self.instr_state.instr = instr;
            self.instr_state.addr_mode = addr_mode;
            self.instr_state.cycle = 2;

            return;
        }

        match (
            self.instr_state.instr,
            self.instr_state.addr_mode,
            self.instr_state.cycle,
        ) {
            (Instr::Brk, AddrMode::Implied, 2) => {
                self.dummy_read(bus);
                self.pc += 1;
            }
            (Instr::Brk, AddrMode::Implied, 3) => {
                self.stack_push((self.pc >> 8) as u8, bus);
                self.s = self.s.wrapping_sub(1);
            }
            (Instr::Brk, AddrMode::Implied, 4) => {
                self.stack_push(self.pc as u8, bus);
                self.s = self.s.wrapping_sub(1);
            }
            (Instr::Brk, AddrMode::Implied, 5) => {
                self.stack_push(self.p.b_flag_set(), bus);
                self.s = self.s.wrapping_sub(1);
            }
            (Instr::Brk, AddrMode::Implied, 6) => {
                // Does it matter if pch is override by 0x0000
                // Does the cpu write to the pcl in this cycle, if not this it have a exterior effect?
                self.pc = self.read(0xfffe, bus) as u16;
            }
            (Instr::Brk, AddrMode::Implied, 7) => {
                self.pc |= (self.read(0xffff, bus) as u16) << 8;
                self.instr_state.fetch_opcode = true;
            }

            (Instr::Rti, AddrMode::Implied, 2) => {
                self.dummy_read(bus);
            }
            (Instr::Rti, AddrMode::Implied, 3) => {
                // Is dummy read before or after incrementing s
                self.stack_dummy_read(bus);
                self.s = self.s.wrapping_add(1);
            }
            (Instr::Rti, AddrMode::Implied, 4) => {
                self.p = StatusFlag::from(self.stack_pop(bus));
                self.s = self.s.wrapping_add(1);
            }
            (Instr::Rti, AddrMode::Implied, 5) => {
                self.pc = self.stack_pop(bus) as u16;
                self.s = self.s.wrapping_add(1);
            }
            (Instr::Rti, AddrMode::Implied, 6) => {
                self.pc |= (self.stack_pop(bus) as u16) << 8;
                self.instr_state.fetch_opcode = true;
            }

            (Instr::Rts, AddrMode::Implied, 2) => {
                self.dummy_read(bus);
            }
            (Instr::Rts, AddrMode::Implied, 3) => {
                self.stack_dummy_read(bus);
                self.s = self.s.wrapping_add(1);
            }
            (Instr::Rts, AddrMode::Implied, 4) => {
                self.pc = self.stack_pop(bus) as u16;
                self.s = self.s.wrapping_add(1);
            }
            (Instr::Rts, AddrMode::Implied, 5) => {
                self.pc |= (self.stack_pop(bus) as u16) << 8;
            }
            (Instr::Rts, AddrMode::Implied, 6) => {
                self.dummy_read(bus);
                self.pc += 1;
                self.instr_state.fetch_opcode = true;
            }

            (Instr::Pha | Instr::Php, AddrMode::Implied, 2) => {
                self.dummy_read(bus);
            }
            (instr @ (Instr::Pha | Instr::Php), AddrMode::Implied, 3) => {
                match instr {
                    Instr::Pha => self.stack_push(self.a, bus),
                    Instr::Php => self.stack_push(self.p.b_flag_set(), bus),
                    _ => unreachable!(),
                }
                self.s = self.s.wrapping_sub(1);
                self.instr_state.fetch_opcode = true;
            }

            (Instr::Pla | Instr::Plp, AddrMode::Implied, 2) => {
                self.dummy_read(bus);
            }
            (Instr::Pla | Instr::Plp, AddrMode::Implied, 3) => {
                self.stack_dummy_read(bus);
                self.s = self.s.wrapping_add(1);
            }
            (instr @ (Instr::Pla | Instr::Plp), AddrMode::Implied, 4) => {
                match instr {
                    Instr::Pha => self.a = self.stack_pop(bus),
                    Instr::Php => self.p = StatusFlag::from(self.stack_pop(bus)),
                    _ => unreachable!(),
                }
                self.instr_state.fetch_opcode = true;
            }

            (Instr::Jsr, AddrMode::Absolute, 2) => {
                self.instr_state.saved_byte = self.read(self.pc, bus);
                self.pc += 1;
            }
            (Instr::Jsr, AddrMode::Absolute, 3) => {
                self.stack_dummy_read(bus);
            }
            (Instr::Jsr, AddrMode::Absolute, 4) => {
                self.stack_push((self.pc >> 8) as u8, bus);
                self.s = self.s.wrapping_sub(1);
            }
            (Instr::Jsr, AddrMode::Absolute, 5) => {
                self.stack_push(self.pc as u8, bus);
                self.s = self.s.wrapping_sub(1);
            }
            (Instr::Jsr, AddrMode::Absolute, 6) => {
                self.pc =
                    self.instr_state.saved_byte as u16 | (self.read(self.pc, bus) as u16) << 8;
                self.instr_state.fetch_opcode = true;
            }

            (instr, AddrMode::Implied, 2) => {
                self.dummy_read(bus);
                self.do_opcode_imp_acc(instr);
                self.instr_state.fetch_opcode = true;
            }

            (instr, AddrMode::Accumulator, 2) => {
                self.dummy_read(bus);
                self.do_opcode_imp_acc(instr);
                self.instr_state.fetch_opcode = true;
            }

            (instr, AddrMode::Immediate, 2) => {
                let val = self.read(self.pc, bus);
                self.pc += 1;
                self.do_opcode_read(val, instr);
                self.instr_state.fetch_opcode = true;
            }

            (Instr::Jmp, AddrMode::Absolute, 2) => {
                self.instr_state.saved_byte = self.read(self.pc, bus);
                self.pc += 1;
            }
            (Instr::Jmp, AddrMode::Absolute, 3) => {
                self.pc =
                    self.instr_state.saved_byte as u16 | (self.read(self.pc, bus) as u16) << 8;
                self.instr_state.fetch_opcode = true;
            }

            (_, AddrMode::Absolute, 2) => {
                self.instr_state.saved_byte = self.read(self.pc, bus);
                self.pc += 1;
            }
            (_, AddrMode::Absolute, 3) => {
                self.instr_state.saved_addr =
                    self.instr_state.saved_byte as u16 | (self.read(self.pc, bus) as u16) << 8;
                self.pc += 1;
            }
            (
                instr @ (Instr::Lda
                | Instr::Ldx
                | Instr::Ldy
                | Instr::Eor
                | Instr::And
                | Instr::Ora
                | Instr::Adc
                | Instr::Sbc
                | Instr::Cmp
                | Instr::Cpx
                | Instr::Cpy
                | Instr::Bit),
                AddrMode::Absolute,
                4,
            ) => {
                let val = self.read(self.instr_state.saved_addr, bus);
                self.do_opcode_read(val, instr);
                self.instr_state.fetch_opcode = true;
            }

            (
                Instr::Asl | Instr::Lsr | Instr::Rol | Instr::Ror | Instr::Inc | Instr::Dec,
                AddrMode::Absolute,
                4,
            ) => {
                self.instr_state.saved_byte = self.read(self.instr_state.saved_addr, bus);
            }

            (
                instr @ (Instr::Asl
                | Instr::Lsr
                | Instr::Rol
                | Instr::Ror
                | Instr::Inc
                | Instr::Dec),
                AddrMode::Absolute,
                5,
            ) => {
                self.write(
                    self.instr_state.saved_byte,
                    self.instr_state.saved_addr,
                    bus,
                );
                self.instr_state.saved_byte =
                    self.do_opcode_rmw(self.instr_state.saved_byte, instr);
            }
            (
                Instr::Asl | Instr::Lsr | Instr::Rol | Instr::Ror | Instr::Inc | Instr::Dec,
                AddrMode::Absolute,
                6,
            ) => {
                self.write(
                    self.instr_state.saved_byte,
                    self.instr_state.saved_addr,
                    bus,
                );
                self.instr_state.fetch_opcode = true;
            }

            (instr @ (Instr::Sta | Instr::Stx | Instr::Sty), AddrMode::Absolute, 4) => {
                let val = self.do_opcode_write(instr);
                self.write(val, self.instr_state.saved_addr, bus);
                self.instr_state.fetch_opcode = true;
            }

            (_, AddrMode::Zeropage, 2) => {
                self.instr_state.saved_byte = self.read(self.pc, bus);
                self.pc += 1;
            }

            (
                instr @ (Instr::Lda
                | Instr::Ldx
                | Instr::Ldy
                | Instr::Eor
                | Instr::And
                | Instr::Ora
                | Instr::Adc
                | Instr::Sbc
                | Instr::Cmp
                | Instr::Cpx
                | Instr::Cpy
                | Instr::Bit),
                AddrMode::Zeropage,
                3,
            ) => {
                let addr =
                    self.instr_state.saved_byte as u16 | (self.instr_state.saved_byte as u16) << 8;
                self.do_opcode_read(self.read(addr, bus), instr);
                self.instr_state.fetch_opcode = true;
            }

            (
                Instr::Asl | Instr::Lsr | Instr::Rol | Instr::Ror | Instr::Inc | Instr::Dec,
                AddrMode::Zeropage,
                3,
            ) => {
                self.instr_state.saved_addr = self.instr_state.saved_byte as u16;
                self.instr_state.saved_byte = self.read(self.instr_state.saved_addr, bus);
            }
            (
                instr @ (Instr::Asl
                | Instr::Lsr
                | Instr::Rol
                | Instr::Ror
                | Instr::Inc
                | Instr::Dec),
                AddrMode::Zeropage,
                4,
            ) => {
                self.write(
                    self.instr_state.saved_byte,
                    self.instr_state.saved_addr,
                    bus,
                );
                self.instr_state.saved_byte =
                    self.do_opcode_rmw(self.instr_state.saved_byte, instr);
            }
            (
                Instr::Asl | Instr::Lsr | Instr::Rol | Instr::Ror | Instr::Inc | Instr::Dec,
                AddrMode::Zeropage,
                5,
            ) => {
                self.write(
                    self.instr_state.saved_byte,
                    self.instr_state.saved_addr,
                    bus,
                );
                self.instr_state.fetch_opcode = true;
            }

            (instr @ (Instr::Sta | Instr::Stx | Instr::Sty), AddrMode::Zeropage, 3) => {
                let addr =
                    self.instr_state.saved_byte as u16 | (self.instr_state.saved_byte as u16) << 8;
                let val = self.do_opcode_write(instr);
                self.write(val, addr, bus);
                self.instr_state.fetch_opcode = true;
            }

            (_, AddrMode::ZeropageX | AddrMode::ZeropageY, 2) => {
                self.instr_state.saved_byte = self.read(self.pc, bus);
                self.pc += 1;
            }
            (_, AddrMode::ZeropageX, 3) => {
                let addr =
                    self.instr_state.saved_byte as u16 | (self.instr_state.saved_byte as u16) << 8;
                self.instr_state.saved_addr = self.read(addr, bus).wrapping_add(self.x) as u16
            }

            (
                instr @ (Instr::Lda
                | Instr::Ldy
                | Instr::Eor
                | Instr::And
                | Instr::Ora
                | Instr::Adc
                | Instr::Sbc
                | Instr::Cmp),
                AddrMode::ZeropageX,
                4,
            ) => {
                let val = self.read(self.instr_state.saved_addr, bus);
                self.do_opcode_read(val, instr);
                self.instr_state.fetch_opcode = true;
            }

            (
                Instr::Asl | Instr::Lsr | Instr::Rol | Instr::Ror | Instr::Inc | Instr::Dec,
                AddrMode::ZeropageX,
                4,
            ) => {
                self.instr_state.saved_byte = self.read(self.instr_state.saved_addr, bus);
            }
            (
                instr @ (Instr::Asl
                | Instr::Lsr
                | Instr::Rol
                | Instr::Ror
                | Instr::Inc
                | Instr::Dec),
                AddrMode::ZeropageX,
                5,
            ) => {
                self.write(
                    self.instr_state.saved_byte,
                    self.instr_state.saved_addr,
                    bus,
                );
                self.instr_state.saved_byte =
                    self.do_opcode_rmw(self.instr_state.saved_byte, instr);
            }
            (
                Instr::Asl | Instr::Lsr | Instr::Rol | Instr::Ror | Instr::Inc | Instr::Dec,
                AddrMode::ZeropageX,
                6,
            ) => {
                self.write(
                    self.instr_state.saved_byte,
                    self.instr_state.saved_addr,
                    bus,
                );
                self.instr_state.fetch_opcode = true;
            }

            (instr @ (Instr::Sta | Instr::Sty), AddrMode::ZeropageX, 4) => {
                let val = self.do_opcode_write(instr);
                self.write(val, self.instr_state.saved_addr, bus);
                self.instr_state.fetch_opcode = true;
            }

            (_, AddrMode::ZeropageY, 3) => {
                let addr =
                    self.instr_state.saved_byte as u16 | (self.instr_state.saved_byte as u16) << 8;
                self.instr_state.saved_addr = self.read(addr, bus).wrapping_add(self.y) as u16
            }

            (instr @ Instr::Ldx, AddrMode::ZeropageY, 4) => {
                let val = self.read(self.instr_state.saved_addr, bus);
                self.do_opcode_read(val, instr);
                self.instr_state.fetch_opcode = true;
            }

            (instr @ Instr::Stx, AddrMode::ZeropageY, 4) => {
                let val = self.do_opcode_write(instr);
                self.write(val, self.instr_state.saved_addr, bus);
                self.instr_state.fetch_opcode = true;
            }

            (_, AddrMode::AbsoluteX | AddrMode::AbsoluteY, 2) => {
                self.instr_state.saved_byte = self.read(self.pc, bus);
                self.pc += 1;
            }
            (_, addr_mode @ (AddrMode::AbsoluteX | AddrMode::AbsoluteY), 3) => {
                let register = if let AddrMode::AbsoluteX = addr_mode {
                    self.x
                } else {
                    self.y
                };

                let addr_high = self.read(self.pc, bus);
                self.pc += 1;
                let (addr_low, page_crossed) =
                    self.instr_state.saved_byte.overflowing_add(register);
                self.instr_state.page_crossed = page_crossed;
                self.instr_state.saved_addr = addr_low as u16 | (addr_high as u16) << 8;
            }

            (
                instr @ (Instr::Lda
                | Instr::Ldx
                | Instr::Ldy
                | Instr::Eor
                | Instr::And
                | Instr::Ora
                | Instr::Adc
                | Instr::Sbc
                | Instr::Cmp),
                AddrMode::AbsoluteX | AddrMode::AbsoluteY,
                4,
            ) => {
                let val = self.read(self.instr_state.saved_addr, bus);
                if self.instr_state.page_crossed {
                    self.instr_state.saved_addr += 0x0100;
                } else {
                    self.do_opcode_read(val, instr);
                    self.instr_state.fetch_opcode = true;
                }
            }

            (
                instr @ (Instr::Lda
                | Instr::Ldy
                | Instr::Eor
                | Instr::And
                | Instr::Ora
                | Instr::Adc
                | Instr::Sbc
                | Instr::Cmp),
                AddrMode::AbsoluteX | AddrMode::AbsoluteY,
                5,
            ) => {
                let val = self.read(self.instr_state.saved_addr, bus);
                self.do_opcode_read(val, instr);
                self.instr_state.fetch_opcode = true;
            }

            (
                Instr::Asl | Instr::Lsr | Instr::Rol | Instr::Ror | Instr::Inc | Instr::Dec,
                AddrMode::AbsoluteX,
                4,
            ) => {
                let _ = self.read(self.instr_state.saved_addr, bus);
                if self.instr_state.page_crossed {
                    self.instr_state.saved_addr += 0x100;
                }
            }
            (
                Instr::Asl | Instr::Lsr | Instr::Rol | Instr::Ror | Instr::Inc | Instr::Dec,
                AddrMode::AbsoluteX,
                5,
            ) => {
                self.instr_state.saved_byte = self.read(self.instr_state.saved_addr, bus);
            }
            (
                instr @ (Instr::Asl
                | Instr::Lsr
                | Instr::Rol
                | Instr::Ror
                | Instr::Inc
                | Instr::Dec),
                AddrMode::AbsoluteX,
                6,
            ) => {
                self.write(
                    self.instr_state.saved_byte,
                    self.instr_state.saved_addr,
                    bus,
                );
                self.instr_state.saved_byte =
                    self.do_opcode_rmw(self.instr_state.saved_byte, instr);
            }
            (
                Instr::Asl | Instr::Lsr | Instr::Rol | Instr::Ror | Instr::Inc | Instr::Dec,
                AddrMode::AbsoluteX,
                7,
            ) => {
                self.write(
                    self.instr_state.saved_byte,
                    self.instr_state.saved_addr,
                    bus,
                );
                self.instr_state.fetch_opcode = true;
            }

            (Instr::Sta, AddrMode::AbsoluteX | AddrMode::AbsoluteY, 4) => {
                self.instr_state.saved_byte = self.read(self.instr_state.saved_addr, bus);
                if self.instr_state.page_crossed {
                    self.instr_state.saved_addr += 0x0100;
                }
            }

            (instr @ Instr::Sta, AddrMode::AbsoluteX | AddrMode::AbsoluteY, 5) => {
                let val = self.do_opcode_write(instr);
                self.write(val, self.instr_state.saved_addr, bus);
                self.instr_state.fetch_opcode = true;
            }

            (_, AddrMode::Relative, 2) => {
                self.instr_state.saved_byte = self.read(self.pc, bus);
                self.pc += 1;
            }
            (instr, AddrMode::Relative, 3) => {
                let next_opcode = self.read(self.pc, bus);
                if self.do_opcode_relative(instr) {
                    let (pcl, page_crossed) =
                        (self.pc as u8).overflowing_add(self.instr_state.saved_byte as i8 as u8);
                    self.pc = (self.pc & 0xff00) | pcl as u16;
                    self.instr_state.page_crossed = page_crossed;
                } else {
                    let (instr, addr_mode) = decode(next_opcode);
                    self.instr_state.instr = instr;
                    self.instr_state.addr_mode = addr_mode;
                    self.instr_state.cycle = 2;
                    self.pc += 1
                }
            }
            (_, AddrMode::Relative, 4) => {
                let next_opcode = self.read(self.pc, bus);
                if self.instr_state.page_crossed {
                    self.pc += 0x0100;
                } else {
                    let (instr, addr_mode) = decode(next_opcode);
                    self.instr_state.instr = instr;
                    self.instr_state.addr_mode = addr_mode;
                    self.instr_state.cycle = 2;
                    self.pc += 1
                }
            }
            (_, AddrMode::Relative, 5) => {
                let next_opcode = self.read(self.pc, bus);
                let (instr, addr_mode) = decode(next_opcode);
                self.instr_state.instr = instr;
                self.instr_state.addr_mode = addr_mode;
                self.instr_state.cycle = 2;
                self.pc += 1
            }

            (_, AddrMode::IndirectX, 2) => {
                self.instr_state.saved_byte = self.read(self.pc, bus);
                self.pc += 1;
            }
            (_, AddrMode::IndirectX, 3) => {
                self.instr_state.saved_byte = self
                    .read(self.instr_state.saved_byte as u16, bus)
                    .wrapping_add(self.x);
            }
            (_, AddrMode::IndirectX, 4) => {
                self.instr_state.saved_addr =
                    self.read(self.instr_state.saved_byte as u16, bus) as u16;
            }
            (_, AddrMode::IndirectX, 5) => {
                self.instr_state.saved_addr |=
                    (self.read(self.instr_state.saved_byte as u16, bus) as u16) << 8;
            }

            (
                instr @ (Instr::Lda
                | Instr::Ora
                | Instr::Eor
                | Instr::And
                | Instr::Adc
                | Instr::Sbc
                | Instr::Cmp),
                AddrMode::IndirectX,
                6,
            ) => {
                let val = self.read(self.instr_state.saved_addr, bus);
                self.do_opcode_read(val, instr);
                self.instr_state.fetch_opcode = true;
            }

            (instr @ Instr::Sta, AddrMode::IndirectX, 6) => {
                let val = self.do_opcode_write(instr);
                self.write(val, self.instr_state.saved_addr, bus);
                self.instr_state.fetch_opcode = true;
            }

            (_, AddrMode::IndirectY, 2) => {
                self.instr_state.saved_byte = self.read(self.pc, bus);
                self.pc += 1;
            }
            (_, AddrMode::IndirectY, 3) => {
                self.instr_state.saved_addr =
                    self.read(self.instr_state.saved_byte as u16, bus) as u16;
            }
            (_, AddrMode::IndirectY, 4) => {
                let addr_high = self.read(self.instr_state.saved_byte.wrapping_add(1) as u16, bus);
                let (addr_low, page_crossed) =
                    (self.instr_state.saved_byte as u8).overflowing_add(self.y);
                self.instr_state.saved_addr = addr_low as u16 | (addr_high as u16) << 8;
                self.instr_state.page_crossed = page_crossed;
            }

            (
                instr @ (Instr::Lda
                | Instr::Ora
                | Instr::Eor
                | Instr::And
                | Instr::Adc
                | Instr::Sbc
                | Instr::Cmp),
                AddrMode::IndirectY,
                5,
            ) => {
                let val = self.read(self.instr_state.saved_addr, bus);
                if self.instr_state.page_crossed {
                    self.instr_state.saved_addr += 0x0100;
                } else {
                    self.do_opcode_read(val, instr);
                    self.instr_state.fetch_opcode = true;
                }
            }
            (
                instr @ (Instr::Lda
                | Instr::Ora
                | Instr::Eor
                | Instr::And
                | Instr::Adc
                | Instr::Sbc
                | Instr::Cmp),
                AddrMode::IndirectY,
                6,
            ) => {
                let val = self.read(self.instr_state.saved_addr, bus);
                self.do_opcode_read(val, instr);
                self.instr_state.fetch_opcode = true;
            }

            (Instr::Sta, AddrMode::IndirectY, 5) => {
                self.read(self.instr_state.saved_addr, bus);
                if self.instr_state.page_crossed {
                    self.instr_state.saved_addr += 0x0100;
                }
            }
            (instr @ Instr::Sta, AddrMode::IndirectY, 6) => {
                let val = self.do_opcode_write(instr);
                self.write(val, self.instr_state.saved_addr, bus);
                self.instr_state.fetch_opcode = true;
            }

            (_, AddrMode::Indirect, 2) => {
                self.instr_state.saved_byte = self.read(self.pc, bus);
                self.pc += 1;
            }
            (_, AddrMode::Indirect, 3) => {
                self.instr_state.saved_addr =
                    self.instr_state.saved_byte as u16 | (self.read(self.pc, bus) as u16) << 8;
                self.pc += 1;
            }
            (_, AddrMode::Indirect, 4) => {
                self.instr_state.saved_byte = self.read(self.instr_state.saved_addr, bus);
            }
            (_, AddrMode::Indirect, 5) => {
                let addr_low = (self.instr_state.saved_addr as u8).wrapping_add(1);
                let pc_high =
                    self.read(self.instr_state.saved_addr & 0xff00 | addr_low as u16, bus);
                self.pc = self.instr_state.saved_byte as u16 | (pc_high as u16) << 8;
            }

            _ => todo!(),
        }

        self.instr_state.cycle += 1;
    }

    fn fetch_opcode(&mut self, bus: &Bus) -> u8 {
        let opcode = bus.read(self.pc);
        self.pc += 1;
        opcode
    }

    // Maybe use only one dummy_read function that takes de address
    fn dummy_read(&self, bus: &Bus) {
        bus.read(self.pc);
    }

    fn stack_dummy_read(&self, bus: &Bus) {
        bus.read(0x0100 + (self.s as u16));
    }

    fn read(&self, addr: u16, bus: &Bus) -> u8 {
        bus.read(addr)
    }

    fn write(&self, value: u8, addr: u16, bus: &mut Bus) {
        bus.write(value, addr);
    }

    fn stack_push(&self, val: u8, bus: &mut Bus) {
        bus.write(val, 0x0100 + (self.s as u16));
    }

    fn stack_pop(&self, bus: &Bus) -> u8 {
        bus.read(0x0100 + (self.s as u16))
    }

    fn do_opcode_imp_acc(&mut self, instr: Instr) {
        match instr {
            // Implied
            Instr::Clc => self.p.carry = false,
            Instr::Cld => self.p.decimal = false,
            Instr::Cli => self.p.interrupt_disable = false,
            Instr::Clv => self.p.overflow = false,
            Instr::Dex => self.dex(),
            Instr::Dey => self.dey(),
            Instr::Inx => self.inx(),
            Instr::Iny => self.iny(),
            Instr::Nop => (),
            Instr::Sec => self.p.carry = true,
            Instr::Sed => self.p.decimal = true,
            Instr::Sei => self.p.interrupt_disable = true,
            Instr::Tax => self.tax(),
            Instr::Tay => self.tay(),
            Instr::Tsx => self.tsx(),
            Instr::Txa => self.txa(),
            Instr::Txs => self.txs(),
            Instr::Tya => self.tya(),

            // Accumulator
            Instr::Asl => self.a = self.asl(self.a),
            Instr::Lsr => self.a = self.lsr(self.a),
            Instr::Rol => self.a = self.rol(self.a),
            Instr::Ror => self.a = self.ror(self.a),
            _ => {
                println!("{:?}", instr);
                unreachable!()
            }
        }
    }

    // fn do_opcode_imm(&mut self, value: u8, instr: Instr) {
    //     match instr {
    //         Instr::Adc => self.adc(value),
    //         Instr::And => self.and(value),
    //         Instr::Cmp => self.cmp(value),
    //         Instr::Cpx => self.cpx(value),
    //         Instr::Cpy => self.cpy(value),
    //         Instr::Eor => self.eor(value),
    //         Instr::Lda => self.lda(value),
    //         Instr::Ldx => self.ldx(value),
    //         Instr::Ldy => self.ldy(value),
    //         Instr::Ora => self.ora(value),
    //         Instr::Sbc => self.adc(!value),
    //         _ => unreachable!(),
    //     }
    // }

    fn do_opcode_read(&mut self, value: u8, instr: Instr) {
        match instr {
            Instr::Lda => self.lda(value),
            Instr::Ldx => self.ldx(value),
            Instr::Ldy => self.ldy(value),
            Instr::Eor => self.eor(value),
            Instr::And => self.and(value),
            Instr::Ora => self.ora(value),
            Instr::Adc => self.adc(value),
            Instr::Sbc => self.adc(!value),
            Instr::Cmp => self.cmp(value),
            Instr::Cpx => self.cpx(value),
            Instr::Cpy => self.cpy(value),
            Instr::Bit => self.bit(value),
            _ => unreachable!(),
        }
    }

    fn do_opcode_rmw(&mut self, value: u8, instr: Instr) -> u8 {
        match instr {
            Instr::Asl => self.asl(value),
            Instr::Lsr => self.lsr(value),
            Instr::Rol => self.rol(value),
            Instr::Ror => self.ror(value),
            Instr::Inc => self.inc(value),
            Instr::Dec => self.dec(value),
            _ => unreachable!(),
        }
    }

    fn do_opcode_write(&mut self, instr: Instr) -> u8 {
        match instr {
            Instr::Sta => self.a,
            Instr::Stx => self.x,
            Instr::Sty => self.y,
            _ => unreachable!(),
        }
    }

    fn do_opcode_relative(&self, instr: Instr) -> bool {
        match instr {
            Instr::Bcc => !self.p.carry,
            Instr::Bcs => self.p.carry,
            Instr::Beq => self.p.zero,
            Instr::Bmi => self.p.negative,
            Instr::Bne => !self.p.zero,
            Instr::Bpl => !self.p.negative,
            Instr::Bvc => !self.p.overflow,
            Instr::Bvs => self.p.overflow,
            _ => unreachable!(),
        }
    }

    fn adc(&mut self, value: u8) {
        // TODO: change to carrying_add when its out of nightly or if I decide to use nightly
        let res: u16 = self.a as u16 + value as u16 + self.p.carry as u16;

        self.p.carry = res > 0xff;
        self.p.zero = res == 0;
        self.p.overflow = (((res as u8 ^ self.a) & (res as u8 ^ value)) >> 7) != 0;
        self.p.negative = (res as i8) < 0;

        self.a = res as u8;
    }

    fn and(&mut self, value: u8) {
        let res = self.a & value;

        self.p.zero = res == 0;
        self.p.negative = (res as i8) < 0;

        self.a = res;
    }

    fn asl(&mut self, value: u8) -> u8 {
        let res = value << 1;

        self.p.carry = value >> 7 == 1;
        self.p.zero = res == 0;
        self.p.negative = (res as i8) < 0;

        res
    }

    fn bit(&mut self, value: u8) {
        let test = self.a & value;

        self.p.zero = test == 0;
        self.p.overflow = bit_to_bool((value >> 6) & 0x01);
        self.p.negative = bit_to_bool((value >> 7) & 0x01);
    }

    fn cmp(&mut self, value: u8) {
        let test = self.a.wrapping_sub(value);

        self.p.carry = self.a >= value;
        self.p.zero = self.a == value;
        self.p.negative = bit_to_bool((test >> 7) & 0x01);
    }

    fn cpx(&mut self, value: u8) {
        let test = self.x.wrapping_sub(value);

        self.p.carry = self.x >= value;
        self.p.zero = self.x == value;
        self.p.negative = bit_to_bool((test >> 7) & 0x01);
    }

    fn cpy(&mut self, value: u8) {
        let test = self.y.wrapping_sub(value);

        self.p.carry = self.y >= value;
        self.p.zero = self.y == value;
        self.p.negative = bit_to_bool((test >> 7) & 0x01);
    }

    fn dec(&mut self, value: u8) -> u8 {
        let res = value.wrapping_sub(1);

        self.p.zero = res == 0;
        self.p.negative = (res as i8) < 0;

        res
    }

    fn dex(&mut self) {
        self.x = self.x.wrapping_sub(1);

        self.p.zero = self.x == 0;
        self.p.negative = (self.x as i8) < 0;
    }

    fn dey(&mut self) {
        self.x = self.x.wrapping_sub(1);

        self.p.zero = self.x == 0;
        self.p.negative = (self.x as i8) < 0;
    }

    fn eor(&mut self, value: u8) {
        let res = self.a ^ value;

        self.p.zero = res == 0;
        self.p.negative = (res as i8) < 0;

        self.a = res;
    }

    fn inc(&mut self, value: u8) -> u8 {
        let res = value.wrapping_add(1);

        self.p.zero = res == 0;
        self.p.negative = (res as i8) < 0;

        res
    }

    fn inx(&mut self) {
        let res = self.x.wrapping_add(1);

        self.p.zero = res == 0;
        self.p.negative = (res as i8) < 0;

        self.p.zero = res == 0;
        self.p.negative = (res as i8) < 0;

        self.x = res;
    }

    fn iny(&mut self) {
        let res = self.y.wrapping_add(1);

        self.p.zero = res == 0;
        self.p.negative = (res as i8) < 0;

        self.p.zero = res == 0;
        self.p.negative = (res as i8) < 0;

        self.y = res;
    }

    fn lda(&mut self, value: u8) {
        self.a = value;

        self.p.zero = value == 0;
        self.p.negative = (value as i8) < 0;
    }

    fn ldx(&mut self, value: u8) {
        self.x = value;

        self.p.zero = value == 0;
        self.p.negative = (value as i8) < 0;
    }

    fn ldy(&mut self, value: u8) {
        self.y = value;

        self.p.zero = value == 0;
        self.p.negative = (value as i8) < 0;
    }

    fn lsr(&mut self, value: u8) -> u8 {
        let res = value >> 1;

        self.p.carry = bit_to_bool(value & 0x01);
        self.p.zero = res == 0;
        self.p.negative = false;

        res
    }

    fn ora(&mut self, value: u8) {
        let res = self.a | value;

        self.p.zero = res == 0;
        self.p.negative = (res as i8) < 0;

        self.a = res;
    }

    fn rol(&mut self, value: u8) -> u8 {
        let res = value << 1 | bool_to_bit(self.p.carry);

        self.p.carry = bit_to_bool((value >> 7) & 0x01);
        self.p.zero = res == 0;
        self.p.negative = bit_to_bool((res >> 7) & 0x01);

        res
    }

    fn ror(&mut self, value: u8) -> u8 {
        let res = value >> 1 | bool_to_bit(self.p.carry) << 7;

        self.p.carry = bit_to_bool(value & 0x01);
        self.p.zero = res == 0;
        self.p.negative = bit_to_bool((res >> 7) & 0x01);

        res
    }

    fn tax(&mut self) {
        self.x = self.a;

        self.p.zero = self.x == 0;
        self.p.negative = (self.x as i8) < 0;
    }

    fn tay(&mut self) {
        self.y = self.a;

        self.p.zero = self.y == 0;
        self.p.negative = (self.y as i8) < 0;
    }

    fn tsx(&mut self) {
        self.x = self.s;

        self.p.zero = self.x == 0;
        self.p.negative = (self.x as i8) < 0;
    }

    fn txa(&mut self) {
        self.a = self.x;

        self.p.zero = self.a == 0;
        self.p.negative = (self.a as i8) < 0;
    }

    fn txs(&mut self) {
        self.s = self.x;

        self.p.zero = self.s == 0;
        self.p.negative = (self.s as i8) < 0;
    }

    fn tya(&mut self) {
        self.a = self.y;

        self.p.zero = self.a == 0;
        self.p.negative = (self.a as i8) < 0;
    }
}

#[derive(Debug)]
struct StatusFlag {
    carry: bool,
    zero: bool,
    interrupt_disable: bool,
    decimal: bool,
    overflow: bool,
    negative: bool,
}

fn bool_to_bit(bool: bool) -> u8 {
    if bool {
        1
    } else {
        0
    }
}

fn bit_to_bool(bit: u8) -> bool {
    match bit {
        0 => false,
        1 => true,
        _ => panic!("Argument bit must be 0 or 1"),
    }
}

impl StatusFlag {
    fn power_on() -> Self {
        StatusFlag {
            carry: false,
            zero: false,
            interrupt_disable: true,
            decimal: false,
            overflow: false,
            negative: false,
        }
    }

    fn b_flag_set(&self) -> u8 {
        bool_to_bit(self.carry)
            | (bool_to_bit(self.zero) << 1)
            | (bool_to_bit(self.interrupt_disable) << 2)
            | (bool_to_bit(self.decimal) << 3)
            // b flag set
            | (1 << 4)
            // always set
            | (1 << 5)
            | (bool_to_bit(self.overflow) << 6)
            | (bool_to_bit(self.negative) << 7)
    }
}

impl From<u8> for StatusFlag {
    fn from(value: u8) -> Self {
        StatusFlag {
            carry: bit_to_bool(value & 0b00000001),
            zero: bit_to_bool((value >> 1) & 0b00000001),
            interrupt_disable: bit_to_bool((value >> 2) & 0b00000001),
            decimal: bit_to_bool((value >> 3) & 0b00000001),
            overflow: bit_to_bool((value >> 6) & 0b00000001),
            negative: bit_to_bool((value >> 7) & 0b00000001),
        }
    }
}

#[derive(Debug)]
struct InstrState {
    instr: Instr,
    addr_mode: AddrMode,
    cycle: usize,
    fetch_opcode: bool,
    saved_byte: u8,
    saved_addr: u16,
    page_crossed: bool,
}

#[derive(Clone, Copy, Debug)]
enum Instr {
    Illegal,
    Adc,
    And,
    Asl,
    Bcc,
    Bcs,
    Beq,
    Bit,
    Bmi,
    Bne,
    Bpl,
    Brk,
    Bvc,
    Bvs,
    Clc,
    Cld,
    Cli,
    Clv,
    Cmp,
    Cpx,
    Cpy,
    Dec,
    Dex,
    Dey,
    Eor,
    Inc,
    Inx,
    Iny,
    Jmp,
    Jsr,
    Lda,
    Ldx,
    Ldy,
    Lsr,
    Nop,
    Ora,
    Pha,
    Php,
    Pla,
    Plp,
    Rol,
    Ror,
    Rti,
    Rts,
    Sbc,
    Sec,
    Sed,
    Sei,
    Sta,
    Stx,
    Sty,
    Tax,
    Tay,
    Tsx,
    Txa,
    Txs,
    Tya,
}

#[derive(Clone, Copy, Debug)]
enum AddrMode {
    Accumulator,
    Absolute,
    AbsoluteX,
    AbsoluteY,
    Immediate,
    Implied,
    Indirect,
    IndirectX,
    IndirectY,
    Relative,
    Zeropage,
    ZeropageX,
    ZeropageY,
}

// Fix: return error  (maybe use thiserror or just use anyhow)
fn decode(opcode: u8) -> (Instr, AddrMode) {
    let aaa = opcode >> 5;
    let bbb = (opcode & 0b00011100) >> 2;
    let cc = opcode & 0b00000011;

    match (cc, aaa, bbb) {
        (0b01, 0b000, 0b000) => (Instr::Ora, AddrMode::IndirectX),
        (0b01, 0b000, 0b001) => (Instr::Ora, AddrMode::Zeropage),
        (0b01, 0b000, 0b010) => (Instr::Ora, AddrMode::Immediate),
        (0b01, 0b000, 0b011) => (Instr::Ora, AddrMode::Absolute),
        (0b01, 0b000, 0b100) => (Instr::Ora, AddrMode::IndirectY),
        (0b01, 0b000, 0b101) => (Instr::Ora, AddrMode::ZeropageX),
        (0b01, 0b000, 0b110) => (Instr::Ora, AddrMode::AbsoluteY),
        (0b01, 0b000, 0b111) => (Instr::Ora, AddrMode::AbsoluteX),

        (0b01, 0b001, 0b000) => (Instr::And, AddrMode::IndirectX),
        (0b01, 0b001, 0b001) => (Instr::And, AddrMode::Zeropage),
        (0b01, 0b001, 0b010) => (Instr::And, AddrMode::Immediate),
        (0b01, 0b001, 0b011) => (Instr::And, AddrMode::Absolute),
        (0b01, 0b001, 0b100) => (Instr::And, AddrMode::IndirectY),
        (0b01, 0b001, 0b101) => (Instr::And, AddrMode::ZeropageX),
        (0b01, 0b001, 0b110) => (Instr::And, AddrMode::AbsoluteY),
        (0b01, 0b001, 0b111) => (Instr::And, AddrMode::AbsoluteX),

        (0b01, 0b010, 0b000) => (Instr::Eor, AddrMode::IndirectX),
        (0b01, 0b010, 0b001) => (Instr::Eor, AddrMode::Zeropage),
        (0b01, 0b010, 0b010) => (Instr::Eor, AddrMode::Immediate),
        (0b01, 0b010, 0b011) => (Instr::Eor, AddrMode::Absolute),
        (0b01, 0b010, 0b100) => (Instr::Eor, AddrMode::IndirectY),
        (0b01, 0b010, 0b101) => (Instr::Eor, AddrMode::ZeropageX),
        (0b01, 0b010, 0b110) => (Instr::Eor, AddrMode::AbsoluteY),
        (0b01, 0b010, 0b111) => (Instr::Eor, AddrMode::AbsoluteX),

        (0b01, 0b011, 0b000) => (Instr::Adc, AddrMode::IndirectX),
        (0b01, 0b011, 0b001) => (Instr::Adc, AddrMode::Zeropage),
        (0b01, 0b011, 0b010) => (Instr::Adc, AddrMode::Immediate),
        (0b01, 0b011, 0b011) => (Instr::Adc, AddrMode::Absolute),
        (0b01, 0b011, 0b100) => (Instr::Adc, AddrMode::IndirectY),
        (0b01, 0b011, 0b101) => (Instr::Adc, AddrMode::ZeropageX),
        (0b01, 0b011, 0b110) => (Instr::Adc, AddrMode::AbsoluteY),
        (0b01, 0b011, 0b111) => (Instr::Adc, AddrMode::AbsoluteX),

        (0b01, 0b100, 0b000) => (Instr::Sta, AddrMode::IndirectX),
        (0b01, 0b100, 0b001) => (Instr::Sta, AddrMode::Zeropage),
        (0b01, 0b100, 0b011) => (Instr::Sta, AddrMode::Absolute),
        (0b01, 0b100, 0b100) => (Instr::Sta, AddrMode::IndirectY),
        (0b01, 0b100, 0b101) => (Instr::Sta, AddrMode::ZeropageX),
        (0b01, 0b100, 0b110) => (Instr::Sta, AddrMode::AbsoluteY),
        (0b01, 0b100, 0b111) => (Instr::Sta, AddrMode::AbsoluteX),

        (0b01, 0b101, 0b000) => (Instr::Lda, AddrMode::IndirectX),
        (0b01, 0b101, 0b001) => (Instr::Lda, AddrMode::Zeropage),
        (0b01, 0b101, 0b010) => (Instr::Lda, AddrMode::Immediate),
        (0b01, 0b101, 0b011) => (Instr::Lda, AddrMode::Absolute),
        (0b01, 0b101, 0b100) => (Instr::Lda, AddrMode::IndirectY),
        (0b01, 0b101, 0b101) => (Instr::Lda, AddrMode::ZeropageX),
        (0b01, 0b101, 0b110) => (Instr::Lda, AddrMode::AbsoluteY),
        (0b01, 0b101, 0b111) => (Instr::Lda, AddrMode::AbsoluteX),

        (0b01, 0b110, 0b000) => (Instr::Cmp, AddrMode::IndirectX),
        (0b01, 0b110, 0b001) => (Instr::Cmp, AddrMode::Zeropage),
        (0b01, 0b110, 0b010) => (Instr::Cmp, AddrMode::Immediate),
        (0b01, 0b110, 0b011) => (Instr::Cmp, AddrMode::Absolute),
        (0b01, 0b110, 0b100) => (Instr::Cmp, AddrMode::IndirectY),
        (0b01, 0b110, 0b101) => (Instr::Cmp, AddrMode::ZeropageX),
        (0b01, 0b110, 0b110) => (Instr::Cmp, AddrMode::AbsoluteY),
        (0b01, 0b110, 0b111) => (Instr::Cmp, AddrMode::AbsoluteX),

        (0b01, 0b111, 0b000) => (Instr::Sbc, AddrMode::IndirectX),
        (0b01, 0b111, 0b001) => (Instr::Sbc, AddrMode::Zeropage),
        (0b01, 0b111, 0b010) => (Instr::Sbc, AddrMode::Immediate),
        (0b01, 0b111, 0b011) => (Instr::Sbc, AddrMode::Absolute),
        (0b01, 0b111, 0b100) => (Instr::Sbc, AddrMode::IndirectY),
        (0b01, 0b111, 0b101) => (Instr::Sbc, AddrMode::ZeropageX),
        (0b01, 0b111, 0b110) => (Instr::Sbc, AddrMode::AbsoluteY),
        (0b01, 0b111, 0b111) => (Instr::Sbc, AddrMode::AbsoluteX),

        (0b10, 0b000, 0b001) => (Instr::Asl, AddrMode::Zeropage),
        (0b10, 0b000, 0b010) => (Instr::Asl, AddrMode::Accumulator),
        (0b10, 0b000, 0b011) => (Instr::Asl, AddrMode::Absolute),
        (0b10, 0b000, 0b101) => (Instr::Asl, AddrMode::ZeropageX),
        (0b10, 0b000, 0b111) => (Instr::Asl, AddrMode::AbsoluteX),

        (0b10, 0b001, 0b001) => (Instr::Rol, AddrMode::Zeropage),
        (0b10, 0b001, 0b010) => (Instr::Rol, AddrMode::Accumulator),
        (0b10, 0b001, 0b011) => (Instr::Rol, AddrMode::Absolute),
        (0b10, 0b001, 0b101) => (Instr::Rol, AddrMode::ZeropageX),
        (0b10, 0b001, 0b111) => (Instr::Rol, AddrMode::AbsoluteX),

        (0b10, 0b010, 0b001) => (Instr::Lsr, AddrMode::Zeropage),
        (0b10, 0b010, 0b010) => (Instr::Lsr, AddrMode::Accumulator),
        (0b10, 0b010, 0b011) => (Instr::Lsr, AddrMode::Absolute),
        (0b10, 0b010, 0b101) => (Instr::Lsr, AddrMode::ZeropageX),
        (0b10, 0b010, 0b111) => (Instr::Lsr, AddrMode::AbsoluteX),

        (0b10, 0b011, 0b001) => (Instr::Ror, AddrMode::Zeropage),
        (0b10, 0b011, 0b010) => (Instr::Ror, AddrMode::Accumulator),
        (0b10, 0b011, 0b011) => (Instr::Ror, AddrMode::Absolute),
        (0b10, 0b011, 0b101) => (Instr::Ror, AddrMode::ZeropageX),
        (0b10, 0b011, 0b111) => (Instr::Ror, AddrMode::AbsoluteX),

        (0b10, 0b100, 0b001) => (Instr::Stx, AddrMode::Zeropage),
        (0b10, 0b100, 0b011) => (Instr::Stx, AddrMode::Absolute),
        (0b10, 0b100, 0b101) => (Instr::Stx, AddrMode::ZeropageY),

        (0b10, 0b101, 0b000) => (Instr::Ldx, AddrMode::Immediate),
        (0b10, 0b101, 0b001) => (Instr::Ldx, AddrMode::Zeropage),
        (0b10, 0b101, 0b011) => (Instr::Ldx, AddrMode::Absolute),
        (0b10, 0b101, 0b101) => (Instr::Ldx, AddrMode::ZeropageY),
        (0b10, 0b101, 0b111) => (Instr::Ldx, AddrMode::AbsoluteY),

        (0b10, 0b110, 0b001) => (Instr::Dec, AddrMode::Zeropage),
        (0b10, 0b110, 0b011) => (Instr::Dec, AddrMode::Absolute),
        (0b10, 0b110, 0b101) => (Instr::Dec, AddrMode::ZeropageX),
        (0b10, 0b110, 0b111) => (Instr::Dec, AddrMode::AbsoluteX),

        (0b10, 0b111, 0b001) => (Instr::Inc, AddrMode::Zeropage),
        (0b10, 0b111, 0b011) => (Instr::Inc, AddrMode::Absolute),
        (0b10, 0b111, 0b101) => (Instr::Inc, AddrMode::ZeropageX),
        (0b10, 0b111, 0b111) => (Instr::Inc, AddrMode::AbsoluteX),

        (0b00, 0b001, 0b001) => (Instr::Bit, AddrMode::Zeropage),
        (0b00, 0b001, 0b011) => (Instr::Bit, AddrMode::Absolute),

        (0b00, 0b010, 0b011) => (Instr::Jmp, AddrMode::Absolute),

        (0b00, 0b011, 0b011) => (Instr::Jmp, AddrMode::Indirect),

        (0b00, 0b100, 0b001) => (Instr::Sty, AddrMode::Zeropage),
        (0b00, 0b100, 0b011) => (Instr::Sty, AddrMode::Absolute),
        (0b00, 0b100, 0b101) => (Instr::Sty, AddrMode::ZeropageX),

        (0b00, 0b101, 0b000) => (Instr::Ldy, AddrMode::Absolute),
        (0b00, 0b101, 0b001) => (Instr::Ldy, AddrMode::Zeropage),
        (0b00, 0b101, 0b011) => (Instr::Ldy, AddrMode::Absolute),
        (0b00, 0b101, 0b101) => (Instr::Ldy, AddrMode::ZeropageX),
        (0b00, 0b101, 0b111) => (Instr::Ldy, AddrMode::AbsoluteX),

        (0b00, 0b110, 0b000) => (Instr::Cpy, AddrMode::Immediate),
        (0b00, 0b110, 0b001) => (Instr::Cpy, AddrMode::Zeropage),
        (0b00, 0b110, 0b011) => (Instr::Cpy, AddrMode::Absolute),

        (0b00, 0b111, 0b000) => (Instr::Cpx, AddrMode::Immediate),
        (0b00, 0b111, 0b001) => (Instr::Cpx, AddrMode::Zeropage),
        (0b00, 0b111, 0b011) => (Instr::Cpx, AddrMode::Absolute),

        (0b00, 0b000, 0b100) => (Instr::Bpl, AddrMode::Relative),

        (0b00, 0b001, 0b100) => (Instr::Bmi, AddrMode::Relative),

        (0b00, 0b010, 0b100) => (Instr::Bvc, AddrMode::Relative),

        (0b00, 0b011, 0b100) => (Instr::Bvs, AddrMode::Relative),

        (0b00, 0b100, 0b100) => (Instr::Bcc, AddrMode::Relative),

        (0b00, 0b101, 0b100) => (Instr::Bcs, AddrMode::Relative),

        (0b00, 0b110, 0b100) => (Instr::Bne, AddrMode::Relative),

        (0b00, 0b111, 0b100) => (Instr::Beq, AddrMode::Relative),

        (0b00, 0b000, 0b000) => (Instr::Brk, AddrMode::Implied),

        (0b00, 0b001, 0b000) => (Instr::Jsr, AddrMode::Absolute),

        (0b00, 0b010, 0b000) => (Instr::Rti, AddrMode::Implied),

        (0b00, 0b011, 0b000) => (Instr::Rts, AddrMode::Implied),

        (0b00, 0b000, 0b010) => (Instr::Php, AddrMode::Implied),

        (0b00, 0b001, 0b010) => (Instr::Plp, AddrMode::Implied),

        (0b00, 0b010, 0b010) => (Instr::Pha, AddrMode::Implied),

        (0b00, 0b011, 0b010) => (Instr::Pla, AddrMode::Implied),

        (0b00, 0b100, 0b010) => (Instr::Dey, AddrMode::Implied),

        (0b00, 0b101, 0b010) => (Instr::Tay, AddrMode::Implied),

        (0b00, 0b110, 0b010) => (Instr::Iny, AddrMode::Implied),

        (0b00, 0b111, 0b010) => (Instr::Inx, AddrMode::Implied),

        (0b00, 0b000, 0b110) => (Instr::Clc, AddrMode::Implied),

        (0b00, 0b001, 0b110) => (Instr::Sec, AddrMode::Implied),

        (0b00, 0b010, 0b110) => (Instr::Cli, AddrMode::Implied),

        (0b00, 0b011, 0b110) => (Instr::Sei, AddrMode::Implied),

        (0b00, 0b100, 0b110) => (Instr::Tya, AddrMode::Implied),

        (0b00, 0b101, 0b110) => (Instr::Clv, AddrMode::Implied),

        (0b00, 0b110, 0b110) => (Instr::Cld, AddrMode::Implied),

        (0b00, 0b111, 0b110) => (Instr::Sed, AddrMode::Implied),

        (0b10, 0b100, 0b010) => (Instr::Txa, AddrMode::Implied),

        (0b10, 0b100, 0b110) => (Instr::Txs, AddrMode::Implied),

        (0b10, 0b101, 0b010) => (Instr::Tax, AddrMode::Implied),

        (0b10, 0b101, 0b110) => (Instr::Tsx, AddrMode::Implied),

        (0b10, 0b110, 0b010) => (Instr::Dex, AddrMode::Implied),

        (0b10, 0b111, 0b010) => (Instr::Nop, AddrMode::Implied),

        _ => (Instr::Illegal, AddrMode::Implied),
    }
}
