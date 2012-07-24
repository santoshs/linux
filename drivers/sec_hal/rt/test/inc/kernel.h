/******************************************************************************
* NORTi4 Kernel Common Definitions/Declarations                               *
*                                                                             *
*  Copyright (c)  1995-2010, MiSPO Co., Ltd.                                  *
*  All rights reserved.                                                       *
*                                                                             *
* 2000-04-11                                                                  *
* 2000-08-14 for Kernel Ver. 1.20                                             *
* 2000-10-02 for Kernel Ver. 4.02.00                                          *
* 2000-10-28 for Kernel Ver. 4.02.00 corrected function number                *
* 2000-11-01 for Kernel Ver. 4.03.00                                          *
* 2001-01-25 for Kernel Ver. 4.03.00 added TA_FPU                          TA *
* 2001-02-28 for Kernel Ver. 4.03.00 supported XASSV                       TA *
* 2001-03-05 for Kernel Ver. 4.03.00 Supported Fujitsu FR30 Series         TA *
* 2001-05-12 for Kernel Ver. 4.03.00 modified INTHDR type for MCC68K       OK *
* 2001-12-13 for Kernel Ver. 4.05.00 Supported Mitsubishi M16C Series      TA *
* 2002-05-14 for Kernel Ver. 4.06.00 modified TSZ_MPL                      TA *
*                                    modified twai_flg parameter           TA *
* 2002-06-04                         modified ret_int, ent_int (ARM)       TA *
* 2002-10-03                         supported EWARM                       TA *
* 2003-06-24 for Kernel Ver. 4.07.00                                       TA *
* 2003-09-25 variable memory block size UINT->SIZE                         TA *
* 2004-01-06 for Kernel Ver. 4.09.00 modified INTHDR(V850)                 TA *
* 2004-09-26 for Kernel Ver. 4.09.00 added TA_VPU                          OK *
* 2004-09-28 Nios II supported                                             SK *
* 2004-07-20 crrected TSZ_MPF Macro                                        TA *
* 2004-10-27 Nios II avoid vdis_psw,vset_psw definition                    SK *
* 2005-02-17 name pointer type "const B" -> "const char"                   TA *
* 2005-04-20 Flag TA_GTHUMB for the GAIO Thumb state was added             TA *
* 2005-05-26 ena_vpu(), dis_vpu() system call were added                   TA *
* 2005-12-26 for ARM VIC                                                   TA *
* 2007-01-20 vena_fpu(), vdis_fpu() system call were added                 YO *
* 2007-09-10 modified TA_FPU                                               YO *
* 2008-05-30 Defined TASK and INTHDR macros for AVR32A                     HM *
* 2008-11-20 modified NOWFPU                                               YO *
* 2008-12-08 ista_alm, istp_alm were added                                    *
*            modified ret_int() dummy definition                           KM *
* 2009-07-21 Added INTHDR definition for ColdFire/CW                       JO *
* 2010-02-08 Added noinline macro to prevent inlining in interrupt handlers   *
*            Corrected blfsz of T_CMPF to blksz as in uITRON4.0 spec       KM *
* 2010-05-14 Added support for power saving mode (KERNEL_PS macro)         KM *
******************************************************************************/

