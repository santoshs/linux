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
 
 

#ifndef _DX_VOS_RNG_H
#define _DX_VOS_RNG_H

/*! \file DX_VOS_RNG.h
    \brief This module defines the wrapper to the OS-dependent random
    seed function.
*/

#include "DX_VOS_BaseTypes.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*! Sets the seed for the random number generator.
	When setting the seed to the same specified value the same random numbers 
	series will be generated in the following calls	to DX_VOS_RandomGen().
	*/
void DX_VOS_SetRandomSeed(DxUint32 seed);

/*! \return a random 32 bit value. */
DxUint32 DX_VOS_RandomGen(void);

void DX_VOS_GenerateRandomVector(DxUint8* outVector, DxUint32 vectorSize);



#ifdef  __cplusplus
}
#endif


#endif /* ifndef _DX_VOS_RNG_H */









