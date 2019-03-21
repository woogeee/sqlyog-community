/* Copyright (C) 2013 Webyog Inc

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

#include "resource.h"
#include "SessionView.h"
#include "Symbols.h"
#include "FrameWindowHelper.h"
#include "FKDropDown.h"


class FKDropDown;

//view constructor
SessionView::SessionView(MDIWindow *wnd, HWND hwndparent):DataView(wnd, hwndparent, (IQueryBuilder *)this)
{
	m_mydata = NULL;
	m_gridwndproc = SessionView::GridWndProc;
}

//destructor
SessionView::~SessionView()
{
}

//create the grid
void SessionView::CreateGrid()
{
	m_hwndgrid = CreateCustomGridEx(m_hwndframe,
		0, 0, 0, 0,
		m_gridwndproc,
		GV_EX_ROWCHECKBOX | GV_EX_OWNERDATA | GV_EX_COL_TOOLTIP, (LPARAM)this);

	CustomGrid_SetOwnerData(m_hwndgrid, wyTrue);
	SetGridFont();
	ShowWindow(m_hwndgrid, SW_HIDE);
	
}

//set the data
void
SessionView::SetData(MySQLDataEx *data)
{
    //set the data for this class
    m_mydata = (MySQLTableDataEx*)data;

    //call the base class version
	DataView::SetData(data);  
}

//function to get the banner text 
void 
SessionView::GetBanner(wyString& bannertext)
{
    bannertext.Clear();

    //set the text
    if(!m_mydata || !m_mydata->m_table.GetLength() || !m_mydata->m_db.GetLength())
    {
        bannertext.SetAs(_("Select a Table/View from the Object Browser"));
    }
}

//create toolbar
void
SessionView::CreateToolBar()
{
	wyUInt32 style = WS_CHILD | CCS_NOPARENTALIGN | TBSTYLE_TOOLTIPS | WS_VISIBLE | CCS_NODIVIDER | TBSTYLE_FLAT;

	m_hwndtoolbar = CreateWindowEx(0, TOOLBARCLASSNAME, NULL, style, 
                                   0, 0, 0, 0, m_hwndframe,
                                   (HMENU)IDC_TOOLBAR, (HINSTANCE)GetModuleHandle(0), NULL);

    //add tool buttons
	//AddToolButtons();

    //size the toolbar
    SendMessage(m_hwndtoolbar, TB_AUTOSIZE, 0, 0);

    //remove auto sizing
    style = GetWindowLongPtr(m_hwndtoolbar, GWL_STYLE);
    SetWindowLongPtr(m_hwndtoolbar, GWL_STYLE, style | CCS_NORESIZE);
}

//function to add tool bar buttons
void 
SessionView::AddToolButtons()
{
	INT			i = 0, j, size, k;
	HICON		hicon;
	TBBUTTON	tbb[30];

	wyInt32 command[] = {
        IDM_IMEX_EXPORTDATA, 
		IDM_DATATOCLIPBOARD,
		IDM_SEPARATOR,	ID_RESULT_INSERT, IDM_DUPLICATE_ROW, ID_RESULT_SAVE,
        ID_RESULT_DELETE, ID_RESULT_CANCEL,
        IDM_SEPARATOR, ID_VIEW_GRIDVIEW, ID_VIEW_FORMVIEW,
        ID_VIEW_TEXTVIEW
    };

	wyUInt32 states[][2] = {
        {TBSTATE_ENABLED, TBSTYLE_BUTTON | BTNS_AUTOSIZE},  
		{TBSTATE_ENABLED, BTNS_WHOLEDROPDOWN}, 
		{TBSTATE_ENABLED, TBSTYLE_SEP}, 
		{TBSTATE_ENABLED, TBSTYLE_BUTTON}, {TBSTATE_ENABLED, TBSTYLE_BUTTON}, {TBSTATE_ENABLED, TBSTYLE_BUTTON}, 
		{TBSTATE_ENABLED, TBSTYLE_BUTTON}, {TBSTATE_ENABLED, TBSTYLE_BUTTON},  
		{TBSTATE_ENABLED, TBSTYLE_SEP}, {TBSTATE_ENABLED, TBSTYLE_BUTTON}, {TBSTATE_ENABLED, TBSTYLE_BUTTON}, 
        {TBSTATE_ENABLED, TBSTYLE_BUTTON}
    };

	wyInt32 imgres[] = {
        IDI_TABLEEXPORT, 
		IDI_EXPORTRESULTCSV,
		IDI_USERS,  
        IDI_RESULTINSERT, IDI_DUPLICATEROW, IDI_SAVE,
		IDI_RESULTDELETE, IDI_RESULTCANCEL, 
        IDI_USERS, IDI_DATAGRID,
        IDI_FORMICON, IDI_TEXTVIEW
    };

	m_himglist = ImageList_Create(ICON_SIZE, ICON_SIZE, ILC_COLOR32  | ILC_MASK, 1, 0);
	//SendMessage(m_hwndtoolbar, TB_SETBUTTONSIZE, 0, MAKELPARAM(25, 25));
	SendMessage(m_hwndtoolbar, TB_SETIMAGELIST, 0, (LPARAM)m_himglist);
	SendMessage(m_hwndtoolbar, TB_SETEXTENDEDSTYLE, 0 , (LPARAM)TBSTYLE_EX_DRAWDDARROWS);
    SendMessage(m_hwndtoolbar, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);

    size = sizeof(command)/sizeof(command[0]);

	for(j = 0, k = 0; j < size; j++)	
	{
#ifndef COMMUNITY
        if(command[j] == ID_VIEW_FORMVIEW && !m_formview)
        {
            continue;
        }
#endif
		hicon = (HICON)LoadImage(pGlobals->m_hinstance, MAKEINTRESOURCE(imgres[j]), IMAGE_ICON, ICON_SIZE, ICON_SIZE, LR_DEFAULTCOLOR);
		i = ImageList_AddIcon(m_himglist, hicon);
		DestroyIcon(hicon);
		memset(&tbb[k], 0, sizeof(TBBUTTON));

		tbb[k].iBitmap = MAKELONG(i, 0);
		tbb[k].idCommand = command[j];
		tbb[k].fsState = (UCHAR)states[j][0];
		tbb[k].fsStyle = (UCHAR)states[j][1];
        ++k;
	}  

    //add extra images
    AddExtraImages(m_himglist);
	SendMessage(m_hwndtoolbar, TB_ADDBUTTONS, (WPARAM)k, (LPARAM) &tbb);

    //set the image indexes as lparam for buttons
    SetImageIndexes(m_hwndtoolbar);
}

//function to resize the view
void 
SessionView::Resize()
{
    RECT	    rcparent, rctemp, tbrect;
	wyInt32		hpos, vpos, width, height,toolwidth, toolheight;
    wyInt32     padding = 6, itemcount;
    TBBUTTON    tbinfo = {0};

    GetClientRect(m_hwndparent, &rcparent);
	
    hpos	= rcparent.left;
    vpos	= rcparent.top = CustomTab_GetTabHeight(m_hwndparent);
	width	= rcparent.right - rcparent.left;
	height	= rcparent.bottom - rcparent.top;
	
    SetWindowPos(m_hwndframe, HWND_BOTTOM, hpos, vpos, width, height,  SWP_NOZORDER);
    GetClientRect(m_hwndframe, &rcparent);

    GetWindowRect(m_hwndtoolbar, &tbrect);
    ResizeRefreshTool(tbrect.bottom - tbrect.top, &tbrect);
    toolheight = tbrect.bottom - tbrect.top;
        
    itemcount = SendMessage(m_hwndtoolbar, TB_BUTTONCOUNT, 0, 0) - 1;
    memset(&rctemp, 0, sizeof(RECT));

    if(itemcount >= 0)
    {
        SendMessage(m_hwndtoolbar, 
            TB_GETBUTTON, 
            itemcount,
            (LPARAM)&tbinfo);

        SendMessage(m_hwndtoolbar, TB_GETRECT, tbinfo.idCommand, (LPARAM)&rctemp);
        rctemp.right += padding;
    }

    toolwidth = rctemp.right;

    if(toolwidth + tbrect.right < rcparent.right)
    {
        toolwidth = rcparent.right - tbrect.right;
    }

    //SetWindowPos(m_hwndpadding, NULL, 0, 0, toolwidth + tbrect.right, DATAVIEW_TOPPADDING, SWP_NOZORDER);
    //SetWindowPos(m_hwndtoolbar, NULL, 0, DATAVIEW_TOPPADDING, toolwidth, toolheight, SWP_NOZORDER);
    //SetWindowPos(m_hwndrefreshtool, NULL, toolwidth, DATAVIEW_TOPPADDING, tbrect.right, toolheight, SWP_NOZORDER);
	SetWindowPos(m_hwndrefreshtool, NULL, 0, DATAVIEW_TOPPADDING, tbrect.right, toolheight, SWP_NOZORDER);
        
    rcparent.top += toolheight + DATAVIEW_TOPPADDING;
    ResizeControls(rcparent);
    InvalidateRect(m_hwndframe, NULL, TRUE);
    UpdateWindow(m_hwndframe);
}

//refresh data view
wyInt32
SessionView::RefreshDataView()
{
    return ExecuteTableData();
}

//reset the button with their original image
void 
SessionView::ResetToolBarButtons()
{
    //refresh toolbar
    if(m_hwndrefreshtool)
    {
        ResetButtons(m_hwndrefreshtool);
    }

    //toolbar
    if(m_hwndtoolbar)
    {
        ResetButtons(m_hwndtoolbar);
    }
}

/// IQueryBuilder implementation
/**
@returns wyString. Caller need to destroy this
*/
void
SessionView::GetQuery(wyString& query)
{
	//from  .ini file
	m_backtick = AppendBackQuotes() == wyTrue ? "`" : "";

	//query.Sprintf("select * from %s%s%s.%s%s%s\r\n", 
	//	m_backtick, m_mydata->m_db.GetString(), m_backtick,
	//	m_backtick, m_mydata->m_table.GetString(), m_backtick);

	query.Sprintf("SELECT * FROM information_schema.PROCESSLIST");
	//get filter info
	//GetFilterInfo(query);

	//get sort info
	//GetSortInfo(query);

	//get limits
	//GetLimits(query);

	query.Add(";\r\n");
}

