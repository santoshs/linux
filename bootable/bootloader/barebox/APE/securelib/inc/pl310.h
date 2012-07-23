/* 
 *  Copyright © Renesas Mobile Corporation 2010 . All rights reserved     
 * 
 *  This material, including documentation and any related source code and information, is protected by 
 *  copyright controlled by Renesas. All rights are reserved. Copying, including reproducing, storing, adapting,
 *  translating and modifying, including decompiling or reverse engineering, any or all of this material
 *  requires the prior written consent of Renesas. This material also contains confidential information, which
 *  may not be disclosed to others without the prior written consent of Renesas.                                                              
 *
 * ***************************************************************************
 * Block name  : ARM PL310 Cache controller
 * Doc. number : DDI0246E
 * Doc. status : Revision r3p1
 *
 * Change history:
 *
 *   1  26-May-2010  Re-Created                              Ossi Lindvall
 *
 * ***************************************************************************
 */ 

#ifndef _PL310_H_
#define _PL310_H_

typedef struct {
  const    uint32_t CacheID;                          /* BASE + 0x000 Read-only */
  const    uint32_t CacheType;                        /* BASE + 0x004 Read-only */
  const    uint32_t reserved0[0x3E];

  volatile uint32_t Control;                          /* BASE + 0x100           */
  volatile uint32_t AuxiliaryControl;                 /* BASE + 0x104           */
  volatile uint32_t TagRAMLatency;                    /* BASE + 0x108           */
  volatile uint32_t DataRAMLatency;                   /* BASE + 0x10C           */

  const    uint32_t reserved1[0x3C];

  volatile uint32_t EventCounterControl;              /* BASE + 0x200           */
  volatile uint32_t EventCounter1Configuration;       /* BASE + 0x204           */
  volatile uint32_t EventCounter0Configuration;       /* BASE + 0x208           */
  volatile uint32_t EventCounter1Value;               /* BASE + 0x20C           */
  volatile uint32_t EventCounter0Value;               /* BASE + 0x210           */
  volatile uint32_t InterruptMask;                    /* BASE + 0x214           */
  const    uint32_t MaskedInterruptStatus;            /* BASE + 0x218           */
  const    uint32_t RawInterruptStatus;               /* BASE + 0x21C           */
  volatile uint32_t InterruptClear; /* W/O!!! */      /* BASE + 0x220           */
  const    uint32_t reserved2[0x143];

  volatile uint32_t CacheSync;                        /* BASE + 0x730           */
  const    uint32_t reserved3[0xF];

  volatile uint32_t InvalidateLineByPA;               /* BASE + 0x770           */
  const    uint32_t reserved4[0x2];

  volatile uint32_t InvalidateByWay;                  /* BASE + 0x77C           */
  const    uint32_t reserved5[0xc];

  volatile uint32_t CleanLineByPA;                    /* BASE + 0x7B0           */
  const    uint32_t reserved6[1];

  volatile uint32_t CleanLineByIndexWay;              /* BASE + 0x7B8           */
  volatile uint32_t CleanByWay;                       /* BASE + 0x7BC           */
  const    uint32_t reserved7[0xc];

  volatile uint32_t CleanAndInvalidateLineByPA;       /* BASE + 0x7F0           */
  const    uint32_t reserved8[1];

  volatile uint32_t CleanAndInvalidateLineByIndexWay; /* BASE + 0x7F8           */
  volatile uint32_t CleanAndInvalidateByWay;          /* BASE + 0x7FC           */
  const    uint32_t reserved9[0x40];

  volatile uint32_t LockdownByWayDSide;               /* BASE + 0x900           */
  volatile uint32_t LockdownByWayISide;               /* BASE + 0x904           */
  volatile uint32_t LockdownByWayDSide_Master_1;      /* BASE + 0x908           */
  volatile uint32_t LockdownByWayISide_Master_1;      /* BASE + 0x90C           */
  const    uint32_t reserveda[0x10];

  volatile uint32_t LockdownByLineEnable;             /* BASE + 0x950           */
  volatile uint32_t UnlockAllLinesByWay;              /* BASE + 0x954           */
  const    uint32_t reservedb[0xAA];

  volatile uint32_t AddressFilteringStart;            /* BASE + 0xC00           */
  volatile uint32_t AddressFilteringEnd;              /* BASE + 0xC04           */
  const    uint32_t reservedc[0xCE];

  volatile uint32_t DebugControl;                     /* BASE + 0xF40           */
  const    uint32_t reservedd[0x7];

  volatile uint32_t PrefetchControl;                  /* BASE + 0xF60           */
  const    uint32_t reservede[0x7];

  volatile uint32_t PowerControl;                     /* BASE + 0xF80           */
} PL310_STR;