#ifndef KERNEL_H
#define KERNEL_H
#ifdef __cplusplus
extern "C" {
#endif
#include "itron.h"

/************************************/
/* Default Constants                */
/************************************/

#ifndef MSGS
#define MSGS        16      /* default length of message (bytes count) */
#endif
#ifndef MSEC
#define MSEC        10      /* default interval time of system clock for V3 */
#endif
#define TIC_NUME    MSEC
#define TIC_DEMO    1

typedef struct v4_t_msg
{   struct v4_t_msg PFAR *next;
    VB msgcont[MSGS];
} V4_T_MSG;

typedef struct v4_t_msg_pri
{   struct v4_t_msg PFAR *next;  /* used by OS (pointer to next message) */
    PRI msgpri;             /* message priority */
    VB msgcont[MSGS];
} V4_T_MSG_PRI;

typedef struct v3_t_msg
{   struct v3_t_msg PFAR *next;  /* used by OS (pointer to next message) */
    PRI msgpri;             /* message priority */
    VB msgcont[MSGS];       /* message container */
} V3_T_MSG;

typedef struct v4_t_msghdr
{   V3_T_MSG PFAR *top;
    V3_T_MSG PFAR *end;
} V4_T_MSGHDR;

typedef struct v4_t_mem
{   struct v4_t_mem PFAR *next;
    unsigned size;
} V4_T_MEM;



#define T_MSG V4_T_MSG
#define T_MSG_PRI V4_T_MSG_PRI
#define TSZ_MPRIHD(e) ((e)*sizeof(V4_T_MSGHDR))

/* limit */

#define WUPCNT_MAX  255
#define SUSCNT_MAX  255
#define SEMCNT_MAX  65535
#define ACTCNT_MAX  255

#define TMAX_MAXSEM SEMCNT_MAX
#define TMAX_WUPCNT WUPCNT_MAX
#define TMAX_SUSCNT SUSCNT_MAX
#define TMAX_ACTCNT ACTCNT_MAX

/************************************/
/* Function Types                   */
/************************************/

/* task type */

#if defined(__TID__)                      /* ICC */
 #if defined(__ICCARM__) || defined(__ICCAVR32__)
  #define TASK    void
 #else
  #define TASK    C_task void
 #endif
#else
#define TASK    void FFAR
#endif

/* timer handler type */

#define TMRHDR  void FFAR

/* interrupt handler type */

#if (defined(__SH7000)||defined(__FR))  /* Green Hills */
 #define INTHDR  __interrupt void
#elif defined(__TID__)                    /* ICC */
 #if defined(__ICCAVR32__)
  #if (__CORE__ == __AVR32A__)
   #define INTHDR  __interrupt void
  #else
   #error Supported only an AVR32A core architecture
  #endif
 #else
  #define INTHDR  void
 #endif
#elif defined(SH7000)                     /* XASSV */
 #define INTHDR  __interrupt
#elif defined(__CPU_FR__)                 /* Softune */
 #define INTHDR __interrupt void
#elif ((defined(__CC_ARM) || defined(__ICCARM__)) && defined(VIC)) /* RVCT or EWARM VIC */
 #define INTHDR __irq void
#elif defined(__MWERKS__)                 /* Metrowerks CodeWarrior */
 #if defined(__COLDFIRE__)
  #define INTHDR  __declspec(interrupt) void FFAR
 #endif
#else
 #define INTHDR  void FFAR
#endif

/* noinline type for interrupt handler  */

#if defined(__GNUC__)
 #define noinline __attribute__((__noinline__))
#elif defined(__CC_ARM)         /* RVCT */
 #define noinline __declspec(noinline)
#else                           /* if noinline macro is not available, */
 #define noinline               /* disable inlining by compiler option */
#endif

/************************************/
/* Data Structure Packet Formats    */
/************************************/

/* cre_tsk */

typedef struct v3_t_ctsk
{   VP exinf;               /* extended information */
    ATR tskatr;             /* task attributes */
    FP task;                /* task start address */
    PRI itskpri;            /* initial task priority */
    int stksz;              /* stack size */
} V3_T_CTSK;

typedef struct v4_t_ctsk
{   ATR tskatr;             /* task attributes */
    VP_INT exinf;           /* extended information */
    FP task;                /* task start address */
    PRI itskpri;            /* initial task priority */
    SIZE stksz;             /* stack size */
    VP stk;                 /* stack top address */
    const char *name;       /* task name pointer */
} V4_T_CTSK;

/* task exception */

#define TBIT_TEXPTN (8*sizeof(TEXPTN))  /* tesk exception causal pattern bit width */

typedef struct v4_t_dtex
{   ATR texatr;             /* exception routine attributes */
    FP texrtn;              /* routine start address */
} V4_T_DTEX;

typedef struct v4_t_rtex
{   STAT texstat;           /* task exception routine state */
    TEXPTN pndptn;          /* causal pattern */
} V4_T_RTEX;

/* ref_tsk */

typedef struct v3_t_rtsk
{   VP exinf;               /* extended information */
    PRI tskpri;             /* current priority */
    UINT tskstat;           /* task state */
  #if (SIZEOF_INT == 1)
    UH tskwait;             /* cause of wait */
  #else
    UINT tskwait;           /* cause of wait */
  #endif
    ID wid;                 /* ID of object being waited for */
    INT wupcnt;             /* wakeup request count */
    INT suscnt;             /* SUSPEND request count */
    ATR tskatr;             /* task attributes */
    FP task;                /* task start address */
    PRI itskpri;            /* initial task priority */
    int stksz;              /* stack size */
} V3_T_RTSK;

typedef struct v4_t_rtsk
{   STAT tskstat;           /* task state */
    PRI tskpri;             /* current priority */
    PRI tskbpri;            /* base task priority */
  #if (SIZEOF_INT == 1)
    UH tskwait;             /* cause of wait */
  #else
    STAT tskwait;           /* cause of wait */
  #endif
    ID wid;                 /* ID of object being waited for */
    TMO lefttmo;            /* left time until timeout */
    UINT actcnt;            /* activate request count */
    UINT wupcnt;            /* wakeup request count */
    UINT suscnt;            /* SUSPEND request count */

    VP exinf;               /* extended information */
    ATR tskatr;             /* task attributes */
    FP task;                /* task start address */
    PRI itskpri;            /* initial task priority */
    SIZE stksz;             /* stack size */
} V4_T_RTSK;

typedef struct v4_t_rtst
{   STAT tskstat;           /* task state */
  #if (SIZEOF_INT == 1)
    UH tskwait;             /* cause of wait */
  #else
    STAT tskwait;           /* cause of wait */
  #endif
} V4_T_RTST;

/* cre_sem */

typedef struct v3_t_csem
{   VP exinf;               /* extended information */
    ATR sematr;             /* semaphore attributes */
    INT isemcnt;            /* initial semaphore count */
    INT maxsem;             /* maximum semaphore count */
} V3_T_CSEM;

typedef struct v4_t_csem
{   ATR sematr;             /* semaphore attributes */
    UINT isemcnt;           /* initial semaphore count */
    UINT maxsem;            /* maximum semaphore count */
    const char *name;       /* object name pointer */
} V4_T_CSEM;

/* ref_sem */

typedef struct v3_t_rsem
{   VP exinf;               /* extended information */
    BOOL_ID wtsk;           /* waiting task ID (FALSE:no task) */
    INT semcnt;             /* current semaphore count */
} V3_T_RSEM;

typedef struct v4_t_rsem
{   ID wtskid;              /* waiting task ID (TSK_NONE:no task) */
    UINT semcnt;             /* current semaphore count */
} V4_T_RSEM;

/* cre_flg */

typedef struct v3_t_cflg
{   VP exinf;               /* extended information */
    ATR flgatr;             /* eventflag attribute */
    UINT iflgptn;           /* initial eventflag */
} V3_T_CFLG;

typedef struct v4_t_cflg
{   ATR flgatr;             /* eventflag attribute */
    FLGPTN iflgptn;         /* initial eventflag */
    const char *name;       /* object name pointer */
} V4_T_CFLG;

/* ref_flg */

typedef struct v3_t_rflg
{   VP exinf;               /* extended information */
    BOOL_ID wtsk;           /* waiting task ID (FALSE:no task) */
    UINT flgptn;            /* eventflag bit pattern */
} V3_T_RFLG;

typedef struct v4_t_rflg
{   ID wtskid;              /* waiting task ID (TSK_NONE:no task) */
    FLGPTN flgptn;          /* eventflag bit pattern */
} V4_T_RFLG;

#define TBIT_FLGPTN (8*sizeof(FLGPTN))  /* eventflag pattern bit width */

/* cre_dtq */

typedef struct v4_t_cdtq
{   ATR dtqatr;             /* data queue attributes */
    UINT dtqcnt;            /* queue size, the number of data */
    VP dtq;                 /* data queue address */
    const char *name;       /* object name pointer */
} V4_T_CDTQ;

/* ref_dtq */

typedef struct v4_t_rdtq
{   ID stskid;              /* task ID waiting for data send */
    ID rtskid;              /* task ID waiting for data receive */
    UINT sdtqcnt;           /* the number of data in queue */
} V4_T_RDTQ;

/* cre_mbx */

#define TSZ_MBF(msgcnt, msgsz)  (SIZE)((((UINT)(msgsz))+sizeof(UH))*((UINT)(msgcnt)))

typedef struct v4_t_cmbx
{   ATR mbxatr;             /* mailbox attributes */
    PRI maxmpri;            /* maximum priority */
    VP mprihd;              /* start address of prioritied msg header */
    const char *name;       /* object name pointer */
} V4_T_CMBX;

typedef struct v3_t_cmbx
{   VP exinf;               /* extended information */
    ATR mbxatr;             /* mailbox attributes */
} V3_T_CMBX;

/* ref_mbx */

typedef struct v4_t_rmbx
{   ID wtskid;              /* waiting task ID (TSK_NONE:no task) */
    T_MSG PFAR *pk_msg;     /* message to be sent next */
} V4_T_RMBX;

typedef struct v3_t_rmbx
{   VP exinf;               /* extended information */
    BOOL_ID wtsk;           /* waiting task ID (FALSE:no task) */
    T_MSG PFAR *pk_msg;     /* message to be sent next */
} V3_T_RMBX;

/* cre_mtx */

typedef struct v4_t_cmtx
{   ATR mtxatr;             /* mutex attributes */
    PRI ceilpri;            /* ceiling priority */
    const char *name;       /* object name pointer */
} V4_T_CMTX;

/* ref_mtx */

typedef struct v4_t_rmtx
{   ID htskid;              /* locked task ID */
    ID wtskid;              /* waiting task ID */
} V4_T_RMTX;

/* cre_mbf */

typedef struct v4_t_cmbf
{   ATR mbfatr;             /* messagebuffer attributes */
    UINT maxmsz;            /* maximum size of messages */
    SIZE mbfsz;             /* messagebuffer size */
    VP mbf;                 /* address of buffer */
    const char *name;       /* object name pointer */
} V4_T_CMBF;

typedef struct v3_t_cmbf
{   VP exinf;               /* extended information */
    ATR mbfatr;             /* messagebuffer attributes */
    int bufsz;              /* messagebuffer size */
    int maxmsz;             /* maximum size of messages */
} V3_T_CMBF;

/* ref_mbf */

typedef struct v4_t_rmbf
{   ID stskid;              /* waiting sender task ID (FALSE:no task) */
    ID rtskid;              /* waiting receiver task ID (FALSE:no task) */
    UINT smsgcnt;           /* size of message to be sent next */
    SIZE fmbfsz;            /* size of free buffer */
} V4_T_RMBF;

typedef struct v3_t_rmbf
{   VP exinf;               /* extended information */
    BOOL_ID wtsk;           /* waiting receiver task ID (FALSE:no task) */
    BOOL_ID stsk;           /* waiting sender task ID (FALSE:no task) */
    int msgsz;              /* size of message to be sent next */
    int frbufsz;            /* size of free buffer */
} V3_T_RMBF;

/* cre_por */

typedef struct v4_t_cpor
{   ATR poratr;             /* port attributes */
    UINT maxcmsz;           /* maximum call message size */
    UINT maxrmsz;           /* maximum reply message size */
    const char *name;       /* object name pointer */
} V4_T_CPOR;

typedef struct v3_t_cpor
{   VP exinf;               /* extended information */
    ATR poratr;             /* port attributes */
    int maxcmsz;            /* maximum call message size */
    int maxrmsz;            /* maximum reply message size */
} V3_T_CPOR;

/* ref_por */

typedef struct v4_t_rpor
{   ID ctskid;             /* waiting caller task ID (FALSE:no task) */
    ID atskid;             /* waiting accepter task ID (FALSE:no task) */
} V4_T_RPOR;

typedef struct v3_t_rpor
{   VP exinf;               /* extended information */
    BOOL_ID wtsk;           /* waiting caller task ID (FALSE:no task) */
    BOOL_ID atsk;           /* waiting accepter task ID (FALSE:no task) */
} V3_T_RPOR;

#define TBIT_RDVPTN (8*sizeof(RDVPTN))  /* rendezvous pattern pattern bit width */

/* ref_rdv */

typedef struct v4_t_rrdv
{   ID wtskid;
} V4_T_RRDV;

/* def_inh */

typedef struct v4_t_dinh
{   ATR inhatr;             /* interrupt handler attributes */
    FP inthdr;              /* interrupt handler address */
    UINT imask;             /* interrupt mask, interrupt priority level */
} V4_T_DINH;

/* cre_isr */

typedef struct v4_t_cisr
{   ATR istatr;             /* interrupt service routine attributes */
    VP_INT exinf;           /* extended information for isr */
    INTNO intno;            /* interrupt number */
    FP isr;                 /* isr address */
    UINT imask;             /* interrupt mask, interrupt priority level */
} V4_T_CISR;

/* ref_isr */

typedef struct v4_t_risr
{
    INTNO intno;            /* interrupt number */
    UINT imask;             /* interrupt mask, interrupt priority level */
} V4_T_RISR;

/* cre_mpl */

typedef struct v4_t_cmpl
{   ATR mplatr;             /* memorypool attributes */
    SIZE mplsz;             /* memorypool size */
    VP mpl;                 /* start address of memory pool */
    const char *name;       /* object name pointer */
} V4_T_CMPL;

typedef struct v3_t_cmpl
{   VP exinf;               /* extended information */
    ATR mplatr;             /* memorypool attributes */
    SIZE mplsz;             /* memorypool size */
} V3_T_CMPL;

/* ref_mpl */

typedef struct v4_t_rmpl
{   ID wtskid;              /* waiting task ID (FALSE:no task) */
    SIZE fmplsz;            /* total size of free memory */
    UINT fblksz;            /* size of largest contiguous memory */
} V4_T_RMPL;

typedef struct v3_t_rmpl
{   VP exinf;               /* extended information */
    BOOL_ID wtsk;           /* waiting task ID (FALSE:no task) */
    SIZE frsz;              /* total size of free memory */
    SIZE maxsz;             /* size of largest contiguous memory */
} V3_T_RMPL;

/* cre_mpf */

typedef struct v4_t_cmpf
{   ATR mpfatr;             /* memorypool attributes */
    UINT blkcnt;            /* block count for entire memorypool */
    UINT blfsz;             /* fixed-size memory block size */
    VP mpf;                 /* start address of memory pool */
    const char *name;       /* object name pointer */
} V4_T_CMPF;

typedef struct v4_t_cmpf2
{   ATR mpfatr;             /* memorypool attributes */
    UINT blkcnt;            /* block count for entire memorypool */
    UINT blksz;             /* fixed-size memory block size */
    VP mpf;                 /* start address of memory pool */
    const char *name;       /* object name pointer */
} V4_T_CMPF2;

typedef struct v3_t_cmpf
{   VP exinf;               /* extended information */
    ATR mpfatr;             /* memorypool attributes */
    int mpfcnt;             /* block count for entire memorypool */
    int blfsz;              /* fixed-size memory block size */
} V3_T_CMPF;

/* ref_mpf */

typedef struct v4_t_rmpf
{   ID wtskid;              /* waiting task ID (TSK_NONE:no task) */
    UINT fblkcnt;           /* free block count */
} V4_T_RMPF;

typedef struct v3_t_rmpf
{   VP exinf;               /* extended information */
    BOOL_ID wtsk;           /* waiting task ID (FALSE:no task) */
    int frbcnt;             /* free block count */
} V3_T_RMPF;

/* cre_cyc, def_cyc */

typedef struct v4_t_ccyc
{   ATR cycatr;             /* cyclic handler attributes */
    VP_INT exinf;               /* extended information */
    FP cychdr;              /* cyclic handler address */
    RELTIM cyctim;            /* cyclic handler activation interval */
    RELTIM cycphs;         /* cyclic phase */
} V4_T_CCYC;

typedef struct v3_t_dcyc
{   VP exinf;               /* extended information */
    ATR cycatr;             /* cyclic handler attributes */
    FP cychdr;              /* cyclic handler address */
    UINT cycact;            /* cyclic handler activation */
    CYCTIME cyctim;         /* cyclic startup period */
} V3_T_DCYC;

/* ref_cyc */

typedef struct v4_t_rcyc
{   STAT cycstat;           /* status of cyclic handler */
    RELTIM lefttim;         /* time left before next handler startup */
} V4_T_RCYC;

typedef struct v3_t_rcyc
{   VP exinf;               /* extended information */
    CYCTIME lfttim;         /* time left before next handler startup */
    UINT cycact;            /* cyclic handler activation */
} V3_T_RCYC;

/* cre_alm, def_alm */

typedef struct v4_t_calm
{   ATR almatr;             /* alarm handler attributes */
    VP_INT exinf;               /* extended information */
    FP almhdr;              /* alarm handler address */
} V4_T_CALM;

typedef struct v3_t_dalm
{   VP exinf;               /* extended information */
    ATR almatr;             /* alarm handler attributes */
    FP almhdr;              /* alarm handler address */
    UINT tmmode;            /* start time specification mode */
    ALMTIME almtim;         /* handler startup time */
} V3_T_DALM;

/* ref_alm */

typedef struct v4_t_ralm
{   STAT almstat;               /* status */
    RELTIM lefttim;         /* time left before next handler startup */
} V4_T_RALM;

typedef struct v3_t_ralm
{   VP exinf;               /* extended information */
    ALMTIME lfttim;         /* time left before next handler startup */
} V3_T_RALM;

/* def_ovr */

typedef struct v4_t_dovr
{   ATR ovratr;             /* overrun handler attributes */
    FP ovrhdr;              /* overrun handler entry address */
    INTNO intno;            /* interrupt number */
    FP ovrclr;              /* interrupt request clear function address */
    UINT imask;             /* interrupt mask, interrupt priority level */
} V4_T_DOVR;

/* ref_ovr */

typedef struct v4_t_rovr
{   STAT ovrstat;           /* overrun handler status */
    OVRTIM leftotm;         /* remained task running time */
} V4_T_ROVR;

/* get_ver, ref_ver */

typedef struct v3_t_ver
{   UH maker;               /* vendor */
    UH id;                  /* format number */
    UH spver;               /* specification version */
    UH prver;               /* product version */
    UH prno[4];             /* product control information */
    UH cpu;                 /* CPU information */
    UH var;                 /* variation descriptor */
} V3_T_VER;

typedef struct v4_t_rver
{   UH maker;               /* vendor */
    UH prid;                  /* format number */
    UH spver;               /* specification version */
    UH prver;               /* product version */
    UH prno[4];             /* product control information */
} V4_T_RVER;


/* ref_sys */

typedef struct v4_t_rsys
{   INT sysstat;            /* system state */
} V4_T_RSYS;

/* ref_cfg */

typedef struct v4_t_rcfg
{   ID tskid_max;
    ID semid_max;
    ID flgid_max;
    ID mbxid_max;
    ID mbfid_max;
    ID porid_max;
    ID mplid_max;
    ID mpfid_max;
    ID cycno_max;
    ID almno_max;
    PRI tpri_max;
    int tmrqsz;
    int cycqsz;
    int almqsz;
    int istksz;
    int tstksz;
    SIZE sysmsz;
    SIZE mplmsz;
    SIZE stkmsz;
    ID dtqid_max;
    ID mtxid_max;
    ID isrid_max;
    ID svcfn_max;
} V4_T_RCFG;

typedef struct v3_t_rcfg
{   ID tskid_max;
    ID semid_max;
    ID flgid_max;
    ID mbxid_max;
    ID mbfid_max;
    ID porid_max;
    ID mplid_max;
    ID mpfid_max;
    HNO cycno_max;
    HNO almno_max;
    PRI tpri_max;
    int tmrqsz;
    int cycqsz;
    int almqsz;
    int istksz;
    int tstksz;
    SIZE sysmsz;
    SIZE mplmsz;
    SIZE stkmsz;
} V3_T_RCFG;

/* def_svc */

typedef struct v4_t_dsvc
{   ATR svcatr;             /* extended SVC attributes */
    FP svcrtn;              /* extended SVC address */
    INT parn;               /* # of SVC parameters  */
} V4_T_DSVC;

/************************************/
/* Common Constants                 */
/************************************/

/* overall */

#ifndef TRUE
#define TRUE        1       /* true */
#endif
#ifndef FALSE
#define FALSE       0       /* false */
#endif

/* tskatr, inhatr, cycatr, almatr, svcatr, excatr */

#define TA_HLNG     0x0000  /* high-level language (C language only) */
#define TA_ACT      0x0002  /* activate after task creation (4.0) */
#if (defined(CPU_SH)&&defined(SH2A))
#define TA_FPU      0x0000
#else
#define TA_FPU      0x0100  /* FPU activate */
#endif
#define TA_VPU      0x0200  /* Vector Unit activate */
#define TA_GTHUMB   0x00010000  /* special flag */
#if (defined(CPU_THUMB)&&defined(GAIO))
#undef  TA_HLNG
#define TA_HLNG     TA_GTHUMB
#endif

/* sematr, mbxatr, mbfatr, mplatr, mpfatr */

#define TA_TFIFO    0x0000  /* waiting tasks are handled by FIFO */
#define TA_TPRI     0x0001  /* waiting tasks are handled by priority */

/* tmout */

#define TMO_POL     0L      /* polling */
#define TMO_FEVR    (-1L)   /* wait forever */

/* tskid */

#define TSK_SELF    0       /* task specifies itself */
#define TSK_NONE    0       /* no task */
#define ID_AUTO     0       /* auto id number assign */

/* tskpri */

#define TPRI_INI    0       /* the initial priority (chg_pri) */
#define TPRI_RUN    0       /* the highest priority (rot_rdq) */
#define TPRI_SELF   0       /* (v4.0) */
#define TMIN_TPRI   1       /* the higest task priority */
#define TMAX_TPRI   (CFG.tpri_max)

/* tskstat */

#define TTS_RUN     0x0001  /* RUNNING */
#define TTS_RDY     0x0002  /* READY */
#define TTS_WAI     0x0004  /* WAITING */
#define TTS_SUS     0x0008  /* SUSPENDED */
#define TTS_WAS     0x000c  /* WAITING-SUSPENDED */
#define TTS_DMT     0x0010  /* DORMANT */

/* task excwption routine */

#define TTEX_ENA    0x00    /* enable */
#define TTEX_DIS    0x01    /* disable */

/* tskwait */
#define TTW_SLP     0x0001  /* wait due to slp_tsk or tslp_tsk */
#define TTW_DLY     0x0002  /* wait due to dly_tsk */
#define TTW_SEM     0x0004  /* wait due to wai_sem or twai_sem */
#define TTW_FLG     0x0008  /* wait due to wai_flg or twai_flg */
#define TTW_SDTQ    0x0010  /* wait due to snd_dtq */
#define TTW_RDTQ    0x0020  /* wait due to rcv_dtq */
#define TTW_MBX     0x0040  /* wait due to rcv_msg or trcv_msg */
#define TTW_MTX     0x0080  /* wait due to loc_mtx */
#define TTW_SMBF    0x0100  /* wait due to snd_mbf or tsnd_mbf */
#define TTW_MBF     0x0200  /* wait due to rcv_mbf or trcv_mbf */
#define TTW_RMBF    0x0200  /* wait due to rcv_mbf or trcv_mbf */
#define TTW_CAL     0x0400  /* wait for rendezvous call */
#define TTW_ACP     0x0800  /* wait for rendezvous accept */
#define TTW_RDV     0x1000  /* wait for rendezvous completion */
#define TTW_MPF     0x2000  /* wait due to get_blf or tget_blf */
#define TTW_MPL     0x4000  /* wait due to get_blk or tget_blk */


/* flgatr */

#define TA_WSGL     0x0000  /* wait single task */
#define TA_CLR      0x0004  /* clear all bits at ready time v4.0 */
#define TA_WMUL     0x0002  /* wait multiple task (0x08 for v3.0)*/

/* mbf.atr */

#define TA_TPRIR    0x0004  /* recieve tasks are handled by priority */

/* wfmode */

#define TWF_ANDW    0x0000  /* AND wait */
#define TWF_ORW     0x0001  /* OR wait (0x02 for v3.0) */
#define TWF_CLR     0x0004  /* clear specification (0x01 for v3.0) */

/* mbxatr */

#define TA_MFIFO    0x0000  /* messages are handled by FIFO */
#define TA_MPRI     0x0002  /* messages are handled by priority */

#define TMIN_MPRI   1       /* highest message priority */

/* mutex attr. */

#define TA_INHERIT  0x0002
#define TA_CEILING  0x0003

/* poratr */

#define TA_NULL     0       /* specifies no particular attributes */

/* cycact */

#define TCY_OFF     0x0000  /* do not invoke cyclic handler */
#define TCY_ON      0x0001  /* invoke cyclic handler */
#define TCY_INI     0x0002  /* initialize cycle count */
#define TA_STA      0x0002  /* invoke cyclic handler (v4.0) */
#define TA_PHS      0x0004  /* keep phase (v4.0) */
#define TCYC_STA    0x0001
#define TCYC_STP    0x0000

/* tmmode */

#define TTM_ABS     0x0000  /* specified as an absolute time */
#define TTM_REL     0x0001  /* specified as a relative time */

/* ref_alm */

#define TALM_STP    0x0000
#define TALM_STA    0x0001

/* ref_ovr */

#define TOVR_STP    0x0000
#define TOVR_STA    0x0001

/* ref_ver */

#define TKERNEL_MAKER   (norti_ver->maker)
#define TKERNEL_PRID    (norti_ver->prid)
#define TKERNEL_SPVER   (norti_ver->spver)
#define TKERNEL_PRVER   (norti_ver->prver)

/* sysstat */

#define TSS_TSK     0       /* task portion (normal state) */
#define TSS_DDSP    1       /* task portion (dispatch disabled) */
#define TSS_LOC     3       /* task portion (interrupt and dispatch disabled)*/
#define TSS_INDP    4       /* task-independent portion */
#define TSS_IMS     2
#define TSS_V4_LOC  0x10
#define TSS_V4_DDSP 0x08
#define TSS_LOC_DDSP    0x18

/************************************/
/* Error Codes                      */
/************************************/

#define E_OK        0       /* normal completion */
#define E_SYS       (-5)    /* system error */
#define E_NOMEM     (-33)   /* insufficient memory */
#define E_NOSPT     (-9)   /* feature not supported */
#define E_INOSPT    (-97)   /* feature not supported by ITRON/FILE */
#define E_RSFN      (-10)   /* reserved function code number */
#define E_RSATR     (-11)   /* reserved attribute */
#define E_PAR       (-17)   /* parameter error */
#define E_ID        (-18)   /* invalid ID number */
#define E_NOID      (-34)   /* no free ID number */
#define E_NOEXS     (-42)   /* object does not exist */
#define E_OBJ       (-41)   /* invalid object state */
#define E_MACV      (-26)   /* memory access disabled or violation */
#define E_OACV      (-27)   /* object access violation */
#define E_CTX       (-25)   /* context error */
#define E_QOVR      (-43)   /* queuing or nesting overflow */
#define E_DLT       (-51)   /* object being waited for was deleted */
#define E_TMOUT     (-50)   /* polling failure or timeout exceeded */
#define E_RLWAI     (-49)   /* WAIT state was forcibly released */
#define E_ILUSE     (-28)   /* illegal use */

/************************************/
/* Function Codes                   */
/************************************/
#define TFN_UNL_CPU     (-0x5a)
#define TFN_LOC_CPU     (-0x59)
#define TFN_DEF_SVC     (-0x6d)
#define TFN_REF_SYS     (-0x61)
#define TFN_REF_CFG     (-0x6f)
#define TFN_REF_VER     (-0x70)
#define TFN_CRE_TSK     (-0x05)
#define TFN_ACRE_TSK    (-0xc1)
#define TFN_DEL_TSK     (-0x06)
#define TFN_REF_TSK     (-0x0f)
#define TFN_REF_TST     (-0x10)
#define TFN_EXT_TSK     (-0x0a)
#define TFN_EXD_TSK     (-0x0b)
#define TFN_STA_TSK     (-0x09)
#define TFN_ACT_TSK     (-0x07)
#define TFN_IACT_TSK    (-0x71)
#define TFN_GET_TID     (-0x56)
#define TFN_TER_TSK     (-0x0c)
#define TFN_CHG_PRI     (-0x0d)
#define TFN_GET_PRI     (-0x0e)
#define TFN_ROT_RDQ     (-0x55)
#define TFN_ENA_DSP     (-0x5c)
#define TFN_DIS_DSP     (-0x5b)
#define TFN_REL_WAI     (-0x15)
#define TFN_IREL_WAI    (-0x73)
#define TFN_SUS_TSK     (-0x16)
#define TFN_RSM_TSK     (-0x17)
#define TFN_FRSM_TSK    (-0x18)
#define TFN_TSLP_TSK    (-0x12)
#define TFN_WUP_TSK     (-0x13)
#define TFN_CAN_WUP     (-0x14)
#define TFN_CRE_FLG     (-0x29)
#define TFN_DEL_FLG     (-0x2a)
#define TFN_REF_FLG     (-0x30)
#define TFN_WAI_FLG     (-0x2d)
#define TFN_CLR_FLG     (-0x2c)
#define TFN_SET_FLG     (-0x2b)
#define TFN_ACRE_FLG    (-0xc3)
#define TFN_ISET_FLG    (-0x76)
#define TFN_CRE_SEM     (-0x21)
#define TFN_ACRE_SEM    (-0xc2)
#define TFN_DEL_SEM     (-0x22)
#define TFN_REF_SEM     (-0x28)
#define TFN_WAI_SEM     (-0x25)
#define TFN_SIG_SEM     (-0x23)
#define TFN_ISIG_SEM    (-0x75)
#define TFN_POL_SEM     (-0x26)
#define TFN_CRE_MBX     (-0x3d)
#define TFN_DEL_MBX     (-0x3e)
#define TFN_REF_MBX     (-0x44)
#define TFN_RCV_MBX     (-0x41)
#define TFN_SND_MBX     (-0x3f)
#define TFN_CRE_MPF     (-0x45)
#define TFN_DEL_MPF     (-0x46)
#define TFN_REF_MPF     (-0x4c)
#define TFN_ACRE_MPF    (-0xc9)
#define TFN_GET_MPF     (-0x49)
#define TFN_REL_MPF     (-0x47)
#define TFN_SET_TIM     (-0x4d)
#define TFN_GET_TIM     (-0x4e)
#define TFN_DLY_TSK     (-0x19)
#define TFN_CRE_ALM     (-0xa9)
#define TFN_DEL_ALM     (-0xaa)
#define TFN_CRE_CYC     (-0x4f)
#define TFN_DEL_CYC     (-0x50)
#define TFN_REF_ALM     (-0xad)
#define TFN_REF_CYC     (-0x53)
#define TFN_STA_ALM     (-0xab)
#define TFN_STP_ALM     (-0xac)
#define TFN_STA_CYC     (-0x51)
#define TFN_STP_CYC     (-0x52)
#define TFN_PGET_MPF    (-0x4a)
#define TFN_PGET_MPL    (-0xa6)
#define TFN_POL_FLG     (-0x2e)
#define TFN_PRCV_MBX    (-0x42)
#define TFN_PRCV_MBF    (-0x92)
#define TFN_PSND_MBF    (-0x8e)
#define TFN_PACP_POR    (-0x9a)
#define TFN_CRE_MPL     (-0xa1)
#define TFN_ACER_MPL    (-0xca)
#define TFN_DEL_MPL     (-0xa2)
#define TFN_REF_MPL     (-0xa8)
#define TFN_GET_MPL     (-0xa5)
#define TFN_REL_MPL     (-0xa3)
#define TFN_CRE_POR     (-0x95)
#define TFN_DEL_POR     (-0x96)
#define TFN_REF_POR     (-0x9e)
#define TFN_CAL_POR     (-0x97)
#define TFN_ACP_POR     (-0x99)
#define TFN_RPL_RDV     (-0x9d)
#define TFN_REF_RDV     (-0x9f)
#define TFN_FWD_POR     (-0x9c)
#define TFN_CRE_MBF     (-0x89)
#define TFN_DEL_MBF     (-0x8a)
#define TFN_REF_MBF     (-0x94)
#define TFN_SND_MBF     (-0x8d)
#define TFN_RCV_MBF     (-0x91)
#define TFN_TGET_MPF    (-0x4b)
#define TFN_TGET_MPL    (-0xa7)
#define TFN_TWAI_FLG    (-0x2f)
#define TFN_TWAI_SEM    (-0x27)
#define TFN_TRCV_MBX    (-0x43)
#define TFN_TSND_MBF    (-0x8f)
#define TFN_TRCV_MBF    (-0x93)
#define TFN_TCAL_POR    (-0x98)
#define TFN_TACP_POR    (-0x9b)
#define TFN_CAN_ACT     (-0x08)
#define TFN_ACRE_CYC    (-0xcb)
#define TFN_ACRE_ALM    (-0xcc)
#define TFN_DEF_TEX     (-0x1b)
#define TFN_RAS_TEX     (-0x1c)
#define TFN_IRAS_TEX    (-0x74)
#define TFN_DIS_TEX     (-0x1d)
#define TFN_ENA_TEX     (-0x1e)
#define TFN_SNS_TEX     (-0x1f)
#define TFN_REF_TEX     (-0x20)
#define TFN_CRE_DTQ     (-0x31)
#define TFN_ACRE_DTQ    (-0xc4)
#define TFN_DEL_DTQ     (-0x32)
#define TFN_SND_DTQ     (-0x35)
#define TFN_PSND_DTQ    (-0x36)
#define TFN_IPSND_DTQ   (-0x77)
#define TFN_TSND_DTQ    (-0x37)
#define TFN_FSND_DTQ    (-0x38)
#define TFN_IFSND_DTQ   (-0x78)
#define TFN_RCV_DTQ     (-0x39)
#define TFN_PRCV_DTQ    (-0x3a)
#define TFN_TRCV_DTQ    (-0x3b)
#define TFN_REF_DTQ     (-0x3c)
#define TFN_CRE_MTX     (-0x81)
#define TFN_ACRE_MTX    (-0xc6)
#define TFN_DEL_MTX     (-0x82)
#define TFN_LOC_MTX     (-0x85)
#define TFN_PLOC_MTX    (-0x86)
#define TFN_TLOC_MTX    (-0x87)
#define TFN_UNL_MTX     (-0x83)
#define TFN_REF_MTX     (-0x88)
#define TFN_SNS_CTX     (-0x5d)
#define TFN_SNS_LOC     (-0x5e)
#define TFN_SNS_DSP     (-0x5f)
#define TFN_SNS_DPN     (-0x60)
#define TFN_CRE_ISR     (-0x66)
#define TFN_DEL_ISR     (-0x67)
#define TFN_REF_ISR     (-0x68)
#define TFN_GET_IMS     (-0x6c)
#define TFN_CHG_IMS     (-0x6b)
#define TFN_SLP_TSK     (-0x11)
#define TFN_DEF_OVR     (-0xb1)
#define TFN_STA_OVR     (-0xb2)
#define TFN_STP_OVR     (-0xb3)
#define TFN_REF_OVR     (-0xb4)
/* V3 call */
#define TFN_DEF_CYC     (-0xfd)
#define TFN_ACT_CYC     (-0xfc)
#define TFN_DEF_ALM     (-0xfb)
#define TFN_RET_TMR     (-0xf6)
#define TFN_ENT_INT     (-0xfa)
#define TFN_RET_INT     (-0xf9)
#define TFN_SIG_OVR     (-0xf3)
#define TFN_GET_VER     (-0xf5)
#define TFN_EXT_EXP     (-0xf4)

/************************************/
/* System Call                      */
/************************************/

/* startup */

ER NORTiAPI sysini(void);
ER NORTiAPI syssta(void);

/* task management */

ER NORTiAPI v4_cre_tsk(ID, const V4_T_CTSK *);
ER NORTiAPI v3_cre_tsk(ID, const V3_T_CTSK *);
#ifdef V3
#define T_CTSK V3_T_CTSK
#define cre_tsk(i, t) v3_cre_tsk(i, t)
#define vcre_tsk(t) v3_cre_tsk(ID_AUTO, t)
#else
#define T_CTSK V4_T_CTSK
#define cre_tsk(i, t) v4_cre_tsk(i, t)
#define acre_tsk(t) (ER_ID)v4_cre_tsk(ID_AUTO, t)
#endif
ER NORTiAPI v4_del_tsk(ID);
#define del_tsk(i) v4_del_tsk(i)
ER NORTiAPI v4_sta_tsk(ID, VP_INT);
ER NORTiAPI v4_act_tsk(ID);
#define sta_tsk(i, c) v4_sta_tsk(i, (VP_INT)(c))
#define vsta_tsk(i, c) v4_sta_tsk(i, (VP_INT)(c))
#define act_tsk(i) v4_act_tsk(i)
#define iact_tsk(i) v4_act_tsk(i)
ER NORTiAPI v4_can_act(ID);
#define can_act(i) v4_can_act(i)
void NORTiAPI v4_ext_tsk(void);
#define ext_tsk() v4_ext_tsk()
void NORTiAPI v4_exd_tsk(void);
#define exd_tsk() v4_exd_tsk()
ER NORTiAPI v4_ter_tsk(ID);
#define ter_tsk(i) v4_ter_tsk(i)

ER NORTiAPI v4_dis_dsp(void);
ER NORTiAPI v4_ena_dsp(void);
ER NORTiAPI v3_dis_dsp(void);
ER NORTiAPI v3_ena_dsp(void);
#ifdef V3
#define dis_dsp() v3_dis_dsp()
#define ena_dsp() v3_ena_dsp()
#else
#define dis_dsp() v4_dis_dsp()
#define ena_dsp() v4_ena_dsp()
#endif
ER NORTiAPI v4_chg_pri(ID, PRI);
#define chg_pri(i, p) v4_chg_pri(i, p)
ER NORTiAPI v4_get_pri(ID, PRI PFAR *);
#define get_pri(i, p) v4_get_pri(i, p)

ER NORTiAPI v4_rot_rdq(PRI);
#define rot_rdq(p) v4_rot_rdq(p)
#define irot_rdq(p) v4_rot_rdq(p)
ER NORTiAPI v4_rel_wai(ID);
#define rel_wai(i) v4_rel_wai(i)
#define irel_wai(i) v4_rel_wai(i)
ER NORTiAPI v4_get_tid(ID PFAR *);
#define get_tid(p) v4_get_tid(p)
#define iget_tid(p) v4_get_tid(p)
ID NORTiAPI v3_vget_tid(void);
#define vget_tid() v3_vget_tid()
ER NORTiAPI v4_ref_tsk(ID, V4_T_RTSK PFAR *);
ER NORTiAPI v3_ref_tsk(V3_T_RTSK PFAR *, ID);
#ifdef V3
#define T_RTSK V3_T_RTSK
#define ref_tsk(t, i) v3_ref_tsk(t,i)
#else
#define T_RTSK V4_T_RTSK
#define ref_tsk(i, t) v4_ref_tsk(i, t)
#endif
#define T_RTST V4_T_RTST
ER NORTiAPI v4_ref_tst(ID, V4_T_RTST PFAR *);
#define ref_tst(i, t) v4_ref_tst(i, t)

/* task-dependent synchronization */

ER NORTiAPI v4_sus_tsk(ID);
#define sus_tsk(i) v4_sus_tsk(i)
ER NORTiAPI v4_rsm_tsk(ID, INT);
#define rsm_tsk(i) v4_rsm_tsk(i, 0)
#define frsm_tsk(i) v4_rsm_tsk(i, 1)
ER NORTiAPI v4_tslp_tsk(TMO);
#define slp_tsk() v4_tslp_tsk(TMO_FEVR)
#define tslp_tsk(t) v4_tslp_tsk(t)
ER NORTiAPI v4_wup_tsk(ID);
#define wup_tsk(i) v4_wup_tsk(i)
#define iwup_tsk(i) v4_wup_tsk(i)
ER_UINT NORTiAPI v4_can_wup(ID);
ER NORTiAPI v3_can_wup(INT PFAR *, ID);
#ifndef V3
#define can_wup(i) v4_can_wup(i)
#else
#define can_wup(c, i) v3_can_wup(c, i)
#endif
void NORTiAPI v3_vcan_wup(void);
#define vcan_wup() v3_vcan_wup()

/* task exception routine */

#define T_DTEX V4_T_DTEX
#define T_RTEX V4_T_RTEX
ER NORTiAPI v4_def_tex(ID, const V4_T_DTEX *);
#define def_tex(i, p) v4_def_tex(i, p)
ER NORTiAPI v4_ras_tex(ID, TEXPTN);
#define ras_tex(i, ptn) v4_ras_tex(i, ptn)
#define iras_tex(i, ptn) v4_ras_tex(i, ptn)
ER NORTiAPI v4_dis_tex(void);
#define dis_tex() v4_dis_tex()
ER NORTiAPI v4_ena_tex(void);
#define ena_tex() v4_ena_tex()
BOOL NORTiAPI v4_sns_tex(void);
#define sns_tex() v4_sns_tex()
ER NORTiAPI v4_ref_tex(ID, V4_T_RTEX PFAR *);
#define ref_tex(i, p) v4_ref_tex(i, p)

/* synchronization and communication */

ER NORTiAPI v4_cre_sem(ID, const V4_T_CSEM *);
ER NORTiAPI v3_cre_sem(ID, const V3_T_CSEM *);
#ifdef V3
#define T_CSEM V3_T_CSEM
#define T_RSEM V3_T_RSEM
#define cre_sem(i, pk) v3_cre_sem(i, pk)
#define vcre_sem(pk) v3_cre_sem(ID_AUTO, pk)
#else
#define T_CSEM V4_T_CSEM
#define T_RSEM V4_T_RSEM
#define cre_sem(i, pk) v4_cre_sem(i, pk)
#define acre_sem(pk) (ER_ID)v4_cre_sem(ID_AUTO, pk)
#endif
ER NORTiAPI v4_del_sem(ID);
#define del_sem(i) v4_del_sem(i)
ER NORTiAPI v4_sig_sem(ID);
#define sig_sem(i) v4_sig_sem(i)
#define isig_sem(i) v4_sig_sem(i)
ER NORTiAPI v4_twai_sem(ID, TMO);
#define wai_sem(i) v4_twai_sem(i,TMO_FEVR)
#define twai_sem(i, t) v4_twai_sem(i, t)
ER NORTiAPI v4_ref_sem(ID, V4_T_RSEM PFAR *);
ER NORTiAPI v3_ref_sem(V3_T_RSEM PFAR *, ID);
#ifdef V3
#define ref_sem(p, i) v3_ref_sem(p, i)
#define preq_sem(i) v4_twai_sem(i,TMO_POL)
#else
#define ref_sem(i, p) v4_ref_sem(i, p)
#define pol_sem(i) v4_twai_sem(i,TMO_POL)
#endif

ER NORTiAPI v4_cre_flg(ID, const V4_T_CFLG *);
ER NORTiAPI v3_cre_flg(ID, const V3_T_CFLG *);
#ifdef V3
#define T_CFLG V3_T_CFLG
#define T_RFLG V3_T_RFLG
#define cre_flg(i, pk) v3_cre_flg(i, pk)
#define vcre_flg(pk) v3_cre_flg(ID_AUTO, pk)
#else
#define T_CFLG V4_T_CFLG
#define T_RFLG V4_T_RFLG
#define cre_flg(i, pk) v4_cre_flg(i, pk)
#define acre_flg(pk) (ER_ID)v4_cre_flg(ID_AUTO, pk)
#endif
ER NORTiAPI v4_del_flg(ID);
#define del_flg(i) v4_del_flg(i)
ER NORTiAPI v4_set_flg(ID, FLGPTN);
#define set_flg(i, u) v4_set_flg(i, u)
#define iset_flg(i, u) v4_set_flg(i, u)
ER NORTiAPI v4_clr_flg(ID, FLGPTN);
#define clr_flg(i, u) v4_clr_flg(i, u)
ER NORTiAPI v4_twai_flg(ID, FLGPTN, MODE, UINT PFAR *, TMO);
#ifdef V3
#define twai_flg(p,i,w,m,t) v4_twai_flg(i,w,m,p,t)
#define wai_flg(p,i,w,m) v4_twai_flg(i,w,m,p,TMO_FEVR)
#define pol_flg(p,i,w,m) v4_twai_flg(i,w,m,p,TMO_POL)
#else
#define twai_flg(i,w,m,p,t) v4_twai_flg(i,w,m,p,t)
#define wai_flg(i,w,m,p) v4_twai_flg(i,w,m,p,TMO_FEVR)
#define pol_flg(i,w,m,p) v4_twai_flg(i,w,m,p,TMO_POL)
#endif
ER NORTiAPI v4_ref_flg(ID, V4_T_RFLG PFAR *);
ER NORTiAPI v3_ref_flg(V3_T_RFLG PFAR *, ID);
#ifdef V3
#define ref_flg(p, i) v3_ref_flg(p, i)
#else
#define ref_flg(i,p) v4_ref_flg(i,p)
#endif

#define TSZ_DTQ(i) ((i)*sizeof(VP))
#define T_CDTQ V4_T_CDTQ
#define T_RDTQ V4_T_RDTQ
ER NORTiAPI v4_cre_dtq(ID, const V4_T_CDTQ *);
#define cre_dtq(i,p) v4_cre_dtq(i,p)
#define acre_dtq(p) v4_cre_dtq(ID_AUTO,p)
ER NORTiAPI v4_del_dtq(ID);
#define del_dtq(i) v4_del_dtq(i)
ER NORTiAPI v4_tsnd_dtq(ID, VP_INT, TMO);
#define snd_dtq(i,d) v4_tsnd_dtq(i,d,TMO_FEVR)
#define psnd_dtq(i,d) v4_tsnd_dtq(i,d,TMO_POL)
#define ipsnd_dtq(i,d) v4_tsnd_dtq(i,d,TMO_POL)
#define tsnd_dtq(i,d,t) v4_tsnd_dtq(i,d,t)
ER NORTiAPI v4_fsnd_dtq(ID, VP_INT);
#define fsnd_dtq(i,d) v4_fsnd_dtq(i,d)
#define ifsnd_dtq(i,d) v4_fsnd_dtq(i,d)
ER NORTiAPI v4_trcv_dtq(ID, VP_INT PFAR *, TMO);
#define rcv_dtq(i,d) v4_trcv_dtq(i,d,TMO_FEVR)
#define prcv_dtq(i,d) v4_trcv_dtq(i,d,TMO_POL)
#define trcv_dtq(i,d,t) v4_trcv_dtq(i,d,t)
ER NORTiAPI v4_ref_dtq(ID, V4_T_RDTQ PFAR *);
#define ref_dtq(i,p) v4_ref_dtq(i,p)

ER NORTiAPI v4_cre_mbx(ID, const V4_T_CMBX *);
ER NORTiAPI v3_cre_mbx(ID, const V3_T_CMBX *);
#ifdef V3
#define cre_mbx(i,pk) v3_cre_mbx(i,pk)
#define vcre_mbx(pk) v3_cre_mbx(ID_AUTO, pk)
#define T_CMBX V3_T_CMBX
#else
#define cre_mbx(i,pk) v4_cre_mbx(i,pk)
#define acre_mbx(pk) (ER_ID)v4_cre_mbx(ID_AUTO,pk)
#define T_CMBX V4_T_CMBX
#endif
ER NORTiAPI v4_del_mbx(ID);
#define del_mbx(i) v4_del_mbx(i)
ER NORTiAPI v4_snd_mbx(ID, VP);
ER NORTiAPI v4_trcv_mbx(ID, VP, TMO);
#ifdef V3
#define T_RMBX V3_T_RMBX
#define snd_msg(i,v) v4_snd_mbx(i, v)
#define rcv_msg(m,i) v4_trcv_mbx(i,m,TMO_FEVR)
#define prcv_msg(m,i) v4_trcv_mbx(i,m,TMO_POL)
#define trcv_msg(m,i,t) v4_trcv_mbx(i,m,t)
#else
#define T_RMBX V4_T_RMBX
#define snd_mbx(i,v) v4_snd_mbx(i, v)
#define rcv_mbx(i,m) v4_trcv_mbx(i,m,TMO_FEVR)
#define prcv_mbx(i,m) v4_trcv_mbx(i,m,TMO_POL)
#define trcv_mbx(i,m,t) v4_trcv_mbx(i,m,t)
#endif
ER NORTiAPI v4_ref_mbx(ID, V4_T_RMBX PFAR *);
ER NORTiAPI v3_ref_mbx(V3_T_RMBX PFAR *, ID);
#ifdef V3
#define ref_mbx(v,i) v3_ref_mbx(v,i)
#else
#define ref_mbx(i,v) v4_ref_mbx(i,v)
#endif

/* extended synchronization and communication */

#define T_CMTX V4_T_CMTX
#define T_RMTX V4_T_RMTX
ER NORTiAPI v4_cre_mtx(ID, const V4_T_CMTX *);
ER NORTiAPI v4_del_mtx(ID);
ER NORTiAPI v4_tloc_mtx(ID,TMO);
ER NORTiAPI v4_unl_mtx(ID);
ER NORTiAPI v4_ref_mtx(ID, V4_T_RMTX PFAR *);
#define cre_mtx(i,p) v4_cre_mtx(i,p)
#define acre_mtx(p) v4_cre_mtx(ID_AUTO,p)
#define del_mtx(i) v4_del_mtx(i)
#define ref_mtx(i,p) v4_ref_mtx(i,p)
#define unl_mtx(i) v4_unl_mtx(i)
#define loc_mtx(i) v4_tloc_mtx(i,TMO_FEVR)
#define ploc_mtx(i) v4_tloc_mtx(i,TMO_POL)
#define tloc_mtx(i,t) v4_tloc_mtx(i,t)

ER NORTiAPI v4_cre_mbf(ID, const V4_T_CMBF *);
ER NORTiAPI v3_cre_mbf(ID, const V3_T_CMBF *);
#ifdef V3
#define T_CMBF V3_T_CMBF
#define cre_mbf(i,p) v3_cre_mbf(i,p)
#define vcre_mbf(p) v3_cre_mbf(ID_AUTO,p)
#else
#define T_CMBF V4_T_CMBF
#define cre_mbf(i,p) v4_cre_mbf(i,p)
#define acre_mbf(p) v4_cre_mbf(ID_AUTO,p)
#endif
ER NORTiAPI v4_del_mbf(ID);
#define del_mbf(i) v4_del_mbf(i)
ER NORTiAPI v4_tsnd_mbf(ID, VP, UINT, TMO);
#define snd_mbf(i,m,s) v4_tsnd_mbf(i,m,s,TMO_FEVR)
#define psnd_mbf(i,m,s) v4_tsnd_mbf(i,m,s,TMO_POL)
#define tsnd_mbf(i,m,s,t) v4_tsnd_mbf(i,m,s,t)
ER_UINT NORTiAPI v4_trcv_mbf(ID, VP, TMO);
ER NORTiAPI v3_trcv_mbf(VP, int PFAR *, ID, TMO);
#ifdef V3
#define trcv_mbf(v,p,i,t) v3_trcv_mbf(v,p,i,t)
#define rcv_mbf(m,s,i) v3_trcv_mbf(m,s,i,TMO_FEVR)
#define prcv_mbf(m,s,i) v3_trcv_mbf(m,s,i,TMO_POL)
#else
#define trcv_mbf(i,m,t) v4_trcv_mbf(i,m,t)
#define rcv_mbf(i,m) v4_trcv_mbf(i,m,TMO_FEVR)
#define prcv_mbf(i,m) v4_trcv_mbf(i,m,TMO_POL)
#endif
ER NORTiAPI v4_ref_mbf(ID,V4_T_RMBF PFAR *);
ER NORTiAPI v3_ref_mbf(V3_T_RMBF PFAR *, ID);
#ifdef V3
#define T_RMBF V3_T_RMBF
#define ref_mbf(p,i) v3_ref_mbf(p,i)
#else
#define T_RMBF V4_T_RMBF
#define ref_mbf(i,p) v4_ref_mbf(i,p)
#endif

ER NORTiAPI v4_cre_por(ID, const V4_T_CPOR *);
ER NORTiAPI v3_cre_por(ID, const V3_T_CPOR *);
#ifdef V3
#define T_CPOR V3_T_CPOR
#define T_RPOR V3_T_RPOR
#define cre_por(i, p) v3_cre_por(i, p)
#define vcre_por(p) v3_cre_por(ID_AUTO, p)
#else
#define T_CPOR V4_T_CPOR
#define T_RPOR V4_T_RPOR
#define cre_por(i, p) v4_cre_por(i, p)
#define acre_por(p) v4_cre_por(ID_AUTO, p)
#endif
ER NORTiAPI v4_del_por(ID);
#define del_por(i) v4_del_por(i)

ER NORTiAPI v3_tcal_por(VP, int PFAR *, ID, UINT, int, TMO);
ER_UINT NORTiAPI v4_tcal_por(ID, RDVPTN, VP, UINT, TMO);
ER NORTiAPI v3_tacp_por(RNO PFAR *, VP, int PFAR *, ID, UINT, TMO);
ER_UINT NORTiAPI v4_tacp_por(ID, RDVPTN, RDVNO PFAR *, VP, TMO);
#ifdef V3
#define tcal_por(m,r,p,c,s,t) v3_tcal_por(m,r,p,c,s,t)
#define cal_por(m,r,p,c,s) v3_tcal_por(m,r,p,c,s,TMO_FEVR)
#define pcal_por(m,r,p,c,s) v3_tcal_por(m,r,p,c,s,1)
#else
#define tcal_por(i,p,m,c,t) v4_tcal_por(i,p,m,c,t)
#define cal_por(i,p,m,c) v4_tcal_por(i,p,m,c,TMO_FEVR)
#define pcal_por(i,p,m,c) v4_tcal_por(i,p,m,c,1)
#endif
#ifdef V3
#define tacp_por(n,m,c,p,a,t) v3_tacp_por(n,m,c,p,a,t)
#define acp_por(n,m,c,p,a) v3_tacp_por(n,m,c,p,a,TMO_FEVR)
#define pacp_por(n,m,c,p,a) v3_tacp_por(n,m,c,p,a,TMO_POL)
#else
#define tacp_por(i,a,p,m,t) v4_tacp_por(i,a,p,m,t)
#define acp_por(i,a,p,m) v4_tacp_por(i,a,p,m,TMO_FEVR)
#define pacp_por(i,a,p,m) v4_tacp_por(i,a,p,m,TMO_POL)
#endif
ER NORTiAPI v4_fwd_por(ID, RDVPTN, RDVNO, VP, UINT);
#define fwd_por(p,c,r,m,s) v4_fwd_por(p,(RDVPTN)c,(RDVNO)r,m,(UINT)s)
ER NORTiAPI v4_rpl_rdv(RDVNO, VP, UINT);
#define rpl_rdv(r,m,s) v4_rpl_rdv((RDVNO)r,m,(UINT)s)
ER NORTiAPI v4_ref_por(ID, V4_T_RPOR PFAR *);
ER NORTiAPI v3_ref_por(V3_T_RPOR PFAR *, ID);
#ifdef V3
#define ref_por(v,i) v3_ref_por(v,i)
#else
#define ref_por(i,v) v4_ref_por(i,v)
#endif
ER NORTiAPI v4_ref_rdv(RDVNO, V4_T_RRDV PFAR *);
#define ref_rdv(r,p) v4_ref_rdv(r,p)
#define T_RRDV V4_T_RRDV

/* interrupt management */

ER NORTiAPI v4_def_inh(UINT, const V4_T_DINH *);
#define def_int(u, p) v4_def_inh(u, p)
#define T_DINT V4_T_DINH
#define def_inh(u, p) v4_def_inh(u, p)
#define T_DINH V4_T_DINH

ER NORTiAPI v4_cre_isr(ID, const V4_T_CISR *);
#define cre_isr(i,p) v4_cre_isr(i,p)
#define acre_isr(p) v4_cre_isr(ID_AUTO, p)
#define T_CISR V4_T_CISR

ER NORTiAPI v4_del_isr(ID);
#define del_isr(i) v4_del_isr(i)

ER NORTiAPI v4_ref_isr(ID, V4_T_RISR PFAR *);
#define ref_isr(i,p) v4_ref_isr(i,p)
#define T_RISR V4_T_RISR

void NORTiAPI v3_ret_int(void);
void NORTiAPI v4_ret_int(void);
void NORTiAPI v3_ret_wup(ID);
#if (defined(CPU_ARM)||defined(CPU_THUMB)||defined(CPU_NIOS2))
#if defined(VIC)
#define ret_int()  v4_ret_int()
#define iret_int() v4_ret_int()
#define ret_wup(i) {v4_wup_tsk(i);v4_ret_int();}
#else
#define ret_int()  return
#define iret_int() return
#define ret_wup(i) {v4_wup_tsk(i);return;}
#endif
#else
#define ret_int() v3_ret_int()
#define iret_int() v3_ret_int()
#define ret_wup(i) v3_ret_wup(i)
#endif
ER NORTiAPI v4_loc_cpu(void);
ER NORTiAPI v4_unl_cpu(void);
BOOL NORTiAPI v4_sns_ctx(void);
BOOL NORTiAPI v4_sns_loc(void);
BOOL NORTiAPI v4_sns_dsp(void);
BOOL NORTiAPI v4_sns_dpn(void);
ER NORTiAPI v3_loc_cpu(void);
ER NORTiAPI v3_dis_int(UINT);
ER NORTiAPI v3_ena_int(UINT);
ER NORTiAPI v3_chg_ims(UINT);
ER NORTiAPI v3_ref_ims(UINT PFAR *);
#ifdef V3
#define loc_cpu() v3_loc_cpu()
#define unl_cpu() v3_unl_cpu()
#define dis_int(u) v3_dis_int(u)
#define ena_int(u) v3_ena_int(u)
#define chg_ims(u) v3_chg_ims(u)
#define ref_ims(p) v3_ref_ims(p)
#else
#define loc_cpu() v4_loc_cpu()
#define iloc_cpu() v4_loc_cpu()
#define dis_int(u) v3_dis_int((UINT)(u))
#define ena_int(u) v3_ena_int((UINT)(u))
#define chg_ims(u) v3_chg_ims((UINT)(u))
#define get_ims(p) v3_ref_ims((UINT PFAR *)(p))
#define unl_cpu() v4_unl_cpu()
#define iunl_cpu() v4_unl_cpu()
#define sns_ctx() v4_sns_ctx()
#define sns_loc() v4_sns_loc()
#define sns_dsp() v4_sns_dsp()
#define sns_dpn() v4_sns_dpn()
#endif

/* memorypool management */

/* Modified by Renesas START */
#define RTOS_MPOOL_BOUNDARY_SIZE (32)      /* Boundary size for allocating a memory block from a memory pool.*/

/*#define TSZ_MPL(c,s)    (SIZE)((((s)+SIZEOF_ALIGN+SIZEOF_ALIGN-1)&~(SIZEOF_ALIGN-1))*(c))*/
#define TSZ_MPL(cnt, size)    (SIZE)(((((size) + SIZEOF_ALIGN + RTOS_MPOOL_BOUNDARY_SIZE-1)&~(RTOS_MPOOL_BOUNDARY_SIZE-1))*(cnt)) + (RTOS_MPOOL_BOUNDARY_SIZE - SIZEOF_ALIGN))

#define MPL_CHKINFO_SIZE (4)
#define MPF_CHKINFO_SIZE (32)

#define MP_CHK_VALUE  (0x01020304)
/* Modified by Renesas END */

ER NORTiAPI v4_cre_mpl(ID, const V4_T_CMPL *);
ER NORTiAPI v3_cre_mpl(ID, const V3_T_CMPL *);
ER NORTiAPI v4_del_mpl(ID);
ER NORTiAPI v4_tget_mpl(ID, SIZE, VP, TMO);
ER NORTiAPI v4_rel_mpl(ID, VP);
ER NORTiAPI v4_ref_mpl(ID,V4_T_RMPL PFAR *);
ER NORTiAPI v3_ref_mpl(V3_T_RMPL PFAR *, ID);
#ifdef V3
#define T_CMPL V3_T_CMPL
#define T_RMPL V3_T_RMPL
#define cre_mpl(i,v) v3_cre_mpl(i,v)
#define vcre_mpl(v) v3_cre_mpl(ID_AUTO,v)
#define tget_blk(b,i,s,t) v4_tget_mpl(i,s,b,t)
#define get_blk(b,i,s) v4_tget_mpl(i,s,b,TMO_FEVR)
#define pget_blk(b,i,s) v4_tget_mpl(i,s,b,TMO_POL)
#define rel_blk(i,v) v4_rel_mpl(i,v)
#define ref_mpl(v,i) v3_ref_mpl(v,i)
#else
#define T_CMPL V4_T_CMPL
#define T_RMPL V4_T_RMPL
#define cre_mpl(i,v) v4_cre_mpl(i,v)
#define acre_mpl(v) v4_cre_mpl(ID_AUTO,v)
#define tget_mpl(i,s,b,t) v4_tget_mpl(i,s,b,t)
#define get_mpl(i,s,b) v4_tget_mpl(i,s,b,TMO_FEVR)
#define pget_mpl(i,s,b) v4_tget_mpl(i,s,b,TMO_POL)
#define rel_mpl(i,v) v4_rel_mpl(i,v)
#define ref_mpl(i,v) v4_ref_mpl(i,v)
#endif
#define del_mpl(i) v4_del_mpl(i)

#define TSZ_MPF(c,s) ((SIZE)((((UINT)(s)+(sizeof(int)-1))&(~(sizeof(int)-1)))*(UINT)(c)))
ER NORTiAPI v4_cre_mpf(ID, const V4_T_CMPF *);
ER NORTiAPI v3_cre_mpf(ID, const V3_T_CMPF *);
ER NORTiAPI v4_del_mpf(ID);
ER NORTiAPI v4_tget_mpf(ID,VP,TMO);
ER NORTiAPI v4_rel_mpf(ID, VP);
ER NORTiAPI v3_ref_mpf(V3_T_RMPF PFAR *, ID);
ER NORTiAPI v4_ref_mpf(ID, V4_T_RMPF PFAR *);
#ifdef V3
#define T_CMPF V3_T_CMPF
#define T_RMPF V3_T_RMPF
#define cre_mpf(i,p) v3_cre_mpf(i,p)
#define vcre_mpf(p) v3_cre_mpf(ID_AUTO,p)
#define get_blf(v,i) v4_tget_mpf(i,v,TMO_FEVR)
#define tget_blf(v,i,t) v4_tget_mpf(i,v,t)
#define pget_blf(v,i) v4_tget_mpf(i,v,TMO_POL)
#define rel_blf(i,v) v4_rel_mpf(i,v)
#define ref_mpf(v,i) v3_ref_mpf(v,i)
#else
#define T_CMPF V4_T_CMPF2
#define T_RMPF V4_T_RMPF
#define cre_mpf(i,p) v4_cre_mpf(i,(V4_T_CMPF *)p)
#define acre_mpf(p) v4_cre_mpf(ID_AUTO,(V4_T_CMPF *)p)
#define tget_mpf(i,v,t) v4_tget_mpf(i,v,t)
#define get_mpf(i,v) v4_tget_mpf(i,v,TMO_FEVR)
#define pget_mpf(i,v) v4_tget_mpf(i,v,TMO_POL)
#define rel_mpf(i,v) v4_rel_mpf(i,v)
#define ref_mpf(i,v) v4_ref_mpf(i,v)
#endif
#define del_mpf(i) v4_del_mpf(i)

/* time management */

ER NORTiAPI v4_set_tim(V4_SYSTIM PFAR *);
ER NORTiAPI v4_get_tim(V4_SYSTIM PFAR *);
ER NORTiAPI v3_set_tim(const SYSTIME PFAR *);
ER NORTiAPI v3_get_tim(SYSTIME PFAR *);
#ifdef V3
#define set_tim(i) v3_set_tim(i)
#define get_tim(i) v3_get_tim(i)
#else
#define SYSTIM V4_SYSTIM
#define set_tim(i) v4_set_tim(i)
#define get_tim(i) v4_get_tim(i)
#endif
ER NORTiAPI v4_dly_tsk(RELTIM);
#define dly_tsk(t) v4_dly_tsk(t)

ER NORTiAPI v4_cre_cyc(ID, const V4_T_CCYC *);
ER NORTiAPI v3_def_cyc(HNO, const V3_T_DCYC *);
ER NORTiAPI v4_del_cyc(ID);
ER NORTiAPI v4_sta_cyc(ID);
ER NORTiAPI v4_stp_cyc(ID);
ER NORTiAPI v3_act_cyc(HNO, UINT);
ER NORTiAPI v4_ref_cyc(ID, V4_T_RCYC PFAR *);
ER NORTiAPI v3_ref_cyc(V3_T_RCYC PFAR *, HNO);
#ifdef V3
#define T_DCYC V3_T_DCYC
#define T_RCYC V3_T_RCYC
#define def_cyc(i,p) v3_def_cyc(i,p)
#define vdef_cyc(p) v3_def_cyc(ID_AUTO,p)
#define act_cyc(i,u) v3_act_cyc(i,u)
#define ref_cyc(p,i) v3_ref_cyc(p,i)
#else
#define T_CCYC V4_T_CCYC
#define T_RCYC V4_T_RCYC
#define cre_cyc(i,p) v4_cre_cyc(i,p)
#define del_cyc(i) v4_del_cyc(i)
#define acre_cyc(p) v4_cre_cyc(ID_AUTO,p)
#define sta_cyc(i) v4_sta_cyc(i)
#define stp_cyc(i) v4_stp_cyc(i)
#define ref_cyc(i,p) v4_ref_cyc(i,p)
#endif

ER NORTiAPI v4_cre_alm(ID, const V4_T_CALM *);
ER NORTiAPI v4_del_alm(ID);
ER NORTiAPI v4_sta_alm(ID, RELTIM);
ER NORTiAPI v4_stp_alm(ID);
ER NORTiAPI v4_ref_alm(ID, V4_T_RALM PFAR *);
ER NORTiAPI v3_def_alm(HNO, const V3_T_DALM *);
ER NORTiAPI v3_ref_alm(V3_T_RALM PFAR *, HNO);
#ifdef V3
#define T_DALM V3_T_DALM
#define T_RALM V3_T_RALM
#define def_alm(i,p) v3_def_alm(i,p)
#define vdef_alm(p) v3_def_alm(ID_AUTO,p)
#define ref_alm(p,i) v3_ref_alm(p,i)
#else
#define T_CALM V4_T_CALM
#define T_RALM V4_T_RALM
#define cre_alm(i,p) v4_cre_alm(i,p)
#define acre_alm(p) v4_cre_alm(ID_AUTO,p)
#define del_alm(i) v4_del_alm(i)
#define sta_alm(i,t) v4_sta_alm(i,t)
#define stp_alm(i) v4_stp_alm(i)
#define ista_alm(i,t) v4_sta_alm(i,t)
#define istp_alm(i) v4_stp_alm(i)
#define ref_alm(i,p) v4_ref_alm(i,p)
#endif

ER NORTiAPI v4_def_ovr(const V4_T_DOVR *);
ER NORTiAPI v4_sta_ovr(ID, OVRTIM);
ER NORTiAPI v4_stp_ovr(ID);
ER NORTiAPI v4_ref_ovr(ID, V4_T_ROVR PFAR *);
#define T_DOVR V4_T_DOVR
#define T_ROVR V4_T_ROVR
#define def_ovr(p) v4_def_ovr(p)
#define sta_ovr(i,t) v4_sta_ovr(i,t)
#define stp_ovr(i) v4_stp_ovr(i)
#define ref_ovr(i,p) v4_ref_ovr(i,p)

void NORTiAPI v3_ret_tmr(void);
#define ret_tmr() v3_ret_tmr()

ER NORTiAPI v4_def_svc(FN, const V4_T_DSVC *);
ER NORTiAPI v4_cal_svc(FN, ...);
#define def_svc v4_def_svc
#define cal_svc v4_cal_svc
#define T_DSVC V4_T_DSVC

/* system management */

#define T_RVER V4_T_RVER
#define T_VER V3_T_VER
ER NORTiAPI v4_ref_ver(V4_T_RVER PFAR *);
#define get_ver(p) v3_get_ver((V3_T_VER PFAR *) p)
#define ref_ver(p) v4_ref_ver(p)
ER NORTiAPI v4_ref_sys(V4_T_RSYS PFAR *);
#define T_RSYS V4_T_RSYS
#define ref_sys(p) v4_ref_sys(p)
ER NORTiAPI v4_ref_cfg(V4_T_RCFG PFAR *);
ER NORTiAPI v3_ref_cfg(V3_T_RCFG PFAR *);
#ifdef V3
#define T_RCFG V3_T_RCFG
#define ref_cfg(p) v3_ref_cfg(p)
#else
#define T_RCFG V4_T_RCFG
#define ref_cfg(p) v4_ref_cfg(p)
#endif
#ifdef KERNEL_PS
ER vdef_tim(void (*str_tim)(void), BOOL (*stp_tim)(void));
ER vdef_idl(void (*power_save)(UINT, BOOL, FP));
#endif

/* interrupt management */

void NORTiAPI v3_ent_int(void);
void NORTiAPI v4_ent_int(void);
UINT NORTiAPI v3_vdis_psw(void);
void NORTiAPI v3_vset_psw(UINT);
void NORTiAPI v3_sig_tim(void);
#define sig_tim() v3_sig_tim()
#define isig_tim() v3_sig_tim()
#if ((defined(CPU_ARM)||defined(CPU_THUMB)||defined(CPU_NIOS2))&&(!defined(VIC)))
#define ent_int()
#define ient_int()
#else
#define ent_int() v3_ent_int()
#define ient_int() v3_ent_int()
#endif
#if (!defined(CPU_NIOS2))
#define vdis_psw() v3_vdis_psw()
#define vset_psw(i) v3_vset_psw(i)
#endif
ER cdecl intini(void);
ER cdecl intsta(void);
void cdecl intext(void);

/* kernel constant data */

extern const V4_T_RCFG cdecl CFG;
extern const V4_T_RVER DFAR cdecl _NORTI_VER;
extern const V4_T_RVER DFAR * const cdecl norti_ver;

/* CPU-dependent */

void vena_vpu(void);
void vdis_vpu(void);

#if defined(CPU_86)
void FFAR NORTiAPI sysini_rom(void);
int FFAR NORTiAPI ent_int2(void);
void FFAR NORTiAPI vdis_pic(void);
void FFAR NORTiAPI vena_pic(void);
void cdecl intsta2(void);
BOOL cdecl judgepc(void);
BOOL cdecl ispc9801(void);
BOOL cdecl isibmpc(void);
extern UB NEAR cdecl CodeView;
#elif defined(CPU_SH)
#if defined(__GNUC__)
void V3_ENT_INT(void);
#define ENT_INT() V3_ENT_INT()
#define v3_ent_int() asm("\tmov.l\tr3,@-r15\n\tmov.l\tr2,@-r15\n\tmov.l\tr1,@-r15\n"); ENT_INT()
#elif defined(__TID__)
void v3_ent_int(void);
#ifndef OPTHL
#define v3_ent_int() asm("\tmov.l\tr0,@-r15\n");v4_ent_int()
#else
#define v3_ent_int() asm("\tmov.l\tr7,@-r15\n");v4_ent_int()
#endif
#endif
#endif

#if (defined(CPU_SH)&&defined(SH2AFPU))
extern volatile int SNEAR * cdecl NOWFPU;
#define vena_fpu() { volatile int fpuctx[18]; NOWFPU = fpuctx; set_fpscr(0x00040001); }
#define vdis_fpu() { NOWFPU = NULL; }
#endif

#ifdef __cplusplus
}
#endif
#endif /* KERNEL_H */
