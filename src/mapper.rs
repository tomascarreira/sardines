pub trait Mapper {
    fn read(&self, address: u16) -> u8;

    fn write(&mut self, value: u8, address: u16);
}
pub mod nrom {
    use crate::mapper::Mapper;

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
                chrrom: rom
                    [16 + 0x4000 * prgrom_size..16 + 0x4000 * prgrom_size + 0x2000 * chrrom_size]
                    .to_vec(),
                prgram: vec![0; 0x2000],
            }
        }
    }

    impl Mapper for NROM {
        fn read(&self, address: u16) -> u8 {
            match address {
                0x0000..=0x401f => unreachable!(),
                0x4020..=0x5fff => todo!(),
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
                0x0000..=0x401f => unreachable!(),
                0x4020..=0x5fff => todo!(),
                0x6000..=0x7fff => self.prgram[(address - 0x6000) as usize] = value,
                0x8000..=0xffff => println!("INFO: writing on read-only prg rom"),
            }
        }
    }
}
pub mod mmc1 {
    use std::cell::Cell;

    use crate::cpu::bit_to_bool;
    use crate::mapper::Mapper;

    pub struct MMC1 {
        prgrom: Vec<u8>,
        chrrom: Vec<u8>,
        prgram: Vec<u8>,
        control: Control,
        chr_bank0: usize,
        chr_bank1: usize,
        prg_bank: PRGBank,
        shift_register: ShiftRegister,
        write_last_cycle: Cell<bool>,
    }

    impl MMC1 {
        pub fn new(rom: &[u8], prgrom_size: usize, chrrom_size: usize) -> Self {
            assert!(prgrom_size == 16 || prgrom_size == 32);
            assert!(chrrom_size == 16 || chrrom_size == 0);
            assert_eq!(rom.len(), 16 + prgrom_size * 0x4000 + chrrom_size * 0x2000);

            let prgrom = rom[16..16 + prgrom_size * 0x4000].to_vec();
            let chrrom = if chrrom_size == 0 {
                vec![0; 0x20000]
            } else {
                rom[16 + 0x4000 * prgrom_size..16 + 0x4000 + 0x2000 * chrrom_size].to_vec()
            };

            MMC1 {
                prgrom,
                chrrom,
                prgram: vec![0; 0x8000],
                control: Control {
                    mirroring: Mirroring::OneScreenLowerBank,
                    prg_rom_bank_mode: PRGROMBankMode::Switch32K,
                    chr_rom_bank_mode: CHRROMBankMode::Switch8K,
                },
                chr_bank0: 0,
                chr_bank1: 0,
                prg_bank: PRGBank {
                    select: 0,
                    prg_ram_chip_enabled: false,
                },
                shift_register: ShiftRegister { reg: 0, shifts: 0 },
                write_last_cycle: Cell::new(false),
            }
        }

        fn calc_prgrom_addr(&self, address: u16) -> usize {
            if self.prgrom.len() == 0x80000 {
                unimplemented!();
            }

            let bank_mode = self.control.prg_rom_bank_mode;
            let bank_select = self.prg_bank.select;

            let addr = address - 0x8000;
            match (bank_mode, addr) {
                (PRGROMBankMode::Switch32K, _) => addr as usize + (bank_select & 0b1110) * 0x8000,
                (PRGROMBankMode::FixFirstSwitchLast16K, 0x0000..=0x3fff) => addr as usize,
                (PRGROMBankMode::FixFirstSwitchLast16K, 0x4000..=0x7fff) => {
                    addr as usize + bank_select * 0x4000 - 0x4000
                }
                (PRGROMBankMode::FixLastSwichFirst16K, 0x0000..=0x3fff) => {
                    addr as usize + bank_select * 0x4000
                }
                (PRGROMBankMode::FixLastSwichFirst16K, 0x4000..=0x7fff) => {
                    addr as usize + self.prgrom.len() - 0x4000
                }
                _ => unreachable!("{:x}", addr),
            }
        }
    }

    impl Mapper for MMC1 {
        fn read(&self, address: u16) -> u8 {
            self.write_last_cycle.set(false);

            self.prgrom[self.calc_prgrom_addr(address)]
        }

        fn write(&mut self, value: u8, address: u16) {
            if (value >> 7) == 1 && (address >> 15) == 1 {
                self.shift_register.reset();
            } else if !self.write_last_cycle.get() && (address >> 15) == 1 {
                if let Some(reg_val) = self.shift_register.shift(value & 0x01) {
                    match address {
                        0x8000..=0x9fff => self.control = Control::from(reg_val),
                        0xa000..=0xbfff => self.chr_bank0 = reg_val as usize,
                        0xc000..=0xdfff => self.chr_bank1 = reg_val as usize,
                        0xe000..=0xffff => self.prg_bank = PRGBank::from(reg_val),
                        _ => unreachable!("{:x}", address),
                    }
                }
            }
            self.write_last_cycle.set(true);

            let addr = self.calc_prgrom_addr(address);
            self.prgrom[addr] = value;
        }
    }

    struct ShiftRegister {
        reg: u8,
        shifts: usize,
    }

    impl ShiftRegister {
        fn shift(&mut self, bit: u8) -> Option<u8> {
            self.reg |= bit << self.shifts;
            self.shifts += 1;

            if self.shifts == 5 {
                let res = self.reg;
                self.reset();
                Some(res)
            } else {
                None
            }
        }

        fn reset(&mut self) {
            self.reg = 0;
            self.shifts = 0;
        }
    }

    #[derive(Copy, Clone)]
    struct Control {
        mirroring: Mirroring,
        prg_rom_bank_mode: PRGROMBankMode,
        chr_rom_bank_mode: CHRROMBankMode,
    }

    impl From<u8> for Control {
        fn from(value: u8) -> Self {
            let mirroring = match value & 0b00000011 {
                0 => Mirroring::OneScreenLowerBank,
                1 => Mirroring::OneScreenUpperBank,
                2 => Mirroring::Vertical,
                3 => Mirroring::Horizontal,
                n @ _ => unreachable!("{n}"),
            };

            let prg_rom_bank_mode = match (value >> 2) & 0b00000011 {
                0 | 1 => PRGROMBankMode::Switch32K,
                2 => PRGROMBankMode::FixFirstSwitchLast16K,
                3 => PRGROMBankMode::FixLastSwichFirst16K,
                n @ _ => unreachable!("{n}"),
            };

            let chr_rom_bank_mode = match (value >> 4) & 0b00000001 {
                0 => CHRROMBankMode::Switch8K,
                1 => CHRROMBankMode::SwitchTwo4K,
                n @ _ => unreachable!("{n}"),
            };

            Control {
                mirroring,
                prg_rom_bank_mode,
                chr_rom_bank_mode,
            }
        }
    }

    #[derive(Copy, Clone)]
    enum Mirroring {
        OneScreenLowerBank,
        OneScreenUpperBank,
        Vertical,
        Horizontal,
    }

    #[derive(Copy, Clone)]
    enum PRGROMBankMode {
        Switch32K,
        FixFirstSwitchLast16K,
        FixLastSwichFirst16K,
    }

    #[derive(Copy, Clone)]
    enum CHRROMBankMode {
        Switch8K,
        SwitchTwo4K,
    }

    #[derive(Copy, Clone)]
    struct PRGBank {
        select: usize,
        prg_ram_chip_enabled: bool,
    }

    impl From<u8> for PRGBank {
        fn from(value: u8) -> Self {
            PRGBank {
                select: (value & 0b00001111) as usize,
                prg_ram_chip_enabled: bit_to_bool((value >> 4) & 0x01),
            }
        }
    }
}
