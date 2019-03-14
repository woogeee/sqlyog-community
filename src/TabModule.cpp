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

#include "FrameWindow.h"
#include "TabModule.h"
#include "MDIWindow.h"
#include "Global.h"
#include "TabTypes.h"
#include "TabEditor.h"
#include "EditorBase.h"
#include "GUIHelper.h"
#include "EditorFont.h"
#include "TabEditorSplitter.h"
#include "AutoCompleteInterface.h"
#include "TableTabInterface.h"
#include "TableTabInterfaceTabMgmt.h"
#include "TabPreview.h"
#include "TabTableData.h"
#include "TableView.h"
#include "TabIndexes.h"
#include "TabForeignKeys.h"


#ifndef COMMUNITY
#include "HelperEnt.h"
#include "TabQueryBuilder.h"
#include "TabSchemaDesigner.h" 
#include "DatabaseSearch.h"
#else
#include "CommunityRibbon.h"
#endif

#include <scilexer.h>


#define IDC_CTAB		WM_USER+116
extern	PGLOBALS		pGlobals;
#define	SIZE_24	        24
#define	SIZE_12	        12


//constructor sets the window handle
TabModule::TabModule(HWND hwnd)
{
	m_hwndparent    = hwnd;

	m_hwnd          = NULL;
    m_historydata   = NULL;
    m_parentptr     = NULL;
    m_pceditorquery = NULL;
    m_pcadvedit     = NULL;
    m_historydata   = NULL;
    m_pctabqb       = NULL;
    m_pctabeditor   = NULL;
	m_pctabhistory	= NULL;
	m_pcsessionbrowser = NULL;
	m_pcinfotab		= NULL;
    m_isdefault     = wyFalse;
	m_hwndcommtytitle = NULL;

	m_infostatusres = NULL;
	m_infovariablesres = NULL;

	m_istabcreate = wyFalse;
	
	m_tableview = NULL;
	m_sessionview = NULL;

	//serailnumber = 1;

	List		*m_mdilistfordropdown = new List();

			m_serialnoqb=1;
			m_serialnosd=1;
			m_serialnoet=1;


#ifdef COMMUNITY	
	m_cribbon = NULL;
#endif	
}

TabModule::~TabModule()
{  
#ifdef COMMUNITY	
	if(m_cribbon)
		delete m_cribbon;

#endif

	if(m_hwnd)
	{
		DestroyWindow(m_hwnd);
		m_hwnd  = NULL;
	}

	if(m_hwndcommtytitle)
	{
		DestroyWindow(m_hwndcommtytitle);
		m_hwndcommtytitle  = NULL;
	}
    
    if(m_historydata)
    {
        delete[] m_historydata;
        m_historydata = NULL;
    }		

	
	if(m_infovariablesres)
	{
		m_parentptr->m_tunnel->mysql_free_result(m_infovariablesres);
		m_infovariablesres = NULL;
	}

	if(m_infostatusres)
	{
		m_parentptr->m_tunnel->mysql_free_result(m_infostatusres);
		m_infostatusres = NULL;
	}
	
	if(m_pctabhistory)
	{
		delete m_pctabhistory;
		m_pctabhistory = NULL;
	}

	if(m_tableview)
	{
		delete m_tableview;
		m_tableview = NULL;
	}
}

//function to create Editortab
wyBool
TabModule::Create(MDIWindow * wnd)
{    
    wyInt32		i,totaltabs = 0;
	tabdetailelem  *temptabdetail;
	tabeditorelem  *temptabeditorele;
    m_isdefault     = wyTrue;
	HTREEITEM	hitem = NULL;
    wyString temptest;   
    CreateTabControl();
	CreateCommunityTabHeader();
    CreateHistoryTab(wnd, wyFalse, wyFalse);
	if(!pGlobals->m_conrestore)
		CreateQueryEditorTab(wnd);    
	else
	{
		totaltabs = wnd->m_listtabdetails->GetCount();
		if(totaltabs > 0)
		{
			temptabdetail = (tabdetailelem*)wnd->m_listtabdetails->GetFirst();
			for(i = 0; i < totaltabs ; i++)
			{
			
				temptabeditorele = new tabeditorelem;
				temptabeditorele->m_ispresent = wyTrue;
				temptabeditorele->m_id = temptabdetail->m_id;
				temptabeditorele->m_tabid = temptabdetail->m_tabid;
				//temptabeditorele->m_position = temptabdetail->m_position;
				temptabeditorele->m_position = i;
				temptabeditorele->m_color = temptabdetail->m_color;
				temptabeditorele->m_fgcolor = temptabdetail->m_fgcolor;
				temptabeditorele->m_isfile = temptabdetail->m_isfile;
				temptabeditorele->m_isedited = temptabdetail->m_isedited;
				temptabeditorele->m_isfocussed = temptabdetail->m_isfocussed;
				temptabeditorele->m_leftortoppercent = temptabdetail->m_leftortoppercent;
				temptabeditorele->m_psztext.SetAs(temptabdetail->m_psztext);
				temptabeditorele->m_tooltiptext.SetAs(temptabdetail->m_tooltiptext);
				temptabeditorele->m_iimage = temptabdetail->m_iimage;
				temptabeditorele->m_tabptr = 0;
				temptabeditorele->m_content = temptabdetail->m_content;
				//if not IDI_QUERY_16 create advanced editor
				//CreateAdvEditorTab(wnd, (wyChar *)temptabeditorele->m_psztext.GetString(), temptabeditorele->m_iimage, hfunctionitem, &temptabeditorele->m_psztext);
				if(temptabdetail->m_iimage != IDI_QUERYBUILDER_16 && temptabdetail->m_iimage != IDI_SCHEMADESIGNER_16)
				{
				if(temptabdetail->m_iimage == IDI_QUERY_16 || temptabeditorele->m_isfile)
				{
					CreateQueryEditorTab(wnd);
					if(!temptabeditorele->m_isfile)
					{
						SetTabRename(temptabdetail->m_psztext.GetAsWideChar(),wyFalse,wnd);
					}
				}
				else
				{
					CreateAdvEditorTab(wnd, (wyChar *)temptabeditorele->m_psztext.GetString(), temptabdetail->m_iimage, hitem, &temptabeditorele->m_psztext);
				}
					temptabeditorele->m_tabptr = (wyInt64)m_pctabeditor;
				temptabeditorele->m_pctabeditor = m_pctabeditor;
				m_pctabeditor->m_pcetsplitter->SetLeftTopPercent(temptabeditorele->m_leftortoppercent);
				}
				else
				if(temptabdetail->m_iimage == IDI_QUERYBUILDER_16)
				{
					//create querybuildertab
#ifndef COMMUNITY
					CreateQueryBuilderTab(wnd);
					temptabeditorele->m_tabptr = (wyInt64)m_pctabqb;
#endif
				}
				else
				{
					//create querybuildertab
#ifndef COMMUNITY
					CreateSchemaDesigner(wnd);
					temptabeditorele->m_tabptr = (wyInt64)m_pctabsd;
#endif
				}
				if(temptabeditorele->m_tabptr != 0)
				wnd->m_listtabeditor->Insert(temptabeditorele);
				temptabdetail = (tabdetailelem*)temptabdetail->m_next;
			 
			}
		}
		else
			CreateQueryEditorTab(wnd);    
	}
    if(pGlobals->m_istabledataunderquery == wyFalse &&
        GetTabOpenPersistence(IDI_TABLE) == wyTrue)
    {
        CreateTabDataTab(wnd);
    }

    if(pGlobals->m_isinfotabunderquery == wyFalse && 
        GetTabOpenPersistence(IDI_TABLEINDEX) == wyTrue)
	{
		CreateInfoTab(wnd);
	}

    if(pGlobals->m_ishistoryunderquery == wyFalse)
    {
        CreateHistoryTab(wnd, GetTabOpenPersistence(IDI_HISTORY), wyFalse);
    }

	m_isdefault = wyFalse;
    return wyTrue;
}

//Community title window
wyBool				
TabModule::CreateCommunityTabHeader()
{
#ifdef COMMUNITY
	m_cribbon = new CommunityRibbon();
	if(m_cribbon->CreateRibbon(m_hwndparent) == wyTrue)
		m_hwndcommtytitle = m_cribbon->m_hwnd;

	else
		return wyFalse;
#endif
	return wyTrue;
}

//Function to o create the TabControl & initialise the  'm_hwnd'
void
TabModule::CreateTabControl()
{
	HWND		    hwndtabctrl;
    TABCOLORINFO    ci = {0};

	VERIFY(hwndtabctrl = CreateCustomTab(m_hwndparent, 0, 0, 0, 0, 
										  TabWndProc, (LPARAM)IDC_CTAB));
	CustomTab_EnableDrag(hwndtabctrl, pGlobals->m_pcmainwin->m_hwndmain, wyTrue);
    CustomTab_EnableAddButton(hwndtabctrl, wyTrue);
    CustomTab_SetClosable(hwndtabctrl, wyTrue, 1);

    if(wyTheme::GetTabColors(COLORS_QUERYTAB, &ci))
    {
        CustomTab_SetColorInfo(hwndtabctrl, &ci);
    }

	MoveWindow(hwndtabctrl, 0, 0, 0, 0, TRUE);

	m_hwnd = hwndtabctrl;
	//UpdateWindow(m_hwnd);
    ShowWindow(m_hwnd, SW_HIDE);
    CustomTab_SetIconList(m_hwnd, pGlobals->m_hiconlist);

	return ;
}

