use std::process::exit;

#[derive(Debug)]
pub struct INes {
    pub mapper: usize,
    pub prgrom_size: usize,
    pub chrrom_size: usize,
    // prgram_size: usize,
    // uses_chrram: bool,
    // trainer_present: bool,
    // battery_backed_prgram: bool,
    // nametable_arragement: NametableArragement,
    // playchoice10_present: bool,
    // vs_unisystem: bool,
    // tv_system: TVSystem,
}

impl INes {
    pub fn new(header: &[u8]) -> Self {
        if header[0..=3] != [0x4e, 0x45, 0x53, 0x1a] {
            exit(1);
        }

        let mapper = (header[6] >> 4 | header[7] & 0b1111000) as usize;
        let prgrom_size = header[4] as usize;
        let chrrom_size = header[5] as usize;

        INes {
            mapper,
            prgrom_size,
            chrrom_size,
        }
    }
}

// enum NametableArragement {
//     Vertical,
//     Horizontal,
// }

// enum TVSystem {
//     NTSC,
//     PAL,
// }
