// 
//  Copyright © Renesas Mobile Corporation 2010 . All rights reserved     
// 
//  This material, including documentation and any related
//  source code and information, is protected by copyright controlled
//  by Renesas. All rights are reserved. Copying, including reproducing,
//  storing, adapting, translating and modifying, including decompiling
//  or reverse engineering, any or all of this material requires the prior
//  written consent of Renesas. This material also contains confidential
//  information, which may not be disclosed to others without the prior
//  written consent of Renesas.
//

/// \file
/// \brief Secured data table of contents definition.
///  In public environment this is private interface
///  (not exported to consumer)

#if defined __SDTOC_DATA_H__
#error "Multiply included"
#endif
#define __SDTOC_DATA_H__

/// \brief Maximum number of SDTOC entries. Must match that of of producer.
#define SDTOC_MAX_ENTRIES        64 
/// \brief Object id denoting the end of SDTOC.
#define SDTOC_END_MARKER 0xFFFFFFFF

/// Secured Data Table of Contents (SDTOC) is an array of structures
/// which describe location of data structures
///
/// \li End of SECURED_DATA_TOC_ENTRY list is indicated by
/// \li \c object_id == SDTOC_END_MARKER (0xFFFFFFFFu)
/// \li Data pointed to by SECURED_DATA_TOC_ENTRY entry
/// \li starts at 32 bit boundary (which implies unnamed
/// \li padding between data elements).
/// \li SECURED_DATA_TOC_ENTRY entries are in no particular order.
/// \li Data elements pointed to by SECURED_DATA_TOC_ENTRY are not
/// \li in any particular order.
///
/// Structure size  16 (0x0010) bytes.
/// 
/// \brief Secured_Data Table of contents

typedef struct {
/*   0 (0x0000) */  uint32_t  object_id;
/*   4 (0x0004) */  uint32_t  start;
/*   8 (0x0008) */  uint32_t  length;
/*  12 (0x000C) */  uint32_t  spare;
} SECURED_DATA_TOC_ENTRY;

// EOF