//function to execute SELECT * query
wyInt32
SessionView::ExecuteTableData()
{
	wyString        query;
    wyInt32         ret,extraindex,j=0,no_row;
    MySQLDataEx*    pdata;
	MYSQL_ROW        fieldrow;

	GetQuery(query);

    //execut query
    m_mydata->m_datares = ExecuteQuery(query);

    //is thread stopped
	if(ThreadStopStatus())
	{	
        return TE_STOPPED;
	}

    //any error? show error dialog
	if(!m_mydata->m_datares)
	{
        return HandleErrors(query);
	}

	
    //allocate row array, if the thread is stopped, delete them

    if((ret = AllocateRowsExArray()) != TE_SUCCESS ||        (ret = GetTableDetails()) != TE_SUCCESS
		)
    {
        pdata = ResetData(m_data);
        delete pdata;
        return ret;
    }
	
	extraindex = GetFieldIndex(m_wnd->m_tunnel, m_data->m_fieldres, "Extra");
	
	no_row = m_wnd->m_tunnel->mysql_num_rows(m_data->m_fieldres);
	
	m_data->m_colvirtual = (wyInt32*)calloc(no_row, sizeof(wyInt32));
	
	while(fieldrow = m_wnd->m_tunnel->mysql_fetch_row(m_data->m_fieldres)){

		if(!strstr(fieldrow[extraindex], "VIRTUAL") && !strstr(fieldrow[extraindex], "PERSISTENT") && !strstr(fieldrow[extraindex], "STORED"))
		{
			m_data->m_colvirtual[j++] = 0;
		}

		else 
		{
			m_data->m_colvirtual[j++] = 1;
		}
	
	}
	
    //add new row in the end
    //AddNewRow();

    return TE_SUCCESS;
}

