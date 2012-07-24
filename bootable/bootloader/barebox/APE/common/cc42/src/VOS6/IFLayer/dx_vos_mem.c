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

 #ifndef DX_NO_DEBUG_MEM
#ifndef DX_DEBUG_MEM
//#define DX_DEBUG_MEM
#endif
#endif
#ifndef DX_NO_DEBUG_PRINT
#ifndef DX_DEBUG_PRINT
//#define DX_DEBUG_PRINT
#endif
#endif
#define DX_DBG_MODULE DX_DBG_MODULE_VOS
#include "DX_VOS_Mem.h"
#include "DX_VOS_BaseMem.h"
/* #include "DX_VOS_DbgPrint.h"
#include "DX_VOS_DbgVosModulesCodes.h" */
#ifdef DX_DEBUG_STACK
#include "VOS_API/DX_VOS_ThreadStorage.h"
static DxVosThreadStorage g_DxStackStart;
static DxVosThreadStorage g_DxStackUsage;

void DX_VOS_StackStart(void* stackStart)
{
    DX_VOS_ThreadStoragePut(g_DxStackStart, stackStart);
}

void DX_VOS_StackEnd(void* stackEnd)
{
    DxInt32 usage = (DxUint32)DX_VOS_ThreadStorageGet(g_DxStackStart) - (DxUint32)stackEnd;
    if (usage > (DxInt32)DX_VOS_ThreadStorageGet(g_DxStackUsage))
        DX_VOS_ThreadStoragePut(g_DxStackUsage, (void*)usage);
}

DxUint32 DX_VOS_GetStackUsage()
{
    return (DxUint32)DX_VOS_ThreadStorageGet(g_DxStackUsage);
}
#endif
#ifdef DX_DEBUG_MEM
static DxUint32 DxMemoryUsage = 0;
DxUint32 DxCurrAllocNum = 0;

#define DX_MAX_ALLOCS_TO_FAIL   10
static DxUint32 DxAllocsToFails[DX_MAX_ALLOCS_TO_FAIL] = { 0 };

DxUint32 DX_VOS_GetMemUsage(void)
{
	DX_RETURN(DxMemoryUsage);
}

#define DX_MEMORY_PREFIX 0x12345678
#define DX_MEMORY_SUFFIX 0x87654321

DxStatus DX_VOS_MemFailAlloc(DxUint32 relativeAlloc)
{
    DxUint32 i = 0;
    for (i = 0; i < DX_ITEMS_IN_ARRAY(DxAllocsToFails); i++)
    {
        if (DxAllocsToFails[i] == 0)
        {
            DxAllocsToFails[i] = DxCurrAllocNum + relativeAlloc;
            DX_RETURN(DX_SUCCESS);
        }
    }
    DX_RETURN(DX_FAILURE);
}
#endif

void DX_VOS_DeallocMem (void** data)
{
    if (data != DX_NULL)
    {
        DX_VOS_MemFree(*data);
        *data = DX_NULL;
    }
}

#if defined(DX_DEBUG_MEM) && defined(DX_DEBUG_PRINT)

void* _DX_VOS_DebugMemMallocNoFail(const DxChar* fileName, DxUint32 lineNum,
                                            const DxChar* funcName, DxUint size)
{
    void* ptr = _DX_VOS_MemMalloc(size);
    if (ptr!= DX_NULL)
        DX_VOS_DebugPrint(DX_DBG_MEM_ALLOC, fileName, lineNum, funcName, DX_DBGPRINT_INFO_LEVEL,
        "Allocated %d bytes at 0x%08X(Usage: %d bytes, No. %d)", size, ptr, DxMemoryUsage, DxCurrAllocNum);
    DX_RETURN(ptr);
}

void* _DX_VOS_DebugMemMalloc(const DxChar* fileName, DxUint32 lineNum,
									  const DxChar* funcName, DxUint size)
{
    DxUint32 i = 0;
    DxCurrAllocNum++;
    if (DxCurrAllocNum != 0)
    {
        for (i = 0; i < DX_ITEMS_IN_ARRAY(DxAllocsToFails); i++)
        {
            if (DxAllocsToFails[i] == DxCurrAllocNum)
            {
                DxAllocsToFails[i] = 0;
                DX_VOS_DebugPrint(DX_DBG_MEM_ALLOC, fileName, lineNum, funcName, DX_DBGPRINT_INFO_LEVEL,
                    "Failing Allocation No. %d of %d bytes (Usage: %d bytes)", DxCurrAllocNum, size, DxMemoryUsage);
                DX_RETURN(DX_NULL);
            }
        }
    }
    return _DX_VOS_DebugMemMallocNoFail(fileName, lineNum, funcName, size);
}

