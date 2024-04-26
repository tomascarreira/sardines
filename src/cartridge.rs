use std::fs;

use crate::{
    ines::INes,
    mapper::{Mapper, NROM},
};

pub struct Cartridge {
    mapper: Box<dyn Mapper>,
}

impl Cartridge {
    pub fn load(file_name: &str) -> Self {
        let rom = fs::read(file_name).unwrap();
        let ines = INes::new(&rom[0..16]);

        let mapper = match ines.mapper {
            0 => NROM::new(&rom, ines.prgrom_size, ines.chrrom_size),
            _ => unimplemented!(),
        };

        Cartridge {
            mapper: Box::new(mapper),
        }
    }

    pub fn read(&self, address: u16) -> u8 {
        self.mapper.read(address)
    }

    pub fn write(&mut self, value: u8, address: u16) {
        self.mapper.write(value, address);
    }
}