/* ----------------------------------------------------------------------------
 *  CacheID                                                           Read-only
 * ----------------------------------------------------------------------------
 */

#define PL310_Implementer_V(x)           (((x) >> 24) & 0xFF)
#define PL310_CACHE_ID_V(x)              (((x) >> 10) & 0x3F)
#define PL310_PartNumber_V(x)            (((x) >>  6) & 0x0F)
#define PL310_Release_V(x)               (((x) >>  0) & 0x3F)

/* ----------------------------------------------------------------------------
 *  CacheType                                                         Read-only
 * ----------------------------------------------------------------------------
 */

#define PL310_DataBanking_V(x)           (((x) >> 31) & 0x001)
#define PL310_Cachetype_V(x)             (((x) >> 25) & 0x00F)
#define PL310_H_V(x)                     (((x) >> 24) & 0x001)
#define PL310_DCacheWaySize_V(x)         (((x) >> 20) & 0x7)
#define PL310_L2_DAssociativity_V(x)     (((x) >> 18) & 0x1)
#define PL310_L2_DCacheLineLength_V(x)   (((x) >> 12) & 0x3)
#define PL310_ICacheWaySize_V(x)         (((x) >>  8) & 0x7)
#define PL310_L2_IAssociativity_V(x)     (((x) >>  6) & 0x1)
#define PL310_L2_ICacheLineLength_V(x)   (((x) >>  0) & 0x3)

/* ----------------------------------------------------------------------------
 *  Control
 * ----------------------------------------------------------------------------
 */
                                                /*      0             1      */
#define PL310_UnifiedCacheEnable     0x00000001 /* bypass mode | enabled     */

/* ----------------------------------------------------------------------------
 *  AuxiliaryControl
 * ----------------------------------------------------------------------------
 */
                                                /*      0             1      */
#define PL310_EarlyBRESPEnable       0x40000000 /* Disabled    | Enabled     */
#define PL310_InstPrefetchEnable     0x20000000 /* Disabled    | Enabled     */
#define PL310_DataPrefetchEnable     0x10000000 /* Disabled    | Enabled     */
#define PL310_NonSecureIntAccCtrl    0x08000000 /* No NS Lock  | NS Lock     */
#define PL310_NonSecureLockdownEna   0x04000000 /* No NS Lock  | NS Lock     */
#define PL310_CacheReplacementPolicy 0x02000000 /* Pseudo-rand | Round-robin */
#define PL310_SharedAttrOverrideEna  0x00400000 /* noncacheab. | forwarded   */
#define PL310_ParityEnable           0x00200000 /* Disabled    | Enabled     */
#define PL310_EventMonitorBusEnable  0x00100000 /* Disabled    | Enabled     */
#define PL310_SharedAttrInvEnable    0x00002000 /* Disabled    | Enabled     */
#define PL310_ExclusiveOperation     0x00001000 /* Disabled    | Enabled     */
#define PL310_StrBuffDevLimitationEnable 0x8000
#define PL310_HighPrioSODevReadsEnable   0x4000
#define PL310_FullLineOfZeroEnable       0x0001

#define PL310_ForceWriteAllocate(x)     (((x) & 0x03) << 23)
#define PL310_WaySize(x)                (((x) & 0x07) << 17)
#define PL310_Associativity(x)          (((x) & 0x01) << 16)

#define PL310_ForceWriteAllocate_V(x)   (((x) >> 23) & 0x03)
#define PL310_WaySize_V(x)              (((x) >> 17) & 0x07)
#define PL310_Associativity_V(x)        (((x) >> 16) & 0x01)

#define PL310_UseAWCACHE                     0x00
#define PL310_ForceNoWriteAllocate           0x01
#define PL310_OverrideAWCACHE                0x02

