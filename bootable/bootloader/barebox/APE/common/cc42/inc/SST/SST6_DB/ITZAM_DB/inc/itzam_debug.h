/*//---------------------------------------------------------------------
//  Algorithmic Conjurings @ http://www.coyotegulch.com
//
//  Itzam - An Embedded Database Engine
//
//  itzam_debug.h
//
//---------------------------------------------------------------------
//
//  Copyright 2004, 2005 Scott Robert Ladd
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
#ifdef SST_DB_ITZAM_DEBUG_MODE_ENABLED

	#if !defined(LIBITZAM_ITZAM_DEBUG_H)
	#define LIBITZAM_ITZAM_DEBUG_H

	#include "DX_VOS_File.h"
	#include "itzam.h"

	#if defined(__cplusplus)
	extern "C" {
	#endif

    /* 
    *
    * Note: The implementation of this function includes argument retState_ptr that can be missing 
    *       in some calls of this function. Those calls are not in use now. If those calls will be
    *	    in use in the future This issue will be repair. 
    *
    */
	void analyze_datafile(itzam_datafile * datafile, DxVosFile *output, itzam_state* retState_ptr);
	void dump_page(itzam_btree * btree, itzam_btree_page * page, itzam_ref parent_ref);
	void dump_btree(itzam_btree * btree);
	void analyze_btree(itzam_btree * btree, DxVosFile *output);

	#if defined(__cplusplus)
	}
	#endif

	#endif

#endif
