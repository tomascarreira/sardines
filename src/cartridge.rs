use std::fs;

use crate::{
    ines::INes,
    mapper::{mmc1::MMC1, nrom::Nrom, Mapper},
};

pub struct Cartridge {
    mapper: Box<dyn Mapper>,
}

impl Cartridge {
    pub fn load(file_name: &str) -> Self {
        let rom = fs::read(file_name).unwrap();
        let ines = INes::new(&rom[0..16]);

        let mapper: Box<dyn Mapper> = match ines.mapper {
            0 => Box::new(Nrom::new(&rom, ines.prgrom_size, ines.chrrom_size)),
            1 => Box::new(MMC1::new(&rom, ines.prgrom_size, ines.chrrom_size)),
            _ => unimplemented!("Unsupported mapper: {}", ines.mapper),
        };

        Cartridge { mapper }
    }

    pub fn read(&self, address: u16) -> u8 {
        self.mapper.read(address)
    }

    pub fn write(&mut self, value: u8, address: u16) {
        self.mapper.write(value, address);
    }

    pub fn chr_read(&self, addr: u16) -> u8 {
        self.mapper.chr_read(addr)
    }

    pub fn chr_write(&mut self, value: u8, addr: u16) {
        self.mapper.chr_write(value, addr)
    }
}
