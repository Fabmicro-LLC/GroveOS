/*
	GroveOS - a tiny single-threaded operating system for ARM Cortex-M4F based microcontrollers

	Written by Ruslan Zalata and Evgeny Korolenko
	
	Copyright (c) 2022, Fabmicro, LLC., Tyumen, Russia.
	All rights reserved.

	email: info@fabmicro.ru

	SPDX-License-Identifier: BSD-2-Clause

*/
	
#include "utils.h"

#pragma pack(1)
#include "elf.h"
#pragma pack()

/*
	Check ELF header for compatibility.

	Returns:
		0 - ELF file is compatibly and can be executed. Variables entry_point, data_section and data_section_size are filled respectively.
		or and error code
*/


int elf_check_file(uint8_t* file_data, uint32_t file_size,  void** entry_point, uint32_t* text_section, uint8_t** data_section, uint32_t* data_section_size)
{

	Elf32_Ehdr *elf_hdr = (Elf32_Ehdr*) file_data;
	int index;


	// Is this ELF ?
	if( *(uint32_t*)(elf_hdr->e_ident) != 0x464c457f) {
		print("elf_check_file() not ELF, magic: %02X %02X %02X %02X at %p\r\n",
			elf_hdr->e_ident[0],
			elf_hdr->e_ident[1],
			elf_hdr->e_ident[2],
			elf_hdr->e_ident[3],
			elf_hdr 
		);

		return	ELF_ERROR_NOT_ELF;
	}

	// Is this 32bit ELF ?
	if( elf_hdr->e_ident[EI_CLASS] != ELFCLASS32) {
		print("elf_check_file() not 32-bit ELF, class: %02X\r\n", elf_hdr->e_ident[4]);
		return	ELF_ERROR_NOT_32BIT;
	}

	// Is this ARM ?
	if( elf_hdr->e_machine != EM_ARM) {
		print("elf_check_file() not ARM, machine: %02X\r\n", elf_hdr->e_machine);
		return	ELF_ERROR_NOT_ARM;
	}
	
	// Is this executable ELF ?
	if( elf_hdr->e_type != ET_EXEC) {
		print("elf_check_file() not executable, type: %02X\r\n", elf_hdr->e_type);
		return	ELF_ERROR_NOT_EXEC;
	}


	// Check for program header existance
	Elf32_Phdr* prg_hdr = (Elf32_Phdr*)(file_data + elf_hdr->e_phoff);
	index = 0;
	while(prg_hdr->p_type != PT_LOAD && index < elf_hdr->e_phnum) {
		prg_hdr = (Elf32_Phdr*) (((uint32_t*)prg_hdr) + elf_hdr->e_phentsize);
		if((uint8_t*)prg_hdr > file_data + file_size) {
			print("elf_check_file() program not found!\r\n");
			return ELF_ERROR_TEXT_NOT_FOUND; 
		}
	}


	*entry_point = NULL;
	*data_section = NULL;
	*data_section_size = 0;

	// Check for data segment existance
	Elf32_Shdr* seg_hdr = (Elf32_Shdr*)(file_data + elf_hdr->e_shoff);
	index = 0;
	do {
/*
		print("elf_check_file(): Section: %d, type: %p, flags: %p, offset: %p\r\n", 
			index, 
			seg_hdr->sh_type, 
			seg_hdr->sh_flags, 
			seg_hdr->sh_offset
		);
*/

		if((seg_hdr->sh_type == SHT_PROGBITS) && (seg_hdr->sh_flags & SHF_EXECINSTR)) {
			*entry_point = (void*) (file_data + seg_hdr->sh_offset + elf_hdr->e_entry);
			*text_section = (uint32_t)file_data + seg_hdr->sh_offset;
//			print("elf_check_file() text segment found at offset %p\r\n", seg_hdr->sh_offset);
		}

		if((seg_hdr->sh_type == SHT_PROGBITS) && (seg_hdr->sh_flags & SHF_WRITE) && (seg_hdr->sh_flags & SHF_ALLOC)) {
//			print("elf_check_file() data segment found at offset %p\r\n", seg_hdr->sh_offset);
			*data_section = (uint8_t*) (file_data) + seg_hdr->sh_offset;
			*data_section_size = seg_hdr->sh_size;
		}

		seg_hdr = (Elf32_Shdr*) (((uint8_t*)seg_hdr) + elf_hdr->e_shentsize);
		index++;
	} while( (uint8_t*)seg_hdr < file_data + file_size && index < elf_hdr->e_shnum);


	if(*entry_point == NULL) {
		print("elf_check_file() No text segment found!\r\n");
		return ELF_ERROR_TEXT_NOT_FOUND;
	}

	print("elf_check_file() global entry: %p, global data: %p, data size: %d\r\n", *entry_point, *data_section, *data_section_size);

	return ELF_ERROR_OK;
} 



