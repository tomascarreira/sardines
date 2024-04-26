use std::env;

mod bus;
mod cartridge;
mod cpu;
mod ines;
mod mapper;
mod nes;
mod ppu;

use cartridge::Cartridge;
use nes::Nes;

fn main() {
    let args: Vec<String> = env::args().collect();
    let mut nes = Nes::new(Cartridge::load(&args[1]));
    nes.run();
}
