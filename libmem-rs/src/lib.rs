/* Disable warnings for libmem compatibility */
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

/* Note: the types and structures must be
 * the same size and aligned with their C variations */
const LM_PATH_MAX : usize = 512;

#[repr(C)]
pub struct lm_process_t {
    pid : u32,
    ppid : u32,
    bits : usize,
    // OBS: if lm_char_t is a wchar_t, these variables won't work. Use Multibyte
    path : [u8; LM_PATH_MAX],
    name : [u8; LM_PATH_MAX]
}

impl lm_process_t {
    pub fn get_pid(&self) -> u32 {
        self.pid
    }

    pub fn get_ppid(&self) -> u32 {
        self.ppid
    }

    pub fn get_bits(&self) -> usize {
        self.bits
    }

    pub fn get_path(&self) -> String {
        String::from_utf8_lossy(&self.path).to_string()
    }

    pub fn get_name(&self) -> String {
        String::from_utf8_lossy(&self.name).to_string()
    }
}

// Raw libmem calls
mod libmem_c {
    use crate::*;

    // link against 'mem' (the lib prefix is appended automatically)
    #[link(name = "mem")]
    extern "C" {
        pub(super) fn LM_GetProcess(pproc : *mut lm_process_t) -> i32;
    }
}

// Rustified libmem calls
pub fn LM_GetProcess() -> Option<lm_process_t> {
    let mut proc = lm_process_t { pid: 0, ppid: 0, bits: 0, path: [0;LM_PATH_MAX], name: [0;LM_PATH_MAX] };
    unsafe {
        if libmem_c::LM_GetProcess(&mut proc as *mut lm_process_t) != 0 {
            return Some(proc);
        }
    }

    return None;
}