LRESULT CALLBACK 
TabModule::TabWndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam, wyBool* pishandled)
{
    MDIWindow*      pmdi = (MDIWindow*)GetWindowLongPtr(GetParent(hwnd), GWLP_USERDATA);
    TabEditor*      ptabeditor;
	CTCITEM	        item = {0};
    wyInt32         seltab, ret;
    TABCOLORINFO    ci;
    COLORREF        clrref;

    *pishandled = wyTrue;

    if((seltab = CustomTab_GetCurSel(hwnd)) != -1)
    {
        item.m_mask = CTBIF_LPARAM | CTBIF_IMAGE;
        CustomTab_GetItem(hwnd, seltab, &item);
    }

    switch(message)
    {
        case WM_NOTIFY:
            if((ret = wyTheme::DrawToolBar(lparam)) != -1)
            {
                return ret;
            }

            if(((LPNMHDR)lparam)->idFrom == IDC_QUERYTAB)
		    {
			    return SendMessage(GetParent(hwnd), WM_NOTIFY, NULL, lparam);
		    }

            if(((LPNMHDR)lparam)->code == NM_GETCOLORS)
            {
                CustomTab_GetColorInfo(hwnd, &ci);
                ((LPNMDVCOLORINFO)lparam)->drawcolor.m_color1 = ci.m_tabtext;
                ((LPNMDVCOLORINFO)lparam)->drawcolor.m_color2 = ci.m_tabbg1;
                return 0;
            }
            else if(((LPNMHDR)lparam)->code == NM_GETHYPERLINKCOLOR)
            {
                CustomTab_GetColorInfo(hwnd, &ci);

                if(ci.m_mask & CTCF_LINK)
                {
                    ((LPNMDVCOLORINFO)lparam)->drawcolor.m_color1 = ci.m_linkcolor;
                }

                return 0;
            }

            if(pmdi && (ptabeditor = pmdi->GetActiveTabEditor()) && ptabeditor->m_peditorbase)
            {
                if(((LPNMHDR)lparam)->hwndFrom == ptabeditor->m_peditorbase->m_hwnd)
                {
                    if(pmdi && pmdi->m_acinterface->OnACNotification(wparam, lparam))
                    {
                        return 1;
                    }            
                    else if(OnScintillaNotification(wparam, lparam, wyTrue))
			        {
                        return 1;
                    }
			    }
            }
			break;

        case UM_SETROWCOUNT:
            pmdi->m_pcquerystatus->AddNumRows(wparam, lparam ? wyTrue : wyFalse);
            return 1;

        case UM_STARTEXECUTION:
            CustomTab_SetClosable(hwnd, wyTrue, CustomTab_GetItemCount(hwnd));
            CustomTab_StartIconAnimation(hwnd, CustomTab_GetCurSel(hwnd));
            SendMessage(pGlobals->m_pcmainwin->m_hwndmain, UM_UPDATEMAINTOOLBAR, WPARAM(0), LPARAM(0)); 
            EnableWindow(pmdi->m_pcqueryobject->m_hwnd, FALSE);
            return 1;

        case UM_ENDEXECUTION:
            CustomTab_SetClosable(hwnd, wyTrue, 1);
            SendMessage(pGlobals->m_pcmainwin->m_hwndmain, UM_UPDATEMAINTOOLBAR, WPARAM(1), LPARAM(0)); 
            EnableWindow(pmdi->m_pcqueryobject->m_hwnd, TRUE);
            CustomTab_StopIconAnimation(hwnd, CustomTab_GetCurSel(hwnd));
            SendMessage(hwnd, UM_SETROWCOUNT, wparam, 0);

            if(lparam)
            {
                SetFocus((HWND)lparam);
            }

            if(item.m_iimage == IDI_TABLE)
            {
                ((TabTableData*)item.m_lparam)->m_isrefreshed = wyTrue;
            }

            return 1;

        case UM_SETSTATUSLINECOL:
            pmdi->m_pcquerystatus->AddLineColNum((HWND)wparam, (wyBool)lparam);
            return 1;

        case WM_CTLCOLORSTATIC:
            if(pmdi && (ptabeditor = pmdi->GetActiveTabEditor()) && 
                ptabeditor->m_peditorbase && ptabeditor->m_peditorbase->m_hwndhelp && (HWND)lparam)
            {
                clrref = SendMessage(ptabeditor->m_peditorbase->m_hwnd, SCI_STYLEGETFORE, STYLE_LINENUMBER, 0);
                SetTextColor((HDC)wparam, clrref);
                SetBkMode((HDC)wparam, TRANSPARENT);
                clrref = SendMessage(ptabeditor->m_peditorbase->m_hwnd, SCI_STYLEGETBACK, STYLE_LINENUMBER, 0);
                SetDCBrushColor((HDC)wparam, clrref);
                return (LRESULT)GetStockObject(DC_BRUSH);
            }

            break;
    }

    *pishandled = wyFalse;
	return 0;
}

// Function creating the normal QueryEditor
wyBool	
TabModule::CreateQueryEditorTab(MDIWindow* wnd, wyInt32 pos, wyBool setfocus,wyBool fromprefencetoggle)
{
	wyInt32				ret, count;
    wyBool              rstatus;
	CTCITEM				item = {0};
	wyString qtabname(""),q("Query");
	wyInt64 serial;
	wyBool found = wyFalse;
	wyInt32 maxsequence = 0;

	//get the max sequence for the query tab
	maxsequence = GetMaxSequence(wnd);
	
	
	serial = maxsequence + 1;//m_serialnoet; //;

	qtabname.Sprintf("%s %d", q.GetString(), serial);

	LPSTR cString = strdup(qtabname.GetString());
	//m_serialnoet++;


	m_istabcreate = wyTrue;
	
	item.m_psztext = cString;//_("Query"); 
	item.m_cchtextmax = qtabname.GetLength(); //strlen(_("Query")); 
	item.m_mask       = CTBIF_IMAGE | CTBIF_TEXT | CTBIF_LPARAM | CTBIF_CMENU | CTBIF_TOOLTIP;
	item.m_iimage     = IDI_QUERY_16;
	item.m_tooltiptext = cString; // _("Query");

	//inserting the window and list of open query tab

	MDIListForDropDrown *p = (MDIListForDropDrown *)pGlobals->m_mdilistfordropdown->GetFirst();
	MDIListForDropDrown *pfound = p;

	while (p)
	{
		if (wnd == p->mdi)
		{
			found = wyTrue;
			pfound = p;
			break;
		}
		p = (MDIListForDropDrown *)p->m_next;
	}
	if (found) {
		ListOfOpenQueryTabs *node2 = new ListOfOpenQueryTabs();
		if (fromprefencetoggle)
		{
			//insert at specified location
			ListOfOpenQueryTabs  *opentab = (ListOfOpenQueryTabs *)p->opentab->GetFirst();
			ListOfOpenQueryTabs  *opentabcurr = (ListOfOpenQueryTabs *)p->opentab->GetFirst();
			wyInt32 tabcount = p->opentab->GetCount();
			wyInt32 i;
			wyString stemp = "";
			for (i = 0; i <= pos; i++)
			{
				opentabcurr = opentab;
				opentab = (ListOfOpenQueryTabs *)opentab->m_next;

			}
			node2->tabname.SetAs(qtabname);
			node2->seqofquerytab = maxsequence + 1;
			node2->tabtype = querytab;
			pfound->opentab->InsertAfter(opentabcurr, node2);

		}
		else
		{
			node2->tabname.SetAs(qtabname);
			node2->seqofquerytab = maxsequence + 1;
			node2->tabtype = querytab;
			pfound->opentab->Insert(node2);
		}

		
	}
	else//its a new window, create a new mdi winodw node and insert into mdiwindow list
	{
		MDIListForDropDrown *node = new MDIListForDropDrown();
		ListOfOpenQueryTabs *node1 = new ListOfOpenQueryTabs();
		node->mdi = wnd;
		node1->tabname.SetAs(qtabname);
		node1->seqofquerytab = maxsequence+1;
		node1->tabtype = querytab;

		node->opentab->Insert(node1);
		pGlobals->m_mdilistfordropdown->Insert(node);

	}

	//end of code

	m_pctabeditor = CreateTabEditor(wnd);
	m_pctabeditor->Create(wnd, NULL, wyTrue);
	
	item.m_lparam = (LPARAM)m_pctabeditor;

	ShowWindow(m_pctabeditor->m_peditorbase->m_hwnd, FALSE);

	count = pos == -1 ? CustomTab_GetItemCount(m_hwnd) : pos;
	VERIFY((ret = CustomTab_InsertItem(m_hwnd, count, &item))!= -1);

	count = CustomTab_GetItemCount(m_hwnd);

	SendMessage(m_hwnd, WM_SETREDRAW, FALSE, NULL);

	if(count > 1 && setfocus == wyTrue)
    {		
        CustomTab_SetCurSel(m_hwnd, pos == -1 ? count - 1 : pos);
		rstatus = CustomTab_EnsureVisible(m_hwnd, pos == -1 ? count - 1 : pos, wyFalse);
				
        if(rstatus == wyFalse)
            CustomTab_EnsureVisible(m_hwnd, 0, wyFalse);
    }

	Resize();

	//Add tool tip
	if(count == 1)
	{
		CreateCustomTabTooltip(m_hwnd);
	}

	m_istabcreate = wyFalse;
	return wyTrue;
}

wyInt32
TabModule::GetMaxSequence(MDIWindow* wnd)
{
	wyInt64  tabcount,i,currseq=0,nextseq=0,maxseq=0;
	wyBool found = wyFalse;
	ListOfOpenQueryTabs *opentab,*curr;
	ListofOpenTabs *listofopentabs;

	MDIListForDropDrown *p = (MDIListForDropDrown *)pGlobals->m_mdilistfordropdown->GetFirst();
	MDIListForDropDrown *pfound = p;

	if (!p)
	{
		return 0;
	}
	if (!wnd)
	{
		return 0;
	}
	while (p)
	{
		if (wnd == p->mdi)
		{
			found = wyTrue;
			pfound = p;
			break;
		}
		p = (MDIListForDropDrown *)p->m_next;
	}
	if (found) {

		listofopentabs = (ListofOpenTabs *)pfound->opentab->GetFirst();
		tabcount = pfound->opentab->GetCount();

		opentab = (ListOfOpenQueryTabs *)pfound->opentab->GetFirst();
		
		//tabcount = p->opentab->GetCount();

		for (i = 0; i < tabcount; i++)
		{
			curr = (ListOfOpenQueryTabs *)listofopentabs;
			currseq = curr->seqofquerytab;
			
			if (curr->tabtype==querytab && currseq>maxseq)
			{
				maxseq = currseq;
			}
			listofopentabs = (ListofOpenTabs *)listofopentabs->m_next;
		}
	}
	return maxseq;
}


wyInt32
TabModule::GetMaxSequenceQB(MDIWindow* wnd)
{
	wyInt64  tabcount, i, currseq = 0, nextseq = 0, maxseq = 0;
	wyBool found = wyFalse;
	ListOfOpenQueryTabs *opentab, *curr;
	ListofOpenTabs *listofopentabs;

	MDIListForDropDrown *p = (MDIListForDropDrown *)pGlobals->m_mdilistfordropdown->GetFirst();
	MDIListForDropDrown *pfound = p;

	if (!p)
	{
		return 0;
	}
	if (!wnd)
	{
		return 0;
	}
	while (p)
	{
		if (wnd == p->mdi)
		{
			found = wyTrue;
			pfound = p;
			break;
		}
		p = (MDIListForDropDrown *)p->m_next;
	}
	if (found) {

		listofopentabs = (ListofOpenTabs *)pfound->opentab->GetFirst();
		tabcount = pfound->opentab->GetCount();

		opentab = (ListOfOpenQueryTabs *)pfound->opentab->GetFirst();

		for (i = 0; i < tabcount; i++)
		{
			curr = (ListOfOpenQueryTabs *)listofopentabs;
			currseq = curr->seqofquerybuilder;

			if (curr->tabtype == querybuilder && currseq>maxseq)
			{
				maxseq = currseq;
			}
			listofopentabs = (ListofOpenTabs *)listofopentabs->m_next;
		}
	}
	return maxseq;
}


wyInt32
TabModule::GetMaxSequenceSD(MDIWindow* wnd)
{
	wyInt64  tabcount, i, currseq = 0, nextseq = 0, maxseq = 0;
	wyBool found = wyFalse;
	ListOfOpenQueryTabs *opentab, *curr;
	ListofOpenTabs *listofopentabs;

	MDIListForDropDrown *p = (MDIListForDropDrown *)pGlobals->m_mdilistfordropdown->GetFirst();
	MDIListForDropDrown *pfound = p;

	if (!p)
	{
		return 0;
	}
	if (!wnd)
	{
		return 0;
	}
	while (p)
	{
		if (wnd == p->mdi)
		{
			found = wyTrue;
			pfound = p;
			break;
		}
		p = (MDIListForDropDrown *)p->m_next;
	}
	if (found) {

		listofopentabs = (ListofOpenTabs *)pfound->opentab->GetFirst();
		tabcount = pfound->opentab->GetCount();

		opentab = (ListOfOpenQueryTabs *)pfound->opentab->GetFirst();

		for (i = 0; i < tabcount; i++)
		{
			curr = (ListOfOpenQueryTabs *)listofopentabs;
			currseq = curr->seqofschemadesigner;

			if (curr->tabtype == schemadesigner && currseq>maxseq)
			{
				maxseq = currseq;
			}
			listofopentabs = (ListofOpenTabs *)listofopentabs->m_next;
		}
	}
	return maxseq;
}

