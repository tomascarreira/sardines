use bus::Bus;
use cpu::Cpu;

mod bus;
mod cpu;

fn main() {
    let mut bus = Bus;
    let mut cpu = Cpu::new();

    cpu.cycle(&mut bus);
}
