use crate::bus::Bus;
use crate::cpu::Cpu;
use crate::ppu::Ppu;

pub struct Nes {
    bus: Bus,
    cpu: Cpu,
    ppu: Ppu,
}