//function to reset the data
MySQLDataEx* 
SessionView::ResetData(MySQLDataEx* pdata)
{
    MySQLTableDataEx*   ptemp = NULL;
    MySQLTableDataEx*   tempdata = (MySQLTableDataEx*)pdata;
    SortAndFilter*      psfnew;

    if(pdata)
    {
        //create temperory object
        ptemp = new MySQLTableDataEx(pdata->m_pmdi);

        //clear the row array for the object
        delete ptemp->m_rowarray;
        ptemp->m_rowarray = NULL;

        //get the original sort and filter for the temperory data
        psfnew = ptemp->m_psortfilter;

        *ptemp = *tempdata;

        //copy the filter into the filter for the temperory data
        if(psfnew)
        {
            *psfnew = *tempdata->m_psortfilter;
        }

        //restore the original filter for the temperory data
        ptemp->m_psortfilter = psfnew;

        //set row array to NULL, so that the initialization module will allocate a new one
        pdata->m_rowarray = NULL;

        //initialize the data
        pdata->Initialize();
    }
    
    return ptemp;
}

//function to draw the bottom ribbon with table name and db name
void 
SessionView::DrawTableInfoRibbon(HDC hdc, RECT drawrect)
{
    HFONT   hfontnormal = GetStockFont(DEFAULT_GUI_FONT);
    HFONT   hfontbold = NULL;
    LOGFONT lf = {0};
    RECT    recttext;

    if(m_mydata && m_mydata->m_db.GetLength() && m_mydata->m_table.GetLength())
    {
        GetObject(hfontnormal, sizeof(lf), &lf);
        lf.lfWeight = FW_SEMIBOLD;
        hfontbold = CreateFontIndirect(&lf);

        hfontbold = (HFONT)SelectObject(hdc, hfontbold);
        memset(&recttext, 0, sizeof(RECT));
        DrawText(hdc, _(L"Database:"), -1, &recttext, DT_SINGLELINE | DT_NOPREFIX | DT_CALCRECT);
        DrawText(hdc, _(L"Database:"), -1, &drawrect, DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX);
        hfontbold = (HFONT)SelectObject(hdc, hfontbold);

        drawrect.left += recttext.right + 5;

        hfontnormal = (HFONT)SelectObject(hdc, hfontnormal);
        memset(&recttext, 0, sizeof(RECT));
        DrawText(hdc, m_mydata->m_db.GetAsWideChar(), -1, &recttext, DT_SINGLELINE | DT_NOPREFIX | DT_CALCRECT);
        DrawText(hdc, m_mydata->m_db.GetAsWideChar(), -1, &drawrect, DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX);
        hfontnormal = (HFONT)SelectObject(hdc, hfontnormal);
        
        drawrect.left += recttext.right + 10;

        hfontbold = (HFONT)SelectObject(hdc, hfontbold);
        memset(&recttext, 0, sizeof(RECT));
        DrawText(hdc, _(L"Table:"), -1, &recttext, DT_SINGLELINE | DT_NOPREFIX | DT_CALCRECT);
        DrawText(hdc, _(L"Table:"), -1, &drawrect, DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX);
        hfontbold = (HFONT)SelectObject(hdc, hfontbold);

        drawrect.left += recttext.right + 5;

        hfontnormal = (HFONT)SelectObject(hdc, hfontnormal);
        memset(&recttext, 0, sizeof(RECT));
        DrawText(hdc, m_mydata->m_table.GetAsWideChar(), -1, &recttext, DT_SINGLELINE | DT_NOPREFIX | DT_CALCRECT);
        DrawText(hdc, m_mydata->m_table.GetAsWideChar(), -1, &drawrect, DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX);
        hfontnormal = (HFONT)SelectObject(hdc, hfontnormal);
    }

    if(hfontbold)
    {
        DeleteFont(hfontbold);
    }

    if(hfontnormal)
    {
        DeleteFont(hfontnormal);
    }
}