void _DX_VOS_DebugMemFree(const DxChar* fileName, DxUint32 lineNum,
									  const DxChar* funcName, void* aBuff)
{
	DxUint32 size = 0;
	if (aBuff != DX_NULL)
	{
		DxUint32* ptr = (DxUint32*)aBuff - 2;
		if (ptr[1] == DX_MEMORY_PREFIX)
			size = ptr[0];
	}
	_DX_VOS_MemFree(aBuff);
	if (size > 0)
		DX_VOS_DebugPrint(DX_DBG_MEM_ALLOC, fileName, lineNum, funcName, DX_DBGPRINT_INFO_LEVEL,
			"Deallocated %d bytes at 0x%08X(Usage: %d bytes, No. %d)", size, aBuff, DxMemoryUsage, DxCurrAllocNum);

}

void* _DX_VOS_DebugMemRealloc(	const DxChar* fileName, DxUint32 lineNum,
										const DxChar* funcName, void* aBuff, DxUint size)
{
    DxUint32 i = 0;
    DxUint32 oldSize = 0;
    void* oldBuff = aBuff;
    DxCurrAllocNum++;
    if (DxCurrAllocNum != 0)
    {
        for (i = 0; i < DX_ITEMS_IN_ARRAY(DxAllocsToFails); i++)
        {
            if (DxAllocsToFails[i] == DxCurrAllocNum)
            {
                DxAllocsToFails[i] = 0;
                DX_VOS_DebugPrint(DX_DBG_MEM_ALLOC, fileName, lineNum, funcName, DX_DBGPRINT_INFO_LEVEL,
                    "Failing Reallocation No. %d of %d bytes (Usage: %d bytes)", DxCurrAllocNum, size, DxMemoryUsage);
                DX_RETURN(DX_NULL);
            }
        }
    }

    if (aBuff != DX_NULL)
	{
		DxUint32* ptr = (DxUint32*)aBuff - 2;
		if (ptr[1] == DX_MEMORY_PREFIX)
			oldSize = ptr[0];
	}
	aBuff = _DX_VOS_MemRealloc(aBuff, size);
	if (aBuff != DX_NULL)
	{
		if (oldSize > 0)
			DX_VOS_DebugPrint(DX_DBG_MEM_ALLOC, fileName, lineNum, funcName, DX_DBGPRINT_INFO_LEVEL,
				"Reallocated %d bytes to %d from 0x%08X to 0x%08X(Usage: %d bytes, No. %d)",
                oldSize, size, oldBuff, aBuff, DxMemoryUsage, DxCurrAllocNum);
		else
			DX_VOS_DebugPrint(DX_DBG_MEM_ALLOC, fileName, lineNum, funcName, DX_DBGPRINT_INFO_LEVEL,
				"Allocated %d bytes at 0x%08X(Usage: %d bytes, No. %d)", size, aBuff, DxMemoryUsage, DxCurrAllocNum);
	}
	DX_RETURN(aBuff);
}
#endif


void* _DX_VOS_MemMalloc(DxUint aSize)
{
	void* ptr = DX_NULL;
	if (aSize == 0)
	{
		/* DX_DBG_PRINT0(DX_DBGPRINT_WARNING_LEVEL,"Warning: Request for allocation of 0 bytes."); */
		DX_RETURN(DX_NULL);
	}

#ifdef DX_DEBUG_MEM
	{
		DxUint32 SizeInDWords = DX_DIVIDE_ROUND_UP(aSize, sizeof(DxUint32));
		ptr = DX_VOS_BaseMemMalloc((SizeInDWords + 3) * sizeof(DxUint32));
		if (ptr == DX_NULL)
		{
			DX_DBG_PRINT1(DX_DBGPRINT_ERROR_LEVEL,"Error: Allocation of %d bytes failed.", aSize);
			DX_RETURN(DX_NULL);
		}
		((DxUint32*)ptr)[0] = aSize;
		((DxUint32*)ptr)[1] = DX_MEMORY_PREFIX;
		((DxUint32*)ptr)[2 + SizeInDWords] = DX_MEMORY_SUFFIX;
		DxMemoryUsage += aSize;
		return ((DxUint32*)ptr) + 2;
	}
#else
	/* ptr =  DX_VOS_BaseMemMalloc(aSize); */ /* WIP_NoOS_R_Loader */
	if (ptr == DX_NULL) ;
		/* DX_DBG_PRINT1(DX_DBGPRINT_ERROR_LEVEL,"Error: Allocation of %d bytes failed.", aSize); */
	DX_RETURN(ptr);
#endif
}

