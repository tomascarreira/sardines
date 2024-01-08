use crate::bus::Bus;

pub struct Cpu {
    a: u8,
    x: u8,
    y: u8,
    s: u8,
    p: StatusFlag,
    pc: u16,
    opcode: Opcode,
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
            opcode: Opcode::Brk {
                addressing_mode: AddressingMode::Implied,
            },
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
                self.opcode = decode(bus.read(self.pc));
                match self.opcode.addressing_mode() {
                    Some(addressing_mode) => match addressing_mode {
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

                    // TODO Do real error handling
                    None => panic!("Could not get addressing mode."),
                }

                self.pc += 1;
            }
            Instr::DummyRead => {
                let _ = bus.read(self.pc);
                // Do opcode
                todo!();
            }
            Instr::FetchOperand => {
                match self
                    .opcode
                    .addressing_mode()
                    .expect("Opcode has an addressing mode.")
                {
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
enum Opcode {
    Illegal,
    Adc { addressing_mode: AddressingMode },
    And { addressing_mode: AddressingMode },
    Asl { addressing_mode: AddressingMode },
    Bcc { addressing_mode: AddressingMode },
    Bcs { addressing_mode: AddressingMode },
    Beq { addressing_mode: AddressingMode },
    Bit { addressing_mode: AddressingMode },
    Bmi { addressing_mode: AddressingMode },
    Bne { addressing_mode: AddressingMode },
    Bpl { addressing_mode: AddressingMode },
    Brk { addressing_mode: AddressingMode },
    Bvc { addressing_mode: AddressingMode },
    Bvs { addressing_mode: AddressingMode },
    Clc { addressing_mode: AddressingMode },
    Cld { addressing_mode: AddressingMode },
    Cli { addressing_mode: AddressingMode },
    Clv { addressing_mode: AddressingMode },
    Cmp { addressing_mode: AddressingMode },
    Cpx { addressing_mode: AddressingMode },
    Cpy { addressing_mode: AddressingMode },
    Dec { addressing_mode: AddressingMode },
    Dex { addressing_mode: AddressingMode },
    Dey { addressing_mode: AddressingMode },
    Eor { addressing_mode: AddressingMode },
    Inc { addressing_mode: AddressingMode },
    Inx { addressing_mode: AddressingMode },
    Iny { addressing_mode: AddressingMode },
    Jmp { addressing_mode: AddressingMode },
    Jsr { addressing_mode: AddressingMode },
    Lda { addressing_mode: AddressingMode },
    Ldx { addressing_mode: AddressingMode },
    Ldy { addressing_mode: AddressingMode },
    Lsr { addressing_mode: AddressingMode },
    Nop { addressing_mode: AddressingMode },
    Ora { addressing_mode: AddressingMode },
    Pha { addressing_mode: AddressingMode },
    Php { addressing_mode: AddressingMode },
    Pla { addressing_mode: AddressingMode },
    Plp { addressing_mode: AddressingMode },
    Rol { addressing_mode: AddressingMode },
    Ror { addressing_mode: AddressingMode },
    Rti { addressing_mode: AddressingMode },
    Rts { addressing_mode: AddressingMode },
    Sbc { addressing_mode: AddressingMode },
    Sec { addressing_mode: AddressingMode },
    Sed { addressing_mode: AddressingMode },
    Sei { addressing_mode: AddressingMode },
    Sta { addressing_mode: AddressingMode },
    Stx { addressing_mode: AddressingMode },
    Sty { addressing_mode: AddressingMode },
    Tax { addressing_mode: AddressingMode },
    Tay { addressing_mode: AddressingMode },
    Tsx { addressing_mode: AddressingMode },
    Txa { addressing_mode: AddressingMode },
    Txs { addressing_mode: AddressingMode },
    Tya { addressing_mode: AddressingMode },
}

impl Opcode {
    fn addressing_mode(&self) -> Option<AddressingMode> {
        match self {
            Opcode::Illegal => None,
            Opcode::Adc { addressing_mode } => Some(*addressing_mode),
            Opcode::And { addressing_mode } => Some(*addressing_mode),
            Opcode::Asl { addressing_mode } => Some(*addressing_mode),
            Opcode::Bcc { addressing_mode } => Some(*addressing_mode),
            Opcode::Bcs { addressing_mode } => Some(*addressing_mode),
            Opcode::Beq { addressing_mode } => Some(*addressing_mode),
            Opcode::Bit { addressing_mode } => Some(*addressing_mode),
            Opcode::Bmi { addressing_mode } => Some(*addressing_mode),
            Opcode::Bne { addressing_mode } => Some(*addressing_mode),
            Opcode::Bpl { addressing_mode } => Some(*addressing_mode),
            Opcode::Brk { addressing_mode } => Some(*addressing_mode),
            Opcode::Bvc { addressing_mode } => Some(*addressing_mode),
            Opcode::Bvs { addressing_mode } => Some(*addressing_mode),
            Opcode::Clc { addressing_mode } => Some(*addressing_mode),
            Opcode::Cld { addressing_mode } => Some(*addressing_mode),
            Opcode::Cli { addressing_mode } => Some(*addressing_mode),
            Opcode::Clv { addressing_mode } => Some(*addressing_mode),
            Opcode::Cmp { addressing_mode } => Some(*addressing_mode),
            Opcode::Cpx { addressing_mode } => Some(*addressing_mode),
            Opcode::Cpy { addressing_mode } => Some(*addressing_mode),
            Opcode::Dec { addressing_mode } => Some(*addressing_mode),
            Opcode::Dex { addressing_mode } => Some(*addressing_mode),
            Opcode::Dey { addressing_mode } => Some(*addressing_mode),
            Opcode::Eor { addressing_mode } => Some(*addressing_mode),
            Opcode::Inc { addressing_mode } => Some(*addressing_mode),
            Opcode::Inx { addressing_mode } => Some(*addressing_mode),
            Opcode::Iny { addressing_mode } => Some(*addressing_mode),
            Opcode::Jmp { addressing_mode } => Some(*addressing_mode),
            Opcode::Jsr { addressing_mode } => Some(*addressing_mode),
            Opcode::Lda { addressing_mode } => Some(*addressing_mode),
            Opcode::Ldx { addressing_mode } => Some(*addressing_mode),
            Opcode::Ldy { addressing_mode } => Some(*addressing_mode),
            Opcode::Lsr { addressing_mode } => Some(*addressing_mode),
            Opcode::Nop { addressing_mode } => Some(*addressing_mode),
            Opcode::Ora { addressing_mode } => Some(*addressing_mode),
            Opcode::Pha { addressing_mode } => Some(*addressing_mode),
            Opcode::Php { addressing_mode } => Some(*addressing_mode),
            Opcode::Pla { addressing_mode } => Some(*addressing_mode),
            Opcode::Plp { addressing_mode } => Some(*addressing_mode),
            Opcode::Rol { addressing_mode } => Some(*addressing_mode),
            Opcode::Ror { addressing_mode } => Some(*addressing_mode),
            Opcode::Rti { addressing_mode } => Some(*addressing_mode),
            Opcode::Rts { addressing_mode } => Some(*addressing_mode),
            Opcode::Sbc { addressing_mode } => Some(*addressing_mode),
            Opcode::Sec { addressing_mode } => Some(*addressing_mode),
            Opcode::Sed { addressing_mode } => Some(*addressing_mode),
            Opcode::Sei { addressing_mode } => Some(*addressing_mode),
            Opcode::Sta { addressing_mode } => Some(*addressing_mode),
            Opcode::Stx { addressing_mode } => Some(*addressing_mode),
            Opcode::Sty { addressing_mode } => Some(*addressing_mode),
            Opcode::Tax { addressing_mode } => Some(*addressing_mode),
            Opcode::Tay { addressing_mode } => Some(*addressing_mode),
            Opcode::Tsx { addressing_mode } => Some(*addressing_mode),
            Opcode::Txa { addressing_mode } => Some(*addressing_mode),
            Opcode::Txs { addressing_mode } => Some(*addressing_mode),
            Opcode::Tya { addressing_mode } => Some(*addressing_mode),
        }
    }
}

#[derive(Copy, Clone)]
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
fn decode(opcode: u8) -> Opcode {
    let aaa = opcode >> 5;
    let bbb = (opcode & 0b00011100) >> 2;
    let cc = opcode & 0b00000011;

    match (cc, aaa, bbb) {
        (0b01, 0b000, 0b000) => Opcode::Ora {
            addressing_mode: AddressingMode::IndirectX,
        },
        (0b01, 0b000, 0b001) => Opcode::Ora {
            addressing_mode: AddressingMode::Zeropage,
        },
        (0b01, 0b000, 0b010) => Opcode::Ora {
            addressing_mode: AddressingMode::Immediate,
        },
        (0b01, 0b000, 0b011) => Opcode::Ora {
            addressing_mode: AddressingMode::Absolute,
        },
        (0b01, 0b000, 0b100) => Opcode::Ora {
            addressing_mode: AddressingMode::IndirectY,
        },
        (0b01, 0b000, 0b101) => Opcode::Ora {
            addressing_mode: AddressingMode::ZeropageX,
        },
        (0b01, 0b000, 0b110) => Opcode::Ora {
            addressing_mode: AddressingMode::AbsoluteY,
        },
        (0b01, 0b000, 0b111) => Opcode::Ora {
            addressing_mode: AddressingMode::AbsoluteX,
        },

        (0b01, 0b001, 0b000) => Opcode::And {
            addressing_mode: AddressingMode::IndirectX,
        },
        (0b01, 0b001, 0b001) => Opcode::And {
            addressing_mode: AddressingMode::Zeropage,
        },
        (0b01, 0b001, 0b010) => Opcode::And {
            addressing_mode: AddressingMode::Immediate,
        },
        (0b01, 0b001, 0b011) => Opcode::And {
            addressing_mode: AddressingMode::Absolute,
        },
        (0b01, 0b001, 0b100) => Opcode::And {
            addressing_mode: AddressingMode::IndirectY,
        },
        (0b01, 0b001, 0b101) => Opcode::And {
            addressing_mode: AddressingMode::ZeropageX,
        },
        (0b01, 0b001, 0b110) => Opcode::And {
            addressing_mode: AddressingMode::AbsoluteY,
        },
        (0b01, 0b001, 0b111) => Opcode::And {
            addressing_mode: AddressingMode::AbsoluteX,
        },

        (0b01, 0b010, 0b000) => Opcode::Eor {
            addressing_mode: AddressingMode::IndirectX,
        },
        (0b01, 0b010, 0b001) => Opcode::Eor {
            addressing_mode: AddressingMode::Zeropage,
        },
        (0b01, 0b010, 0b010) => Opcode::Eor {
            addressing_mode: AddressingMode::Immediate,
        },
        (0b01, 0b010, 0b011) => Opcode::Eor {
            addressing_mode: AddressingMode::Absolute,
        },
        (0b01, 0b010, 0b100) => Opcode::Eor {
            addressing_mode: AddressingMode::IndirectY,
        },
        (0b01, 0b010, 0b101) => Opcode::Eor {
            addressing_mode: AddressingMode::ZeropageX,
        },
        (0b01, 0b010, 0b110) => Opcode::Eor {
            addressing_mode: AddressingMode::AbsoluteY,
        },
        (0b01, 0b010, 0b111) => Opcode::Eor {
            addressing_mode: AddressingMode::AbsoluteX,
        },

        (0b01, 0b011, 0b000) => Opcode::Adc {
            addressing_mode: AddressingMode::IndirectX,
        },
        (0b01, 0b011, 0b001) => Opcode::Adc {
            addressing_mode: AddressingMode::Zeropage,
        },
        (0b01, 0b011, 0b010) => Opcode::Adc {
            addressing_mode: AddressingMode::Immediate,
        },
        (0b01, 0b011, 0b011) => Opcode::Adc {
            addressing_mode: AddressingMode::Absolute,
        },
        (0b01, 0b011, 0b100) => Opcode::Adc {
            addressing_mode: AddressingMode::IndirectY,
        },
        (0b01, 0b011, 0b101) => Opcode::Adc {
            addressing_mode: AddressingMode::ZeropageX,
        },
        (0b01, 0b011, 0b110) => Opcode::Adc {
            addressing_mode: AddressingMode::AbsoluteY,
        },
        (0b01, 0b011, 0b111) => Opcode::Adc {
            addressing_mode: AddressingMode::AbsoluteX,
        },

        (0b01, 0b100, 0b000) => Opcode::Sta {
            addressing_mode: AddressingMode::IndirectX,
        },
        (0b01, 0b100, 0b001) => Opcode::Sta {
            addressing_mode: AddressingMode::Zeropage,
        },
        (0b01, 0b100, 0b011) => Opcode::Sta {
            addressing_mode: AddressingMode::Absolute,
        },
        (0b01, 0b100, 0b100) => Opcode::Sta {
            addressing_mode: AddressingMode::IndirectY,
        },
        (0b01, 0b100, 0b101) => Opcode::Sta {
            addressing_mode: AddressingMode::ZeropageX,
        },
        (0b01, 0b100, 0b110) => Opcode::Sta {
            addressing_mode: AddressingMode::AbsoluteY,
        },
        (0b01, 0b100, 0b111) => Opcode::Sta {
            addressing_mode: AddressingMode::AbsoluteX,
        },

        (0b01, 0b101, 0b000) => Opcode::Lda {
            addressing_mode: AddressingMode::IndirectX,
        },
        (0b01, 0b101, 0b001) => Opcode::Lda {
            addressing_mode: AddressingMode::Zeropage,
        },
        (0b01, 0b101, 0b010) => Opcode::Lda {
            addressing_mode: AddressingMode::Immediate,
        },
        (0b01, 0b101, 0b011) => Opcode::Lda {
            addressing_mode: AddressingMode::Absolute,
        },
        (0b01, 0b101, 0b100) => Opcode::Lda {
            addressing_mode: AddressingMode::IndirectY,
        },
        (0b01, 0b101, 0b101) => Opcode::Lda {
            addressing_mode: AddressingMode::ZeropageX,
        },
        (0b01, 0b101, 0b110) => Opcode::Lda {
            addressing_mode: AddressingMode::AbsoluteY,
        },
        (0b01, 0b101, 0b111) => Opcode::Lda {
            addressing_mode: AddressingMode::AbsoluteX,
        },

        (0b01, 0b110, 0b000) => Opcode::Cmp {
            addressing_mode: AddressingMode::IndirectX,
        },
        (0b01, 0b110, 0b001) => Opcode::Cmp {
            addressing_mode: AddressingMode::Zeropage,
        },
        (0b01, 0b110, 0b010) => Opcode::Cmp {
            addressing_mode: AddressingMode::Immediate,
        },
        (0b01, 0b110, 0b011) => Opcode::Cmp {
            addressing_mode: AddressingMode::Absolute,
        },
        (0b01, 0b110, 0b100) => Opcode::Cmp {
            addressing_mode: AddressingMode::IndirectY,
        },
        (0b01, 0b110, 0b101) => Opcode::Cmp {
            addressing_mode: AddressingMode::ZeropageX,
        },
        (0b01, 0b110, 0b110) => Opcode::Cmp {
            addressing_mode: AddressingMode::AbsoluteY,
        },
        (0b01, 0b110, 0b111) => Opcode::Cmp {
            addressing_mode: AddressingMode::AbsoluteX,
        },

        (0b01, 0b111, 0b000) => Opcode::Sbc {
            addressing_mode: AddressingMode::IndirectX,
        },
        (0b01, 0b111, 0b001) => Opcode::Sbc {
            addressing_mode: AddressingMode::Zeropage,
        },
        (0b01, 0b111, 0b010) => Opcode::Sbc {
            addressing_mode: AddressingMode::Immediate,
        },
        (0b01, 0b111, 0b011) => Opcode::Sbc {
            addressing_mode: AddressingMode::Absolute,
        },
        (0b01, 0b111, 0b100) => Opcode::Sbc {
            addressing_mode: AddressingMode::IndirectY,
        },
        (0b01, 0b111, 0b101) => Opcode::Sbc {
            addressing_mode: AddressingMode::ZeropageX,
        },
        (0b01, 0b111, 0b110) => Opcode::Sbc {
            addressing_mode: AddressingMode::AbsoluteY,
        },
        (0b01, 0b111, 0b111) => Opcode::Sbc {
            addressing_mode: AddressingMode::AbsoluteX,
        },

        (0b10, 0b000, 0b001) => Opcode::Asl {
            addressing_mode: AddressingMode::Zeropage,
        },
        (0b10, 0b000, 0b010) => Opcode::Asl {
            addressing_mode: AddressingMode::Accumulator,
        },
        (0b10, 0b000, 0b011) => Opcode::Asl {
            addressing_mode: AddressingMode::Absolute,
        },
        (0b10, 0b000, 0b101) => Opcode::Asl {
            addressing_mode: AddressingMode::ZeropageX,
        },
        (0b10, 0b000, 0b111) => Opcode::Asl {
            addressing_mode: AddressingMode::AbsoluteX,
        },

        (0b10, 0b001, 0b001) => Opcode::Rol {
            addressing_mode: AddressingMode::Zeropage,
        },
        (0b10, 0b001, 0b010) => Opcode::Rol {
            addressing_mode: AddressingMode::Accumulator,
        },
        (0b10, 0b001, 0b011) => Opcode::Rol {
            addressing_mode: AddressingMode::Absolute,
        },
        (0b10, 0b001, 0b101) => Opcode::Rol {
            addressing_mode: AddressingMode::ZeropageX,
        },
        (0b10, 0b001, 0b111) => Opcode::Rol {
            addressing_mode: AddressingMode::AbsoluteX,
        },

        (0b10, 0b010, 0b001) => Opcode::Lsr {
            addressing_mode: AddressingMode::Zeropage,
        },
        (0b10, 0b010, 0b010) => Opcode::Lsr {
            addressing_mode: AddressingMode::Accumulator,
        },
        (0b10, 0b010, 0b011) => Opcode::Lsr {
            addressing_mode: AddressingMode::Absolute,
        },
        (0b10, 0b010, 0b101) => Opcode::Lsr {
            addressing_mode: AddressingMode::ZeropageX,
        },
        (0b10, 0b010, 0b111) => Opcode::Lsr {
            addressing_mode: AddressingMode::AbsoluteX,
        },

        (0b10, 0b011, 0b001) => Opcode::Ror {
            addressing_mode: AddressingMode::Zeropage,
        },
        (0b10, 0b011, 0b010) => Opcode::Ror {
            addressing_mode: AddressingMode::Accumulator,
        },
        (0b10, 0b011, 0b011) => Opcode::Ror {
            addressing_mode: AddressingMode::Absolute,
        },
        (0b10, 0b011, 0b101) => Opcode::Ror {
            addressing_mode: AddressingMode::ZeropageX,
        },
        (0b10, 0b011, 0b111) => Opcode::Ror {
            addressing_mode: AddressingMode::AbsoluteX,
        },

        (0b10, 0b100, 0b001) => Opcode::Stx {
            addressing_mode: AddressingMode::Zeropage,
        },
        (0b10, 0b100, 0b011) => Opcode::Stx {
            addressing_mode: AddressingMode::Absolute,
        },
        (0b10, 0b100, 0b101) => Opcode::Stx {
            addressing_mode: AddressingMode::ZeropageY,
        },

        (0b10, 0b101, 0b000) => Opcode::Ldx {
            addressing_mode: AddressingMode::Immediate,
        },
        (0b10, 0b101, 0b001) => Opcode::Ldx {
            addressing_mode: AddressingMode::Zeropage,
        },
        (0b10, 0b101, 0b011) => Opcode::Ldx {
            addressing_mode: AddressingMode::Absolute,
        },
        (0b10, 0b101, 0b101) => Opcode::Ldx {
            addressing_mode: AddressingMode::ZeropageY,
        },
        (0b10, 0b101, 0b111) => Opcode::Ldx {
            addressing_mode: AddressingMode::AbsoluteY,
        },

        (0b10, 0b110, 0b001) => Opcode::Dec {
            addressing_mode: AddressingMode::Zeropage,
        },
        (0b10, 0b110, 0b011) => Opcode::Dec {
            addressing_mode: AddressingMode::Absolute,
        },
        (0b10, 0b110, 0b101) => Opcode::Dec {
            addressing_mode: AddressingMode::ZeropageX,
        },
        (0b10, 0b110, 0b111) => Opcode::Dec {
            addressing_mode: AddressingMode::AbsoluteX,
        },

        (0b10, 0b111, 0b001) => Opcode::Inc {
            addressing_mode: AddressingMode::Zeropage,
        },
        (0b10, 0b111, 0b011) => Opcode::Inc {
            addressing_mode: AddressingMode::Absolute,
        },
        (0b10, 0b111, 0b101) => Opcode::Inc {
            addressing_mode: AddressingMode::ZeropageX,
        },
        (0b10, 0b111, 0b111) => Opcode::Inc {
            addressing_mode: AddressingMode::AbsoluteX,
        },

        (0b00, 0b001, 0b001) => Opcode::Bit {
            addressing_mode: AddressingMode::Zeropage,
        },
        (0b00, 0b001, 0b011) => Opcode::Bit {
            addressing_mode: AddressingMode::Absolute,
        },

        (0b00, 0b010, 0b011) => Opcode::Jmp {
            addressing_mode: AddressingMode::Absolute,
        },

        (0b00, 0b011, 0b011) => Opcode::Jmp {
            addressing_mode: AddressingMode::Indirect,
        },

        (0b00, 0b100, 0b001) => Opcode::Sty {
            addressing_mode: AddressingMode::Zeropage,
        },
        (0b00, 0b100, 0b011) => Opcode::Sty {
            addressing_mode: AddressingMode::Absolute,
        },
        (0b00, 0b100, 0b101) => Opcode::Sty {
            addressing_mode: AddressingMode::ZeropageX,
        },

        (0b00, 0b101, 0b000) => Opcode::Ldy {
            addressing_mode: AddressingMode::Absolute,
        },
        (0b00, 0b101, 0b001) => Opcode::Ldy {
            addressing_mode: AddressingMode::Zeropage,
        },
        (0b00, 0b101, 0b011) => Opcode::Ldy {
            addressing_mode: AddressingMode::Absolute,
        },
        (0b00, 0b101, 0b101) => Opcode::Ldy {
            addressing_mode: AddressingMode::ZeropageX,
        },
        (0b00, 0b101, 0b111) => Opcode::Ldy {
            addressing_mode: AddressingMode::AbsoluteX,
        },

        (0b00, 0b110, 0b000) => Opcode::Cpy {
            addressing_mode: AddressingMode::Immediate,
        },
        (0b00, 0b110, 0b001) => Opcode::Cpy {
            addressing_mode: AddressingMode::Zeropage,
        },
        (0b00, 0b110, 0b011) => Opcode::Cpy {
            addressing_mode: AddressingMode::Absolute,
        },

        (0b00, 0b111, 0b000) => Opcode::Cpx {
            addressing_mode: AddressingMode::Immediate,
        },
        (0b00, 0b111, 0b001) => Opcode::Cpx {
            addressing_mode: AddressingMode::Zeropage,
        },
        (0b00, 0b111, 0b011) => Opcode::Cpx {
            addressing_mode: AddressingMode::Absolute,
        },

        (0b00, 0b000, 0b100) => Opcode::Bpl {
            addressing_mode: AddressingMode::Implied,
        },

        (0b00, 0b001, 0b100) => Opcode::Bmi {
            addressing_mode: AddressingMode::Implied,
        },

        (0b00, 0b010, 0b100) => Opcode::Bvc {
            addressing_mode: AddressingMode::Implied,
        },

        (0b00, 0b011, 0b100) => Opcode::Bvs {
            addressing_mode: AddressingMode::Implied,
        },

        (0b00, 0b100, 0b100) => Opcode::Bcc {
            addressing_mode: AddressingMode::Implied,
        },

        (0b00, 0b101, 0b100) => Opcode::Bcs {
            addressing_mode: AddressingMode::Implied,
        },

        (0b00, 0b110, 0b100) => Opcode::Bne {
            addressing_mode: AddressingMode::Implied,
        },

        (0b00, 0b111, 0b100) => Opcode::Beq {
            addressing_mode: AddressingMode::Implied,
        },

        (0b00, 0b000, 0b000) => Opcode::Brk {
            addressing_mode: AddressingMode::Implied,
        },

        (0b00, 0b001, 0b000) => Opcode::Jsr {
            addressing_mode: AddressingMode::Absolute,
        },

        (0b00, 0b010, 0b000) => Opcode::Rti {
            addressing_mode: AddressingMode::Implied,
        },

        (0b00, 0b011, 0b000) => Opcode::Rts {
            addressing_mode: AddressingMode::Implied,
        },

        (0b00, 0b000, 0b010) => Opcode::Php {
            addressing_mode: AddressingMode::Implied,
        },

        (0b00, 0b001, 0b010) => Opcode::Plp {
            addressing_mode: AddressingMode::Implied,
        },

        (0b00, 0b010, 0b010) => Opcode::Pha {
            addressing_mode: AddressingMode::Implied,
        },

        (0b00, 0b011, 0b010) => Opcode::Pla {
            addressing_mode: AddressingMode::Implied,
        },

        (0b00, 0b100, 0b010) => Opcode::Dey {
            addressing_mode: AddressingMode::Implied,
        },

        (0b00, 0b101, 0b010) => Opcode::Tay {
            addressing_mode: AddressingMode::Implied,
        },

        (0b00, 0b110, 0b010) => Opcode::Iny {
            addressing_mode: AddressingMode::Implied,
        },

        (0b00, 0b111, 0b010) => Opcode::Inx {
            addressing_mode: AddressingMode::Implied,
        },

        (0b00, 0b000, 0b110) => Opcode::Clc {
            addressing_mode: AddressingMode::Implied,
        },

        (0b00, 0b001, 0b110) => Opcode::Sec {
            addressing_mode: AddressingMode::Implied,
        },

        (0b00, 0b010, 0b110) => Opcode::Cli {
            addressing_mode: AddressingMode::Implied,
        },

        (0b00, 0b011, 0b110) => Opcode::Sei {
            addressing_mode: AddressingMode::Implied,
        },

        (0b00, 0b100, 0b110) => Opcode::Tya {
            addressing_mode: AddressingMode::Implied,
        },

        (0b00, 0b101, 0b110) => Opcode::Clv {
            addressing_mode: AddressingMode::Implied,
        },

        (0b00, 0b110, 0b110) => Opcode::Cld {
            addressing_mode: AddressingMode::Implied,
        },

        (0b00, 0b111, 0b110) => Opcode::Sed {
            addressing_mode: AddressingMode::Implied,
        },

        (0b10, 0b100, 0b010) => Opcode::Txa {
            addressing_mode: AddressingMode::Implied,
        },

        (0b10, 0b100, 0b110) => Opcode::Txs {
            addressing_mode: AddressingMode::Implied,
        },

        (0b10, 0b101, 0b010) => Opcode::Tax {
            addressing_mode: AddressingMode::Implied,
        },

        (0b10, 0b101, 0b110) => Opcode::Tsx {
            addressing_mode: AddressingMode::Implied,
        },

        (0b10, 0b110, 0b010) => Opcode::Dex {
            addressing_mode: AddressingMode::Implied,
        },

        (0b10, 0b111, 0b010) => Opcode::Nop {
            addressing_mode: AddressingMode::Implied,
        },

        _ => Opcode::Illegal,
    }
}