wyBool
TabModule::CreateAdvEditorTab(MDIWindow *wnd, wyChar* title, wyInt32 image, HTREEITEM hitem, wyString* strhitemname)
{
	wyInt32				ret, count;
	CTCITEM				item = {0};
	wyString			objectname, tempfilename;

	objectname.SetAs(title);

	 //truncate object name if length is greater than 24
	//take 1st 12 characters .... and last 12 characters
	if(objectname.GetLength() >  SIZE_24)
	{
		tempfilename.SetAs(objectname);

		//take 1st 12 characters
		tempfilename.Strip(objectname.GetLength() - SIZE_12);
		
		tempfilename.Add("...");

		//take last 12 characters
		tempfilename.Add(objectname.Substr((objectname.GetLength() - SIZE_12), SIZE_12));

		objectname.SetAs(tempfilename);
	}

	item.m_psztext    = (wyChar*)objectname.GetString();
	item.m_cchtextmax = objectname.GetLength();
	item.m_mask       = CTBIF_IMAGE | CTBIF_TEXT | CTBIF_LPARAM | CTBIF_CMENU  | CTBIF_TOOLTIP;
	item.m_iimage     = image;
	item.m_tooltiptext = title;
	
	

	m_istabcreate = wyTrue;

	m_pctabeditor = CreateTabEditor(wnd);		// sets the current 'TabEditor' pointer
	m_pctabeditor->Create(wnd, hitem, wyFalse, strhitemname);	// to create EditorProc

	item.m_lparam = (LPARAM)m_pctabeditor;

	ShowWindow(m_pctabeditor->m_hwnd, FALSE);

	count = CustomTab_GetItemCount(m_hwnd);
    //SendMessage(m_hwnd, WM_SETREDRAW, FALSE, NULL);
	VERIFY((ret = CustomTab_InsertItem(m_hwnd, count, &item))!= -1);
	count = CustomTab_GetItemCount(m_hwnd);
	
	if(count > 1)	
    {
        CustomTab_SetCurSel(m_hwnd, count - 1);
	    CustomTab_EnsureVisible(m_hwnd, count - 1, wyFalse);
    }

	//ShowWindow(m_hwnd, TRUE);			

	Resize();
	m_istabcreate = wyFalse;
	
	if(objectname.GetLength()>0)
	UpdateDropDownStruct(wnd, objectname.GetString());

	return wyTrue;

}

//Function to create a new TabEditor tab
TabEditor *
TabModule::CreateTabEditor(MDIWindow * wnd)
{
	TabEditor		*pctabeditor;

	pctabeditor  =	new TabEditor(m_hwnd);

	pctabeditor->SetParentPtr(this);              // to set the tabmodule pointer
		
	return pctabeditor;
}

wyBool
TabModule::CreateQueryBuilderTab(MDIWindow * wnd)
{
#ifndef COMMUNITY
	wyInt32				ret, count = 0;
    wyBool              rstatus;
	CTCITEM				item = {0};
	wyInt64			serialqb=0,maxofqb=0;
	wyBool found = wyFalse;
	wyString qtabname = "",q="Query Builder";
	MDIListForDropDrown *pfound; 
	wyString tname;

	//Get the global list of Query drop down
	MDIListForDropDrown *p = (MDIListForDropDrown *)pGlobals->m_mdilistfordropdown->GetFirst();
	pfound = p;
	maxofqb = GetMaxSequenceQB(wnd);
	serialqb = maxofqb + 1;

	tname.Sprintf("%s %d", q.GetString(), serialqb);
	
	// get the number of tabs
    count = CustomTab_GetItemCount(m_hwnd);

	item.m_psztext = (wyChar *)tname.GetString();// _("Query Builder");
	item.m_cchtextmax = tname.GetLength(); //strlen(_("Query Builder"));
	item.m_mask       = CTBIF_IMAGE | CTBIF_TEXT | CTBIF_LPARAM | CTBIF_CMENU  | CTBIF_TOOLTIP;
	item.m_iimage     = IDI_QUERYBUILDER_16;
	m_pctabqb		  = CreateTabQB(wnd);
	item.m_tooltiptext = (wyChar *)tname.GetString();//_("Query Builder");

	//to transfer the updated name into title
	m_pctabqb->m_tabnameforqb.SetAs(tname.GetString());
	
	if(!m_pctabqb)
		return wyFalse;
			
	if(m_pctabqb->Create(wnd) == wyFalse)
	{
		delete m_pctabqb;
		return wyFalse;
	}

	m_istabcreate = wyTrue;
	
	item.m_lparam     = (LPARAM)m_pctabqb;

	// create new tab
    VERIFY((ret = CustomTab_InsertItem(m_hwnd, count, &item))!= -1);
	
    count = CustomTab_GetItemCount(m_hwnd);

	
	SendMessage(m_hwnd, WM_SETREDRAW, FALSE, NULL);
		
    //Checks whether the QB is creating by default or by explicitly by keyboard or menu
    if(count > 1 && m_isdefault == wyFalse)
    {		
        CustomTab_SetCurSel(m_hwnd, count - 1);
		rstatus = CustomTab_EnsureVisible(m_hwnd, count - 1, wyFalse);
				
        if(rstatus == wyFalse)
            CustomTab_EnsureVisible(m_hwnd, 0, wyFalse);
    }

	//to add query builder to the drop doen structure
	if (p)
	{
		if (wnd)
		{
			while (p)
			{
				if (wnd == p->mdi)
				{
					found = wyTrue;
					pfound = p;
					break;
				}
				p = (MDIListForDropDrown *)p->m_next;
			}

			if (found) {
				ListOfOpenQueryTabs *node2 = new ListOfOpenQueryTabs();
				node2->tabname.SetAs(tname.GetString());
				node2->seqofquerybuilder = maxofqb + 1;
				node2->tabtype = querybuilder;
				pfound->opentab->Insert(node2);
				//loop to get last node in the list and insert the new tab at that position
			}
			else//its a new window, create a new mdi winodw node and insert into mdiwindow list
			{
				MDIListForDropDrown *node = new MDIListForDropDrown();
				ListOfOpenQueryTabs *node1 = new ListOfOpenQueryTabs();
				node->mdi = wnd;
				node1->tabname.SetAs(tname.GetString());
				node1->seqofquerybuilder = maxofqb + 1;
				node1->tabtype = querybuilder;
				node->opentab->Insert(node1);
				pGlobals->m_mdilistfordropdown->Insert(node);

			}

		}
	}
	
    //ShowWindow(m_hwnd , TRUE);		

	Resize();
	m_istabcreate = wyFalse;



#endif
    return wyTrue;
}
#ifndef COMMUNITY
// Function to instantiate the TabQueryBuilder
TabQueryBuilder*
TabModule::CreateTabQB(MDIWindow * wnd)
{
	TabQueryBuilder *ptabqb = NULL;

	ptabqb = new TabQueryBuilder(m_hwnd);
	ptabqb->SetParentPtr(this); 
	
	return ptabqb;
}
#endif

wyBool
TabModule::IsValidFocusInOB(wyInt32 subtabindex)
{
    wyBool  ret = wyTrue;
    MDIWindow*  wnd = GetActiveWin();
    wyInt32     image = -1;

    image = wnd->m_pcqueryobject->GetSelectionImage();

    switch(image)
    {
    case NFOLDER:
        {
            HTREEITEM   hitem;
            wyWChar     data[SIZE_64 + 1];
            wyString    foldertext;

            hitem = TreeView_GetSelection(wnd->m_pcqueryobject->m_hwnd);
            GetNodeText(wnd->m_pcqueryobject->m_hwnd, hitem, data, SIZE_64);

            foldertext.SetAs(data);

            if(foldertext.Compare("Columns") == 0)
            {
                //if(subtabindex != 0 && subtabindex != -1)
                ret = wyTrue;
            }
            else if(foldertext.Compare("Indexes") == 0)
            {
                //if(subtabindex != 1)
                ret = wyTrue;
            }
            else
                ret = wyFalse;
        }
        break;

    case NTABLE:
    case NINDEX:
    case NPRIMARYINDEX:
        ret = wyTrue;
        break;
        
    case NCOLUMN:
    case NPRIMARYKEY:
        if(subtabindex != 0 && subtabindex != -1)
            ret = wyFalse;
        break;

    default:
        ret = wyFalse;
    }

    return ret;
}

wyString
TabModule::SetTabSequence(wyString s,MDIWindow *wnd)
{
	//MDIWindow *p = GetActiveWin();
	wyInt32 maxseq = 0;
	wyString tabname;

	maxseq=GetMaxSequenceQB(wnd);

	//m_serialnoqb;
	tabname.Sprintf("%s %d", s.GetString(), maxseq+1);
	//m_serialnoqb++;
	return tabname.GetString();
}


