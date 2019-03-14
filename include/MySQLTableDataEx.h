/* Copyright (C) 2013 Webyog Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307, USA

*/

/*********************************************

Author: Vishal P.R, Janani SriGuha

*********************************************/

#ifndef _MYSQLTABLEDATAEX_H_
#define _MYSQLTABLEDATAEX_H_

#include "DataView.h"
#include "CustGrid.h"


class MySQLDataEx;


//Data for TableView
class MySQLTableDataEx : public MySQLDataEx
{
public:
	///Constructor
	/**
	@param pmdi     : IN MDIWindow pointer
	*/
	MySQLTableDataEx(MDIWindow* pmdi);

	~MySQLTableDataEx()
	{
	}

	///Function to handle the view persistence for the table
	/**
	@param isset    : IN wyTrue if it should be saved, wyFalse if you want to load
	@returns ViewType enumeration
	*/
	ViewType        HandleViewPersistence(wyBool isset);

	///Function handles the limit persistance for the table
	/**
	@param isset    : IN wyTrue if it should be saved, wyFalse if you want to load
	@returns void
	*/
	void            HandleLimitPersistence(wyBool isset);
};

#endif