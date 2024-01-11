use crate::bus::Bus;

pub struct Cpu {
    a: u8,
    x: u8,
    y: u8,
    s: u8,
    p: StatusFlag,
    pc: u16,
    opcode: Opcode,
    addressing_mode: AddressingMode,
    curr_instr: Instr,
    next_instr: Instr,
    operand: u8,
    address: u16,
}

impl Cpu {
    pub fn new() -> Self {
        Cpu {
            a: 0,
            x: 0,
            y: 0,
            s: 0xfd,
            p: StatusFlag {
                carry: false,
                zero: false,
                interrupt_disable: true,
                decimal: false,
                overflow: false,
                negative: false,
            },
            // Fix: pc is got from the reset vector
            pc: 0,
            opcode: Opcode::Brk,
            addressing_mode: AddressingMode::Implied,
            curr_instr: Instr::FetchAddressHigh,
            next_instr: Instr::FetchOpcode,
            operand: 0,
            address: 0,
        }
    }

    pub fn cycle(&mut self, bus: &mut Bus) {
        self.curr_instr = self.next_instr;

        match self.curr_instr {
            Instr::FetchOpcode => {
                (self.opcode, self.addressing_mode) = decode(bus.read(self.pc));
                match (self.opcode, self.addressing_mode) {
                    (Opcode::Illegal, _) => todo!(),

                    (_, addressing_mode) => match addressing_mode {
                        AddressingMode::Implied | AddressingMode::Accumulator => {
                            self.next_instr = Instr::DummyRead
                        }
                        AddressingMode::Immediate
                        | AddressingMode::Zeropage
                        | AddressingMode::ZeropageX
                        | AddressingMode::ZeropageY
                        | AddressingMode::IndirectX
                        | AddressingMode::IndirectY => self.next_instr = Instr::FetchOperand,
                        AddressingMode::Absolute
                        | AddressingMode::AbsoluteX
                        | AddressingMode::AbsoluteY
                        | AddressingMode::Indirect
                        | AddressingMode::Relative => self.next_instr = Instr::FetchAddressLow,
                    },
                }

                self.pc += 1;
            }
            Instr::DummyRead => {
                let _ = bus.read(self.pc);
                // Do opcode
                todo!();
            }
            Instr::FetchOperand => {
                match self.addressing_mode {
                    AddressingMode::Immediate => {
                        self.operand = bus.read(self.pc);
                        // Do opcode
                        todo!();
                        self.next_instr = Instr::FetchOpcode;
                    }
                    AddressingMode::Zeropage => {
                        self.address = bus.read(self.pc) as u16;
                        self.next_instr = Instr::ReadAddressToOperand;
                    }
                    AddressingMode::ZeropageX => todo!(),
                    AddressingMode::ZeropageY => todo!(),
                    AddressingMode::IndirectX => todo!(),
                    AddressingMode::IndirectY => todo!(),
                    _ => panic!("Cannot be other addressing mode."),
                }

                self.pc += 1;
            }
            _ => todo!(),
        }
    }
}

struct StatusFlag {
    carry: bool,
    zero: bool,
    interrupt_disable: bool,
    decimal: bool,
    overflow: bool,
    negative: bool,
}

