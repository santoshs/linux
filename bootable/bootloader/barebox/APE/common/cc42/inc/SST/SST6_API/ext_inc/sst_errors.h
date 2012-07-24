/**************************************************************************
 *   Copyright 2009 © Discretix Technologies Ltd. This software is         *
 *   protected by copyright, international treaties and various patents.   *
 *   Any copy or reproduction of this Software as permitted below, must    *
 *   include this Copyright Notice as well as any other notices provided   *
 *   under such license.                                                   *
 *                                                                         *
 *   This program shall be governed by, and may be used and redistributed  *
 *   under the terms and conditions of the GNU Lesser General Public       *
 *   License, version 2.1, as published by the Free Software Foundation.   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY liability and WARRANTY; without even the implied      *
 *   warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.      *
 *   See the GNU General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this program; if not, please write to the          *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
 
 /*! \file 
**********************************************************************************	
* Title:						Discretix SST Errors API header file						 					
*																			
* Filename:					    sst_errors.h															
*																			
* Project, Target, subsystem:	SST 6.0, Stubs, API
* 
* Created:						06.06.2007														
*
* Modified:						07.06.2007										
*
* \Author						Ira Boguslavsky														
*																			
* \Remarks						
**********************************************************************************/


#ifndef _DX_SST_ERRORS_H_
    #define _DX_SST_ERRORS_H_