wyBool
TabModule::CreateTableTabInterface(MDIWindow *wnd, wyBool isaltertable, wyInt32 setfocustotab)
{
    wyInt32             ret, count = 0, tabindex = -1;
    wyBool              rstatus,retval;
    CTCITEM             item = {0};
    TableTabInterface   *ptabinterface = NULL;
    wyString            tabtitle, objectname, temptabname;
 
#ifndef COMMUNITY
	if(wnd->m_conninfo.m_isreadonly == wyTrue)
	{
		return wyTrue;
	}
#endif
    if(isaltertable)
    {
        if(!IsValidFocusInOB(setfocustotab))
            return wyFalse;

        if(wnd->m_pcqueryobject->m_seltable.GetLength())
            tabtitle.SetAs(wnd->m_pcqueryobject->m_seltable);

        if(IsAlterTableTabOpen(tabtitle, tabindex))
        {
            if(CustomTab_GetCurSel(m_hwnd) != tabindex)
            {
                CustomTab_SetCurSel(m_hwnd, tabindex);
                CustomTab_EnsureVisible(m_hwnd, tabindex);
            }
            

            CTCITEM             tmpitem = {0};
            tmpitem.m_mask = CTBIF_IMAGE | CTBIF_LPARAM;

            CustomTab_GetItem(m_hwnd, tabindex, &tmpitem);

            TableTabInterface* tmptabint =  NULL;
            tmptabint = (TableTabInterface*) tmpitem.m_lparam;
            /*
            if(tmptabint->m_ptabintmgmt->GetActiveTabImage() == IDI_COLUMN && (setfocustotab == TABCOLUMNS))
                return wyTrue;

            if(tmptabint->m_ptabintmgmt->GetActiveTabImage() == IDI_TABIMG_INDEXES && (setfocustotab == TABINDEXES))
                return wyTrue;

            if(tmptabint->m_ptabintmgmt->GetActiveTabImage() == IDI_TABIMG_FOREIGNKEYS && (setfocustotab == TABFK))
                return wyTrue;

            if(tmptabint->m_ptabintmgmt->GetActiveTabImage() == IDI_TABLEOPTIONS && (setfocustotab == TABADVANCED))
                return wyTrue;
            */
            /*
            if(setfocustotab == -1)
                setfocustotab = 0;
            */
            if(setfocustotab != -1 && CustomTab_GetCurSel(tmptabint->m_ptabintmgmt->m_hwnd) != setfocustotab)
                tmptabint->m_ptabintmgmt->SelectTab(setfocustotab);

            tmptabint->SetInitFocus();
            return wyTrue;
        }
    }
    else
        tabtitle.SetAs(_("New Table"));

    objectname.SetAs(tabtitle);

    if(objectname.GetLength() >  SIZE_24)
	{
		temptabname.SetAs(objectname);

		//take 1st 12 characters
		temptabname.Strip(objectname.GetLength() - SIZE_12);
		
		temptabname.Add("...");

		//take last 12 characters
		temptabname.Add(objectname.Substr((objectname.GetLength() - SIZE_12), SIZE_12));

		objectname.SetAs(temptabname);
	}
    
    wyString    tooltipstr;

    if(isaltertable)
    {
        wyString    dbstr, tblstr;
        dbstr.SetAs(wnd->m_pcqueryobject->m_seldatabase);
        tblstr.SetAs(wnd->m_pcqueryobject->m_seltable);

        dbstr.FindAndReplace("`", "``");
        tblstr.FindAndReplace("`", "``");

        tooltipstr.Sprintf("`%s`.`%s`", dbstr.GetString(), tblstr.GetString());
    }
    else
    {
        tooltipstr.SetAs(_("New Table"));
    }

    count = CustomTab_GetItemCount(m_hwnd);
    m_istabcreate = wyTrue;
    item.m_tooltiptext = (wyChar*)tooltipstr.GetString(); //_("New Table");
    //tabtitle.SetAs(objectname);
    item.m_psztext = (wyChar*)objectname.GetString(); //_("New Table");
    item.m_cchtextmax = tabtitle.GetLength();
    item.m_mask = CTBIF_IMAGE | CTBIF_TEXT | CTBIF_LPARAM | CTBIF_TOOLTIP;
    if(isaltertable)
        item.m_iimage = IDI_ALTERTABLE;
    else
        item.m_iimage = IDI_CREATETABLE;
    
	SendMessage(m_hwnd, WM_SETREDRAW, FALSE, NULL);
    //ptabinterface = new TableTabInterface(m_hwnd);
    
	ptabinterface = new TableTabInterface(m_hwnd, wyFalse, isaltertable, setfocustotab);
    ptabinterface->SetParentPtr(this);
    if(ptabinterface)
    {
        retval = ptabinterface->Create();
        if(retval == wyFalse)
        {
            SendMessage(m_hwnd, WM_SETREDRAW, TRUE, 0);
            delete ptabinterface;
            return wyFalse;
        }

        item.m_lparam     = (LPARAM)ptabinterface;

        // create new tab
		VERIFY((ret = CustomTab_InsertItem(m_hwnd, count, &item))!= -1);		
		count = CustomTab_GetItemCount(m_hwnd);

		if(count > 1 && m_isdefault == wyFalse)
		{			
            CustomTab_SetCurSel(m_hwnd, count - 1);
            //CustomTab_SetCurSel(ptabinterface->m_ptabinterfacetabmgmt->m_hwnd, 0);      //Remove this code line after developing Fields Tab
			rstatus = CustomTab_EnsureVisible(m_hwnd, count - 1, wyFalse);
					
			if(rstatus == wyFalse)
				CustomTab_EnsureVisible(m_hwnd, 0, wyFalse);
		}

        //ShowWindow(m_hwnd, TRUE);			
		Resize();
		m_istabcreate = wyFalse;
		UpdateDropDownStruct(wnd, tabtitle.GetString());

		return wyTrue;
    }

    return wyFalse;
}

void
TabModule::UpdateDropDownStruct(MDIWindow *wnd,wyString tabname)
{
	//to initialise the structure for drop down
	MDIListForDropDrown *p = (MDIListForDropDrown *)pGlobals->m_mdilistfordropdown->GetFirst();
	wyBool found = wyFalse;
	MDIListForDropDrown *pfound = p;
	ListOfOpenQueryTabs *newnode;

	if (!p)
	{
		return;
	}
	if (!wnd)
	{
		return;
	}

	//To search for the particular tab
	while (p)
	{
		if (wnd == p->mdi)
		{
			found = wyTrue;
			pfound = p;
			break;
		}
		p = (MDIListForDropDrown *)p->m_next;
	}

	if (found) {

		//search for the particular tab which is modified

		newnode = new ListOfOpenQueryTabs();
		newnode->tabname.SetAs(tabname);
		pfound->opentab->Insert(newnode);
	}
}


wyBool
TabModule::IsAlterTableTabOpen(wyString& tblname, wyInt32& tabindex)
{
    wyUInt32            count = 0;
    CTCITEM             item = {0};
    TableTabInterface   *ptabint = NULL;

    count   =   CustomTab_GetItemCount(m_hwnd);

    item.m_mask = CTBIF_IMAGE | CTBIF_LPARAM;

    for(int i=0; i<count; i++)
    {
        tabindex = i;
        CustomTab_GetItem(m_hwnd, i, &item);
        switch(item.m_iimage)
        {
        case IDI_ALTERTABLE:
            ptabint = (TableTabInterface*) item.m_lparam;
            if(ptabint->m_origtblname.Compare(GetActiveWin()->m_pcqueryobject->m_seltable) == 0 &&
                ptabint->m_dbname.Compare(GetActiveWin()->m_pcqueryobject->m_seldatabase) == 0)
            {
                return wyTrue;
            }
            break;
        }
    }
    
    tabindex = -1;
    return wyFalse;
}

//Handle to create SchemaDesigner
wyBool
TabModule::CreateSchemaDesigner(MDIWindow * wnd)
{
#ifndef COMMUNITY
    wyInt32				ret, count = 0;
	wyBool				rstatus = wyFalse, retval;
	CTCITEM				item = {0};
	TabSchemaDesigner	*ptabschemadesigner = NULL;
	wyInt64			serialsd,maxofsd=0;

	maxofsd = GetMaxSequenceSD(wnd);
	serialsd = maxofsd + 1;
	
	MDIListForDropDrown *p = (MDIListForDropDrown *)pGlobals->m_mdilistfordropdown->GetFirst();
	wyBool found = wyFalse;
	wyString qtabname = "",q= _("Schema Designer");
	MDIListForDropDrown *pfound = p;
	
	wyString s;
	s.Sprintf("%s %d", q.GetString(), serialsd);
	
	// get the number of tabs
    count = CustomTab_GetItemCount(m_hwnd);

	item.m_psztext = (wyChar *)s.GetString(); //_("Schema Designer");
	item.m_cchtextmax = s.GetLength(); //strlen(_("Schema Designer"));
	item.m_mask       = CTBIF_IMAGE | CTBIF_TEXT | CTBIF_LPARAM | CTBIF_CMENU  | CTBIF_TOOLTIP;
	item.m_iimage     = IDI_SCHEMADESIGNER_16;
	item.m_tooltiptext = (wyChar *)s.GetString();//cString;//_("Schema Designer");
	
	

	ptabschemadesigner = new TabSchemaDesigner(m_hwnd);
	m_pctabsd = ptabschemadesigner;
	if(ptabschemadesigner)
	{				
		retval = ptabschemadesigner->Create(wnd);
		if(retval == wyFalse)
		{
			delete ptabschemadesigner;
			return wyFalse;
		}

		m_istabcreate = wyTrue;

		//to transfer the updated name into title
		ptabschemadesigner->m_tabnamefordropdown.SetAs(s.GetString());

		ptabschemadesigner->SetParentPtr(this);

		item.m_lparam     = (LPARAM)ptabschemadesigner;

		// create new tab
		VERIFY((ret = CustomTab_InsertItem(m_hwnd, count, &item))!= -1);		
		count = CustomTab_GetItemCount(m_hwnd);

		
		
		SendMessage(m_hwnd, WM_SETREDRAW, FALSE, NULL);

		if(count > 1 && m_isdefault == wyFalse)
		{			
            CustomTab_SetCurSel(m_hwnd, count - 1);
			rstatus = CustomTab_EnsureVisible(m_hwnd, count - 1, wyFalse);
					
			if(rstatus == wyFalse)
				CustomTab_EnsureVisible(m_hwnd, 0, wyFalse);
		}

		if (p)
		{
			if (wnd)
			{
				//to add query builder to the drop doen structure
				while (p)
				{
					if (wnd == p->mdi)
					{
						found = wyTrue;
						pfound = p;
						break;
					}
					p = (MDIListForDropDrown *)p->m_next;
				}

				if (found) {
					ListOfOpenQueryTabs *node2 = new ListOfOpenQueryTabs();
					node2->tabname.SetAs(s.GetString());
					node2->seqofschemadesigner = maxofsd + 1;
					node2->tabtype = schemadesigner;
					pfound->opentab->Insert(node2);
					//loop to get last node in the list and insert the new tab at that position
				}
				else//its a new window, create a new mdi winodw node and insert into mdiwindow list
				{
					MDIListForDropDrown *node = new MDIListForDropDrown();
					ListOfOpenQueryTabs *node1 = new ListOfOpenQueryTabs();
					node->mdi = wnd;
					node1->tabname.SetAs(s.GetString());
					node1->seqofschemadesigner = maxofsd + 1;
					node1->tabtype = schemadesigner;

					node->opentab->Insert(node1);
					pGlobals->m_mdilistfordropdown->Insert(node);

				}

			}
		}
		
		//ShowWindow(m_hwnd, TRUE);			
		Resize();		
		m_istabcreate = wyFalse;

		return wyTrue;
	}
#endif

	return wyFalse;
}

//Search tab
wyBool
TabModule::CreateDatabaseSearchTab(MDIWindow * wnd, wyBool isdefault)
{
#ifndef COMMUNITY	
	TabDbSearch  *pdbsearch = NULL;
	
	pdbsearch = new(std::nothrow)TabDbSearch(wnd);
	if(!pdbsearch )
	{
		return wyFalse;
	}
	
	SendMessage(m_hwnd, WM_SETREDRAW, FALSE, NULL);

	pdbsearch->SetParentPtr(this);
	pdbsearch->CreateInterface(isdefault);

	Resize();
#endif
	return wyTrue;
}

VOID 
TabModule::InsertTabIntoDropDownStruct(MDIWindow * wnd,pretabtype tabtype)
{
	wyBool found = wyFalse;
	MDIListForDropDrown *p = (MDIListForDropDrown *)pGlobals->m_mdilistfordropdown->GetFirst();
	if (!p)
	{
		return;
	}
	if (!wnd)
	{
		return;
	}
	while (p)
	{
		if (wnd == p->mdi)
		{
			found = wyTrue;
			//pfound = p;
			break;
		}
		p = (MDIListForDropDrown *)p->m_next;
	}
	if (found) {
		ListOfOpenQueryTabs *newnode = new ListOfOpenQueryTabs();
		if (tabtype = ishistory)
		{
			newnode->tabname.SetAs("History");
		}
		else if (tabtype = isinfo)
		{
			newnode->tabname.SetAs("Info");
		}
		else if (tabtype = isdata)
		{
			newnode->tabname.SetAs("Table Data");
		}
		p->opentab->Insert(newnode);
	}
}

