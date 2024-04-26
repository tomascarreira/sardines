pub trait Mapper {
    fn read(&self, address: u16) -> u8;

    fn write(&mut self, value: u8, address: u16);
}

pub struct NROM {
    prgrom: Vec<u8>,
    chrrom: Vec<u8>,
    prgram: Vec<u8>,
}

impl NROM {
    pub fn new(rom: &[u8], prgrom_size: usize, chrrom_size: usize) -> Self {
        assert!(prgrom_size == 1 || prgrom_size == 2);
        assert_eq!(chrrom_size, 1);
        assert_eq!(rom.len(), 16 + prgrom_size * 0x4000 + chrrom_size * 0x2000);
        NROM {
            prgrom: rom[16..16 + 0x4000 * prgrom_size].to_vec(),
            chrrom: rom[16 + 0x4000..16 + 0x4000 + 0x2000 * chrrom_size].to_vec(),
            prgram: vec![0; 0x2000],
        }
    }
}

impl Mapper for NROM {
    fn read(&self, address: u16) -> u8 {
        match address {
            0x0000..=0x5fff => unreachable!(),
            0x6000..=0x7fff => self.prgram[(address - 0x6000) as usize],
            0x8000..=0xbfff => self.prgrom[(address - 0x8000) as usize],
            0xc000..=0xffff => {
                if self.prgrom.len() == 0x4000 {
                    self.prgrom[(address - 0xc000) as usize]
                } else {
                    self.prgrom[(address - 0x8000) as usize]
                }
            }
        }
    }

    fn write(&mut self, value: u8, address: u16) {
        match address {
            0x0000..=0x5fff => unreachable!(),
            0x6000..=0x7fff => self.prgram[(address - 0x6000) as usize] = value,
            0x8000..=0xffff => println!("INFO: writing on read-only prg rom"),
        }
    }
}