//get the index of the column from the field res
wyInt32         
SessionView::GetFieldResultIndexOfColumn(wyInt32 col)
{
    //since we are doing a select * and the columns appear in the same order as field res, column index itself is the index of the column in field res
    return col;
}

//function to show help file
void
SessionView::ShowHelpFile()
{
    ShowHelp("http://sqlyogkb.webyog.com/article/102-data-manipulation");
}

//get the database name
wyBool 
SessionView::GetDBName(wyString& db, wyInt32 col)
{
    db.SetAs(m_data->m_db);
    return db.GetLength() ? wyTrue : wyFalse;
}

//get table name
wyBool 
SessionView::GetTableName(wyString& table, wyInt32 col)
{
    table.SetAs(m_data->m_table);
    return table.GetLength() ? wyTrue : wyFalse;
}

//get column name
wyBool 
SessionView::GetColumnName(wyString& column, wyInt32 col)
{
    if(m_data && m_data->m_datares && col >= 0 && col < m_data->m_datares->field_count)
    {
        if(m_data->m_datares->fields[col].name && m_data->m_datares->fields[col].name[0])
        {
            column.SetAs(m_data->m_datares->fields[col].name, m_wnd->m_ismysql41);
        }

        return column.GetLength() ? wyTrue : wyFalse;
    }

    return wyFalse;
}

