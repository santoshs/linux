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

#ifndef TOC_DATA_H
#define TOC_DATA_H

/// \brief Maximum number of DATA_TOC entries. Must match that of of producer.
#define TOC_MAX_ENTRIES        64 
/// \brief Object id denoting the end of DATA_TOC.
#define TOC_END_MARKER 0xFFFFFFFF

/// Secured Data Table of Contents (DATA_TOC) is an array of structures
/// which describe location of data structures
///
/// \li End of DATA_TOC_ENTRY list is indicated by
/// \li \c object_id == DATA_TOC_END_MARKER (0xFFFFFFFFu)
/// \li Data pointed to by DATA_TOC_ENTRY entry
/// \li starts at 32 bit boundary (which implies unnamed
/// \li padding between data elements).
/// \li DATA_TOC_ENTRY entries are in no particular order.
/// \li Data elements pointed to by DATA_TOC_ENTRY are not
/// \li in any particular order.
///
/// Structure size  16 (0x0010) bytes.
/// 
/// \brief Data Table of contents

typedef struct {
/*   0 (0x0000) */  uint32_t  object_id;
/*   4 (0x0004) */  uint32_t  start;
/*   8 (0x0008) */  uint32_t  length;
/*  12 (0x000C) */  uint32_t  spare;
} DATA_TOC_ENTRY;

// EOF
#endif /* TOC_DATA_H */
