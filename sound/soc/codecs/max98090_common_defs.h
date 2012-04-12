/* max98090_common_defs.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

/*! 
  @file max98090_common_defs.h
  
  @brief Audio LSI driver common header file.
  
  
  @author author Xxxx Xxxx
  
  @attention first version is a draft.
  
  @date 2012/03/15 first version.
*/

#ifndef __MAX98090_COMMON_DEFS_H__
#define __MAX98090_COMMON_DEFS_H__

/*---------------------------------------------------------------------------*/
/* include files                                                             */
/*---------------------------------------------------------------------------*/
/* none */

/*---------------------------------------------------------------------------*/
/* typedef declaration                                                       */
/*---------------------------------------------------------------------------*/
/* none */

/*---------------------------------------------------------------------------*/
/* define macro declaration                                                  */
/*---------------------------------------------------------------------------*/
#define BIT0    0x01
#define BIT1    0x02
#define BIT2    0x04
#define BIT3    0x08
#define BIT4    0x10
#define BIT5    0x20
#define BIT6    0x40
#define BIT7    0x80

/*---------------------------------------------------------------------------*/
/* define function macro declaration                                         */
/*---------------------------------------------------------------------------*/
/* none */

/*---------------------------------------------------------------------------*/
/* enum declaration                                                          */
/*---------------------------------------------------------------------------*/
/* none */

/*---------------------------------------------------------------------------*/
/* structure declaration                                                     */
/*---------------------------------------------------------------------------*/
/*!
  @brief register bit information.
*/
typedef struct {
    int shift;          //!< bit shift information.
    int mask;           //!< bit mask information.
} sMax98090BitInfo;

/*---------------------------------------------------------------------------*/
/* extern variable declaration                                               */
/*---------------------------------------------------------------------------*/
/* none */

/*---------------------------------------------------------------------------*/
/* extern function declaration                                               */
/*---------------------------------------------------------------------------*/
/* none */

/*---------------------------------------------------------------------------*/
/* prototype declaration                                                     */
/*---------------------------------------------------------------------------*/
/* none */

/*---------------------------------------------------------------------------*/
/* inline function implementation                                            */
/*---------------------------------------------------------------------------*/
/* none */

#endif  /* __MAX98090_COMMON_DEFS_H__ */