//function to get auto incrment column index
wyInt32 
SessionView::GetAutoIncrIndex()
{
	wyInt32		colindex = 0;
	MYSQL_ROW	myrow;
	wyInt32		extraval;
	wyString	myrowstr;

    //if no field res
    if(!m_mydata->m_fieldres)
    {
        return -1;
    }

    //get the column index of extra in the field res
	m_wnd->m_tunnel->mysql_data_seek(m_mydata->m_fieldres, 0);
	extraval = GetFieldIndex(m_mydata->m_fieldres, "extra", m_wnd->m_tunnel, &m_wnd->m_mysql);

    //now loop through and check for auto_increment in extra column
	while(myrow = m_wnd->m_tunnel->mysql_fetch_row(m_mydata->m_fieldres))
	{
        if(myrow[extraval] && strstr(myrow[extraval], "auto_increment")) 
        {
            return colindex;
        }
			
        colindex++;
	}

	return -1;
}

ThreadExecStatus SessionView::GetTableDetails()
{
	wyString    query;
	MYSQL_RES   *myres = NULL, *fieldres = NULL, *keyres = NULL;
	MYSQL_ROW   myrow = NULL;

	//we can be sure that the form view need to be refreshed
	SetRefreshStatus(wyTrue, FORMVIEW_REFRESHED);

	//get create table statement
	//query.Sprintf("show create table `%s`.`%s`", m_data->m_db.GetString(), m_data->m_table.GetString());
	query.Sprintf("show create table `%s`.`%s`", "information_schema", "PROCESSLIST");
	//query.Sprintf("Show PROCESSLIST");
	myres = ExecuteQuery(query);

	//if the thread is stopped, cleanup and abort
	if (ThreadStopStatus())
	{
		if (myres)
		{
			m_wnd->m_tunnel->mysql_free_result(myres);
		}

		return TE_STOPPED;
	}

	//show any errors
	if (!myres)
	{
		HandleErrors(query);
		return TE_ERROR;
	}

	//get field res
	//query.Sprintf("show full fields from `%s`.`%s`", m_data->m_db.GetString(), m_data->m_table.GetString());
	query.Sprintf("show full fields from `%s`.`%s`", "information_schema", "PROCESSLIST");
	//query.Sprintf("Show PROCESSLIST");
	fieldres = ExecuteQuery(query);

	//if the thread is stopped, cleanup and abort
	if (ThreadStopStatus())
	{
		m_wnd->m_tunnel->mysql_free_result(myres);

		if (fieldres)
		{
			m_wnd->m_tunnel->mysql_free_result(fieldres);
		}

		return TE_STOPPED;
	}

	//show any errors
	if (!fieldres)
	{
		HandleErrors(query);
		m_wnd->m_tunnel->mysql_free_result(myres);
		return TE_ERROR;
	}


	//get the key res
	query.Sprintf("show keys from `%s`.`%s`", "information_schema", "PROCESSLIST");
	//query.Sprintf("Show PROCESSLIST");
	keyres = ExecuteQuery(query);

	//if the thread is stopped, cleanup and abort
	if (ThreadStopStatus())
	{
		m_wnd->m_tunnel->mysql_free_result(myres);
		m_wnd->m_tunnel->mysql_free_result(fieldres);

		if (keyres)
		{
			m_wnd->m_tunnel->mysql_free_result(keyres);
		}

		return TE_STOPPED;
	}

	//show any errors
	if (!keyres)
	{
		HandleErrors(query);
		m_wnd->m_tunnel->mysql_free_result(myres);
		m_wnd->m_tunnel->mysql_free_result(fieldres);
		return TE_ERROR;
	}

	//now set the create table statement
	myrow = m_wnd->m_tunnel->mysql_fetch_row(myres);
	m_data->m_createtablestmt.SetAs(myrow[1], m_wnd->m_ismysql41);
	m_wnd->m_tunnel->mysql_free_result(myres);

	//free any existing fied res
	if (m_data->m_fieldres)
	{
		m_wnd->m_tunnel->mysql_free_result(m_data->m_fieldres);
	}

	//free any existing key res
	if (m_data->m_keyres)
	{
		m_wnd->m_tunnel->mysql_free_result(m_data->m_keyres);
	}

	//set the new field res and key res
	m_data->m_fieldres = fieldres;
	m_data->m_keyres = keyres;

	return TE_SUCCESS;
}