#define PL310_WaySize_Func_16KB              0x01
#define PL310_WaySize_Func_32KB              0x02
#define PL310_WaySize_Func_64KB              0x03
#define PL310_WaySize_Func_128KB             0x04
#define PL310_WaySize_Func_256KB             0x05
#define PL310_WaySize_Func_512KB             0x06

#define PL310_Associativity_Func_8_way       0x00
#define PL310_Associativity_Func_16_way      0x01

/* ----------------------------------------------------------------------------
 *  TagRAMLatency
 *  DataRAMLatency
 * ----------------------------------------------------------------------------
 */

#define PL310_RAMWriteAccessLatency(x)      (((x) & 0x07) <<  8)
#define PL310_RAMReadAccessLatencycy(x)     (((x) & 0x07) <<  4)
#define PL310_RAMSetupLatency(x)            (((x) & 0x07) <<  0)

#define PL310_RAMWriteAccessLatency_V(x)    (((x) >>  8) & 0x07)
#define PL310_RAMReadAccessLatencycy_V(x)   (((x) >>  4) & 0x07)
#define PL310_RAMSetupLatency_V(x)          (((x) >>  0) & 0x07)

/* ----------------------------------------------------------------------------
 *  EventCounterControl
 * ----------------------------------------------------------------------------
 */

#define PL310_EventCounter1Reset            0x00000004
#define PL310_EventCounter0Reset            0x00000002
#define PL310_EventCounterEna               0x00000001

/* ----------------------------------------------------------------------------
 *  EventCounter1Configuration
 *  EventCounter0Configuration
 * ----------------------------------------------------------------------------
 */

#define PL310_EventCounterSource(x)         (((x) & 0x0F) << 2)
#define PL310_EventCounterIntGen(x)         (((x) & 0x03) << 0)

#define PL310_EventCounterSource_V(x)       (((x) >> 2) & 0x0F)
#define PL310_EventCounterIntGen_V(x)       (((x) >> 0) & 0x03)

#define PL310_EventCounterSource_Disabled   0x0
#define PL310_EventCounterSource_CO         0x1 /* Eviction of a line from the L2 cache.         */
#define PL310_EventCounterSource_DRHIT      0x2 /* Data read hit in the L2 cache.                */
#define PL310_EventCounterSource_DRREQ      0x3 /* Data read lookup to the L2 cache.             */
#define PL310_EventCounterSource_DWHIT      0x4 /* Data write hit in the L2 cache.               */
#define PL310_EventCounterSource_DWREQ      0x5 /* Data write lookup to the L2 cache.
                                                   Subsequently results in a hit or miss.        */
#define PL310_EventCounterSource_DWTREQ     0x6 /* Data write lookup to the L2 cache with
                                                   Write-Through attribute. Subsequently
                                                   results in a hit or miss.                     */
#define PL310_EventCounterSource_IRHIT      0x7 /* Instruction read hit in the L2 cache.         */
#define PL310_EventCounterSource_IRREQ      0x8 /* Instruction read lookup to the L2 cache.
                                                   Subsequently results in a hit or miss.        */
#define PL310_EventCounterSource_WA         0x9 /* Allocation into the L2 cache caused by a write,
                                                   with Write-Allocate attribute, miss.          */
#define PL310_EventCounterSource_IPFALLOC   0xA /* Allocation of a prefetch generated by L2C-310
                                                   into the L2 cache.                            */
#define PL310_EventCounterSource_EPFHIT     0xB /* Prefetch hint hits in the L2 cache.           */
#define PL310_EventCounterSource_EPFALLOC   0xC /* Prefetch hint allocated into the L2 cache.    */
#define PL310_EventCounterSource_SRRCVD     0xD /* Speculative read received by slave port.      */
#define PL310_EventCounterSource_SRCONF     0xE /* Speculative read confirmed in slave port.     */
#define PL310_EventCounterSource_EPFRCVD    0xF /* Prefetch hint received by slave port.         */

#define PL310_EventCounterInt_OverflowEna   0x02
#define PL310_EventCounterInt_IncEna        0x01
#define PL310_EventCounterInt_Dis           0x00

/* ----------------------------------------------------------------------------
 *  EventCounter1Value
 *  EventCounter0Value
 * ----------------------------------------------------------------------------
 */

/* ----------------------------------------------------------------------------
 *  InterruptMask
 *  MaskedInterruptStatus
 *  RawInterruptStatus
 *  InterruptClear
 * ----------------------------------------------------------------------------
 */
                                                /*      0             1      */