void _DX_VOS_MemFree(void* aBuff)
{
	if (aBuff == DX_NULL)
		return;
#ifdef DX_DEBUG_MEM
	{
		DxUint32* ptr = (DxUint32*)aBuff - 2;
		DxUint32 OriginalSizeInDWords = DX_DIVIDE_ROUND_UP(ptr[0], sizeof(DxUint32));
		if (ptr[OriginalSizeInDWords + 2] != DX_MEMORY_SUFFIX)
			DX_DBG_PRINT1(DX_DBGPRINT_INFO_LEVEL, "Heap suffix is corrupt at address 0x%08x", ptr + ptr[0] + 2);
		if (ptr[1] != DX_MEMORY_PREFIX)
			DX_DBG_PRINT1(DX_DBGPRINT_INFO_LEVEL, "Heap prefix is corrupt at address 0x%08x", ptr + 1);
		else
			DxMemoryUsage -= ptr[0];

		DX_VOS_BaseMemFree(ptr);
	}
#else
	/* DX_VOS_BaseMemFree(aBuff); */ /* WIP_NoOS_R_Loader */
#endif
}

void* _DX_VOS_MemRealloc(void* aBuff, DxUint aSize)
{
	if (aSize == 0)
	{
		/* DX_DBG_PRINT0(DX_DBGPRINT_WARNING_LEVEL,"Warning: Request for reallocation of 0 bytes."); */
		DX_RETURN(DX_NULL);
	}

#ifdef DX_DEBUG_MEM
	{
		DxUint32* ptr = DX_NULL;
		DxUint32 SizeInDWords = DX_DIVIDE_ROUND_UP(aSize, sizeof(DxUint32));
		DxUint32 OriginalSizeInDWords = 0 ;
        DxUint32 OriginalSize = 0 ;
		if (aBuff != DX_NULL)
		{
			ptr = (DxUint32*)aBuff - 2;

			if (ptr[1] != DX_MEMORY_PREFIX)
				DX_DBG_PRINT1(DX_DBGPRINT_INFO_LEVEL, "Heap prefix is corrupt at address 0x%08x", ptr + 1);
            else {
                OriginalSize = ptr[0];
                OriginalSizeInDWords = DX_DIVIDE_ROUND_UP(OriginalSize, sizeof(DxUint32));
                if (ptr[OriginalSizeInDWords + 2] != DX_MEMORY_SUFFIX)
				DX_DBG_PRINT1(DX_DBGPRINT_INFO_LEVEL, "Heap suffix is corrupt at address 0x%08x", ptr + ptr[0] + 2);
            }
		}

		ptr = (DxUint32*)DX_VOS_BaseMemRealloc(ptr, (SizeInDWords + 3) * sizeof(DxUint32));
		if (ptr == DX_NULL)
		{
			DX_DBG_PRINT2(DX_DBGPRINT_ERROR_LEVEL,"Error: Reallocation of %d bytes failed (Usage: %d)", aSize, DxMemoryUsage);
			DX_RETURN(DX_NULL);
		}

        DxMemoryUsage -= OriginalSize;
		((DxUint32*)ptr)[0] = aSize;
		((DxUint32*)ptr)[1] = DX_MEMORY_PREFIX;
		((DxUint32*)ptr)[2 + SizeInDWords] = DX_MEMORY_SUFFIX;
		DxMemoryUsage += aSize;
		return ((DxUint32*)ptr) + 2;
	}
#else
	/* aBuff = DX_VOS_BaseMemRealloc(aBuff, aSize); */ /* WIP_NoOS_R_Loader */
	if (aBuff == DX_NULL) ;
		/* DX_DBG_PRINT1(DX_DBGPRINT_ERROR_LEVEL,"Error: Reallocation of %d bytes failed.", aSize); */
	DX_RETURN(aBuff);
#endif
}

DxStatus DX_VOS_MemCpy(void* aTarget,DxUint aTargetSize, const void* aSource, DxUint aSourceSize)
{
	// These conversions are needed for pointer arithmetic.
	DX_DECLARE(DxStatus, result, DX_SUCCESS);
	if (aSourceSize == 0)
		DX_RETURN(DX_SUCCESS);
	if (aSourceSize > aTargetSize)
	{
		result = DX_BUFFER_IS_NOT_BIG_ENOUGH;
		aSourceSize = aTargetSize;
	}

    DX_ASSERT_PARAM(aTarget != DX_NULL);
    DX_ASSERT_PARAM(aSource != DX_NULL);

    if (aTarget == aSource)
        DX_RETURN(DX_SUCCESS);

	DX_VOS_BaseMemCopy(aTarget, aSource, aSourceSize);
    if (result != DX_SUCCESS)
	RETURN_OLD_ERROR(result);
	DX_RETURN(DX_SUCCESS);
/*
	if ((DxUint)(Target - Source) < aSize)
	{
		for (DxUint i = 0; i < aSize; ++i)
			*Target++ = *Source++;
	}
	else if ((DxUint)(Source - Target) < aSize)
	{
		Target += aSize;
		aSource += aSize;
		while (aSize-- > 0)
			*--aTarget = *--aSource;
	}
	else
		memmove(aTarget, aSource, aSize);
	*/
}

