/*//---------------------------------------------------------------------
//  Algorithmic Conjurings @ http://www.coyotegulch.com
//
//  Itzam - An Embedded Database Engine
//
//  itzam.h
//
//---------------------------------------------------------------------
//
//  Copyright 2004, 2005, 2006 Scott Robert Ladd
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the
//      Free Software Foundation, Inc.
//      59 Temple Place - Suite 330
//      Boston, MA 02111-1307, USA.
//
//-----------------------------------------------------------------------
//
//  For more information on this software package, please visit
//  Scott's web site, Coyote Gulch Productions, at:
//
//      http://www.coyotegulch.com
//
//-----------------------------------------------------------------------*/

#if !defined(LIBITZAM_ITZAM_H)
#define LIBITZAM_ITZAM_H

#if defined(__cplusplus)
extern "C" {
#endif

#if defined(_MSC_VER)
#pragma warning (disable : 4786 4244 4267 4101)
#endif

/*// figure out size of file references*/
#if !defined(ITZAM32) && !defined(ITZAM64) && !defined(JAVACOMPAT)
#if defined(__LP64__) || defined(__IA64__) || defined(_WIN64)
#define ITZAM64 1
#define ITZAM_MAX_FILE_SIZE 9223372036854775808
#else
#define ITZAM32 1
#define ITZAM_MAX_FILE_SIZE 2147483648
#endif
#endif

/********************************************************************************/
/* This must be the first file to be included due to specific definitions in use*/
#include "itzam_dx_convert.h"						
/********************************************************************************/
#include "sst_vcrys.h"
#include "NVS.h"
#define DLL_EXPORT
#ifdef DX_RENESAS_G3_V2
#include "DX_VOS_BaseTypes.h"
#include "DX_VOS_Mem.h"
DxUint32_t DX_SEP_WordsMemCopy(void * aDst, void *aSrc, DxUint32_t len);
#define DX_VOS_FastMemCpy		DX_SEP_WordsMemCopy
#endif
/*============================================================
 enumerated types*/
typedef enum
{
    ITZAM_ERROR_SIGNATURE,
    ITZAM_ERROR_VERSION,
    ITZAM_ERROR_64BIT_DB,
    ITZAM_ERROR_WRITE_FAILED,
    ITZAM_ERROR_OPEN_FAILED,
    ITZAM_ERROR_READ_FAILED,
    ITZAM_ERROR_CLOSE_FAILED,
    ITZAM_ERROR_SEEK_FAILED,
    ITZAM_ERROR_TELL_FAILED,
    ITZAM_ERROR_DUPE_REMOVE,
    ITZAM_ERROR_FLUSH_FAILED,
    ITZAM_ERROR_TOO_SMALL,
    ITZAM_ERROR_PAGE_NOT_FOUND,
    ITZAM_ERROR_LOST_KEY,
    ITZAM_ERROR_KEY_NOT_WRITTEN,
    ITZAM_ERROR_KEY_SEEK_FAILED,
    ITZAM_ERROR_KEY_REMOVE_FAILED,
    ITZAM_ERROR_REC_SEEK_FAILED,
    ITZAM_ERROR_REC_REMOVE_FAILED,
    ITZAM_ERROR_DELLIST_NOT_READ,
    ITZAM_ERROR_DELLIST_NOT_WRITTEN,
    ITZAM_ERROR_ITERATOR_COUNT,
    ITZAM_ERROR_REWRITE_DELETED,
    ITZAM_ERROR_INVALID_COL,
    ITZAM_ERROR_INVALID_ROW,
    ITZAM_ERROR_INVALID_HASH,
    ITZAM_ERROR_MALLOC,
    ITZAM_ERROR_READ_DELETED,
    ITZAM_ERROR_INVALID_RECORD,
    ITZAM_ERROR_INVALID_FILE_LOCK_MODE,
    ITZAM_ERROR_FILE_LOCK_FAILED,
    ITZAM_ERROR_UNLOCK_FAILED,
    ITZAM_ERROR_REC_SIZE,
    ITZAM_ERROR_TRANSACTION_OVERLAP,
    ITZAM_ERROR_NO_TRANSACTION,

	/*DX addition*/
	ITZAM_ERROR_INTEGRITY_COMPROMISED,

	ITZAM_ERROR_FORCE_INT32= 0x7FFFFFFF /* force enum to 32 bit in all compilers */

} itzam_error;

typedef enum
{
    ITZAM_OKAY,
    ITZAM_FAILED,
    ITZAM_VERSION_ERROR,
    ITZAM_DX_ITERATOR_AT_END,
    ITZAM_AT_BEGIN,
    ITZAM_NOT_FOUND,
    ITZAM_DUPLICATE,
    ITZAM_32BIT_OVERFLOW,
    ITZAM_DATA_WRITE_FAILED,
    ITZAM_REF_SIZE_MISMATCH,
	/*DX addition*/
	ITZAM_DX_ERROR_INTEGRITY,
	ITZAM_DX_ERROR_VOS,
	ITZAM_DX_ITERATOR_AT_BEGIN,
	ITZAM_DX_ERROR_INSUFFICIENT_SCRATCH_BUFFER,
	ITZAM_DX_ERROR_PAGE_ALLOCATE,
	ITZAM_DX_ERROR_DATA_OFFSET,
	ITZAM_DX_ERROR_MAC_STACK,
    ITZAM_DX_ERROR_WRONG_MEMORY_ID,
    ITZAM_DX_ERROR_NVS_TXN_MAX_SIZE_EXCEEDED,
    ITZAM_DX_ERROR_NVS_MAIN_MAX_SIZE_EXCEEDED,
    ITZAM_DX_ERROR_NVS_WRITE,
    ITZAM_DX_ERROR_NVS_READ,
    ITZAM_DX_ERROR_NVS_ACCESS,
    ITZAM_DX_ERROR_NVS_MAX_SIZE_TOO_BIG,
    ITZAM_DX_ERROR_NVS_MAX_SIZE_TOO_SMALL,
    ITZAM_DX_ERROR_READ_PASSED_USED_SIZE,
    ITZAM_DX_STATE_FORCE_INT32= 0x7FFFFFFF /* force enum to 32 bit in all compilers */

} itzam_state;

/*============================================================
 simple types*/
#if defined(ITZAM64) || defined(JAVACOMPAT)
#error size 64
typedef int64_t itzam_ref;
typedef int64_t itzam_int;
#else
typedef int32_t itzam_ref;
typedef int32_t itzam_int;
#endif

typedef struct  
{
	DxUint32_t	msbField;
	DxUint32_t	lsbField;

}itzam_record_handle;

#define SST_ITZAM_HANDLE_COPY(targetHandle,sourceHandle)  \
		targetHandle.msbField = sourceHandle.msbField; \
		targetHandle.lsbField = sourceHandle.lsbField

#define SST_ITZAM_HANDLE_INIT(handle) \
		handle.msbField = ITZAM_NULL_KEY;\
		handle.lsbField = ITZAM_NULL_KEY

#define SST_ITZAM_IS_NULL_HANDLE(handle) \
		((handle.msbField == ITZAM_NULL_KEY)&&(handle.lsbField == ITZAM_NULL_KEY))

#define SST_ITZAM_HANDLE_IS_EQUAL(handle1,handle2)	\
		((handle1.msbField == handle2.msbField) && (handle1.lsbField == handle2.lsbField))

/*============================================================
/ constants*/
#define ITZAM_NULL_REF (itzam_ref)(-1)
#define ITZAM_NULL_KEY (DxUint32_t)(0xFFFFFFFF)
#define ITZAM_M_USED_SIZE_INITIALIZER  (DxUint32_t)(0xFFFFFFFF)//(sizeof(itzam_datafile_header))
/*============================================================
// general function types*/
typedef void itzam_error_handler(itzam_error error);

/*
// functions of this type must return:
//      < 0    key1 is before key2
//        0    keys are equal
//      > 0    key1 is after key2*/
typedef int itzam_key_comparator(const itzam_record_handle *key1, const itzam_record_handle *key2);

/*// built-in key comparisons*/
int itzam_comparator_int32(const void * key1, const void * key2);
int itzam_comparator_uint32(const void * key1, const void * key2);
int itzam_comparator_int64(const void * key1, const void * key2);
int itzam_comparator_uint64(const void * key1, const void * key2);
int itzam_comparator_string(const void * key1, const void * key2);
int itzam_comparator_string_ignore_case(const void * key1, const void * key2);

/*// a callback function used to retrieve whatever data is associated with a reference*/
typedef BOOL itzam_export_callback(itzam_ref ref, void ** record, itzam_int * rec_len);

/*----------------------------------------------------------
// retrieve a string that describes an itzam state*/
const char * itzam_get_state_description(itzam_state state);

/*// set the default error handler*/
void itzam_set_default_error_handler(itzam_error_handler * error_handler);

/*// get the default error handler*/
itzam_error_handler * itzam_get_default_error_handler(void);

/*============================================================
// variable-length data file structures*/


// data file header, prefix for entire data file
#define  ITZAM_DATAFILE_SIGNATURE (uint32_t)0x4D5A5449 /*// ITZM*/
#define  ITZAM_RECORD_SIGNATURE   (uint32_t)0x525A5449 /*// ITZR*/


#if defined(ITZAM64)
#define ITZAM_DATAFILE_VERSION    (uint32_t)0x40040000 /*// 64.MM.MM.MM*/
#else
#define ITZAM_DATAFILE_VERSION    (uint32_t)0x20040000 /*// 32.MM.MM.MM*/
#endif

#define ITZAM_DELLIST_BLOCK_SIZE   (itzam_int)256

/*// flags and masks for record identification*/
#define ITZAM_RECORD_FLAGS_GENERAL  (int32_t)0x000000ff
#define ITZAM_RECORD_IN_USE         (int32_t)0x00000001
#define ITZAM_RECORD_DELLIST        (int32_t)0x00000002
#define ITZAM_RECORD_SCHEMA         (int32_t)0x00000004
#define ITZAM_RECORD_TRAN_HEADER    (int32_t)0x00000010
#define ITZAM_RECORD_TRAN_RECORD    (int32_t)0x00000020
#define ITZAM_RECORD_EMPTY		    (int32_t)0x00000040
#define ITZAM_RECORD_MODIFY	        (int32_t)0x00000080

#define ITZAM_RECORD_FLAGS_BTREE     (int32_t)0x00000f00
#define ITZAM_RECORD_BTREE_HEADER    (int32_t)0x00000100
#define ITZAM_RECORD_BTREE_PAGE      (int32_t)0x00000200
#define ITZAM_RECORD_BTREE_KEY       (int32_t)0x00000400

#define ITZAM_RECORD_FLAGS_MATRIX    (int32_t)0x0000f000
#define ITZAM_RECORD_MATRIX_HEADER   (int32_t)0x00001000
#define ITZAM_RECORD_MATRIX_TABLE    (int32_t)0x00002000

#define ITZAM_RECORD_FLAGS_HASH      (int32_t)0x000f0000
#define ITZAM_RECORD_HASH_HEADER     (int32_t)0x00010000
#define ITZAM_RECORD_HASH_TABLE      (int32_t)0x00020000
#define ITZAM_RECORD_HASH_KEY        (int32_t)0x00040000
#define ITZAM_RECORD_HASH_LIST_ENTRY (int32_t)0x00080000

/*// locking mechanism type*/
typedef enum
{
    ITZAM_FILE_LOCK_NONE,   /*// no locking at all*/
    ITZAM_FILE_LOCK_OS,     /*// use native Unix-style file locking*/
    ITZAM_FILE_LOCK_FILE,   /*// use a lock file*/

    ITZAM_FILE_LOCK_MODE_FORCE_INT32= 0x7FFFFFFF /* force enum to 32 bit in all compilers */

}itzam_file_lock_mode;

/*// record header, prefix for every record in a datafile*/
typedef struct
{
    uint32_t	m_signature; /*// four bytes that identify this as an itzam record*/
    uint32_t	m_flags;     /*// a set of ITZAM_RECORD_* bit settings*/
    DxUint32_t	m_length;    /*// record length*/
    DxUint32_t	m_rec_len;   /*// number of bytes in use by actual data*/
}itzam_record_header;

/*// structures for list of deleted records*/
typedef struct
{
    DxUint32_t   m_table_size; /*// number of entries in the table*/
}itzam_dellist_header;

typedef struct
{
    itzam_ref   m_where;    /*// reference for this deleted record*/
    itzam_int   m_length;   /*// length of the deleted record*/
}itzam_dellist_entry;

/*// transaction definitions*/
typedef enum
{
    ITZAM_TRAN_OP_WRITE,
    ITZAM_TRAN_OP_REMOVE,

    ITZAM_OP_TYPE_FORCE_INT32= 0x7FFFFFFF /* force enum to 32 bit in all compilers */

}itzam_op_type;

/* Operation type enumerator - used for sufficient space calculation */
typedef enum
{
    SST_ITZAM_INSERT,
    SST_ITZAM_DELETE,
    SST_ITZAM_MODIFY,
    SST_ITZAM_MODIFY_NO_LENGTH_CHANGE,

    ITZAM_DATA_OP_TYPE_FORCE_INT32= 0x7FFFFFFF /* force enum to 32 bit in all compilers */

}itzam_data_op_type;


/*// data file header*/
typedef struct
{
    uint32_t	m_signature;        /* file type signature*/
    uint32_t	m_version;          /* version of this file type*/
    itzam_ref	m_dellist_ref;      /* position of deleted list in file*/
    itzam_ref	m_schema_ref;       /* position of XML schema that describes this file*/
    itzam_ref	m_index_list_ref;   /* position of index reference list*/
    itzam_ref	m_transaction_tail; /* last item in the current transaction*/
	DxUint32_t	m_used_size;		/* the size of the datafile*/
    DxUint32_t  m_max_size;         /* the maximal datafile size in bytes*/
}itzam_datafile_header;

/*// transaction operation header*/
typedef struct
{
    itzam_ref             m_where;
    itzam_ref             m_prev_tran;
    itzam_record_header   m_record_header;
    itzam_ref             m_record_where;
    itzam_op_type         m_type;
}itzam_op_header;


/*// working storage for a loaded datafile*/
typedef struct t_itzam_datafile
{
    DxUint32_t                m_file;           /*// file associated with this datafile DX: memory id (NVS)*/
	struct t_itzam_datafile * m_tran_file;      /*// transaction file*/
    itzam_datafile_header     m_header;         /*// header information*/
    itzam_error_handler *     m_error_handler;  /*// function to handle errors that occur*/
    itzam_dellist_header      m_dellist_header; /*// header for current deleted record list*/
    itzam_dellist_entry *     m_dellist;        /*// pointer to list of deleted records*/
    BOOL                      m_is_open;        /*// is the file currently open?*/
    BOOL                      m_locked ;        /*// is the file currently locked?*/
    BOOL                      m_in_transaction; /*// are we journalizing a transaction?*/
    itzam_file_lock_mode      m_file_lock_mode; /*// file locking mode*/
    BOOL                      m_tran_replacing; /*// set when a write replaces a record during a write */
#ifndef SST_DX_ITZAM_CONVERT
	#ifndef _MSC_VER
	    struct flock              m_file_lock;      /*// fcntl lock*/
	#endif
#endif
}itzam_datafile;

/*----------------------------------------------------------
// prototypes for variable-length reference data file*/
itzam_datafile * itzam_datafile_alloc(void);

BOOL itzam_datafile_free(itzam_datafile * datafile);

itzam_state itzam_datafile_create(itzam_datafile * datafile,
                                  const char * filename,
                                  itzam_file_lock_mode lock_mode);

itzam_state itzam_datafile_open(itzam_datafile	*datafile,
                                const char		*filename,/*DX: not used in SST implementation*/
                                itzam_file_lock_mode lock_mode,
                                BOOL			read_only,
								BOOL			recover,
								DxBool_t		*powerDownRecover_ptr,/*did DB recover from power down*/
								DxByte_t		*scratchBuffer_ptr,
								DxUint32_t		scratchBufferSize);

itzam_state itzam_datafile_close(itzam_datafile * datafile);

BOOL itzam_datafile_lock(itzam_datafile * datafile, BOOL read_only);

BOOL itzam_datafile_unlock(itzam_datafile * datafile);

BOOL itzam_datafile_is_open(itzam_datafile * datafile);

int itzam_datafile_get_refbits(itzam_datafile * datafile);

/*
 * Removed by Raviv, Feb. 2008.
 * This function is not used in our code and it forces the compiler to 
 * add float support libraries to the code.
 *
 */

/*
float itzam_datafile_get_version(itzam_datafile * datafile);
*/

void itzam_datafile_set_error_handler(itzam_datafile * datafile,
                                      itzam_error_handler * error_handler);


/* 
*
* Note: The implementation of this function includes argument retState_ptr that can be missing 
*       in some calls of this function. Those calls are not in use now. If those calls will be
*	    in use in the future This issue will be repair. 
*
*/
itzam_ref itzam_datafile_get_next_open(itzam_datafile * datafile,
                                       itzam_int length,
                                       itzam_state*   retState_ptr);

/* 
*
* Note: The implementation of this function includes argument retState_ptr that can be missing 
*       in some calls of this function. Those calls are not in use now. If those calls will be
*	    in use in the future This issue will be repair. 
*
*/
itzam_ref itzam_datafile_write_flags(itzam_datafile * datafile,
                                     const void * data,
                                     itzam_int length,
                                     itzam_ref where,
									 DxUint32_t	offset,
									 int32_t flags,
									 DxByte_t		*scratchBuffer_ptr,
									 DxUint32_t		scratchBufferSize,
                                     itzam_state*   retState_ptr);

itzam_state itzam_datafile_read(itzam_datafile	*datafile,
                                void			*data,
								itzam_ref		where,
								DxUint32_t		offset,
								DxBool_t		skip_header,
                                DxUint32_t		*max_length);

itzam_state itzam_datafile_read_alloc(itzam_datafile * datafile,
                                      void ** data,
                                      itzam_int * length);

itzam_state itzam_datafile_remove(itzam_datafile	*datafile,
								  itzam_ref			where,
								  DxByte_t			*scratchBuffer_ptr,
								  DxUint32_t		scratchBufferSize);

itzam_state itzam_datafile_transaction_start(itzam_datafile * datafile);

itzam_state itzam_datafile_transaction_commit(itzam_datafile	*datafile);

itzam_state itzam_datafile_transaction_rollback(itzam_datafile	*datafile,
												DxByte_t		*scratchBuffer_ptr,
												DxUint32_t		scratchBufferSize);

/*============================================================
// B-tree types data structures*/

/*// a page in the B-tree*/
/*defines the user data of the record, includes the length of the record and the its MAC value*/
typedef struct  
{
	SSTVCRYSMACfield_t	m_MAC; /*MAC on data referenced in a page.*/
	DxUint32_t			m_Len; /*Length of the data referenced in a page.*/
	SSTVCRYSIvCounter_t m_IV;  /*IV of the record.*/

}itzam_record_user_data;

/*defines the record info (key+user data)*/
typedef struct  
{
	itzam_record_handle		m_keys;	/*the key of the record*/
	itzam_record_user_data fields;	/*user data of the record*/
	
}itzam_record_info;

/*defines the user data of the link (son page), includes the its MAC value*/
typedef struct  
{
	SSTVCRYSMACfield_t	m_MAC; /*MAC on data referenced in a page.*/

}itzam_link_user_data;

/*defines the link info (link ref + user data) */
typedef struct 
{
	itzam_ref			m_links	;		/*reference of the child page*/
	itzam_link_user_data	fields;	/*user data of the link (son)*/

}itzam_link_info;

#define itzamRecordUserDataInit(recUD)	DX_VOS_MemSet(&recUD,0,sizeof(itzam_record_user_data))
#define itzamLinkUserDataInit(linkUD)	DX_VOS_MemSet(&linkUD,0,sizeof(itzam_link_user_data))
#define itzamMACFieldInit(macField)		DX_VOS_MemSet(macField,0,sizeof(SSTVCRYSMACfield_t))
#define itzamIVFieldInit(ivField)		DX_VOS_MemSet(ivField,0,sizeof(SSTVCRYSIvCounter_t))

#define itzamRecordUserDataCopy(targetUD_ptr,sourceUD)								\
        DX_VOS_FastMemCpy(&targetUD_ptr,&sourceUD,sizeof(itzam_record_user_data))
#define itzamLinkUserDataCopy(targetUD_ptr,sourceUD)								\
        DX_VOS_FastMemCpy(&targetUD_ptr,&sourceUD,sizeof(itzam_link_user_data))
#define itzamMACFieldCopy(targetMAC,sourceMAC)										\
		/*lint -save -e545*/														\
		/* Disable PCLINT Warning 545: Suspicious use of & */						\
        DX_VOS_FastMemCpy(&targetMAC,&sourceMAC,sizeof(SSTVCRYSMACfield_t))			\
		/*lint -restore*/
#define itzamRecordUserInfoCopy(targetUI_ptr,sourceUI)								\
        DX_VOS_FastMemCpy(&targetUI_ptr,&sourceUI,sizeof(itzam_record_info))
#define itzamIVFieldCopy(targetMAC,sourceMAC)										\
		/*lint -save -e545*/														\
		/* Disable PCLINT Warning 545: Suspicious use of & */						\
		DX_VOS_FastMemCpy(&targetMAC,&sourceMAC,sizeof(SSTVCRYSIvCounter_t))			\
		/*lint -restore*/

/*// page header, prefix for pages*/
typedef struct
{
    itzam_ref				m_where;    /* // location in page file*/
    itzam_ref				m_parent;    /*// parent in page file*/
    uint16_t				m_key_count; /*// actual  # of keys*/
	itzam_link_user_data	m_userData;		/*MAC of the page.*/
}itzam_btree_page_header;


typedef struct itzam_btree_page_t
{
    /*// the actual data for this reference*/
    uint8_t * m_data;

    /*// header information*/
    itzam_btree_page_header * m_header;

	/*// these elements are pointers into m_data*/
	itzam_record_info	*recordUserData;	/*records data*/
	itzam_ref			*m_refs;			/* reference pointers*/
	itzam_link_info		*linkUserData;		/*links data*/
    
}itzam_btree_page;

/*! \brief define the data needed to perform a search operation, 
when the data was already read at some point **/
typedef struct
{
	itzam_btree_page * m_page;
	int                m_index;
	BOOL               m_found;
} search_result;

/*// B-tree constants*/
#define ITZAM_BTREE_VERSION        (uint32_t)0x00030100
#define ITZAM_BTREE_ORDER_MINIMUM  (uint16_t)4
#define ITZAM_BTREE_ORDER_DEFAULT  (uint16_t)8
#define ITZAM_BTREE_ORDER_HUGEFILE (uint16_t)20

#define SST_ITZAM_SPEICAL_DATA_MAX_SIZE (SST_VCRYS_NUMBER_OF_INTERNAL_KEYS * SST_VCRYS_INTERNAL_WRAPPED_KEY_SIZE_IN_BYTES)

typedef struct
{
    uint32_t			m_version;     /*// version of this file structure*/
    uint32_t			m_sizeof_page; /*// sizeof(itzam_btree_page) used in creating this btreefile*/
    uint16_t			m_order;       /*// number of keys per page*/
    itzam_int			m_count;       /*// counts number of active records*/
    itzam_int			m_ticker;      /*// counts the total number of new records added*/
    itzam_ref			m_where;       /*// pointer to location of this header*/
    itzam_ref			m_root_where;  /*// pointer to location of root reference in file*/
    itzam_ref			m_schema_ref;  /*// links to a copy of the XML schema for this table*/
	SSTVCRYSMACfield_t	m_MoM;			/*MAC of MACs (MoM) of the entire B-tree*/
	DxByte_t			m_sstData[SST_ITZAM_SPEICAL_DATA_MAX_SIZE];	/*field used by the SST to store special data */
	DxUint32_t			m_sstDataSize;
}itzam_btree_header;

/*// working storage for a loaded B-tree*/
typedef struct
{
    itzam_datafile *         m_datafile;       /*// file associated with this btree file*/
    itzam_btree_header       m_header;         /*// header data describing the tree structure*/
    BOOL                     m_free_datafile;  /*// free the datafile object when closed*/
    uint16_t                 m_links_size;     /*// number of links per page; order + 1; calculated at creation time*/
    uint16_t                 m_min_keys;       /*// minimum # of keys; order / 2; calculated at creation time*/
    itzam_key_comparator *   m_key_comparator; /*// function to compare keys*/
    itzam_ref                m_saved_header;
	
}itzam_btree;

/*----------------------------------------------------------
// prototypes for B-tree file*/
itzam_btree * itzam_btree_alloc(void);

BOOL itzam_btree_free(itzam_btree * btree);

/*DX note: order is determine in completion and is not passed from user */
itzam_state itzam_btree_create(itzam_btree * btree,
							   DxNvsMemoryId_t  mainMemoryId, /*DX: memory id instead of file name*/
							   DxNvsMemoryId_t  secondaryMemoryID,
                               itzam_file_lock_mode lock_mode,
                               uint16_t order,
                               itzam_key_comparator * key_comparator);

itzam_state itzam_btree_create_embedded(itzam_btree * btree,
                                        itzam_datafile * datafile,
                                        uint16_t order,
                                        itzam_key_comparator * key_comparator);

itzam_state itzam_btree_open(itzam_btree * btree,
							 DxNvsMemoryId_t  mainMemoryId, /*DX: memory id instead of file name*/
							 DxNvsMemoryId_t  secondaryMemoryID,
                             itzam_file_lock_mode lock_mode,
                             itzam_key_comparator * key_comparator,
                             BOOL			read_only,
							 BOOL			recover,				/*perform recover?*/
							 DxBool_t		*powerDownRecover_ptr,	/*did DB recoveredfrom power down?*/
							 DxByte_t		*scratchBuffer_ptr,
							 DxUint32_t		scratchBufferSize);

itzam_state itzam_btree_open_embedded(itzam_btree * btree,
                                      itzam_datafile * datafile,
                                      itzam_ref header_ref,
                                      itzam_key_comparator * key_comparator);

itzam_state itzam_btree_close(itzam_btree * btree);

BOOL itzam_btree_lock(itzam_btree * btree, BOOL read_only);

BOOL itzam_btree_unlock(itzam_btree * btree);

BOOL itzam_btree_is_open(itzam_btree * btree);

void itzam_btree_set_error_handler(itzam_btree * btree,
                                   itzam_error_handler * error_handler);

itzam_state itzam_btree_insert(itzam_btree * btree,
                               const void * key,
                               itzam_int key_len,
                               itzam_ref reference,
							   itzam_record_user_data recUserData);

itzam_state itzam_btree_insert_rec(	itzam_btree * btree,
									const void * key,
									itzam_int key_len,
									const void * record,
									itzam_ref rec_len,
									DxUint32_t flags,
									SSTVCRYSIvCounter_t ivCounter,/*DX ADD - IV of the record*/
									DxByte_t	*scratchBuffer_ptr,
									DxUint32_t	scratchBufferSize);

itzam_state itzam_btree_read_rec(itzam_btree *btree,
                                 const void  *key,
                                 void		 *record,
								 DxUint32_t  offset,/*DX add - offset in the record */
                                 DxUint32_t  *max_rec_len,
								 DxByte_t	 *scratchBuffer_ptr,/*DX add - integrity */
								 DxUint32_t	  scratchBufferSize);/*DX add - integrity */

itzam_ref itzam_btree_find(itzam_btree * btree,
                           const void * key,
						   DxBool_t *isIntegrityAssured);/*DX add - integrity error propagation*/

itzam_state itzam_btree_remove(itzam_btree	*btree,
                               const void	*key,
							   itzam_ref	*ref,
							   DxByte_t		*scratchBuffer_ptr,
							   DxUint32_t	scratchBufferSize);

itzam_state itzam_btree_optimize(itzam_btree * btree,
                                 const char * temp_name);

itzam_state itzam_btree_transaction_start(itzam_btree * btree);

itzam_state itzam_btree_transaction_commit(itzam_btree * btree,
										   DxBool_t		isReflashSensitive,
										   DxByte_t		*scratchBuffer_ptr,
										   DxUint32_t	scratchBufferSize);

itzam_state itzam_btree_transaction_rollback(itzam_btree *btree,
											 DxByte_t	*scratchBuffer_ptr,
											 DxUint32_t	scratchBufferSize);

/* This function changes the file signature so it will be recognized in the future as
an Itzam database file.
This function should be the last one called in the database creation sequence.*/
itzam_state itzam_btree_mark_db_ready(itzam_btree *btree);



#if defined(__cplusplus)
}
#endif

#endif
