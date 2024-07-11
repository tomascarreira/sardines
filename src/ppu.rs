use std::cell::Cell;

use crate::{
    bus::Bus,
    cpu::{bit_to_bool, bool_to_bit},
};

pub struct Ppu {
    registers: Registers,
    vram: Vec<u8>,
    oam: Vec<u8>,
    palette: Vec<u8>,

    internal_bus_latch: Cell<u8>,
    read_buffer: Cell<u8>,
    v: u16,
    t: u16,
    x: u8,
    w: Cell<bool>,
}

impl Ppu {
    pub fn new() -> Self {
        Ppu {
            registers: Registers::new(),
            vram: vec![0; 0x1000],
            oam: vec![0; 64 * 4],
            palette: vec![0; 0x20],
            internal_bus_latch: Cell::new(0),
            read_buffer: Cell::new(0),
            v: 0,
            t: 0,
            x: 0,
            w: Cell::new(false),
        }
    }

    pub fn cycle(&mut self, bus: &mut Bus) {}

    // TODO: change this register type to something more expressive, maybe an enum
    pub fn register_read(&self, register: u8) -> u8 {
        match register {
            0 => self.internal_bus_latch.get(),
            1 => self.internal_bus_latch.get(),
            2 => {
                // Dafuk!
                let res: u8 = <&PpuStatus as Into<u8>>::into(&self.registers.ppu_status.clone())
                    | self.internal_bus_latch.get() & 0b0001_1111;
                self.internal_bus_latch.set(res);
                self.registers.ppu_status.vertical_blank.set(false);
                self.w.set(false);
                res
            }
            3 => self.internal_bus_latch.get(),
            4 => {
                let res = self.registers.oam_data;
                self.internal_bus_latch.set(res);
                res
            }
            5 => self.internal_bus_latch.get(),
            6 => self.internal_bus_latch.get(),
            7 => {
                let res = self.read_buffer.get();
                self.internal_bus_latch.set(res);
                self.read_buffer.set(self.vram[self.v as usize]);
                // TODO: Implement different behavoir during rendering
                // TODO: Use Cell::update() when it is merged from nightly
                let mut ppu_addr = self.registers.ppu_addr.get();
                // TODO: Handle integer wrapping
                ppu_addr += if self.registers.ppu_ctrl.vram_addr_inc == 0 {
                    1
                } else if self.registers.ppu_ctrl.vram_addr_inc == 1 {
                    32
                } else {
                    unreachable!()
                };
                self.registers.ppu_addr.replace(ppu_addr);

                res
            }
            _ => unreachable!(),
        }
    }

    pub fn register_write(&mut self, value: u8, register: u8) {
        match register {
            0 => {
                self.registers.ppu_ctrl = PpuCtrl::from(value);
                self.t = (self.t & 0b0111_0011_1111_1111) | ((value as u16 & 0b0000_0011) << 10)
            }
            1 => self.registers.ppu_mask = PpuMask::from(value),
            2 => (),
            3 => self.registers.oam_adr = value,
            4 => {
                // TODO: behaviour is different when ppu is rendering
                self.oam[self.registers.oam_adr as usize] = value;
                self.registers.oam_data += 1;
            }
            5 => {
                if self.w.get() {
                    self.t = (self.t & 0b0000_1100_0001_1111)
                        | ((value as u16 & 0b1100_0000) << 2)
                        | ((value as u16 & 0b0011_1000) << 2)
                        | ((value as u16 & 0b0000_0111) << 12);
                    self.w.set(false);
                } else {
                    self.t = (self.t & 0b0111_1111_1110_0000) | ((value as u16 & 0b1111_1000) >> 3);
                    self.x = value & 0b0000_0111;
                    self.w.set(true);
                }
            }
            6 => {
                if self.w.get() {
                    self.t = (self.t & 0b1111_1111_0000_0000) | value as u16;
                    self.v = self.t;
                    self.w.set(false);
                } else {
                    self.t = (self.t & 0b0000_0000_1111_1111) | ((value as u16 & 0b0011_1111) << 8);
                    self.w.set(true);
                }
            }
            7 => {
                self.vram[self.v as usize] = value;
                // TODO: Implement different behavoir during rendering
                // TODO: Use Cell::update() when it is merged from nightly
                let mut ppu_addr = self.registers.ppu_addr.get();
                // TODO: Handle integer wrapping
                ppu_addr += if self.registers.ppu_ctrl.vram_addr_inc == 0 {
                    1
                } else if self.registers.ppu_ctrl.vram_addr_inc == 1 {
                    32
                } else {
                    unreachable!()
                };
                self.registers.ppu_addr.replace(ppu_addr);
            }
            _ => unimplemented!(),
        }

        self.internal_bus_latch.set(value);
    }