//History tab
wyBool
TabModule::CreateHistoryTab(MDIWindow * wnd, wyBool showtab, wyBool setfocus)
{
    TabEditor* pte;
	wyBool found = wyFalse;

    if(!m_pctabhistory)
	{
		m_pctabhistory = new(std::nothrow)TabHistory(m_hwnd, wnd);
		
        if(!m_pctabhistory )
		{
			return wyFalse;
		}

		m_pctabhistory->Create();
    }

    if(showtab)
	{
        if(pGlobals->m_ishistoryunderquery == wyTrue)
        {
            if(!(pte = GetActiveTabEditor()))
            {
                CreateQueryEditorTab(wnd);
                pte = GetActiveTabEditor();
            }

            pte->m_pctabmgmt->SelectFixedTab(IDI_HISTORY);
        }
        else
        {
            m_pctabhistory->Resize();
            m_pctabhistory->Show(setfocus);
            GetTabOpenPersistence(IDI_HISTORY, wyTrue, wyTrue);
			if (setfocus == wyFalse)
			{
				MDIListForDropDrown *p = (MDIListForDropDrown *)pGlobals->m_mdilistfordropdown->GetFirst();
				if (p)
				{
					if (wnd)
					{
						while (p)
						{
							if (wnd == p->mdi)
							{
								found = wyTrue;
								//pfound = p;
								break;
							}
							p = (MDIListForDropDrown *)p->m_next;
						}
						if (found) {
							ListOfOpenQueryTabs *newhistorynode = new ListOfOpenQueryTabs();
							newhistorynode->tabname.SetAs("History");
							p->opentab->Insert(newhistorynode);
						}
					}
				}
				
			}
        }
	}

	

	return wyTrue;
}

//Session Browser tab
wyBool
TabModule::CreateSessionBrowserTab(MDIWindow * wnd, wyBool isnewtab, wyBool setfocus)
{
	wyInt32             count;
	wyInt32				i, seltab;
	CTCITEM				tmp = { 0 };
	wyString			buffer;
	TabSessionBrowser*       temptab = NULL;
	MySQLTableDataEx*   tempdata;
	wyChar              buff[140];
	wyBool              istabfound = wyFalse, istableselected = wyFalse;
	TabEditor*          pte;

	if (isnewtab == wyFalse && pGlobals->m_istabledataunderquery == wyTrue)
	{
		if (!(pte = GetActiveTabEditor()))
		{
			CreateQueryEditorTab(wnd);
			pte = GetActiveTabEditor();
		}

		pte->m_pctabmgmt->SelectFixedTab(IDI_TABLE);
		return wyTrue;
	}

	if (!m_sessionview)
	{
		m_sessionview = new SessionView(wnd, m_hwnd);
		m_sessionview->Create();
		m_sessionview->Resize();
	}

	seltab = CustomTab_GetCurSel(m_hwnd);
	count = CustomTab_GetItemCount(m_hwnd);
	tmp.m_mask = CTBIF_IMAGE | CTBIF_LPARAM | CTBIF_TOOLTIP;
	tmp.m_tooltiptext = buff;
	buffer.Sprintf("Session Browser");
	
	if (wnd->m_pcqueryobject->IsSelectionOnTable() == wyTrue)
	{
		istableselected = wyTrue;
		//buffer.Sprintf("`%s`.`%s`", wnd->m_pcqueryobject->m_seldatabase.GetString(), wnd->m_pcqueryobject->m_seltable.GetString());
		buffer.Sprintf("Session Browser");
	}
	else
	{
		buffer.Sprintf(_("Table Data"));
	}

	/*
	for (i = 0; i < count; i++)
	{
		CustomTab_GetItem(m_hwnd, i, &tmp);

		if (tmp.m_iimage == IDI_TABLE && ((temptab = (TabSessionBrowser*)tmp.m_lparam))->m_istabsticky == isnewtab && isnewtab == wyFalse)
		{
			if (istableselected == wyTrue &&
				(!temptab->m_tabledata ||
					temptab->m_tabledata->m_db.Compare(wnd->m_pcqueryobject->m_seldatabase) ||
					temptab->m_tabledata->m_table.Compare(wnd->m_pcqueryobject->m_seltable)))
			{
				tempdata = temptab->m_tabledata;
				temptab->m_tabledata = new MySQLTableDataEx(wnd);
				temptab->m_tableview->SetData(temptab->m_tabledata);
				delete tempdata;
				//buffer.Sprintf("`%s`.`%s`", temptab->m_tabledata->m_db.GetString(), temptab->m_tabledata->m_table.GetString());
				buffer.Sprintf("Session Browser");
				tmp.m_mask = CTBIF_TOOLTIP | CTBIF_TEXT | CTBIF_IMAGE | CTBIF_LPARAM;
				tmp.m_tooltiptext = (wyChar*)buffer.GetString();
				tmp.m_psztext = _("Table Data");
				tmp.m_cchtextmax = strlen(_("Table Data"));
				CustomTab_SetItem(m_hwnd, i, &tmp);
				temptab->m_isrefreshed = wyFalse;
			}
			else
			{
				temptab->m_isrefreshed = wyTrue;
			}

			if (setfocus == wyTrue)
			{
				CustomTab_EnsureVisible(m_hwnd, i);

				if (seltab != i)
				{
					CustomTab_SetCurSel(m_hwnd, i);
				}
				else if (temptab->m_isrefreshed == wyFalse)
				{
					temptab->ShowTabContent(i, wyTrue);
				}
			}

			istabfound = wyTrue;
			break;
		}
		else
		{
			if (tmp.m_iimage == IDI_TABLE && ((temptab = (TabSessionBrowser*)tmp.m_lparam))->m_istabsticky == isnewtab
				&& isnewtab == wyTrue && !buffer.Compare(tmp.m_tooltiptext))
			{
				CustomTab_SetCurSel(m_hwnd, i);
				CustomTab_EnsureVisible(m_hwnd, i);
				istabfound = wyTrue;
				return wyTrue;
			}
		}
	}
	*/
	//if (istabfound == wyFalse)
	{
		temptab = new TabSessionBrowser(wnd, m_hwnd, isnewtab);

		if (setfocus == wyFalse)
		{
			delete temptab->m_tabledata;
			temptab->m_tabledata = NULL;
		}

		temptab->CreateTab(setfocus);

		if (isnewtab == wyFalse)
		{
			GetTabOpenPersistence(IDI_TABLE, wyTrue, wyTrue);
		}

		//add the table data tab to drop down
		//AddTabletabIntoDropDown(wnd, temptab, setfocus);
	}
	
	return wyTrue;
}

//Info tab
wyBool
TabModule::CreateInfoTab(MDIWindow* wnd, wyBool setfocus)
{
	wyInt32     i, count;
    CTCITEM     ctci = {0};
    TabObject*  ptabobj;
    TabEditor*  pte;

    if(pGlobals->m_isinfotabunderquery == wyTrue)
    {
        if(!(pte = GetActiveTabEditor()))
        {
            CreateQueryEditorTab(wnd);
            pte = GetActiveTabEditor();
        }

        pte->m_pctabmgmt->SelectFixedTab(IDI_TABLEINDEX);
        return wyTrue;
    }

    count = CustomTab_GetItemCount(m_hwnd);
    
    for(i = 0; i < count; ++i)
    {
        ctci.m_mask = CTBIF_IMAGE;
        CustomTab_GetItem(m_hwnd, i, &ctci);

        if(ctci.m_iimage == IDI_TABLEINDEX)
        {
            if(setfocus == wyTrue)
            {
                CustomTab_SetCurSel(m_hwnd, i, 0);
                CustomTab_EnsureVisible(m_hwnd, i);
            }

            return wyTrue;
        }
    }

	ptabobj = new(std::nothrow) TabObject(m_hwnd, wnd);
    SendMessage(m_hwnd, WM_SETREDRAW, FALSE, NULL);
	count = ptabobj->CreateInfoTab(wnd, m_hwnd);
	Resize();
    GetTabOpenPersistence(IDI_TABLEINDEX, wyTrue, wyTrue);

    if(setfocus)
	{
		CustomTab_SetCurSel(m_hwnd, count);
        CustomTab_EnsureVisible(m_hwnd, count);
	}
	if (setfocus == wyFalse)
	{
		wyBool found = wyFalse;
		MDIListForDropDrown *p = (MDIListForDropDrown *)pGlobals->m_mdilistfordropdown->GetFirst();
		if (p)
		{
			if (wnd)
			{
				while (p)
				{
					if (wnd == p->mdi)
					{
						found = wyTrue;
						//pfound = p;
						break;
					}
					p = (MDIListForDropDrown *)p->m_next;
				}
				if (found) {
					ListOfOpenQueryTabs *newhistorynode = new ListOfOpenQueryTabs();
					newhistorynode->tabname.SetAs("Info");
					p->opentab->Insert(newhistorynode);
				}
			}
		}
		
	}
	return wyTrue;
}

