#include "MySQLTableDataEx.h"

//constructor
MySQLTableDataEx::MySQLTableDataEx(MDIWindow* pmdi) : MySQLDataEx(pmdi)
{
	//if OB selection is on table/view, set db and table name
	if (pmdi->m_pcqueryobject->IsSelectionOnTable() == wyTrue)
	{
		m_db.SetAs(pmdi->m_pcqueryobject->m_seldatabase);
		m_table.SetAs(m_pmdi->m_pcqueryobject->m_seltable);
	}

	//get the view
	m_viewtype = HandleViewPersistence(wyFalse);

	//get the limits
	HandleLimitPersistence(wyFalse);

	//create server side sort/filtering
	m_psortfilter = new SortAndFilter(wyFalse, this);
}

//function to handle limit persistence
void
MySQLTableDataEx::HandleLimitPersistence(wyBool isset)
{
	wyInt32 limit = -1;
	wyInt64 startrow = -1;

	//get limit persistence?
	if (isset == wyFalse)
	{
		//get the limits, if it is false, set the values to zero
		if ((m_islimit = HandleDatatabLimitPersistance(m_db.GetString(), m_table.GetString(), (wyInt64*)&m_startrow, (wyInt32*)&m_limit)) == wyFalse)
		{
			m_startrow = 0;
			m_limit = 0;
		}

		//set the values
		m_startrow = max(m_startrow, 0);
		m_limit = max(m_limit, 0);
	}
	//set limit persistence
	else
	{
		//set the values
		if (m_islimit == wyTrue)
		{
			startrow = m_startrow;
			limit = m_limit;
		}

		//store it
		HandleDatatabLimitPersistance(m_db.GetString(), m_table.GetString(), &startrow, &limit, wyTrue);
	}
}

//function to handle view persistence for a table/view
ViewType
MySQLTableDataEx::HandleViewPersistence(wyBool isset)
{
	ViewType    view;

	//precaution
	if (!m_db.GetLength() && !m_table.GetLength())
	{
		return m_viewtype;
	}

	//get/set view persistence
	view = m_viewtype;
	view = (ViewType)HandleTableViewPersistance(m_db.GetString(),
		m_table.GetString(),
		m_viewtype, isset);

	//if it is form view, make sure we have valid licenece
	if (isset == wyFalse && view == FORM)
	{
#ifndef COMMUNITY
		if (!pGlobals->m_entlicense.CompareI("Professional"))
		{
			view = GRID;
		}
#else
		view = GRID;
#endif
	}

	return view;
}