#define PL310_InterruptMask_DECERR       (1<<8)
#define PL310_InterruptMask_SLVERR       (1<<7)
#define PL310_InterruptMask_ERRRD        (1<<6)
#define PL310_InterruptMask_ERRRT        (1<<5)
#define PL310_InterruptMask_ERRWD        (1<<4)
#define PL310_InterruptMask_ERRWT        (1<<3)
#define PL310_InterruptMask_PARRD        (1<<2)
#define PL310_InterruptMask_PARRT        (1<<1)
#define PL310_InterruptMask_ECNTR        (1<<0)

/* ----------------------------------------------------------------------------
 *  CacheSync
 * ----------------------------------------------------------------------------
 */

/* ----------------------------------------------------------------------------
 *  InvalidateLineByPA
 *  CleanLineByPA
 *  CleanAndInvalidateLineByPA
 * ----------------------------------------------------------------------------
 */

/* ----------------------------------------------------------------------------
 *  InvalidateByWay
 *  CleanLineByIndexWay
 *  CleanAndInvalidateLineByIndexWay
 * ----------------------------------------------------------------------------
 */

#define PL310_IndexWay_Way(x)        (((x) & 0x0000F) << 28)
#define PL310_IndexWay_Index(x)      (((x) & 0x3FFFF) <<  5)

#define PL310_IndexWay_Way_V(x)      (((x) >> 28) & 0x0000F)
#define PL310_IndexWay_Index_V(x)    (((x) >>  5) & 0x3FFFF)
#define PL310_IndexWay_C_V(x)        (((x) >>  0) & 0x00001)

/* ----------------------------------------------------------------------------
 *  CleanByWay
 *  CleanAndInvalidateByWay
 * ----------------------------------------------------------------------------
 */

#define PL310_Way_WayBits(x)         (((x) & 0xFF) <<  0)

#define PL310_Way_WayBits_V(x)       (((x) >>  0) & 0xFF)

/* ----------------------------------------------------------------------------
 *  LockdownByWayDSide
 *  LockdownByWayISide
 * ----------------------------------------------------------------------------
 */

/* ----------------------------------------------------------------------------
 *  AddressFilteringStart
 *  AddressFilteringEnd
 * ----------------------------------------------------------------------------
 */

#define PL310_AddressFilteringStart(x)    (((x) & 0x0FFF) << 20)
#define PL310_AddressFilteringEnable(x)   (((x) & 0x0001) <<  0)

#define PL310_AddressFilteringStart_V(x)  (((x) >> 20) & 0x0FFF)
#define PL310_AddressFilteringEnable_V(x) (((x) >>  0) & 0x0001)

/* ----------------------------------------------------------------------------
 *  DebugControl
 * ----------------------------------------------------------------------------
 */
                                                /*      0             1      */
#define PL310_SPNIDEN                0x00000004 /* read-only                 */
#define PL310_DisableWriteBack       0x00000002 /* Enabled     | Force WT    */
#define PL310_DisableCacheLinefill   0x00000001 /* Enabled     | Disabled    */


/* ----------------------------------------------------------------------------
 *  PrefetchControl
 * ----------------------------------------------------------------------------
 */
                                                /*      0             1      */
#define PL310_DoubleLinefillEnable    (1 << 30)
#define PL310_Inst_Prefetch_Enable    (1 << 29)
#define PL310_Data_Prefetch_Enable    (1 << 28)
#define PL310_DblLineFWRAPReadDisable (1 << 27)
#define PL310_PrefetchDropEnable      (1 << 24)
#define PL310_IncrDblLinefillEnable   (1 << 23)
#define PL310_NotSameIDExclSeqEnable  (1 << 21)
#define PL310_PrefetchOffset(x)       (((x) & 0x1F) << 0)

#define PL310_PrefetchOffset_V(x)     (((x) >> 0) & 0x1F)

/* ----------------------------------------------------------------------------
 *  PowerControl
 * ----------------------------------------------------------------------------
 */
                                                /*      0             1      */
#define PL310_DynamicClkGating_en     (1 <<  1) /* Masked      | Enabled     */
#define PL310_StandbyMode_en          (1 <<  0) /* Masked      | Enabled     */

extern PL310_STR    pl310;

#endif /* _PL310_H_ */