//Info tab
wyBool
TabModule::CreateTabDataTab(MDIWindow * wnd, wyBool isnewtab, wyBool setfocus)
{
	wyInt32             count;
    wyInt32				i, seltab;
    CTCITEM				tmp = {0};
	wyString			buffer;
    TabTableData*       temptab = NULL;
    MySQLTableDataEx*   tempdata;
    wyChar              buff[140];
    wyBool              istabfound = wyFalse, istableselected = wyFalse;
    TabEditor*          pte;

    if(isnewtab == wyFalse && pGlobals->m_istabledataunderquery == wyTrue)
    {
        if(!(pte = GetActiveTabEditor()))
        {
            CreateQueryEditorTab(wnd);
            pte = GetActiveTabEditor();
        }

        pte->m_pctabmgmt->SelectFixedTab(IDI_TABLE);
        return wyTrue;
    }

	if(!m_tableview)
	{
		m_tableview	= new TableView(wnd, m_hwnd);
		m_tableview->Create();
        m_tableview->Resize();
	}
    
	seltab = CustomTab_GetCurSel(m_hwnd);
	count = CustomTab_GetItemCount(m_hwnd);
    tmp.m_mask = CTBIF_IMAGE | CTBIF_LPARAM | CTBIF_TOOLTIP;
    tmp.m_tooltiptext = buff;

    if(wnd->m_pcqueryobject->IsSelectionOnTable() == wyTrue)
    {
        istableselected = wyTrue;
        buffer.Sprintf("`%s`.`%s`", wnd->m_pcqueryobject->m_seldatabase.GetString(), wnd->m_pcqueryobject->m_seltable.GetString());
    }
    else
    {
        buffer.Sprintf(_("Table Data"));
    }

    for(i = 0; i < count; i++)
    {
        CustomTab_GetItem(m_hwnd, i, &tmp);

        if(tmp.m_iimage == IDI_TABLE && ((temptab = (TabTableData*)tmp.m_lparam))->m_istabsticky == isnewtab && isnewtab == wyFalse)
        {
            if(istableselected == wyTrue && 
                (!temptab->m_tabledata || 
                temptab->m_tabledata->m_db.Compare(wnd->m_pcqueryobject->m_seldatabase) ||  
                temptab->m_tabledata->m_table.Compare(wnd->m_pcqueryobject->m_seltable)))
            {
                tempdata = temptab->m_tabledata;
                temptab->m_tabledata = new MySQLTableDataEx(wnd);
                temptab->m_tableview->SetData(temptab->m_tabledata);
                delete tempdata;
                buffer.Sprintf("`%s`.`%s`", temptab->m_tabledata->m_db.GetString(), temptab->m_tabledata->m_table.GetString());
                tmp.m_mask = CTBIF_TOOLTIP | CTBIF_TEXT | CTBIF_IMAGE | CTBIF_LPARAM;
                tmp.m_tooltiptext = (wyChar*)buffer.GetString();
                tmp.m_psztext = _("Table Data");
                tmp.m_cchtextmax = strlen(_("Table Data"));
                CustomTab_SetItem(m_hwnd, i, &tmp);
                temptab->m_isrefreshed = wyFalse;
            }
            else
            {
                temptab->m_isrefreshed = wyTrue;
            }
            
            if(setfocus == wyTrue)
            {
                CustomTab_EnsureVisible(m_hwnd, i);
                    
                if(seltab != i)
                {
                    CustomTab_SetCurSel(m_hwnd, i);
                }
                else if(temptab->m_isrefreshed == wyFalse)
                {
                    temptab->ShowTabContent(i, wyTrue);
                }
            }

            istabfound = wyTrue;
            break;
        }
        else
        {
            if(tmp.m_iimage == IDI_TABLE && ((temptab = (TabTableData*)tmp.m_lparam))->m_istabsticky == isnewtab 
                && isnewtab == wyTrue && !buffer.Compare(tmp.m_tooltiptext))
            {
                CustomTab_SetCurSel(m_hwnd, i);
                CustomTab_EnsureVisible(m_hwnd, i);
                istabfound = wyTrue;
                return wyTrue;
            }
        }
    }

    if(istabfound == wyFalse)
    {
        temptab = new TabTableData(wnd, m_hwnd, isnewtab);

        if(setfocus == wyFalse)
        {
            delete temptab->m_tabledata;
            temptab->m_tabledata = NULL;
        }

        temptab->CreateTab(setfocus);

        if(isnewtab == wyFalse)
        {
            GetTabOpenPersistence(IDI_TABLE, wyTrue, wyTrue);
        }
		
			//add the table data tab to drop down
			AddTabletabIntoDropDown(wnd, temptab, setfocus);
    }
	return wyTrue;
}

VOID 
TabModule::AddTabletabIntoDropDown(MDIWindow *wnd, TabTableData*       temptab, wyBool setfocus)
{
	wyBool found = wyFalse;
	MDIListForDropDrown *p = (MDIListForDropDrown *)pGlobals->m_mdilistfordropdown->GetFirst();
	if (!p)
	{
		return;
	}
	else if (!wnd)
	{
		return;
	}
	
	while (p)
	{
		if (wnd == p->mdi)
		{
			found = wyTrue;
			//pfound = p;
			break;
		}
		p = (MDIListForDropDrown *)p->m_next;
	}
	if (found) {
		ListOfOpenQueryTabs *newnode = new ListOfOpenQueryTabs();
		//newnode->tabname.SetAs("Table Data");
		if (!temptab->m_tabledata)
		{
			if (setfocus == wyFalse) {
				newnode->tabname.SetAs("Table Data");
				p->opentab->Insert(newnode);
			}
		}
		else
		{
			newnode->tabname.SetAs(temptab->m_tabledata->m_table.GetString());
			p->opentab->Insert(newnode);
		}
		
	}
}

/* Resize for Tab
If this fun calls not during the Create any Tab, the m_istabcreate is wyFalse and handling the flickering.
While Creating tab, the Create tab function itself handles the WM_SETREDRAW
*/
void
TabModule::Resize(wyBool issetredraw)  
{
	RECT				rcmain, rcvsplitter;
	wyInt32				hpos, vpos, width, height; 
    wyInt32				tabcount, selindex, headht = 0;
    CTCITEM				item;
	MDIWindow			*pcmdiwindow;
	TabTypes			*ptabtypes;
	FrameWindowSplitter *pcqueryvsplitter;

	pcmdiwindow		 =	(MDIWindow*)GetWindowLongPtr(m_hwndparent, GWLP_USERDATA);
	pcqueryvsplitter = pcmdiwindow->GetVSplitter();

	VERIFY(GetClientRect(m_hwndparent, &rcmain));
	
	VERIFY(GetWindowRect(pcqueryvsplitter->GetHwnd(), &rcvsplitter));

	VERIFY(MapWindowPoints(NULL, m_hwndparent,(LPPOINT)&rcvsplitter, 2));
	
	hpos = (rcvsplitter.right);

	vpos = 2;
	width =	(rcmain.right - hpos) - 2;
	height = rcmain.bottom - vpos - 2;			
	
	if(issetredraw == wyFalse &&  m_istabcreate == wyFalse)
		SendMessage(m_hwnd, WM_SETREDRAW, FALSE, NULL);

#ifdef COMMUNITY
	wyInt32 headvpos = 0;
	
	headvpos = vpos;
    headht = 20;
	VERIFY(MoveWindow(m_hwndcommtytitle, hpos, headvpos, width, headht, TRUE));
    InvalidateRect(m_hwndcommtytitle, NULL, TRUE);
    UpdateWindow(m_hwndcommtytitle);
	height = height - 20;
	
#endif

	vpos = vpos + headht;

	VERIFY(MoveWindow(m_hwnd, hpos, vpos, width, height, TRUE));

    if(m_tableview)
    {
        m_tableview->Resize();
    }

    	
	selindex = CustomTab_GetCurSel(m_hwnd);

	item.m_mask = CTBIF_LPARAM;

	for(tabcount = 0; tabcount < CustomTab_GetItemCount(m_hwnd); tabcount++)
	{
		CustomTab_GetItem(m_hwnd, tabcount, &item);
		ptabtypes =(TabTypes *)item.m_lparam;
				
		ptabtypes->Resize();
		ptabtypes->HandleTabControls(tabcount,  selindex);

		if(m_istabcreate == wyFalse && issetredraw == wyFalse)
			ptabtypes->HandleFlicker();		
	}	

	if(issetredraw == wyFalse)
	{
		SendMessage(m_hwnd, WM_SETREDRAW, TRUE, 0);
		InvalidateRect(m_hwnd, NULL, TRUE);
		UpdateWindow(m_hwnd);
	}

	return;
}

//To get the Active Tab
TabTypes*
TabModule::GetActiveTabType()
{
	CTCITEM         item;
	TabTypes		*ptabtypes;
	wyInt32         itemindex;

    item.m_mask       = CTBIF_IMAGE | CTBIF_LPARAM;

	itemindex	 =	CustomTab_GetCurSel(m_hwnd);
	
	//Avoids handling index of '-1'
	if(itemindex < 0)
        return NULL;

	CustomTab_GetItem(m_hwnd, itemindex, &item);

	ptabtypes = (TabTypes *)item.m_lparam;

	return ptabtypes;
}

TabEditor*
TabModule::GetActiveTabEditor()
{	
    return GetTabEditorAt(CustomTab_GetCurSel(m_hwnd));
}

TabEditor*	
TabModule::GetTabEditorAt(wyInt32 index)
{
    CTCITEM	item = {0};

	if(index < 0)
    {
        return NULL;
    }

    item.m_mask = CTBIF_IMAGE | CTBIF_LPARAM;
	CustomTab_GetItem(m_hwnd, index, &item);

	if(item.m_iimage != IDI_QUERYBUILDER_16 && item.m_iimage != IDI_SCHEMADESIGNER_16 && item.m_iimage != IDI_HISTORY 
		&& item.m_iimage != IDI_TABLEINDEX && item.m_iimage != IDI_CREATETABLE && item.m_iimage != IDI_ALTERTABLE
        && item.m_iimage != IDI_TABLE && item.m_iimage != IDI_DATASEARCH) 
	{
		return (TabEditor*)item.m_lparam;
	}

	return NULL;
}

TabTableData* 
TabModule::GetActiveTabTableData()
{
    CTCITEM	item = {0};
	wyInt32 itemindex;

    item.m_mask = CTBIF_IMAGE | CTBIF_LPARAM;
	itemindex =	CustomTab_GetCurSel(m_hwnd);
	
	if(itemindex < 0)
    {
        return NULL;
    }

	CustomTab_GetItem(m_hwnd, itemindex, &item);

	if(item.m_iimage == IDI_TABLE) 
	{
		return (TabTableData*)item.m_lparam;
	}

    return NULL;
}

TabHistory*
TabModule::GetActiveHistoryTab()
{	
	return m_pctabhistory;
}

TabObject*
TabModule::GetActiveInfoTab()
{	
	CTCITEM	item = {0};
	wyInt32 itemindex;

    item.m_mask = CTBIF_IMAGE | CTBIF_LPARAM;
	itemindex =	CustomTab_GetCurSel(m_hwnd);
	
	if(itemindex < 0)
    {
        return NULL;
    }

	CustomTab_GetItem(m_hwnd, itemindex, &item);

	if(item.m_iimage == IDI_TABLEINDEX) 
	{
		return (TabObject*)item.m_lparam;
	}

    return NULL;
}

wyInt32
TabModule::GetActiveTabImage()
{
	CTCITEM				item;
	wyInt32				itemindex;

    item.m_mask       = CTBIF_IMAGE | CTBIF_LPARAM;

	itemindex	 =	CustomTab_GetCurSel(m_hwnd);
	
	if(itemindex < 0)
        return 0;

	CustomTab_GetItem(m_hwnd, itemindex, &item);

	return item.m_iimage;	
}


LPSTR
TabModule::GetActiveTabText()
{
	CTCITEM				item;
	wyInt32				itemindex;
	wyString			name;

    item.m_mask       = CTBIF_IMAGE | CTBIF_LPARAM;

	itemindex	 =	CustomTab_GetCurSel(m_hwnd);
	
	if(itemindex < 0)
        return 0;

	CustomTab_GetItem(m_hwnd, itemindex, &item);
	return item.m_psztext;	
}