void DX_VOS_FastMemCpy(void* aTarget, const void* aSource, DxUint aSourceSize)
{
    if (aSourceSize == 0)
        return;
	if (aTarget == DX_NULL || aSource == DX_NULL)
    {
		/* DX_DBG_PRINT0(DX_DBGPRINT_ERROR_LEVEL, "NULL pointer was transfered to DX_VOS_FastMemCpy"); */
        return;
    }

    if (aTarget == aSource)
        return;

	DX_VOS_BaseMemCopy(aTarget, aSource, aSourceSize);
}

DxStatus DX_VOS_MemCpyReverse(void* aTarget,DxUint aTargetSize, const void* aSource, DxUint aSourceSize)
{
    // These conversions are needed for pointer arithmetic.
    DxInt32 signedSize =0;
    DX_DECLARE(DxStatus, result, DX_SUCCESS);
    if (aSourceSize > aTargetSize)
    {
        result = DX_BUFFER_IS_NOT_BIG_ENOUGH;
        aSourceSize = aTargetSize;
    }
    if (aSourceSize == 0)
        DX_RETURN(DX_SUCCESS);
    signedSize  = aSourceSize;
    DX_VERIFY_PARAM(aTarget != DX_NULL);
    DX_VERIFY_PARAM(aSource != DX_NULL);
    DX_VERIFY_PARAM((((DxUint8*)aTarget - (DxUint8*)aSource) >= signedSize) ||
                       (((DxUint8*)aSource - (DxUint8*)aTarget) >= signedSize)||
                       (aSource == aTarget) );
    DX_VOS_FastMemCpyReverse(aTarget, aSource, aSourceSize);
	if (result != DX_SUCCESS)
		RETURN_OLD_ERROR(result);
	DX_RETURN(DX_SUCCESS);

}

void DX_VOS_FastMemCpyReverse(void* aTarget, const void* aSource, DxUint aSourceSize)
{
    DxUint i = 0;

    if ((aTarget == DX_NULL || aSource == DX_NULL) && aSourceSize != 0)
    {
        /* DX_DBG_PRINT0(DX_DBGPRINT_ERROR_LEVEL, "NULL pointer was transfered to DX_VOS_FastMemCpy"); */
        return;
    }
    // if there is no overlap copy normally
    if ((((DxUint8*)aTarget - (DxUint8*)aSource) >= (DxInt32)aSourceSize) || (((DxUint8*)aSource - (DxUint8*)aTarget) >= (DxInt32)aSourceSize))
    {
        for (; i < aSourceSize; ++i)
            ((DxUint8*)aTarget)[aSourceSize-i-1] = ((DxUint8*)aSource)[i];
    }
    else  // otherwise swap - (we assume it is the same buffer)
    {
        DxUint8 buffer = 0;
        for (; i < aSourceSize/2 ; ++i)
        {
            buffer = ((DxUint8*)aSource)[i];
            ((DxUint8*)aTarget)[i] = ((DxUint8*)aSource)[aSourceSize-i-1];
            ((DxUint8*)aTarget)[aSourceSize-i-1] = buffer;
        }
    }

}



void DX_VOS_MemSet6(void* aTarget, DxUint8 aChar, DxUint aSize)
{
	if (aSize == 0 || aTarget == DX_NULL)
		return;

	DX_VOS_BaseMemSet(aTarget, aChar, aSize);
}

void DX_VOS_MemSetZero6(void* aTarget, DxUint aSize)
{
    if (aSize == 0 || aTarget == DX_NULL)
        return;

	DX_VOS_BaseMemSetZero(aTarget, aSize);
}

DxInt DX_VOS_MemCmp(const void* aTarget, const void* aSource, DxUint aSize)
{
	if (aSize == 0)
		return 0;

	if (aTarget == DX_NULL)
	{
		if (aSource == DX_NULL)
			return 0;
		return -1;
	}
	if (aSource == DX_NULL)
		return 1;

	return DX_VOS_BaseMemCmp(aTarget, aSource, aSize);
}