// Fix: an instruction can havae an addressing mode that its not possible
// Instruction::Brk(Acumulator)
#[derive(Clone, Copy)]
enum Opcode {
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

#[derive(Clone, Copy)]
enum AddressingMode {
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

enum InstructionType {
    Read,
    ReadModifyWrite,
    Write,
    Brk,
    Rti,
    Rts,
    Pha,
    Php,
    Pla,
    Plp,
    Jsr,
    Jmp,
    Other,
}

#[derive(Copy, Clone)]
enum Instr {
    FetchOpcode,
    FetchOperand,
    FetchAddressLow,
    FetchAddressHigh,
    FetchPointer,
    ReadAddressToOperand,
    ReadAddressToPcl,
    ReadAddressToPch,
    ReadPointerToAddressLow,
    ReadPointerToAddressHigh,
    DummyRead,
    LoadAccumulator,
    WriteOperandToAddress,
    AddXToAddress,
    AddYToAddress,
    AddXToAddressNoPageCrossing,
    AddYToAddressNoPageCrossing,
    AddXToPointerNoPageCrossing,
    FixAddressHigh,
    CopyAddressToPc,
}

// Fix: return error  (maybe use thiserror or just use anyhow)
fn decode(opcode: u8) -> (Opcode, AddressingMode) {
    let aaa = opcode >> 5;
    let bbb = (opcode & 0b00011100) >> 2;
    let cc = opcode & 0b00000011;

    match (cc, aaa, bbb) {
        (0b01, 0b000, 0b000) => (Opcode::Ora, AddressingMode::IndirectX),
        (0b01, 0b000, 0b001) => (Opcode::Ora, AddressingMode::Zeropage),
        (0b01, 0b000, 0b010) => (Opcode::Ora, AddressingMode::Immediate),
        (0b01, 0b000, 0b011) => (Opcode::Ora, AddressingMode::Absolute),
        (0b01, 0b000, 0b100) => (Opcode::Ora, AddressingMode::IndirectY),
        (0b01, 0b000, 0b101) => (Opcode::Ora, AddressingMode::ZeropageX),
        (0b01, 0b000, 0b110) => (Opcode::Ora, AddressingMode::AbsoluteY),
        (0b01, 0b000, 0b111) => (Opcode::Ora, AddressingMode::AbsoluteX),

        (0b01, 0b001, 0b000) => (Opcode::And, AddressingMode::IndirectX),
        (0b01, 0b001, 0b001) => (Opcode::And, AddressingMode::Zeropage),
        (0b01, 0b001, 0b010) => (Opcode::And, AddressingMode::Immediate),
        (0b01, 0b001, 0b011) => (Opcode::And, AddressingMode::Absolute),
        (0b01, 0b001, 0b100) => (Opcode::And, AddressingMode::IndirectY),
        (0b01, 0b001, 0b101) => (Opcode::And, AddressingMode::ZeropageX),
        (0b01, 0b001, 0b110) => (Opcode::And, AddressingMode::AbsoluteY),
        (0b01, 0b001, 0b111) => (Opcode::And, AddressingMode::AbsoluteX),

        (0b01, 0b010, 0b000) => (Opcode::Eor, AddressingMode::IndirectX),
        (0b01, 0b010, 0b001) => (Opcode::Eor, AddressingMode::Zeropage),
        (0b01, 0b010, 0b010) => (Opcode::Eor, AddressingMode::Immediate),
        (0b01, 0b010, 0b011) => (Opcode::Eor, AddressingMode::Absolute),
        (0b01, 0b010, 0b100) => (Opcode::Eor, AddressingMode::IndirectY),
        (0b01, 0b010, 0b101) => (Opcode::Eor, AddressingMode::ZeropageX),
        (0b01, 0b010, 0b110) => (Opcode::Eor, AddressingMode::AbsoluteY),
        (0b01, 0b010, 0b111) => (Opcode::Eor, AddressingMode::AbsoluteX),

        (0b01, 0b011, 0b000) => (Opcode::Adc, AddressingMode::IndirectX),
        (0b01, 0b011, 0b001) => (Opcode::Adc, AddressingMode::Zeropage),
        (0b01, 0b011, 0b010) => (Opcode::Adc, AddressingMode::Immediate),
        (0b01, 0b011, 0b011) => (Opcode::Adc, AddressingMode::Absolute),
        (0b01, 0b011, 0b100) => (Opcode::Adc, AddressingMode::IndirectY),
        (0b01, 0b011, 0b101) => (Opcode::Adc, AddressingMode::ZeropageX),
        (0b01, 0b011, 0b110) => (Opcode::Adc, AddressingMode::AbsoluteY),
        (0b01, 0b011, 0b111) => (Opcode::Adc, AddressingMode::AbsoluteX),

        (0b01, 0b100, 0b000) => (Opcode::Sta, AddressingMode::IndirectX),
        (0b01, 0b100, 0b001) => (Opcode::Sta, AddressingMode::Zeropage),
        (0b01, 0b100, 0b011) => (Opcode::Sta, AddressingMode::Absolute),
        (0b01, 0b100, 0b100) => (Opcode::Sta, AddressingMode::IndirectY),
        (0b01, 0b100, 0b101) => (Opcode::Sta, AddressingMode::ZeropageX),
        (0b01, 0b100, 0b110) => (Opcode::Sta, AddressingMode::AbsoluteY),
        (0b01, 0b100, 0b111) => (Opcode::Sta, AddressingMode::AbsoluteX),

        (0b01, 0b101, 0b000) => (Opcode::Lda, AddressingMode::IndirectX),
        (0b01, 0b101, 0b001) => (Opcode::Lda, AddressingMode::Zeropage),
        (0b01, 0b101, 0b010) => (Opcode::Lda, AddressingMode::Immediate),
        (0b01, 0b101, 0b011) => (Opcode::Lda, AddressingMode::Absolute),
        (0b01, 0b101, 0b100) => (Opcode::Lda, AddressingMode::IndirectY),
        (0b01, 0b101, 0b101) => (Opcode::Lda, AddressingMode::ZeropageX),
        (0b01, 0b101, 0b110) => (Opcode::Lda, AddressingMode::AbsoluteY),
        (0b01, 0b101, 0b111) => (Opcode::Lda, AddressingMode::AbsoluteX),

        (0b01, 0b110, 0b000) => (Opcode::Cmp, AddressingMode::IndirectX),
        (0b01, 0b110, 0b001) => (Opcode::Cmp, AddressingMode::Zeropage),
        (0b01, 0b110, 0b010) => (Opcode::Cmp, AddressingMode::Immediate),
        (0b01, 0b110, 0b011) => (Opcode::Cmp, AddressingMode::Absolute),
        (0b01, 0b110, 0b100) => (Opcode::Cmp, AddressingMode::IndirectY),
        (0b01, 0b110, 0b101) => (Opcode::Cmp, AddressingMode::ZeropageX),
        (0b01, 0b110, 0b110) => (Opcode::Cmp, AddressingMode::AbsoluteY),
        (0b01, 0b110, 0b111) => (Opcode::Cmp, AddressingMode::AbsoluteX),

        (0b01, 0b111, 0b000) => (Opcode::Sbc, AddressingMode::IndirectX),
        (0b01, 0b111, 0b001) => (Opcode::Sbc, AddressingMode::Zeropage),
        (0b01, 0b111, 0b010) => (Opcode::Sbc, AddressingMode::Immediate),
        (0b01, 0b111, 0b011) => (Opcode::Sbc, AddressingMode::Absolute),
        (0b01, 0b111, 0b100) => (Opcode::Sbc, AddressingMode::IndirectY),
        (0b01, 0b111, 0b101) => (Opcode::Sbc, AddressingMode::ZeropageX),
        (0b01, 0b111, 0b110) => (Opcode::Sbc, AddressingMode::AbsoluteY),
        (0b01, 0b111, 0b111) => (Opcode::Sbc, AddressingMode::AbsoluteX),

        (0b10, 0b000, 0b001) => (Opcode::Asl, AddressingMode::Zeropage),
        (0b10, 0b000, 0b010) => (Opcode::Asl, AddressingMode::Accumulator),
        (0b10, 0b000, 0b011) => (Opcode::Asl, AddressingMode::Absolute),
        (0b10, 0b000, 0b101) => (Opcode::Asl, AddressingMode::ZeropageX),
        (0b10, 0b000, 0b111) => (Opcode::Asl, AddressingMode::AbsoluteX),

        (0b10, 0b001, 0b001) => (Opcode::Rol, AddressingMode::Zeropage),
        (0b10, 0b001, 0b010) => (Opcode::Rol, AddressingMode::Accumulator),
        (0b10, 0b001, 0b011) => (Opcode::Rol, AddressingMode::Absolute),
        (0b10, 0b001, 0b101) => (Opcode::Rol, AddressingMode::ZeropageX),
        (0b10, 0b001, 0b111) => (Opcode::Rol, AddressingMode::AbsoluteX),

        (0b10, 0b010, 0b001) => (Opcode::Lsr, AddressingMode::Zeropage),
        (0b10, 0b010, 0b010) => (Opcode::Lsr, AddressingMode::Accumulator),
        (0b10, 0b010, 0b011) => (Opcode::Lsr, AddressingMode::Absolute),
        (0b10, 0b010, 0b101) => (Opcode::Lsr, AddressingMode::ZeropageX),
        (0b10, 0b010, 0b111) => (Opcode::Lsr, AddressingMode::AbsoluteX),

        (0b10, 0b011, 0b001) => (Opcode::Ror, AddressingMode::Zeropage),
        (0b10, 0b011, 0b010) => (Opcode::Ror, AddressingMode::Accumulator),
        (0b10, 0b011, 0b011) => (Opcode::Ror, AddressingMode::Absolute),
        (0b10, 0b011, 0b101) => (Opcode::Ror, AddressingMode::ZeropageX),
        (0b10, 0b011, 0b111) => (Opcode::Ror, AddressingMode::AbsoluteX),

        (0b10, 0b100, 0b001) => (Opcode::Stx, AddressingMode::Zeropage),
        (0b10, 0b100, 0b011) => (Opcode::Stx, AddressingMode::Absolute),
        (0b10, 0b100, 0b101) => (Opcode::Stx, AddressingMode::ZeropageY),

        (0b10, 0b101, 0b000) => (Opcode::Ldx, AddressingMode::Immediate),
        (0b10, 0b101, 0b001) => (Opcode::Ldx, AddressingMode::Zeropage),
        (0b10, 0b101, 0b011) => (Opcode::Ldx, AddressingMode::Absolute),
        (0b10, 0b101, 0b101) => (Opcode::Ldx, AddressingMode::ZeropageY),
        (0b10, 0b101, 0b111) => (Opcode::Ldx, AddressingMode::AbsoluteY),

        (0b10, 0b110, 0b001) => (Opcode::Dec, AddressingMode::Zeropage),
        (0b10, 0b110, 0b011) => (Opcode::Dec, AddressingMode::Absolute),
        (0b10, 0b110, 0b101) => (Opcode::Dec, AddressingMode::ZeropageX),
        (0b10, 0b110, 0b111) => (Opcode::Dec, AddressingMode::AbsoluteX),

        (0b10, 0b111, 0b001) => (Opcode::Inc, AddressingMode::Zeropage),
        (0b10, 0b111, 0b011) => (Opcode::Inc, AddressingMode::Absolute),
        (0b10, 0b111, 0b101) => (Opcode::Inc, AddressingMode::ZeropageX),
        (0b10, 0b111, 0b111) => (Opcode::Inc, AddressingMode::AbsoluteX),

        (0b00, 0b001, 0b001) => (Opcode::Bit, AddressingMode::Zeropage),
        (0b00, 0b001, 0b011) => (Opcode::Bit, AddressingMode::Absolute),

        (0b00, 0b010, 0b011) => (Opcode::Jmp, AddressingMode::Absolute),

        (0b00, 0b011, 0b011) => (Opcode::Jmp, AddressingMode::Indirect),

        (0b00, 0b100, 0b001) => (Opcode::Sty, AddressingMode::Zeropage),
        (0b00, 0b100, 0b011) => (Opcode::Sty, AddressingMode::Absolute),
        (0b00, 0b100, 0b101) => (Opcode::Sty, AddressingMode::ZeropageX),

        (0b00, 0b101, 0b000) => (Opcode::Ldy, AddressingMode::Absolute),
        (0b00, 0b101, 0b001) => (Opcode::Ldy, AddressingMode::Zeropage),
        (0b00, 0b101, 0b011) => (Opcode::Ldy, AddressingMode::Absolute),
        (0b00, 0b101, 0b101) => (Opcode::Ldy, AddressingMode::ZeropageX),
        (0b00, 0b101, 0b111) => (Opcode::Ldy, AddressingMode::AbsoluteX),

        (0b00, 0b110, 0b000) => (Opcode::Cpy, AddressingMode::Immediate),
        (0b00, 0b110, 0b001) => (Opcode::Cpy, AddressingMode::Zeropage),
        (0b00, 0b110, 0b011) => (Opcode::Cpy, AddressingMode::Absolute),

        (0b00, 0b111, 0b000) => (Opcode::Cpx, AddressingMode::Immediate),
        (0b00, 0b111, 0b001) => (Opcode::Cpx, AddressingMode::Zeropage),
        (0b00, 0b111, 0b011) => (Opcode::Cpx, AddressingMode::Absolute),

        (0b00, 0b000, 0b100) => (Opcode::Bpl, AddressingMode::Implied),

        (0b00, 0b001, 0b100) => (Opcode::Bmi, AddressingMode::Implied),

        (0b00, 0b010, 0b100) => (Opcode::Bvc, AddressingMode::Implied),

        (0b00, 0b011, 0b100) => (Opcode::Bvs, AddressingMode::Implied),

        (0b00, 0b100, 0b100) => (Opcode::Bcc, AddressingMode::Implied),

        (0b00, 0b101, 0b100) => (Opcode::Bcs, AddressingMode::Implied),

        (0b00, 0b110, 0b100) => (Opcode::Bne, AddressingMode::Implied),

        (0b00, 0b111, 0b100) => (Opcode::Beq, AddressingMode::Implied),

        (0b00, 0b000, 0b000) => (Opcode::Brk, AddressingMode::Implied),

        (0b00, 0b001, 0b000) => (Opcode::Jsr, AddressingMode::Absolute),

        (0b00, 0b010, 0b000) => (Opcode::Rti, AddressingMode::Implied),

        (0b00, 0b011, 0b000) => (Opcode::Rts, AddressingMode::Implied),

        (0b00, 0b000, 0b010) => (Opcode::Php, AddressingMode::Implied),

        (0b00, 0b001, 0b010) => (Opcode::Plp, AddressingMode::Implied),

        (0b00, 0b010, 0b010) => (Opcode::Pha, AddressingMode::Implied),

        (0b00, 0b011, 0b010) => (Opcode::Pla, AddressingMode::Implied),

        (0b00, 0b100, 0b010) => (Opcode::Dey, AddressingMode::Implied),

        (0b00, 0b101, 0b010) => (Opcode::Tay, AddressingMode::Implied),

        (0b00, 0b110, 0b010) => (Opcode::Iny, AddressingMode::Implied),

        (0b00, 0b111, 0b010) => (Opcode::Inx, AddressingMode::Implied),

        (0b00, 0b000, 0b110) => (Opcode::Clc, AddressingMode::Implied),

        (0b00, 0b001, 0b110) => (Opcode::Sec, AddressingMode::Implied),

        (0b00, 0b010, 0b110) => (Opcode::Cli, AddressingMode::Implied),

        (0b00, 0b011, 0b110) => (Opcode::Sei, AddressingMode::Implied),

        (0b00, 0b100, 0b110) => (Opcode::Tya, AddressingMode::Implied),

        (0b00, 0b101, 0b110) => (Opcode::Clv, AddressingMode::Implied),

        (0b00, 0b110, 0b110) => (Opcode::Cld, AddressingMode::Implied),

        (0b00, 0b111, 0b110) => (Opcode::Sed, AddressingMode::Implied),

        (0b10, 0b100, 0b010) => (Opcode::Txa, AddressingMode::Implied),

        (0b10, 0b100, 0b110) => (Opcode::Txs, AddressingMode::Implied),

        (0b10, 0b101, 0b010) => (Opcode::Tax, AddressingMode::Implied),

        (0b10, 0b101, 0b110) => (Opcode::Tsx, AddressingMode::Implied),

        (0b10, 0b110, 0b010) => (Opcode::Dex, AddressingMode::Implied),

        (0b10, 0b111, 0b010) => (Opcode::Nop, AddressingMode::Implied),

        _ => (Opcode::Illegal, AddressingMode::Implied),
    }
}

fn nextInstr(curr: Instr) -> Instr {}
