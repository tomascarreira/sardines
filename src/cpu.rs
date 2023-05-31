pub struct Cpu {
    a: u8,
    x: u8,
    y: u8,
    s: u8,
    p: StatusFlag,
    pc: u16,
}

impl Cpu {
    pub fn new() -> Self {
        Cpu {
            a: 0,
            x: 0,
            y: 0,
            s: 0xff,
            p: StatusFlag {
                carry: false,
                zero: false,
                interrupt_disable: true,
                decimal: false,
                overflow: false,
                negative: false,
            },
            pc: 0,
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

enum Instruction {
    Illegal,
    Adc(AddressingMode),
    And(AddressingMode),
    Asl(AddressingMode),
    Bcc(AddressingMode),
    Bcs(AddressingMode),
    Beq(AddressingMode),
    Bit(AddressingMode),
    Bmi(AddressingMode),
    Bne(AddressingMode),
    Bpl(AddressingMode),
    Brk(AddressingMode),
    Bvc(AddressingMode),
    Bvs(AddressingMode),
    Clc(AddressingMode),
    Cld(AddressingMode),
    Cli(AddressingMode),
    Clv(AddressingMode),
    Cmp(AddressingMode),
    Cpx(AddressingMode),
    Cpy(AddressingMode),
    Dec(AddressingMode),
    Dex(AddressingMode),
    Dey(AddressingMode),
    Eor(AddressingMode),
    Inc(AddressingMode),
    Inx(AddressingMode),
    Iny(AddressingMode),
    Jmp(AddressingMode),
    Jsr(AddressingMode),
    Lda(AddressingMode),
    Ldx(AddressingMode),
    Ldy(AddressingMode),
    Lsr(AddressingMode),
    Nop(AddressingMode),
    Ora(AddressingMode),
    Pha(AddressingMode),
    Php(AddressingMode),
    Pla(AddressingMode),
    Plp(AddressingMode),
    Rol(AddressingMode),
    Ror(AddressingMode),
    Rti(AddressingMode),
    Rts(AddressingMode),
    Sbc(AddressingMode),
    Sec(AddressingMode),
    Sed(AddressingMode),
    Sei(AddressingMode),
    Sta(AddressingMode),
    Stx(AddressingMode),
    Sty(AddressingMode),
    Tax(AddressingMode),
    Tay(AddressingMode),
    Tsx(AddressingMode),
    Txa(AddressingMode),
    Txs(AddressingMode),
    Tya(AddressingMode),
}

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

// Fix: return error  (maybe use thiserror or just use anyhow)
fn decode(opcode: u8) -> Instruction {
    let aaa = opcode >> 5;
    let bbb = (opcode & 0b00011100) >> 2;
    let cc = opcode & 0b00000011;

    match (cc, aaa, bbb) {
        (0b01, 0b000, 0b000) => Instruction::Ora(AddressingMode::IndirectX),
        (0b01, 0b000, 0b001) => Instruction::Ora(AddressingMode::Zeropage),
        (0b01, 0b000, 0b010) => Instruction::Ora(AddressingMode::Immediate),
        (0b01, 0b000, 0b011) => Instruction::Ora(AddressingMode::Absolute),
        (0b01, 0b000, 0b100) => Instruction::Ora(AddressingMode::IndirectY),
        (0b01, 0b000, 0b101) => Instruction::Ora(AddressingMode::ZeropageX),
        (0b01, 0b000, 0b110) => Instruction::Ora(AddressingMode::AbsoluteY),
        (0b01, 0b000, 0b111) => Instruction::Ora(AddressingMode::AbsoluteX),

        (0b01 ,0b001, 0b000) => Instruction::And(AddressingMode::IndirectX),
        (0b01 ,0b001, 0b001) => Instruction::And(AddressingMode::Zeropage),
        (0b01 ,0b001, 0b010) => Instruction::And(AddressingMode::Immediate),
        (0b01 ,0b001, 0b011) => Instruction::And(AddressingMode::Absolute),
        (0b01 ,0b001, 0b100) => Instruction::And(AddressingMode::IndirectY),
        (0b01 ,0b001, 0b101) => Instruction::And(AddressingMode::ZeropageX),
        (0b01 ,0b001, 0b110) => Instruction::And(AddressingMode::AbsoluteY),
        (0b01 ,0b001, 0b111) => Instruction::And(AddressingMode::AbsoluteX),

        (0b01, 0b010, 0b000) => Instruction::Eor(AddressingMode::IndirectX),
        (0b01, 0b010, 0b001) => Instruction::Eor(AddressingMode::Zeropage),
        (0b01, 0b010, 0b010) => Instruction::Eor(AddressingMode::Immediate),
        (0b01, 0b010, 0b011) => Instruction::Eor(AddressingMode::Absolute),
        (0b01, 0b010, 0b100) => Instruction::Eor(AddressingMode::IndirectY),
        (0b01, 0b010, 0b101) => Instruction::Eor(AddressingMode::ZeropageX),
        (0b01, 0b010, 0b110) => Instruction::Eor(AddressingMode::AbsoluteY),
        (0b01, 0b010, 0b111) => Instruction::Eor(AddressingMode::AbsoluteX),

        (0b01, 0b011, 0b000) => Instruction::Adc(AddressingMode::IndirectX),
        (0b01, 0b011, 0b001) => Instruction::Adc(AddressingMode::Zeropage),
        (0b01, 0b011, 0b010) => Instruction::Adc(AddressingMode::Immediate),
        (0b01, 0b011, 0b011) => Instruction::Adc(AddressingMode::Absolute),
        (0b01, 0b011, 0b100) => Instruction::Adc(AddressingMode::IndirectY),
        (0b01, 0b011, 0b101) => Instruction::Adc(AddressingMode::ZeropageX),
        (0b01, 0b011, 0b110) => Instruction::Adc(AddressingMode::AbsoluteY),
        (0b01, 0b011, 0b111) => Instruction::Adc(AddressingMode::AbsoluteX),

        (0b01, 0b100, 0b000) => Instruction::Sta(AddressingMode::IndirectX),
        (0b01, 0b100, 0b001) => Instruction::Sta(AddressingMode::Zeropage),
        (0b01, 0b100, 0b011) => Instruction::Sta(AddressingMode::Absolute),
        (0b01, 0b100, 0b100) => Instruction::Sta(AddressingMode::IndirectY),
        (0b01, 0b100, 0b101) => Instruction::Sta(AddressingMode::ZeropageX),
        (0b01, 0b100, 0b110) => Instruction::Sta(AddressingMode::AbsoluteY),
        (0b01, 0b100, 0b111) => Instruction::Sta(AddressingMode::AbsoluteX),

        (0b01, 0b101, 0b000) => Instruction::Lda(AddressingMode::IndirectX),
        (0b01, 0b101, 0b001) => Instruction::Lda(AddressingMode::Zeropage),
        (0b01, 0b101, 0b010) => Instruction::Lda(AddressingMode::Immediate),
        (0b01, 0b101, 0b011) => Instruction::Lda(AddressingMode::Absolute),
        (0b01, 0b101, 0b100) => Instruction::Lda(AddressingMode::IndirectY),
        (0b01, 0b101, 0b101) => Instruction::Lda(AddressingMode::ZeropageX),
        (0b01, 0b101, 0b110) => Instruction::Lda(AddressingMode::AbsoluteY),
        (0b01, 0b101, 0b111) => Instruction::Lda(AddressingMode::AbsoluteX),

        (0b01, 0b110, 0b000) => Instruction::Cmp(AddressingMode::IndirectX),
        (0b01, 0b110, 0b001) => Instruction::Cmp(AddressingMode::Zeropage),
        (0b01, 0b110, 0b010) => Instruction::Cmp(AddressingMode::Immediate),
        (0b01, 0b110, 0b011) => Instruction::Cmp(AddressingMode::Absolute),
        (0b01, 0b110, 0b100) => Instruction::Cmp(AddressingMode::IndirectY),
        (0b01, 0b110, 0b101) => Instruction::Cmp(AddressingMode::ZeropageX),
        (0b01, 0b110, 0b110) => Instruction::Cmp(AddressingMode::AbsoluteY),
        (0b01, 0b110, 0b111) => Instruction::Cmp(AddressingMode::AbsoluteX),

        (0b01, 0b111, 0b000) => Instruction::Sbc(AddressingMode::IndirectX),
        (0b01, 0b111, 0b001) => Instruction::Sbc(AddressingMode::Zeropage),
        (0b01, 0b111, 0b010) => Instruction::Sbc(AddressingMode::Immediate),
        (0b01, 0b111, 0b011) => Instruction::Sbc(AddressingMode::Absolute),
        (0b01, 0b111, 0b100) => Instruction::Sbc(AddressingMode::IndirectY),
        (0b01, 0b111, 0b101) => Instruction::Sbc(AddressingMode::ZeropageX),
        (0b01, 0b111, 0b110) => Instruction::Sbc(AddressingMode::AbsoluteY),
        (0b01, 0b111, 0b111) => Instruction::Sbc(AddressingMode::AbsoluteX),

        (0b10, 0b000, 0b001) => Instruction::Asl(AddressingMode::Zeropage),
        (0b10, 0b000, 0b010) => Instruction::Asl(AddressingMode::Accumulator),
        (0b10, 0b000, 0b011) => Instruction::Asl(AddressingMode::Absolute),
        (0b10, 0b000, 0b101) => Instruction::Asl(AddressingMode::ZeropageX),
        (0b10, 0b000, 0b111) => Instruction::Asl(AddressingMode::AbsoluteX),

        (0b10, 0b001, 0b001) => Instruction::Rol(AddressingMode::Zeropage),
        (0b10, 0b001, 0b010) => Instruction::Rol(AddressingMode::Accumulator),
        (0b10, 0b001, 0b011) => Instruction::Rol(AddressingMode::Absolute),
        (0b10, 0b001, 0b101) => Instruction::Rol(AddressingMode::ZeropageX),
        (0b10, 0b001, 0b111) => Instruction::Rol(AddressingMode::AbsoluteX),

        (0b10, 0b010, 0b001) => Instruction::Lsr(AddressingMode::Zeropage),
        (0b10, 0b010, 0b010) => Instruction::Lsr(AddressingMode::Accumulator),
        (0b10, 0b010, 0b011) => Instruction::Lsr(AddressingMode::Absolute),
        (0b10, 0b010, 0b101) => Instruction::Lsr(AddressingMode::ZeropageX),
        (0b10, 0b010, 0b111) => Instruction::Lsr(AddressingMode::AbsoluteX),

        (0b10, 0b011, 0b001) => Instruction::Ror(AddressingMode::Zeropage),
        (0b10, 0b011, 0b010) => Instruction::Ror(AddressingMode::Accumulator),
        (0b10, 0b011, 0b011) => Instruction::Ror(AddressingMode::Absolute),
        (0b10, 0b011, 0b101) => Instruction::Ror(AddressingMode::ZeropageX),
        (0b10, 0b011, 0b111) => Instruction::Ror(AddressingMode::AbsoluteX),

        (0b10, 0b100, 0b001) => Instruction::Stx(AddressingMode::Zeropage),
        (0b10, 0b100, 0b011) => Instruction::Stx(AddressingMode::Absolute),
        (0b10, 0b100, 0b101) => Instruction::Stx(AddressingMode::ZeropageY),

        (0b10, 0b101, 0b000) => Instruction::Ldx(AddressingMode::Immediate),
        (0b10, 0b101, 0b001) => Instruction::Ldx(AddressingMode::Zeropage),
        (0b10, 0b101, 0b011) => Instruction::Ldx(AddressingMode::Absolute),
        (0b10, 0b101, 0b101) => Instruction::Ldx(AddressingMode::ZeropageY),
        (0b10, 0b101, 0b111) => Instruction::Ldx(AddressingMode::AbsoluteY),

        (0b10, 0b110, 0b001) => Instruction::Dec(AddressingMode::Zeropage),
        (0b10, 0b110, 0b011) => Instruction::Dec(AddressingMode::Absolute),
        (0b10, 0b110, 0b101) => Instruction::Dec(AddressingMode::ZeropageX),
        (0b10, 0b110, 0b111) => Instruction::Dec(AddressingMode::AbsoluteX),

        (0b10, 0b111, 0b001) => Instruction::Inc(AddressingMode::Zeropage),
        (0b10, 0b111, 0b011) => Instruction::Inc(AddressingMode::Absolute),
        (0b10, 0b111, 0b101) => Instruction::Inc(AddressingMode::ZeropageX),
        (0b10, 0b111, 0b111) => Instruction::Inc(AddressingMode::AbsoluteX),

        (0b00, 0b001, 0b001) => Instruction::Bit(AddressingMode::Zeropage),
        (0b00, 0b001, 0b011) => Instruction::Bit(AddressingMode::Absolute),

        (0b00, 0b010, 0b011) => Instruction::Jmp(AddressingMode::Absolute),

        (0b00, 0b011, 0b011) => Instruction::Jmp(AddressingMode::Indirect),

        (0b00, 0b100, 0b001) => Instruction::Sty(AddressingMode::Zeropage),
        (0b00, 0b100, 0b011) => Instruction::Sty(AddressingMode::Absolute),
        (0b00, 0b100, 0b101) => Instruction::Sty(AddressingMode::ZeropageX),

        (0b00, 0b101, 0b000) => Instruction::Ldy(AddressingMode::Absolute),
        (0b00, 0b101, 0b001) => Instruction::Ldy(AddressingMode::Zeropage),
        (0b00, 0b101, 0b011) => Instruction::Ldy(AddressingMode::Absolute),
        (0b00, 0b101, 0b101) => Instruction::Ldy(AddressingMode::ZeropageX),
        (0b00, 0b101, 0b111) => Instruction::Ldy(AddressingMode::AbsoluteX),

        (0b00, 0b110, 0b000) => Instruction::Cpy(AddressingMode::Immediate),
        (0b00, 0b110, 0b001) => Instruction::Cpy(AddressingMode::Zeropage),
        (0b00, 0b110, 0b011) => Instruction::Cpy(AddressingMode::Absolute),

        (0b00, 0b111, 0b000) => Instruction::Cpx(AddressingMode::Immediate),
        (0b00, 0b111, 0b001) => Instruction::Cpx(AddressingMode::Zeropage),
        (0b00, 0b111, 0b011) => Instruction::Cpx(AddressingMode::Absolute),

        (0b00, 0b000, 0b100) => Instruction::Bpl(AddressingMode::Implied),

        (0b00, 0b001, 0b100) => Instruction::Bmi(AddressingMode::Implied),

        (0b00, 0b010, 0b100) => Instruction::Bvc(AddressingMode::Implied),

        (0b00, 0b011, 0b100) => Instruction::Bvs(AddressingMode::Implied),

        (0b00, 0b100, 0b100) => Instruction::Bcc(AddressingMode::Implied),

        (0b00, 0b101, 0b100) => Instruction::Bcs(AddressingMode::Implied),

        (0b00, 0b110, 0b100) => Instruction::Bne(AddressingMode::Implied),

        (0b00, 0b111, 0b100) => Instruction::Beq(AddressingMode::Implied),

        (0b00, 0b000, 0b000) => Instruction::Brk(AddressingMode::Implied),

        (0b00, 0b001, 0b000) => Instruction::Jsr(AddressingMode::Absolute),

        (0b00, 0b010, 0b000) => Instruction::Rti(AddressingMode::Implied),

        (0b00, 0b011, 0b000) => Instruction::Rts(AddressingMode::Implied),

        _ => Instruction::Illegal,
    }
}

enum DecodeError {
    Default,
}
