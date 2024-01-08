use crate::cartridge::Cartridge;

// 2 KiB of ram
const RAM_SIZE: usize = 0x800;

pub struct Bus {
    open_bus: u8,
    ram: [u8; RAM_SIZE],
    cartridge: Cartridge,
    ppu_registers: PpuRegisters,
    apu_registers: ApuRegisters,
    io_registers: IORegisters,
}

impl Bus {
    pub fn new(cartridge: Cartridge) -> Self {
        Bus {
            open_bus: 0x00,
            // Is it ok to have this must memory on the stack?
            ram: [0x00; RAM_SIZE],
            cartridge,
            ppu_registers: PpuRegisters {},
            apu_registers: ApuRegisters {},
            io_registers: IORegisters {},
        }
    }

    pub fn read(&self, address: u16) -> u8 {
        match address {
            // Can the compiler see that it does not need array bound checking
            0x0000..=0x1fff => self.ram[(address % RAM_SIZE as u16) as usize],
            0x2000..=0x3fff => todo!(),
            0x4000..=0x4017 => todo!(),
            0x4018..=0x401f => todo!(),
            0x4020..=0xffff => todo!(),
        }
    }

    pub fn write(&mut self, value: u8, address: u16) {
        match address {
            // Can the compiler see that it does not need array bound checking
            0x0000..=0x1fff => self.ram[(address % RAM_SIZE as u16) as usize] = value,
            0x2000..=0x3fff => todo!(),
            0x4000..=0x4017 => todo!(),
            0x4018..=0x401f => todo!(),
            0x4020..=0xffff => todo!(),
        }
    }
}

struct PpuRegisters {}

struct ApuRegisters {}

struct IORegisters {}
