use crate::bus::Bus;
use crate::cartridge::Cartridge;
use crate::cpu::Cpu;
use crate::ppu::Ppu;

pub struct Nes {
    bus: Bus,
    cpu: Cpu,
    ppu: Ppu,
}

impl Nes {
    pub fn new(cart: Cartridge) -> Self {
        Nes {
            bus: Bus::new(cart),
            cpu: Cpu::new(),
            ppu: Ppu {},
        }
    }

    pub fn run(&mut self) {
        loop {
            println!("{}", self.cpu);
            self.cpu.cycle(&mut self.bus);
        }
    }
}