void
SessionView::ShowContextMenu(wyInt32 row, wyInt32 col, LPPOINT pt)
{
	HMENU       hmenu, htrackmenu;
	wyBool      iscolreadonly = wyTrue, iscolnullable = wyFalse, iscolhasdefault = wyFalse;
	wyString    column, columntype;
	wyInt32     copymenupos = 14, i, iscolvirtual = 0;
	HWND        hwndtoolbar;
	wyBool		isunsort = wyFalse;

	//load menu, localize it and disable all the items
	hmenu = LoadMenu(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_SESSIONMENU));
	
	LocalizeMenu(hmenu);
	htrackmenu = GetSubMenu(hmenu, 0);
	DisableMenuItems(htrackmenu);

	EnableMenuItem(htrackmenu, IDM_KILL_SESSION, MF_ENABLED);
	//get toolbar handle containing reset filter button
	//if the toolbar is not found, delete all the filter related menu items
	
	if (!GetToolBarFromID(ID_RESETFILTER))
	{
		DeleteMenu(htrackmenu, 10, MF_BYPOSITION);
		DeleteMenu(htrackmenu, 10, MF_BYPOSITION);
		copymenupos -= 2;
	}

	//if valid data and result
	if (m_data && m_data->m_datares)
	{
		//if it is not text view
		if (m_data->m_viewtype != TEXT)
		{
			//export button is found and the button is enabled in the toolbar, then enable the mneu item
			if ((hwndtoolbar = GetToolBarFromID(IDM_IMEX_EXPORTDATA)) &&
				SendMessage(hwndtoolbar, TB_GETSTATE, IDM_IMEX_EXPORTDATA, 0) == TBSTATE_ENABLED)
			{
				EnableMenuItem(htrackmenu, IDM_IMEX_EXPORTDATA, MF_ENABLED);
			}

			//if db name and table name are valid, then enable the menu items based on their counterpart in toolbar
			if (m_data->m_db.GetLength() && m_data->m_table.GetLength())
			{
				EnableMenuItem(htrackmenu, ID_RESULT_INSERT,
					((hwndtoolbar = GetToolBarFromID(ID_RESULT_INSERT)) &&
						SendMessage(hwndtoolbar, TB_GETSTATE, ID_RESULT_INSERT, 0) == TBSTATE_ENABLED) ? MF_ENABLED : MF_GRAYED);
				EnableMenuItem(htrackmenu, ID_RESULT_SAVE,
					((hwndtoolbar = GetToolBarFromID(ID_RESULT_SAVE)) &&
						SendMessage(hwndtoolbar, TB_GETSTATE, ID_RESULT_SAVE, 0) == TBSTATE_ENABLED) ? MF_ENABLED : MF_GRAYED);
				EnableMenuItem(htrackmenu, ID_RESULT_DELETE,
					((hwndtoolbar = GetToolBarFromID(ID_RESULT_DELETE)) &&
						SendMessage(hwndtoolbar, TB_GETSTATE, ID_RESULT_DELETE, 0) == TBSTATE_ENABLED) ? MF_ENABLED : MF_GRAYED);
				EnableMenuItem(htrackmenu, ID_RESULT_CANCEL, ((hwndtoolbar = GetToolBarFromID(ID_RESULT_CANCEL)) &&
					SendMessage(hwndtoolbar, TB_GETSTATE, ID_RESULT_CANCEL, 0) == TBSTATE_ENABLED) ? MF_ENABLED : MF_GRAYED);

				//if current row is duplicatable, then enable duplicate row item
				if (IsCurrRowDuplicatable())
				{
					EnableMenuItem(htrackmenu, IDM_DUPLICATE_ROW, MF_ENABLED);
				}
			}

			if (row >= 0)
			{
				if (col >= 0)
				{
					GetColumnName(column, col);

					//if the column is not read-only
					if ((iscolreadonly = IsColumnReadOnly(col)) == wyFalse && (iscolvirtual = IsColumnVirtual(col)) != 1)
					{
						//check whether the column is nullable
						iscolnullable = IsNullable(m_wnd->m_tunnel, m_data->m_fieldres, (wyChar*)column.GetString());

						if (!iscolnullable)
						{
							//if column type is timestamp,leave the set null key enabled as timestamp takes null and set to current timestamp
							columntype = GetDataType(m_wnd->m_tunnel, m_data->m_fieldres, (wyChar*)column.GetString());
							if (columntype.CompareI("timestamp") == 0)
							{
								iscolnullable = wyTrue;
							}
						}

						//does the column has any defaults
						iscolhasdefault = (GetDefaultValue(m_wnd->m_tunnel, m_data->m_fieldres, NULL, (wyChar*)column.GetString())) ? wyTrue : wyFalse;
					}
				}
			}

			//enable/disable the menu items
			EnableMenuItem(htrackmenu, IDC_SETEMPTY, iscolreadonly == wyTrue || iscolvirtual == wyTrue ? MF_GRAYED : MF_ENABLED);
			EnableMenuItem(htrackmenu, IDC_SETNULL, iscolnullable == wyFalse ? MF_GRAYED : MF_ENABLED);
			EnableMenuItem(htrackmenu, IDC_SETDEF, iscolhasdefault == wyFalse ? MF_GRAYED : MF_ENABLED);

			//now enable the copy menu based on the button state in toolbar
			if ((hwndtoolbar = GetToolBarFromID(IDM_DATATOCLIPBOARD)) &&
				SendMessage(hwndtoolbar, TB_GETSTATE, IDM_DATATOCLIPBOARD, 0) == TBSTATE_ENABLED)
			{
				SetCopyMenu(GetSubMenu(htrackmenu, copymenupos), row, col);
			}
		}
	}

	//if the reset filter in toolbar is enabled, enable filter menu
	if ((hwndtoolbar = GetToolBarFromID(ID_RESETFILTER)) &&
		SendMessage(hwndtoolbar, TB_GETSTATE, ID_RESETFILTER, 0) == TBSTATE_ENABLED)
	{
		SetFilterMenu(GetSubMenu(htrackmenu, 11), row, col);
	}
	if (m_data && m_data->m_psortfilter)
		for (i = 0; i < m_data->m_psortfilter->m_sortcolumns; ++i)
		{
			if (m_data->m_psortfilter->m_sort[i].m_currsorttype != ST_NONE)
			{
				isunsort = wyTrue;
				i = m_data->m_psortfilter->m_sortcolumns;
			}
		}
	EnableMenuItem(htrackmenu, ID_UNSORT, isunsort == wyFalse ? MF_GRAYED : MF_ENABLED);