// Set the EditorBase font
void
TabModule::SetTabFont()
{
	CTCITEM         item;

	TabEditor		*pctabeditor;
	TabHistory		*pctabhistory;
	TabObject		*pcinfotab;
    TabTableData    *pctabtabledata;
    TableTabInterface  *pctabint;

#ifndef COMMUNITY
    TabQueryBuilder *ptabbq;
    TabDbSearch     *ptabdbsearch;
#endif

	wyInt32	        itemindex, totalitems;

	totalitems = CustomTab_GetItemCount(m_hwnd);
    item.m_mask  =	CTBIF_LPARAM | CTBIF_IMAGE;

	for(itemindex = 0; itemindex < totalitems; itemindex++)
	{
		CustomTab_GetItem(m_hwnd, itemindex, &item);
		if(item.m_iimage != IDI_QUERYBUILDER_16 && item.m_iimage != IDI_SCHEMADESIGNER_16 && item.m_iimage != IDI_HISTORY 
		    && item.m_iimage != IDI_TABLEINDEX && item.m_iimage != IDI_CREATETABLE && item.m_iimage != IDI_ALTERTABLE
            && item.m_iimage != IDI_TABLE && item.m_iimage != IDI_DATASEARCH)
		//if(item.m_iimage == IDI_QUERY_16)
        {
		    pctabeditor = (TabEditor *)item.m_lparam;
		    EditorFont::SetFont(pctabeditor->m_peditorbase->m_hwnd, "EditFont", wyTrue);
            if(pctabeditor->m_pctabmgmt->m_presultview)
            {
                pctabeditor->m_pctabmgmt->m_presultview->SetAllFonts();
            }
            if(pctabeditor->m_pctabmgmt->m_pcquerymessageedit != NULL)
            {
                pctabeditor->m_pctabmgmt->m_pcquerymessageedit->SetFont();
            }
            if(pctabeditor->m_pctabmgmt->m_ptabletab && pctabeditor->m_pctabmgmt->m_ptabletab->m_ptableview)
            {
                pctabeditor->m_pctabmgmt->m_ptabletab->m_ptableview->SetAllFonts();
            }

            if(pctabeditor->m_pctabmgmt->m_phistory)
            {
                pctabeditor->m_pctabmgmt->m_phistory->SetFont();
            }

            if(pctabeditor->m_pctabmgmt->m_pqueryobj)
            {
                pctabeditor->m_pctabmgmt->m_pqueryobj->m_pobjectinfo->SetFont();
            }
        }
		if(item.m_iimage == IDI_HISTORY)
		{
			pctabhistory = (TabHistory *)item.m_lparam;
		    EditorFont::SetFont(pctabhistory->m_hwnd, "HistoryFont", wyTrue);
		}

		if(item.m_iimage == IDI_TABLEINDEX)
		{
			pcinfotab = (TabObject *)item.m_lparam;
			EditorFont::SetFont(pcinfotab->m_hwnd, "HistoryFont", wyTrue);
		}

        if(item.m_iimage == IDI_TABLE)
        {
            pctabtabledata = (TabTableData*)item.m_lparam;
            pctabtabledata->m_tableview->SetAllFonts();
        }

#ifndef COMMUNITY
        if(item.m_iimage == IDI_QUERYBUILDER_16)
        {        
		    ptabbq = (TabQueryBuilder *)item.m_lparam;
		    EditorFont::SetFont(ptabbq->m_hwndedit, "EditFont", wyTrue);
        }

        if(item.m_iimage == IDI_DATASEARCH)
        {
            ptabdbsearch = (TabDbSearch*)item.m_lparam;
            ptabdbsearch->m_pdataview->SetAllFonts();
        }
#endif
        if(item.m_iimage == IDI_CREATETABLE || item.m_iimage == IDI_ALTERTABLE)
        {
            pctabint = (TableTabInterface *) item.m_lparam;
            EditorFont::SetFont(pctabint->m_ptabintmgmt->m_tabpreview->m_hwndpreview, "HistoryFont", wyTrue);
            pctabint->SetAllFonts();
        }
	}
	return;
}

// Set the EditorBase font color.
void  
TabModule::SetTabFontColor()
{
	CTCITEM         item;
	TabEditor		*pctabeditor;
	TabHistory		*pctabhistory;
	TabObject		*pcinfotab;
    TabTableData    *pctabtabledata;
#ifndef COMMUNITY
    TabQueryBuilder *ptabbq;
    TabDbSearch*    pdbsearch;
#endif
	wyInt32	        itemindex, totalitems;

	totalitems = CustomTab_GetItemCount(m_hwnd);
    item.m_mask  =	CTBIF_LPARAM | CTBIF_IMAGE;

	for(itemindex = 0; itemindex < totalitems; itemindex++)
	{
		CustomTab_GetItem(m_hwnd, itemindex, &item);

        if(item.m_iimage != IDI_QUERYBUILDER_16 && item.m_iimage != IDI_SCHEMADESIGNER_16 && item.m_iimage != IDI_HISTORY 
		    && item.m_iimage != IDI_TABLEINDEX && item.m_iimage != IDI_CREATETABLE && item.m_iimage != IDI_ALTERTABLE
            && item.m_iimage != IDI_TABLE && item.m_iimage != IDI_DATASEARCH) 
        {
		    pctabeditor = (TabEditor *)item.m_lparam;

		    EditorFont::SetColor(pctabeditor->m_peditorbase->m_hwnd, wyTrue);
			EditorFont::SetCase(pctabeditor->m_peditorbase->m_hwnd);
		    EditorFont::SetWordWrap(pctabeditor->m_peditorbase->m_hwnd);

            // Change Folding Colors
            EnableFolding(pctabeditor->m_peditorbase->m_hwnd);

            //Change Brace light and Brace Bad color
            SetParanthesisHighlighting(pctabeditor->m_peditorbase->m_hwnd);

            if(pctabeditor->m_pctabmgmt->m_pcquerymessageedit != NULL)
            {
                pctabeditor->m_pctabmgmt->m_pcquerymessageedit->SetColor();
            }

            if(pctabeditor->m_pctabmgmt->m_presultview)
            {
                pctabeditor->m_pctabmgmt->m_presultview->SetColor();
            }

            if(pctabeditor->m_pctabmgmt->m_ptabletab && pctabeditor->m_pctabmgmt->m_ptabletab->m_ptableview)
            {
                pctabeditor->m_pctabmgmt->m_ptabletab->m_ptableview->SetColor();
            }

            if(pctabeditor->m_pctabmgmt->m_phistory)
            {
                pctabeditor->m_pctabmgmt->m_phistory->SetColor();
            }

            if(pctabeditor->m_pctabmgmt->m_pqueryobj)
            {
                pctabeditor->m_pctabmgmt->m_pqueryobj->m_pobjectinfo->SetColor();
            }

            if(pGlobals->m_isautocompletehelp == wyTrue && pctabeditor->m_peditorbase->m_hwndhelp)
            {
                InvalidateRect(pctabeditor->m_peditorbase->m_hwndhelp,NULL,FALSE);
            }
        }

        if(item.m_iimage == IDI_TABLE)
        {
            pctabtabledata = (TabTableData*)item.m_lparam;
            if(pctabtabledata->m_tableview)
            {
                pctabtabledata->m_tableview->SetColor();
            }
        }

		if(item.m_iimage == IDI_HISTORY)
        {
		    pctabhistory = (TabHistory *)item.m_lparam;
			EditorFont::SetColor(pctabhistory->m_hwnd, wyTrue);
			EditorFont::SetCase(pctabhistory->m_hwnd);
		    EditorFont::SetWordWrap(pctabhistory->m_hwnd);
        }

		if(item.m_iimage == IDI_TABLEINDEX)
		{
			pcinfotab = (TabObject *)item.m_lparam;
            pcinfotab->m_pobjinfo->SetColor();
			EditorFont::SetCase(pcinfotab->m_hwnd);
		    EditorFont::SetWordWrap(pcinfotab->m_hwnd);
        }
       
#ifndef COMMUNITY
        if(item.m_iimage == IDI_QUERYBUILDER_16)
        {
            ptabbq = (TabQueryBuilder *)item.m_lparam;
            EditorFont::SetColor(ptabbq->m_hwndedit, wyTrue);
			EditorFont::SetCase(ptabbq->m_hwndedit);
            EditorFont::SetWordWrap(ptabbq->m_hwndedit);
        }

        if(item.m_iimage == IDI_DATASEARCH)
        {
            pdbsearch = (TabDbSearch*)item.m_lparam;
            pdbsearch->m_pdataview->SetColor();
        }
#endif
        if(item.m_iimage == IDI_CREATETABLE || item.m_iimage == IDI_ALTERTABLE)
        {
            TableTabInterface *ptabint = (TableTabInterface *) item.m_lparam;
            EditorFont::SetColor(ptabint->m_ptabintmgmt->m_tabpreview->m_hwndpreview, wyTrue);
			EditorFont::SetCase(ptabint->m_ptabintmgmt->m_tabpreview->m_hwndpreview);
        }

	}
	return;
}

// Set font color for TabHistory for all TabEditor(s).
void
TabModule::SetHistoryColor()
{
	CTCITEM         item;
	TabEditor		*pctabeditor;
	wyInt32	        itemindex, totalitems;

	totalitems = CustomTab_GetItemCount(m_hwnd);
	item.m_mask  =	CTBIF_LPARAM | CTBIF_IMAGE;

	for(itemindex = 0; itemindex < totalitems; itemindex++)
	{
		CustomTab_GetItem(m_hwnd, itemindex, &item);

        if(item.m_iimage == IDI_QUERYBUILDER_16 || item.m_iimage == IDI_SCHEMADESIGNER_16  || item.m_iimage == IDI_DATASEARCH)
            continue;

        if(item.m_iimage == IDI_CREATETABLE || item.m_iimage == IDI_ALTERTABLE)
        {
            TableTabInterface *ptabint;
            ptabint = (TableTabInterface *)item.m_lparam;
            EditorFont::SetColor(ptabint->m_ptabintmgmt->m_tabpreview->m_hwnd, wyTrue);
		    EditorFont::SetCase(ptabint->m_ptabintmgmt->m_tabpreview->m_hwnd);
            continue;
        }
		pctabeditor = (TabEditor *)item.m_lparam;

}

	return;
}

//Set the TabHistory font common for all TabEditor(s).
void
TabModule::SetHistoryFont()
{
	CTCITEM         item;
	TabTypes		*pctabtype;
	wyInt32	        itemindex, totalitems;

	totalitems = CustomTab_GetItemCount(m_hwnd);
    item.m_mask  =	CTBIF_LPARAM | CTBIF_IMAGE;

	for(itemindex = 0; itemindex < totalitems; itemindex++)
	{
		CustomTab_GetItem(m_hwnd, itemindex, &item);
        
        if(item.m_iimage == IDI_QUERYBUILDER_16 || item.m_iimage == IDI_SCHEMADESIGNER_16 || item.m_iimage == IDI_DATASEARCH)
            continue;

		if(item.m_iimage == IDI_HISTORY || item.m_iimage == IDI_TABLEINDEX)
		{
			pctabtype = (TabTypes *)item.m_lparam;
			EditorFont::SetFont(pctabtype->m_hwnd,  "HistoryFont", wyTrue);
		}

        if(item.m_iimage == IDI_CREATETABLE)
        {
            TableTabInterface *ptabint;
            ptabint = (TableTabInterface *)item.m_lparam;
            EditorFont::SetFont(ptabint->m_ptabintmgmt->m_tabpreview->m_hwnd,  "HistoryFont", wyTrue);
            continue;
        }
	}

	return;
}

void
TabModule::SetBackQuotesOption()
{
#ifndef COMMUNITY
    CTCITEM         item;
    TabQueryBuilder *ptabqb;
    
	wyInt32	        itemindex, totalitems;

	totalitems = CustomTab_GetItemCount(m_hwnd);
    item.m_mask  =	CTBIF_LPARAM | CTBIF_IMAGE;

	for(itemindex = 0; itemindex < totalitems; itemindex++)
	{
		CustomTab_GetItem(m_hwnd, itemindex, &item);
        
        if(item.m_iimage != IDI_QUERYBUILDER_16)
            continue;

        ptabqb = (TabQueryBuilder*)item.m_lparam;

        ptabqb->SetBackTickOption();
        
        //set m_isautomated to prevent OnGenerateQuery() from setting the dirty title
        ptabqb->m_isautomated = wyTrue;
        ptabqb->OnGenerateQuery(0);		
        ptabqb->m_isautomated = wyFalse;
	}
#endif

	return;
}

