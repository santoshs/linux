// SPDX-License-Identifier: GPL-2.0

//! Simple Character Utility for Loading Localities
//!
//! scull driver based on scull driver implementation in Linux Device Drivers
//! book.

#![no_std]
#![feature(allocator_api, global_asm)]

use alloc::boxed::Box;
use core::pin::Pin;
use kernel::prelude::*;
use kernel::{chrdev, cstr, file_operations::FileOperations};
use std_semaphore::Semaphore;

module! {
    type: RustScull,
    name: b"rust_scull",
    author: b"Rust for Linux Contributors",
    description: b"SCULL driver implementation",
    license: b"GPL V2",
    params: {
        scull_major: i32 {
            default: 0,
            permissions: 0o000,
            description: b"scull major number",
        },
        scull_minor: i32 {
            default: 0,
            permissions: 0o000,
            description: b"scull minor number",
        },
        scull_nr_devs: i32 {
            default: 4,
            permissions: 0o000,
            description: b"number of scull devices",
        },
        scull_quantum: i32 {
            default: 4000,
            permissions: 0o000,
            description: b"scull quantum size",
        },
        scull_qset: i32 {
            default: 1000,
            permissions: 0o000,
            description: b"scull quantum set",
        },
    },
}

#[derive(Default)]
struct RustScullFile;

impl FileOperations for RustScullFile {
    kernel::declare_file_operations!();
}

struct ScullQset<T> {
    data: Box<T>,
    next: *mut ScullQset,
}

struct ScullDev {
    data: Pin<Box<ScullQset>>,
    quantum: i32,
    qset: i32,
    size: usize,
    access_key: u32,
    sem: Semaphore,
}

struct RustScull {
    _dev: Pin<Box<chrdev::Registration<4>>>,
    scull_devices: Pin<Box<ScullDev>>,
}

impl KernelModule for RustScull {
    fn init() -> KernelResult<Self> {
        pr_info!("Initialising the rust scull driver\n");

        let mut chrdev_reg = chrdev::Registration::new_pinned(
            cstr!("rust_scull"),
            scull_minors.read(),
            &THIS_MODULE,
        )?;

        {
            let nr_devs = scull_nr_devs.read();

            for _ in 0..*nr_devs {
                chrdev_reg.as_mut().register::<RustScullFile>()?;
            }
        }

        Ok(RustScull { _dev: chrdev_reg })
    }
}

impl Drop for RustScull {
    fn drop(&mut self) {
        pr_info!("rust_scull: Cleaning up\n");
    }
}
