use crate::cartridge::Cartridge;
use crate::ppu::Ppu;

// 2 KiB of ram
const RAM_SIZE: usize = 0x800;

pub struct Bus {
    open_bus: u8,
    ram: Vec<u8>,
    cart: Cartridge,
    ppu: Ppu,
    apu_registers: ApuRegisters,
    io_registers: IORegisters,
}

impl Bus {
    pub fn new(cart: Cartridge, ppu: Ppu) -> Self {
        Bus {
            open_bus: 0x00,
            ram: vec![0; RAM_SIZE],
            cart,
            ppu,
            apu_registers: ApuRegisters {},
            io_registers: IORegisters {},
        }
    }

    pub fn read(&self, address: u16) -> u8 {
        match address {
            // Can the compiler see that it does not need array bound checking?
            0x0000..=0x1fff => self.ram[(address % RAM_SIZE as u16) as usize],
            0x2000..=0x3fff => self.ppu.register_read((address % 8) as u8),
            0x4000..=0x4017 => 0,
            0x4018..=0x401f => todo!(),
            0x4020..=0xffff => self.cart.read(address),
        }
    }

    pub fn write(&mut self, value: u8, address: u16) {
        match address {
            // Can the compiler see that it does not need array bound checking?
            0x0000..=0x1fff => self.ram[(address % RAM_SIZE as u16) as usize] = value,
            0x2000..=0x3fff => self.ppu.register_write(value, (address % 8) as u8),
            0x4000..=0x4017 => (),
            0x4018..=0x401f => todo!(),
            0x4020..=0xffff => self.cart.write(value, address),
        }
    }

    pub fn chr_read(&self, addr: u16) -> u8 {
        self.cart.chr_read(addr)
    }

    pub fn chr_write(&mut self, value: u8, addr: u16) {
        self.cart.chr_write(value, addr)
    }

    pub fn get_reset_vector(&self) -> u16 {
        self.read(0xfffc) as u16 | (self.read(0xfffd) as u16) << 8
    }
}

struct ApuRegisters {}

struct IORegisters {}
