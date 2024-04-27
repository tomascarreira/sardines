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
        let bus = Bus::new(cart);
        let reset_vector = bus.get_reset_vector();
        Nes {
            bus,
            cpu: Cpu::new(reset_vector),
            ppu: Ppu {},
        }
    }

    pub fn run(&mut self) {
        loop {
            self.cpu.cycle(&mut self.bus);
        }
    }
}