#ifndef COMMUNITY
	if (m_wnd->m_conninfo.m_isreadonly == wyTrue)
	{
		EnableMenuItem(htrackmenu, ID_RESULT_INSERT, MF_GRAYED);
		EnableMenuItem(htrackmenu, ID_RESULT_SAVE, MF_GRAYED);
		EnableMenuItem(htrackmenu, ID_RESULT_DELETE, MF_GRAYED);
		EnableMenuItem(htrackmenu, ID_RESULT_CANCEL, MF_GRAYED);
		EnableMenuItem(htrackmenu, IDC_SETEMPTY, MF_GRAYED);
		EnableMenuItem(htrackmenu, IDC_SETDEF, MF_GRAYED);
		EnableMenuItem(htrackmenu, IDC_SETNULL, MF_GRAYED);
		EnableMenuItem(htrackmenu, IDM_DUPLICATE_ROW, MF_GRAYED);
	}
#endif
	//set owner draw property and show menu
	m_htrackmenu = htrackmenu;
	wyTheme::SetMenuItemOwnerDraw(m_htrackmenu);
	TrackPopupMenu(m_htrackmenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt->x, pt->y, 0, m_hwndframe, NULL);
	DestroyMenu(hmenu);
	m_htrackmenu = NULL;
}

void
SessionView::OnContextMenu(LPARAM lparam)
{
	wyInt32     row, col;
	POINT       pt = { 0, 0 };
	RECT        rect;

	//if there is a valid point
	if (lparam)
	{
		//get the cell that contains the point
		pt.x = GET_X_LPARAM(lparam);
		pt.y = GET_Y_LPARAM(lparam);
		CustomGrid_GetItemFromPoint(m_hwndgrid, &pt, &row, &col);
	}
	//otherwise it is invoked throgugh keyboard thus we will display the menu at currently selected row and col
	else
	{
		//get the row and column
		row = CustomGrid_GetCurSelRow(m_hwndgrid);
		col = CustomGrid_GetCurSelCol(m_hwndgrid);

		if (row >= 0 && col >= 0)
		{
			//find the cell right bottom corner
			CustomGrid_GetSubItemRect(m_hwndgrid, row, col, &rect);
			pt.x = rect.right;
			pt.y = rect.bottom;
		}
	}

	//map it to screen cordinates and show context menu
	MapWindowPoints(m_hwndgrid, NULL, &pt, 1);
	ShowContextMenu(row, col, &pt);
}

