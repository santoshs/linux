/*
 * Copyright (C) 2010 The Android Open Source Project
 * Copyright (C) 2012 Renesas Mobile Corporation
 * All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ext4_utils.h"
#include "sparse_format.h"
#include "sparse_crc32.h"
#include "simg2img.h"

#include "fb_flash.h"
#include "fb_common.h"
#include "string.h"

#define COPY_BUF_SIZE (1024*1024)

unsigned char *copybuf;
unsigned char *inbuff;
unsigned long long g_start_address;

#define SPARSE_HEADER_MAJOR_VER 1
#define SPARSE_HEADER_LEN       (sizeof(sparse_header_t))
#define CHUNK_HEADER_LEN (sizeof(chunk_header_t))

int process_raw_chunk(u32 blocks, u32 blk_sz, u32 *crc32)
{
	u64 len = (u64)blocks * blk_sz;
	int ret;
#if 0
	unsigned long chunk;

	while (len) {
		chunk = (len > COPY_BUF_SIZE) ? COPY_BUF_SIZE : len;
		// *crc32 = sparse_crc32(*crc32, inbuff, chunk);
		ret = fb_flash_write(inbuff, chunk, g_start_address);
		if(ret != FB_OK) {
			/* process error here */
		} else {
			inbuff += chunk;
			len -= chunk;
			g_start_address += chunk;
		}
	}
#endif
	ret = fb_flash_write(inbuff, len, g_start_address);
		if(ret != FB_OK) {
			/* process error here */
			return -1;
		} else {
			inbuff += len;
			g_start_address += len;
		}
	return blocks;
}

int process_skip_chunk(u32 blocks, u32 blk_sz)
{
	/* len needs to be 64 bits, as the sparse file specifies the skip amount
	 * as a 32 bit value of blocks.
	 */
	u64 len = (u64)blocks * blk_sz;
	g_start_address += len; /* Move the pointer to next chunk */
	return blocks;
}

int sparse(unsigned char *buff , unsigned long long start_address)
{
	unsigned int i;
	int ret = SPARSE_OK;
	u32 crc32 = 0;
	u32 total_blocks = 0;
	sparse_header_t* sparse_header;
	chunk_header_t* chunk_header;
	
	inbuff = buff;
	g_start_address = start_address;
	copybuf = (unsigned char*)SPARSE_TEMP_BUFF;
	
	/* Get sprase header */
	sparse_header = (sparse_header_t*)inbuff;
	inbuff += SPARSE_HEADER_LEN;
	/* Check magic number (0xED26FF3A)*/
	if (sparse_header->magic != SPARSE_HEADER_MAGIC) {
		/* Bad magic */
		ret = ERR_BAD_MAGIC;
		return ret;
	}
	/* Check major version */
	if (sparse_header->major_version != SPARSE_HEADER_MAJOR_VER) {
		/* Unknown major version number */
		ret = ERR_UNKNOWN_MAJOR_VERSION;
	}
	if (sparse_header->file_hdr_sz > SPARSE_HEADER_LEN) {
		/* Skip the remaining bytes in a header that is longer than
		 * we expected.
		 */
		inbuff += (sparse_header->file_hdr_sz - SPARSE_HEADER_LEN);
		
	}
	for (i=0; i<sparse_header->total_chunks; i++) {
		/* Read chunk header */
		chunk_header = (chunk_header_t*)inbuff;
		inbuff += CHUNK_HEADER_LEN;
		if (sparse_header->chunk_hdr_sz > CHUNK_HEADER_LEN) {
			/* Skip the remaining bytes in a header that is longer than
			 * we expected.
			 */
			inbuff += (sparse_header->chunk_hdr_sz - CHUNK_HEADER_LEN);
		}

		switch (chunk_header->chunk_type) {
		    case CHUNK_TYPE_RAW:	/* Raw data */
				if (chunk_header->total_sz != (sparse_header->chunk_hdr_sz +
					 (chunk_header->chunk_sz * sparse_header->blk_sz)) ) {
					 ret  = ERR_BOGUS_CHUNKS_SIZE;
					 return ret;
				}

				total_blocks += process_raw_chunk(chunk_header->chunk_sz, sparse_header->blk_sz, &crc32);
				break;   
		   case CHUNK_TYPE_DONT_CARE:
				if (chunk_header->total_sz != sparse_header->chunk_hdr_sz) {
					ret  = ERR_BOGUS_CHUNKS_SIZE;
					return ret;
				}
				total_blocks += process_skip_chunk(chunk_header->chunk_sz, sparse_header->blk_sz);
				break;
		    default:
				ret = ERR_CHUNKTYPE_UNKNOWN;
				return ret;
		}
	}
	/* If the last chunk was a skip, then the code just did a seek, but
	 * no write, and the file won't actually be the correct size.  This
	 * will make the file the correct size.  Make sure the offset is
	 * computed in 64 bits, and the function called can handle 64 bits.
	 */
	if (sparse_header->total_blks != total_blocks) {
		ret = SPARSE_NG;
	}
	return ret;
}

/*	int check_sparse_form(unsigned char *pbuff)
*		- pbuff: pointer to data tobe checked
*		- return	1 if sprase form
*					0 if not sparse form
*/
int check_sparse_form(unsigned char *pbuff)
{
	sparse_header_t* sparse_header;
	sparse_header = (sparse_header_t*)pbuff;
	if (sparse_header->magic != SPARSE_HEADER_MAGIC) {
		return NOT_SPARSE_FORM;	/* Not in sparse form data */
	} else {
		return IS_SPARSE_FORM;	/* Sparse form data */
	}
}