void
TabModule::Refresh()
{
	wyInt32			tabicon = 0;

	tabicon = GetActiveTabImage();
	if (IDI_CREATETABLE == tabicon || IDI_ALTERTABLE == tabicon)
	{
		TableTabInterfaceTabMgmt *tabintmgmt;
		tabintmgmt = ((TableTabInterface*)GetActiveTabType())->m_ptabintmgmt;
		tabintmgmt->m_tabindexes->Refresh();
		tabintmgmt->m_tabfk->Refresh();

		if (tabintmgmt->GetActiveTabImage() == IDI_TABPREVIEW)
			tabintmgmt->m_tabpreview->GenerateAndSetPreviewContent();
	}

}

// function to handle Edit menu items for each TabEditor ( CTRL+L, CTL+2, CTRL+3).
wyBool
TabModule::HandleCheckMenu(MDIWindow * wnd, wyBool ischecked, wyUInt32 menuid)
{
    long        lstyle;		
	HMENU       hmenu, hsubmenu;

	VERIFY(hmenu = GetMenu(pGlobals->m_pcmainwin->m_hwndmain));
	
	lstyle = GetWindowLongPtr(wnd->m_hwnd, GWL_STYLE);

	if(lstyle & WS_MAXIMIZE && wyTheme::IsSysmenuEnabled(wnd->m_hwnd))
		VERIFY(hsubmenu = GetSubMenu(hmenu, 2));
	else
		VERIFY(hsubmenu = GetSubMenu(hmenu, 1));
	
	lstyle = (ischecked == wyFalse)?(MF_UNCHECKED):(MF_CHECKED);

	CheckMenuItem(hsubmenu, menuid, MF_BYCOMMAND | lstyle);

	return wyTrue;
}

//Setting tab name
void
TabModule::SetTabName(wyWChar *filename, wyBool isshowext, wyBool isedited)
{
	CTCITEM         item;
	wyInt32         itemindex, tabimage;
    wyWChar			fname[MAX_PATH] = {0}, ext[MAX_PATH] = {0};
	wyString		file, extn, tempfilename;
	wyString        path;
	wyString newname;

	tabimage = GetActiveTabImage();
	_wsplitpath(filename, NULL, NULL, fname, ext);

	file.SetAs(fname);
	extn.SetAs(ext);

	path.SetAs(filename);

    //truncate file name if length is greater than 24
	//take 1st 12 characters .... and last 12 characters
	if(file.GetLength() >  SIZE_24)
	{
		tempfilename.SetAs(file);

		//take 1st 12 characters
		tempfilename.Strip(file.GetLength() - SIZE_12);
		
		tempfilename.Add("...");

		//take last 12 characters
		tempfilename.Add(file.Substr((file.GetLength() - SIZE_12), SIZE_12));

		file.SetAs(tempfilename);
	}

	if(isshowext == wyTrue && extn.GetLength())
		file.AddSprintf("%s", extn.GetString());	

	if(isedited == wyTrue)
	{
		file.AddSprintf("%s", "*");
		
	}
	
		
	itemindex	 =	CustomTab_GetCurSel(m_hwnd);
	
	item.m_mask         = CTBIF_TEXT | CTBIF_IMAGE | CTBIF_CMENU | CTBIF_TOOLTIP;
	item.m_psztext      = (wyChar*)file.GetString();
	item.m_cchtextmax   = file.GetLength();	
	item.m_iimage		= tabimage;
	item.m_tooltiptext  = (wyChar*)path.GetString();
	
	newname = file.GetString();
	VERIFY(CustomTab_SetItem(m_hwnd, itemindex, &item));

	UpdateNameinStruc(itemindex, newname.GetString());
	return;	
}

VOID 
TabModule::UpdateNameinStruc(wyInt32 itemindex, wyString newname)
{
	MDIListForDropDrown *pfound = NULL, *p;
	wyBool foundmodifiedtab = wyFalse, found = wyFalse;
	wyInt32 tabcount, tabindexindropdown,tabnumber,tabn;
	wyString s2,sname;

	MDIWindow *wnd = GetActiveWin();

	p = (MDIListForDropDrown *)pGlobals->m_mdilistfordropdown->GetFirst();
	if (wnd)
	{
		//Update the tabname in query drop down
		while (p)
		{
			if (wnd == p->mdi)
			{
				found = wyTrue;
				pfound = p;
				break;
			}
			p = (MDIListForDropDrown *)p->m_next;
		}
		if (found) {
			if (pfound) {

				//get the tab which is modifed
				tabcount = p->opentab->GetCount();

				ListOfOpenQueryTabs *node2 = new ListOfOpenQueryTabs();
				node2 = (ListOfOpenQueryTabs *)p->opentab->GetFirst();
				for (tabindexindropdown = 0; tabindexindropdown < tabcount; tabindexindropdown++)
				{
					if (tabindexindropdown == itemindex)//as indexs for tabs starts from 0 
					{
						foundmodifiedtab = wyTrue;
						break;
					}
					node2 = (ListOfOpenQueryTabs *)node2->m_next;
				}
				if (foundmodifiedtab)
				{
					

					if (node2->tabtype == querytab)
					{
						node2->seqofquerytab = 0;
					}

						if (node2->tabtype == querybuilder)
						{
							s2.SetAs(newname.GetString());
							s2.SetAs(s2.Substr(0, 13));
							s2.LTrim();
							s2.RTrim();

							tabn = s2.Compare("Query Builder");
							if (tabn>0) {
								node2->seqofquerybuilder = 0;
							}
							if (tabn == 0)
							{
								
								sname.SetAs(newname.Substr(13, newname.GetLength()));
								sname.LTrim();
								sname.RTrim();
								tabnumber = sname.GetAsInt32();
								if (tabnumber != node2->seqofquerybuilder)
								{
									node2->seqofquerybuilder = tabnumber;
								}

							}
							
						}
						else if (node2->tabtype == schemadesigner)
						{
							s2.SetAs(newname.GetString());
							s2.SetAs(s2.Substr(0, 15));
							s2.LTrim();
							s2.RTrim();

							tabn = s2.Compare("Schema Designer");
							if (tabn>0) {
								node2->seqofschemadesigner = 0;
							}
							if (tabn == 0)
							{
								
								sname.SetAs(newname.Substr(15, newname.GetLength()));
								sname.LTrim();
								sname.RTrim();
								tabnumber = sname.GetAsInt32();
								if (tabnumber != node2->seqofschemadesigner)
								{
									node2->seqofschemadesigner = tabnumber;
								}
								
							}
							
						}
						else if (node2->tabtype == datasearch)
						{
							node2->seqofdatasearch = 0;
						}
						wnd->m_isfromsaveas = wyFalse;

					node2->tabname.SetAs(newname.GetString());
				}
			}
		}
	}
}

//Setting tab name
void
TabModule::SetTabRename(wyWChar *name, wyBool isedited,MDIWindow *wnd, wyBool isfromrenamedlg)
{
	CTCITEM         item;
	wyInt32         itemindex, tabimage,tabindexindropdown,tabcount=0, tabn,tabnumber;
	wyString		newname;
	wyBool found = wyFalse,foundmodifiedtab=wyFalse;
	wyString qtabname = "",newname1="",tabname1="", sname="",s2="";
	MDIListForDropDrown *pfound=NULL ,*p;
	
//	MDIWindow *wnd = GetActiveWin();
	p = (MDIListForDropDrown *)pGlobals->m_mdilistfordropdown->GetFirst();
	
	tabimage = GetActiveTabImage();
	newname.SetAs(name);
	if(isedited == wyTrue)
	{
		newname.AddSprintf("%s", "*");
		
	}
	
	itemindex	 =	CustomTab_GetCurSel(m_hwnd);
	
	item.m_mask         = CTBIF_TEXT | CTBIF_IMAGE | CTBIF_CMENU | CTBIF_TOOLTIP;
	item.m_psztext      = (wyChar*)newname.GetString();
	item.m_cchtextmax   = newname.GetLength();	
	item.m_iimage		= tabimage;
	item.m_tooltiptext  = (wyChar*)newname.GetString();
	
	//m_pctabeditor->m_tabtitle.SetAs(item.m_psztext);
	VERIFY(CustomTab_SetItem(m_hwnd, itemindex, &item));

	//Update the tabname in query drop down
	while (p)
	{
		if (wnd == p->mdi)
		{
			found = wyTrue;
			pfound = p;
			break;
		}
		p = (MDIListForDropDrown *)p->m_next;
	}
	if (found) {
		if (pfound) {

			//get the tab which is modifed
			tabcount = p->opentab->GetCount();

			ListOfOpenQueryTabs *node2;// = new ListOfOpenQueryTabs();
			node2 = (ListOfOpenQueryTabs *)p->opentab->GetFirst();
			for (tabindexindropdown = 0; tabindexindropdown < tabcount; tabindexindropdown++)
			{
				if (tabindexindropdown == itemindex)//as indexs for tabs starts from 0 
				{
					foundmodifiedtab = wyTrue;
					break;
				}
				node2 = (ListOfOpenQueryTabs *)node2->m_next;
			}
			if (foundmodifiedtab)
			{
				if (isfromrenamedlg || node2->tabname.CompareI(newname.GetString()))
				{ 
					if (node2->tabtype == querytab)
					{
						//Fix to update sequence number from old and new session
						s2.SetAs(newname.GetString());
						s2.SetAs(s2.Substr(0, 5));
						s2.LTrim();
						s2.RTrim();

						tabn = s2.Compare("Query");
						if (tabn > 0) {
							node2->seqofquerytab = 0;
						}
						if (tabn == 0)
						{
							if (newname.GetLength()>5)//tabn=0 : tabname is restored name "Query" and if length>5 : sequence from 13.1.2 session with sequence i.e. restore from old session
							{
								sname.SetAs(newname.Substr(5, newname.GetLength()));
								sname.LTrim();
								sname.RTrim();
								tabnumber = sname.GetAsInt32();
								if (tabnumber != node2->seqofquerytab)
								{
									node2->seqofquerytab = tabnumber;
								}
							}
							else
							{
								node2->seqofquerytab = 0;// length = 5 : no sequence to append i.e.restore from old session
							}

						}
					}

				}
				/*if (isfromrenamedlg || node2->tabname.CompareI(newname.GetString()))
				{
					node2->seqofquerytab = 0;
				}*/
				node2->tabname.SetAs(newname.GetString());
			}

		}
	}

	return;	
}
	
// sets parent MDIwindow pointer.
wyBool	
TabModule::SetParentPtr(MDIWindow *wnd)
{
	m_parentptr = wnd;

	return wyTrue;
}

//Get the parent connection window pointer.
MDIWindow *
TabModule::GetParentPtr()
{
	return m_parentptr;
}

//to get current window handle.
HWND
TabModule::GetHwnd()
{
	return m_hwnd;
}

//to get parent Conn. Window handle.
HWND
TabModule::GetparentHwnd()
{
	return m_hwndparent;
}

