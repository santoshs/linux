/* simg2img.h
 *
 * Copyright (C) 2012 Renesas Mobile Corporation
 * All rights reserved.
 *
 */
 
#ifndef __SIMG2IMG_H__
#define __SIMG2IMG_H__

#define		SPARSE_OK					0

#define		IS_SPARSE_FORM				1
#define		NOT_SPARSE_FORM				0

#define		SPARSE_NG					-900
#define		ERR_BAD_MAGIC				-901
#define		ERR_UNKNOWN_MAJOR_VERSION	-902
#define		ERR_BOGUS_CHUNKS_SIZE		-903
#define		ERR_CHUNKTYPE_UNKNOWN		-904


int sparse(unsigned char *buff , unsigned long long start_address);
int check_sparse_form(unsigned char *pbuff);

#endif /* __SIMG2IMG_H__ */