DxByte* DX_VOS_MemChr(const DxByte* data, DxUint32 dataSize, DxByte chVal)
{
    DxUint32 i = 0;
    for (i = 0; i < dataSize; i++)
        if (data[i] == chVal)
            return (DxByte*)data + i;
    return DX_NULL;
}

/*DxStatus DX_VOS_MemCopy16bitTo8bit(DxInt8* aTarget, DxUint aTargetSize,
								   const DxInt16* aSource, DxUint aSourceSize)
{
	DX_DECLARE(DxStatus, result, DX_SUCCESS);
	if (aSourceSize > aTargetSize * sizeof(DxInt16))
	{
		result = DX_BUFFER_IS_NOT_BIG_ENOUGH;
		aSourceSize = aTargetSize * sizeof(DxInt16);
	}

	if (aSourceSize == 0)
		RETURN_OLD_ERROR(result);

    DX_ASSERT_PARAM(aTarget != DX_NULL);
    DX_ASSERT_PARAM(aSource != DX_NULL);

	aSourceSize = aSourceSize / sizeof(DxInt16);

	if ((void*)aTarget > (void*)aSource &&
		(void*)(aTarget + aSourceSize) < (void*)(aSource + aSourceSize))
		RETURN_NEW_ERROR(DX_BAD_ARGUMENTS);

	if ((void*)aTarget > (void*)aSource &&
		(void*)aTarget < (void*)(aSource + aSourceSize))
	{
		aTarget += aSourceSize;
		aSource += aSourceSize;
		while (aSourceSize-- > 0)
			*aTarget-- = (DxInt8)*aSource--;
	} else {
		while (aSourceSize-- > 0)
			*aTarget++ = (DxInt8)*aSource++;
	}
	if (result != DX_SUCCESS)
		RETURN_OLD_ERROR(result);
	DX_RETURN(DX_SUCCESS);
}*/

/*DxStatus DX_VOS_MemCopy8bitTo16bit(DxInt16* aTarget, DxUint aTargetSize,
								   const DxInt8* aSource, DxUint aSourceSize )
{
	DX_DECLARE(DxStatus, result, DX_SUCCESS);
	if (aSourceSize * sizeof(DxInt16) > aTargetSize)
	{
		result = DX_BUFFER_IS_NOT_BIG_ENOUGH;
		aSourceSize = aTargetSize / sizeof(DxInt16);
	}

	if (aSourceSize == 0)
		RETURN_OLD_ERROR(result);

    DX_ASSERT_PARAM(aTarget != DX_NULL);
    DX_ASSERT_PARAM(aSource != DX_NULL);

	if ((void*)aTarget < (void*)aSource &&
		(void*)(aTarget + aSourceSize) > (void*)(aSource + aSourceSize))
		RETURN_NEW_ERROR(DX_BAD_ARGUMENTS);

	if ((void*)aTarget >= (void*)aSource &&
		(void*)aTarget < (void*)(aSource + aSourceSize))
	{
		aTarget += aSourceSize;
		aSource += aSourceSize;
		while (aSourceSize-- > 0)
			*aTarget-- = *aSource--;
	} else {
		while (aSourceSize-- > 0)
			*aTarget++ = *aSource++;
	}
	if (result != DX_SUCCESS)
		RETURN_OLD_ERROR(result);
	DX_RETURN(DX_SUCCESS);
}*/

//#ifdef DX_USE_LEGACY_VOS
#undef DX_VOS_MemSet
DxStatus DX_VOS_MemSet(void* aTarget, DxUint8 aChar, DxUint aSize)
{
    if (aSize == 0)
        DX_RETURN(DX_SUCCESS);
    DX_ASSERT_PARAM(aTarget != DX_NULL);
    DX_VOS_BaseMemSet(aTarget, aChar, aSize);
    DX_RETURN(DX_SUCCESS);
}

#undef DX_VOS_MemSetZero

DxStatus DX_VOS_MemSetZero(void* aTarget, DxUint aSize)
{
    if (aSize == 0)
        DX_RETURN(DX_SUCCESS);
    DX_ASSERT_PARAM(aTarget != DX_NULL);
    DX_VOS_BaseMemSetZero(aTarget, aSize);
    DX_RETURN(DX_SUCCESS);
}

#undef DX_VOS_MemCopy
DxStatus DX_VOS_MemCopy(void* aTarget, const void* aSource, DxUint aSourceSize)
{
	DX_VOS_FastMemCpy(aTarget, aSource, aSourceSize);
	DX_RETURN(DX_SUCCESS);
}
//#endif
