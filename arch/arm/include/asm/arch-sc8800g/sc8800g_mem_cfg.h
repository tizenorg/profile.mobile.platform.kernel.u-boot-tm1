/******************************************************************************
 ** File Name:    sc8800g_mem_cfg.h                                         *
 ** Author:       Steve.Zhan                                                 *
 ** DATE:         06/05/2010                                                  *
 ** Copyright:    2010 Spreatrum, Incoporated. All Rights Reserved.           *
 ** Description:                                                              *
 ******************************************************************************/
/******************************************************************************
 **                   Edit    History                                         *
 **---------------------------------------------------------------------------*
 ** DATE          NAME            DESCRIPTION                                 *
 ** 06/05/2010    Steve.Zhan      Create.                                     *
 ******************************************************************************/
#ifndef _SC8800G_MEM_CFG_H_
#define _SC8800G_MEM_CFG_H_

#include "sci_types.h"
#include "sc8800g_reg_base.h"
#include "sc8800g_reg_ahb.h"
#include "iram_mgr.h"

/*----------------------------------------------------------------------------*
 **                         Dependencies                                      *
 **------------------------------------------------------------------------- */

/**---------------------------------------------------------------------------*
 **                             Compiler Flag                                 *
 **--------------------------------------------------------------------------*/
#ifdef   __cplusplus
extern   "C"
{
#endif
/**---------------------------------------------------------------------------*
**                               Micro Define                                **
**---------------------------------------------------------------------------*/
typedef struct
{
    uint32  mem_start;
    uint32  mem_end;
    uint32  acc_condition;
} MEMORY_ACCESS_INFO_T;

#define READ_ACC        (1 << 0)
#define WRITE_ACC       (1 << 1)
#define BYTE_ACC        (1 << 2)
#define WORD_ACC        (1 << 3)
#define DWORD_ACC       (1 << 4)

#define SCI_NULL                    0x0
/**----------------------------------------------------------------------------*
**                           Function Prototype                               **
**----------------------------------------------------------------------------*/
MEMORY_ACCESS_INFO_T *Mem_getAccTable (uint32 *pSize);


/**----------------------------------------------------------------------------*
**                         Compiler Flag                                      **
**----------------------------------------------------------------------------*/
#ifdef   __cplusplus
}
#endif
/**---------------------------------------------------------------------------*/
#endif
// End
