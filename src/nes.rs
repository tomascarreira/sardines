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
            println!(
                "Output of test: {:x} {:x} {:x} {:x}",
                self.bus.read(0x6000),
                self.bus.read(0x6001),
                self.bus.read(0x6002),
                self.bus.read(0x6003)
            );
            // println!(
            //     "Output of test: {:x} {:x}",
            //     self.bus.read(0x0002),
            //     self.bus.read(0x6003),
            // );
            self.cpu.cycle(&mut self.bus);
        }
    }
}
