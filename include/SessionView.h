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

#ifndef _SESSIONVIEW_H_
#define _SESSIONVIEW_H_

#include "DataView.h"
#include "CustGrid.h"
#include "MySQLTableDataEx.h"

//some forward declarations
class DataView;
class TableView;
class MySQLDataEx;
class FormView;
class MySQLTableDataEx;

//Class representing table view
class SessionView : public DataView, public IQueryBuilder
{
	public:
        ///Constuctor
        /**
        @param wnd              : IN MDI window pointer
        @param hwndparent       : IN parent window handle
        */
		SessionView(MDIWindow *wnd, HWND hwndparent);
		    
        ///Destructor
		~SessionView();

        ///Function to set the data
        /**
        @param data             : IN data pointer
        @returns void
        */
		void                    SetData(MySQLDataEx *data);
	
        ///Functions to resize the window
        /**
        @returns void
        */
		void                    Resize();

        ///Reset all toolbar button images to its original state.
        /**
        @returns void
        */
        void                    ResetToolBarButtons();

		/// IQueryBuilder implementation
		/**
		@returns void. But caller needs to destroy query.(they own it)
		*/
		void				GetQuery(wyString& query);
	
		ThreadExecStatus	GetTableDetails();
	protected:
        ///Function to create toolbars and additional controls
        /**
        @returns void
        */
        void                    CreateToolBar();

        ///Function to add tool buttons
        /**
        @returns void
        */
		void                    AddToolButtons();

        ///The function initializes the required members and allocate a new object and copy the members to be freed. 
        /**
        @param pdata            : IN/OUT data
        @returns allocated data that should be deleted
        */
        MySQLDataEx*            ResetData(MySQLDataEx* pdata);

        ///Function gets the selected database
        /**
        @param db               : OUT database name
        @param col              : IN  column index
        @returns wyTrue on success else wyFalse
        */
        wyBool                  GetDBName(wyString& db, wyInt32 col);

        ///Function gets the selected table
        /**
        @param table            : OUT table name
        @param col              : IN  column index
        @returns wyTrue on success else wyFalse
        */
        wyBool                  GetTableName(wyString& table, wyInt32 col);

        ///Function gets the original column name associated with a column in the result set
        /**
        @param column           : OUT column name
        @param col              : IN  column index
        @returns wyTrue on success else wyFalse
        */
        wyBool                  GetColumnName(wyString& column, wyInt32 col);

        ///Function gets the banner text. The text is used to draw the text if there is no table is loaded into the view
        /**
        @param bannertext       : OUT text to be drawn
        @returns void
        */
        void                    GetBanner(wyString& bannertext);

        ///Reexecute the query
        /**
        @returns void
        */
        wyInt32                 RefreshDataView();

        ///Function to get index of the column from the field res
        /**
        @param col              : IN column index
        @returns index of the column on success else -1
        */
        wyInt32                 GetFieldResultIndexOfColumn(wyInt32 col);

        ///Actual function that executes the query
        /**
        @returns error status
        */
        wyInt32                 ExecuteTableData();

        ///Function draws the table info on info ribbon at the bottom
        /**
        @param hdc              : IN device context
        @param pdrawrect        : IN drawing rectangle
        @returns void
        */
        void                    DrawTableInfoRibbon(HDC hdc, RECT drawrect);

        ///Function to show the help file relevent to result tab
        /**
        @returns void
        */
        void                    ShowHelpFile();

        ///Gets auto increment column index.
        /**
        @returns auto increment column index if exists, otherwise -1
        */
        wyInt32                 GetAutoIncrIndex();

        ///Data
		MySQLTableDataEx*		m_mydata;

		void	ShowContextMenu(wyInt32 row, wyInt32 col, LPPOINT pt);
		void	OnContextMenu(LPARAM lparam);
		static LRESULT CALLBACK GridWndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
		void	OnWMCommand(WPARAM wparam, LPARAM lparam);
		void	OnKillSession(wyInt32 id);
};

#endif