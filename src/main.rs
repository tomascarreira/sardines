mod bus;
mod cartridge;
mod cpu;
mod nes;
mod ppu;

use cartridge::Cartridge;
use nes::Nes;

fn main() {
    let mut nes = Nes::new(Cartridge {});
    nes.run();
}