    fn read(&self, addr: u16, bus: &Bus) -> u8 {
        match addr {
            0x0000..=0x1fff => bus.chr_read(addr),
            0x2000..=0x3eff => self.vram[((addr - 0x2000) % 0x1000) as usize],
            0x3f00..=0x3fff => self.palette[(addr - 0x3f00) as usize],
            _ => unreachable!(),
        }
    }

    fn write(&mut self, value: u8, addr: u16, bus: &mut Bus) {
        match addr {
            0x0000..=0x1fff => bus.chr_write(value, addr),
            0x2000..=0x3eff => self.vram[((addr - 0x2000) % 0x1000) as usize] = value,
            0x3f00..=0x3fff => self.palette[(addr - 0x3f00) as usize] = value,
            _ => unreachable!(),
        }
    }
}

struct Registers {
    ppu_ctrl: PpuCtrl,
    ppu_mask: PpuMask,
    ppu_status: PpuStatus,
    oam_adr: u8,
    oam_data: u8,
    ppu_scroll: u8,
    ppu_addr: Cell<u8>,
    ppu_data: u8,
}

impl Registers {
    fn new() -> Self {
        Registers {
            ppu_ctrl: PpuCtrl::new(),
            ppu_mask: PpuMask::new(),
            ppu_status: PpuStatus::new(),
            oam_adr: 0,
            oam_data: 0,
            ppu_scroll: 0,
            ppu_addr: Cell::new(0),
            ppu_data: 0,
        }
    }
}

struct PpuCtrl {
    base_nametable_addr: u8,
    vram_addr_inc: u8,
    sprite_pattern_table_addr: u8,
    background_pattern_table_addr: u8,
    sprite_size: u8,
    ppu_master_slave_select: u8,
    generate_nmi: bool,
}

impl PpuCtrl {
    fn new() -> Self {
        PpuCtrl {
            base_nametable_addr: 0,
            vram_addr_inc: 0,
            sprite_pattern_table_addr: 0,
            background_pattern_table_addr: 0,
            sprite_size: 0,
            ppu_master_slave_select: 0,
            generate_nmi: false,
        }
    }
}

impl From<u8> for PpuCtrl {
    fn from(value: u8) -> Self {
        PpuCtrl {
            base_nametable_addr: value & 0b0000_0011 >> 0,
            vram_addr_inc: value & 0b0000_0100 >> 2,
            sprite_pattern_table_addr: value & 0b0000_1000 >> 3,
            background_pattern_table_addr: value & 0b0001_0000 >> 4,
            sprite_size: value & 0b0010_0000 >> 5,
            ppu_master_slave_select: value & 0b0100_0000 >> 6,
            generate_nmi: bit_to_bool(value & 0b1000_0000 >> 7),
        }
    }
}

struct PpuMask {
    greyscale: bool,
    show_background_in_leftmost_8_pixels_of_screen: bool,
    show_sprites_in_leftmost_8_pixels_of_screen: bool,
    show_background: bool,
    show_sprites: bool,
    emphasize_red: bool,
    emphasize_green: bool,
    emphasize_blue: bool,
}

impl PpuMask {
    fn new() -> Self {
        PpuMask {
            greyscale: false,
            show_background_in_leftmost_8_pixels_of_screen: false,
            show_sprites_in_leftmost_8_pixels_of_screen: false,
            show_background: false,
            show_sprites: false,
            emphasize_red: false,
            emphasize_green: false,
            emphasize_blue: false,
        }
    }
}

impl From<u8> for PpuMask {
    fn from(value: u8) -> Self {
        PpuMask {
            greyscale: bit_to_bool(value & 0b0000_0001 >> 0),
            show_background_in_leftmost_8_pixels_of_screen: bit_to_bool(value & 0b0000_0010 >> 1),
            show_sprites_in_leftmost_8_pixels_of_screen: bit_to_bool(value & 0b0000_0100 >> 2),
            show_background: bit_to_bool(value & 0b0000_1000 >> 3),
            show_sprites: bit_to_bool(value & 0b0001_0000 >> 4),
            emphasize_red: bit_to_bool(value & 0b0010_0000 >> 5),
            emphasize_green: bit_to_bool(value & 0b0100_0000 >> 6),
            emphasize_blue: bit_to_bool(value & 0b1000_0000 >> 7),
        }
    }
}

#[derive(Clone)]
struct PpuStatus {
    sprite_overflow: bool,
    sprite_0_hit: bool,
    vertical_blank: Cell<bool>,
}

impl PpuStatus {
    fn new() -> Self {
        PpuStatus {
            sprite_overflow: false,
            sprite_0_hit: false,
            vertical_blank: Cell::new(false),
        }
    }
}

impl From<&PpuStatus> for u8 {
    fn from(ppu_ctrl: &PpuStatus) -> Self {
        bool_to_bit(ppu_ctrl.sprite_overflow) << 5
            | bool_to_bit(ppu_ctrl.sprite_0_hit) << 6
            | bool_to_bit(ppu_ctrl.vertical_blank.get()) << 7
    }
}
