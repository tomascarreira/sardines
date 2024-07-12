use crate::bus::Bus;
use crate::cartridge::Cartridge;
use crate::cpu::Cpu;
use crate::ppu::Ppu;

pub struct Nes {
    bus: Bus,
    cpu: Cpu,
}

impl Nes {
    pub fn new(cart: Cartridge) -> Self {
        let bus = Bus::new(cart, Ppu::new());
        let reset_vector = bus.get_reset_vector();
        Nes {
            bus,
            cpu: Cpu::new(reset_vector),
        }
    }

    pub fn run(&mut self) {
        loop {
            self.cpu.cycle(&mut self.bus);
            self.bus.ppu.cycle(&mut self.bus.cart);
            self.bus.ppu.cycle(&mut self.bus.cart);
            self.bus.ppu.cycle(&mut self.bus.cart);
        }
    }
}