#ifdef __cplusplus
extern "C" {
#endif
    	
	/*----------- SST Error definitions----------------------------------*/
 
    /*! \brief SST API Return Codes base                         **/
    #define SST_RC_ERROR_BASE	                (0x0E000000UL)
    
    /*! \brief Operation completed successfully                  **/
    #define SST_RC_OK	                        (0x00000000UL)
    
    /*! \brief Operation failed                                  **/
    #define SST_RC_FAIL	                        (SST_RC_ERROR_BASE+0x2)
    
    /*! \brief Operation failed -   Memory allocation            **/
    #define SST_RC_ERROR_MEM_ALLOC              (SST_RC_ERROR_BASE+0x3)
    
    /*! \brief Operation failed -   SST_Init() was already called.**/
    #define SST_RC_ERROR_MODULE_INIT_FAIL       (SST_RC_ERROR_BASE+0x4)
    
    /*! \brief Operation failed -   Module termination failure   **/
    #define SST_RC_ERROR_MODULE_TERMINATE_FAIL  (SST_RC_ERROR_BASE+0x5)
    
    /*! \brief Operation failed	-   Handle not free              **/
    #define SST_RC_ERROR_HANDLE_NOT_FREE        (SST_RC_ERROR_BASE+0x6)
        
    /*! \brief Operation failed - input Error MainMemoryId equal SecondaryMemoryID */
    #define SST_RC_ERROR_MAINMEMID_EQUAL_SECMEMID	(SST_RC_ERROR_BASE+0x7)

    /*! \brief Operation failed -   Invalid transaction          **/
    #define SST_RC_ERROR_TXN_INVALID	        (SST_RC_ERROR_BASE+0x8)
    
    /*! \brief Operation failed - Null pointer received as input  **/
    #define SST_RC_ERROR_NULL_POINTER	        (SST_RC_ERROR_BASE+0x9)
    
    /*! \brief Operation failed - Wrong challenge size received  **/
    #define SST_RC_ERROR_CHALLENGE_SIZE	        (SST_RC_ERROR_BASE+0xA)
    
    /*! \brief Operation failed -   Handle mismatch, the given handle
		is either invalid or not of expected type  **/
    #define SST_RC_ERROR_HANDLE_MISMATCH	    (SST_RC_ERROR_BASE+0xB)
    
    /*! \brief Operation failed -   Handle not found             **/
    #define SST_RC_ERROR_HANDLE_NOT_FOUND	    (SST_RC_ERROR_BASE+0xC)
    
    /*! \brief Operation failed -   No free session available    **/
    #define SST_RC_ERROR_NO_FREE_SESSION	    (SST_RC_ERROR_BASE+0xD)
    
    /*! \brief Operation failed -   Invalid session              **/
    #define SST_RC_ERROR_SESSION_INVALID	    (SST_RC_ERROR_BASE+0xE)
    
    /*! \brief Operation failed -   Invalid challenge            **/
    #define SST_RC_ERROR_CHALLENGE_INVALID	    (SST_RC_ERROR_BASE+0xF)
    
    /*! \brief Operation failed -   Session is empty             **/
    #define SST_RC_ERROR_SESSION_EMPTY	        (SST_RC_ERROR_BASE+0x10)
    
    /*! \brief Operation failed -   Access denied                **/
    #define SST_RC_ERROR_ACCESS_DENIED	        (SST_RC_ERROR_BASE+0x11)
    
    /*! \brief Operation failed -   No one can access an object  **/
    #define SST_RC_ERROR_ZOMBIE	                (SST_RC_ERROR_BASE+0x12)
    
    /*! \brief Operation failed -   Erroneous data size          **/
    #define SST_RC_ERROR_DATA_SIZE	            (SST_RC_ERROR_BASE+0x13)
    
    /*! \brief Operation failed	-   Erroneous data offset        **/
    #define SST_RC_ERROR_DATA_OFFSET	        (SST_RC_ERROR_BASE+0x14)
    
    /*! \brief Operation failed	-   Semaphore timed out before the operation could be completed  **/
    #define SST_RC_ERROR_TIMEOUT	            (SST_RC_ERROR_BASE+0x15)
    
    /*! \brief Operation failed	-   General VOS error            **/
    #define SST_RC_ERROR_VOS	                (SST_RC_ERROR_BASE+0x16)
    
	/*! \brief Operation failed	-   Session is full		         **/
    #define SST_RC_ERROR_SESSION_FULL           (SST_RC_ERROR_BASE+0x17)
    
    /*! \brief Operation failed	-   No object of given type       **/
    #define SST_RC_ERROR_ITER_END_OF_TYPE	    (SST_RC_ERROR_BASE+0x18)
    
	/*! \brief Operation failed	-   The authenticator is already bound to the object **/
    #define SST_RC_ERROR_AUTH_ALREADY_BOUND	    (SST_RC_ERROR_BASE+0x19)

    /*! \brief Operation failed -   General VCRYS error          **/
    #define SST_RC_ERROR_VCRYS	               (SST_RC_ERROR_BASE+0x1A)

    /*! \brief Operation failed -   Insufficient workspace given by user **/
    #define SST_RC_ERROR_WORKSPACE             (SST_RC_ERROR_BASE+0x1B)

    /*! \brief Operation failed -   General VDB error            **/
    #define SST_RC_ERROR_VDB 	               (SST_RC_ERROR_BASE+0x1C)
	
    /*! \brief Operation failed -   No free challenge available  **/
    #define SST_RC_ERROR_NO_FREE_CHALLENGE     (SST_RC_ERROR_BASE+0x1D)

    /*! \brief Operation failed -   No free bind entry available for this object **/
    #define SST_RC_ERROR_MAX_BOUND_REACHED     (SST_RC_ERROR_BASE+0x1E)

    /*! \brief Operation failed -   The DB Integrity has been compromised**/
    #define SST_RC_ERROR_INTEGRITY_COMPROMIZED (SST_RC_ERROR_BASE+0x1F)

    /*! \brief Operation failed -   The SST is closed, call SST_Init()   **/
    #define SST_RC_ERROR_CLOSED                (SST_RC_ERROR_BASE+0x20)
    
	/*! \brief Operation failed -   This operation is not supported YET in the SST **/
	#define SST_RC_ERROR_FEATURE_NOT_SUPPORTED	(SST_RC_ERROR_BASE+0x21)

	/*! \brief Operation failed -   The (RSA) Key requested was not found**/	
	#define SST_RC_ERROR_KEY_NOT_FOUND			(SST_RC_ERROR_BASE+0x22)
   
    /*! \brief Operation failed -   User Authentication failed		     **/	
	#define SST_RC_ERROR_AUTH_FAILED			(SST_RC_ERROR_BASE+0x23)

    /*! \brief Operation failed	-   General Index lookup error            **/
    #define SST_RC_ERROR_IX	                    (SST_RC_ERROR_BASE+0x24)

    /*! \brief Operation failed	-   No more handles error                 **/
    #define SST_RC_ERROR_NO_MORE_HANDLES        (SST_RC_ERROR_BASE+0x25)

    /*! \brief Operation failed	-   Duplicate String                      **/
    #define SST_RC_ERROR_DUPLICATE_STRING       (SST_RC_ERROR_BASE+0x26)

    /*! \brief Operation failed	-   Handle not found                      **/
    #define SST_RC_ERROR_INDEX_HANDLE_NOT_FOUND (SST_RC_ERROR_BASE+0x27)

    /*! \brief Operation failed	-   Invalid parameter                     **/
    #define SST_RC_ERROR_INVALID_PARAM          (SST_RC_ERROR_BASE+0x28)
	
	/*! \brief Operation failed	-   Invalid parameter                     **/
	#define SST_RC_ERROR_ITER_INVALID          (SST_RC_ERROR_BASE+0x29)

	/*! \brief Operation failed -   unaligned workspace given by user	  **/
	#define SST_RC_ERROR_WORKSPACE_NOT_ALIGN	(SST_RC_ERROR_BASE+0x2A)

	/*! \brief Operation failed -   unaligned buffer given by user		  **/
	#define SST_RC_ERROR_BUFFER_NOT_ALIGN		(SST_RC_ERROR_BASE+0x2B)
	
	/*! \brief Fatal error occurred during SST operation                  **/
	#define SST_RC_ERROR_FATAL					(SST_RC_ERROR_BASE+0x2C)
	
    /*! \brief Operation failed -   Lookup handle not bound error	  **/
    #define SST_RC_ERROR_LOOKUP_HANDLE_NOT_BOUND    (SST_RC_ERROR_BASE+0x2F)

	/*! \brief Operation failed -   Transaction has not been committed  **/
	#define SST_RC_ERROR_TRANSACTION_NOT_ENDED		(SST_RC_ERROR_BASE+0x30)

    /*! \brief Operation failed -   Wrong memory ID used                **/
    #define SST_RC_ERROR_WRONG_MEMORY_ID            (SST_RC_ERROR_BASE+0x31)
            
    /*! \brief Operation failed -   The max main NVS size was exceeded  **/
    #define SST_RC_ERROR_NVS_MAIN_MAX_SIZE_EXCEEDED (SST_RC_ERROR_BASE+0x32)

    /*! \brief Operation failed -  The max transaction NVS size was exceeded **/
    #define SST_RC_ERROR_NVS_TXN_MAX_SIZE_EXCEEDED  (SST_RC_ERROR_BASE+0x33)

    /*! \brief Operation failed -   Error occurred in NVS write access  **/
    #define SST_RC_ERROR_NVS_WRITE                  (SST_RC_ERROR_BASE+0x34)

    /*! \brief Operation failed -   Error occurred in NVS read access   **/
    #define SST_RC_ERROR_NVS_READ                   (SST_RC_ERROR_BASE+0x35)

    /*! \brief Operation failed -   The requested max size is too big   **/
    #define SST_RC_ERROR_NVS_MAX_SIZE_TOO_BIG       (SST_RC_ERROR_BASE+0x36)

    /*! \brief Operation failed -   The requested max size is too small **/
    #define SST_RC_ERROR_NVS_MAX_SIZE_TOO_SMALL     (SST_RC_ERROR_BASE+0x37)

    /*! \brief Operation failed -   Error occurred in NVS access        **/
    #define SST_RC_ERROR_NVS_ACCESS                 (SST_RC_ERROR_BASE+0x38)

    /*! \brief Operation failed -   Partially completed operation       **/
    #define SST_RC_ERROR_PARTIALLY_COMPLETED    	(SST_RC_ERROR_BASE+0x39)

    /*! \brief Operation failed -   illegal mode regarding to permissions**/
    #define SST_RC_ERROR_MODE_ILLEGAL    	        (SST_RC_ERROR_BASE+0x3A)

    /*! \brief Operation failed -   mode out of range, not in enum boundaries**/
    #define SST_RC_ERROR_MODE_OUT_OF_RANGE    	    (SST_RC_ERROR_BASE+0x3B)
        
    /*! \brief Operation failed -   Module termination debug failure   **/
    #define SST_RC_ERROR_MODULE_TERMINATE_DBG_FAIL  (SST_RC_ERROR_BASE+0x3C)

    /*! \brief Operation failed -   authenticator type is wrong   **/
    #define SST_RC_ERROR_AUTH_WRONG_TYPE            (SST_RC_ERROR_BASE+0x3D)
    
    /*! \brief Operation failed -   already initialized   **/
    #define SST_RC_ERROR_ALREADY_INITIALIZED        (SST_RC_ERROR_BASE+0x3E)

    /*! \brief Operation failed -   the given key type is wrong or unsupported    **/
    #define SST_RC_ERROR_INVALID_KEY_TYPE           (SST_RC_ERROR_BASE+0x3F)

#ifdef __cplusplus
}
#endif       
#endif  /* _DX_SST_ERRORS_H_ */