//grid window proc
LRESULT CALLBACK
SessionView::GridWndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	SessionView* pviewdata = (SessionView*)CustomGrid_GetLongData(hwnd);


	switch (message)
	{
	case GVN_RBUTTONDOWN:
		pviewdata->OnContextMenu(lparam);
		return TRUE;
	case GVN_BEGINLABELEDIT:
		return FALSE;

	}

	return DataView::GridWndProc(hwnd, message, wparam, lparam);
}

void
SessionView::OnWMCommand(WPARAM wparam, LPARAM lparam)
{
	wyInt32 id = 0;
	MySQLRowEx* row = NULL;
	switch (LOWORD(wparam))
	{
		//add a new row
	case IDM_KILL_SESSION:
		row = m_data->m_rowarray->GetRowExAt(m_data->m_selrow);
		id = atoi(row->m_row[0]);
		OnKillSession(id);
		RefreshDataView();
		break;
	case ID_RESULT_INSERT:
	case ID_RESULT_DELETE:
	case IDC_SETNULL:
	case IDC_SETDEF:
	case IDC_SETEMPTY:
	case ID_RESULT_SAVE:
		return;
	}

	DataView::OnWMCommand(wparam, lparam);

}

void SessionView::OnKillSession(wyInt32 id)
{
	wyString query;

	query.Sprintf("kill %d", id);
	ExecuteQuery(query);
	Execute(TA_REFRESH, wyTrue, wyTrue, LA_LIMITCLICK);
}