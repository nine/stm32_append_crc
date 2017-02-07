# stm32_append_crc

This program can be used to append a CRC32 checksum compatible STM32 ARM Cortex processors to binary images.

CRC32 Parameter:
- Polynomial: 0x04C11DB7
- Initial value: 0xFFFFFFFF


## Example Usage

### Create Binary with a Atollic TrueSTUDIO 

Add following commands under "C/C++ Build" -> "Settings" -> "Build Steps" -> "Post Build Steps"
<pre>
arm-atollic-eabi-objcopy -O binary ${BuildArtifactFileBaseName}.elf Application.bin;
stm32_append_crc.exe -p -s 256k -f Application.bin;
arm-atollic-eabi-objcopy -I binary -O ihex --change-addresses 0x08060000 Application.bin Application.hex
</pre>

### Test Integrity of Binary Data

<pre>

#define APPLICATION_FLASH_ADDR 0x08060000
#define APPLICATION_FLASH_LEN  0x00020000
#define CRC_OK 0


uint32_t flashApplicationCrcCheck() {
	uint32_t *flashPtr = (uint32_t *)APPLICATION_FLASH_ADDR;
	uint32_t crc = CRC_OK-1;

	RCC->AHB1ENR |= RCC_AHB1Periph_CRC; // enable clock for crc-module
	CRC->CR = CRC_CR_RESET;
	uint32_t idx = 0;
	for (idx = 0; idx < (APPLICATION_FLASH_LEN/4); idx++) {
		CRC->DR = flashPtr[idx];
	}
	crc = (CRC->DR);
	RCC->AHB1ENR &= ~RCC_AHB1Periph_CRC; // disable clock for crc-module

	return crc;
}
</pre>

Example Call:
<pre>
uint32_t crc32 = CRC_OK;
crc32 = flashApplicationCrcCheck();

if (crc32 == CRC_OK) {
	// do some stuff...
}
</pre>



## LICENSE

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program. If not, see [gpl](www.gnu.org/licenses/).

## Contributions

### stm32_crc.c
<pre>
crctab and the crc formula is taken from  Linux coreutils cksum.c and is 
Copyright (C) 1992, 1995-2006, 2008-2010 Free Software Foundation, Inc.
Written by Q. Frank Xia, qx@math.columbia.edu.
Cosmetic changes and reorganization by David MacKenzie, djm@gnu.ai.mit.edu.

How to calculate crc the way the ST cortex processors do was described by www.st.com MySTForum
post at 8/31/2009 9:11 PM  by brian.d.myers 

The rest of this source is Copyright (C) 2010 Odd Arild Olsen. 
</pre>

### cpp_option_parser
This program uses a modified version of [cpp_option_parser](https://github.com/nerzadler/cpp_option_parser) by [nerzadler](https://github.com/nerzadler).



## Contact

Please use the contact-form provided by github.
