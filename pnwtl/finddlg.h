/**
 * @file finddlg.h
 * @brief Find and Replace dialogs for PN WTL
 *
 * Note that the dialogs multiply inherit from CWinDataExchange,
 * this adds WTLs DDX support.
 *
 * To do: Implemented a GetClass message for CChildFrame but still
 *        have to keep a pointer to the mainframe. Implement some
 *        kind of global context thing which includes a pointer to
 *        an interface which includes a GetCurrentEditor method.
 */

#ifndef finddlg_h__included
#define finddlg_h__included

/**
 * @class CSearchDlg
 * @brief template class providing basic functionality for the search and replace dialogs.
 */
template <class T, class TBase = CWindow>
class ATL_NO_VTABLE CSearchDlg : public CDialogImpl<T, TBase>
{
public:
	BEGIN_MSG_MAP(CSearchDlg)
	END_MSG_MAP()

protected:
	int DoRegExpInsert(LPCTSTR insert, CString& str, int offset)
	{
		///@todo Make the GetLastSel combo for WTL
		CHARRANGE/*&*/ lastsel /*= m_FindTextCombo.GetLastSel()*/;
		lastsel.cpMax = 0;
		lastsel.cpMin = 0;

		int strl = str.GetLength();
		bool sel = (lastsel.cpMin != lastsel.cpMax);
		
		if(!sel && strl == lastsel.cpMin)
		{
			// We are just adding to the end.
			str += insert;
		}
		else
		{
			if(sel)
			{
				str.Delete(lastsel.cpMin, lastsel.cpMax-lastsel.cpMin);
			}
			str.Insert(lastsel.cpMin, insert);
		}

		int ret = lastsel.cpMin;
		if(offset != 0)
		{
			ret += offset;
		}
		else
			ret += _tcslen(insert);

		return ret;
	}

	int GetRegExpString(int nID, CString& Text)
	{
		int offset = 0;

		switch(nID)
		{
			case ID_REGEXP_ANYCHARACTER:
				Text = _T(".");
				break;
			case ID_REGEXP_CHARACTERINRANGE:
				Text = _T("[]");
				offset = 1;
				break;
			case ID_REGEXP_CHARACTERNOTINRANGE:
				Text = _T("[^]");
				offset = 2;
				break;
			case ID_REGEXP_BEGINNINGOFLINE:
				Text = _T("^");
				break;
			case ID_REGEXP_ENDOFLINE:
				Text = _T("$");
				break;
			case ID_REGEXP_TAGGEDEXPRESSSION:
				Text = _T("\\(\\)");
				offset = 2;
				break;
			case ID_REGEXP_NOT:
				Text = _T("~");
				break;
			case ID_REGEXP_OR:
				Text = _T("\\!");
				break;
			case ID_REGEXP_0ORMOREMATCHES:
				Text = _T("*");
				break;
			case ID_REGEXP_1ORMOREMATCHES:
				Text = _T("+");
				break;
			case ID_REGEXP_GROUP:
				Text = _T("\\{\\}");
				offset = 2;
				break;
		};

		return offset;
	}

	void DoREHelperMenu(LPRECT rc)
	{
		HMENU hRegExpMenu = ::LoadMenu(_Module.m_hInst, MAKEINTRESOURCE(IDR_POPUP_REGEXP));
		HMENU hPopup = ::GetSubMenu(hRegExpMenu, 0);

		::TrackPopupMenu(hPopup, 0, rc->right, rc->top, 0, m_hWnd, NULL);

		::DestroyMenu(hRegExpMenu);
	}

	CChildFrame* GetCurrentEditorWnd()
	{
		return CChildFrame::FromHandle(GetCurrentEditor(m_pParent));
	}

protected:
	CWindow* m_pParent;
};

class CFindDlg : public CSearchDlg<CFindDlg>,
					public CWinDataExchange<CFindDlg>
{
public:
	enum { IDD = IDD_FIND };

	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		return IsDialogMessage(pMsg);
	}

	BEGIN_MSG_MAP(CFindDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
		COMMAND_ID_HANDLER(IDC_FINDNEXT_BUTTON, OnFindNextClicked)
		COMMAND_ID_HANDLER(IDC_REHELPER_BUTTON, OnReHelperClicked)
		COMMAND_RANGE_HANDLER(ID_REGEXP_ANYCHARACTER, ID_REGEXP_GROUP, OnReInsertClicked)
		MESSAGE_HANDLER(WM_SHOWWINDOW, OnShowWindow)
	END_MSG_MAP()

	BEGIN_DDX_MAP(CFindDlg)
		DDX_TEXT(IDC_FINDTEXT_COMBO, m_FindText)
		DDX_CHECK(IDC_MATCHCASE_CHECK, m_bMatchCase)
		DDX_CHECK(IDC_MATCHWHOLE_CHECK, m_bMatchWhole)
		DDX_CHECK(IDC_SEARCHALL_CHECK, m_bSearchAll)
		DDX_CHECK(IDC_REGEXP_CHECK, m_bRegExp)
		DDX_RADIO(IDC_UP_RADIO, m_Direction)
	END_DDX_MAP()

	CFindDlg(CWindow* pParent)
	{
		m_pParent = pParent;
	}

	//DDX_CONTROL(IDC_FINDTEXT_COMBO, m_FindTextCombo)

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		m_Direction = 1;

		m_ReHelperBtn.Attach(GetDlgItem(IDC_REHELPER_BUTTON));
		m_FindTextCombo.Attach(GetDlgItem(IDC_FINDTEXT_COMBO));

		CenterWindow(GetParent());
		return TRUE;
	}

	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		//DoDataExchange(TRUE);
		//EndDialog(wID);
		ShowWindow(SW_HIDE);
		return 0;
	}

	LRESULT OnFindNextClicked(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CChildFrame* editor = GetCurrentEditorWnd();
	
		if(editor != NULL)
		{
			if( editor->FindNext(GetFindOptions()) )
			{
				// Default Visual C++, old PN and others behaviour:
				///@todo implement window stays open behaviour as well...
				ShowWindow(SW_HIDE);
			}
		}
		return TRUE;
	}

	LRESULT OnReHelperClicked(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CRect rc;
		m_ReHelperBtn.GetWindowRect(&rc);
		DoREHelperMenu(rc);

		return TRUE;
	}

	SFindOptions* GetFindOptions()
	{
		DoDataExchange(TRUE);

		SFindOptions* pOptions = theApp.GetFindOptions();

		pOptions->FindText			= m_FindText;
		pOptions->Direction			= (m_Direction == 1);
		pOptions->MatchCase			= (m_bMatchCase == TRUE);
		pOptions->MatchWholeWord	= (m_bMatchWhole == TRUE);
		pOptions->SearchAll			= (m_bSearchAll == TRUE);
		pOptions->UseRegExp			= (m_bRegExp == TRUE);
		
		///@todo Add a user interface counterpart for the loop search option.
		pOptions->Loop				= true;

		return pOptions;
	}

	LRESULT OnShowWindow(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		
		if((BOOL)wParam)
		{
			SFindOptions* pOptions = theApp.GetFindOptions();
			
			m_FindText = pOptions->FindText;
			m_Direction = pOptions->Direction;
			m_bMatchCase = pOptions->MatchCase;
			m_bMatchWhole = pOptions->MatchWholeWord;
			m_bSearchAll = pOptions->SearchAll;
			m_bRegExp = pOptions->UseRegExp;
			
			// Do the funky DDX thang...
			DoDataExchange(FALSE);
		}
		return 0;
	}

	LRESULT OnReInsertClicked(WORD /*wNotifyCode*/, WORD nID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CString Text;
		int offset = GetRegExpString(nID, Text);

		CString str;
		DoDataExchange(TRUE);
		str = m_FindText;
		int pos = DoRegExpInsert(Text, str, offset);
		m_FindTextCombo.SetWindowText(str);
		m_FindTextCombo.SetFocus();
		m_FindTextCombo.SetEditSel(pos, pos);
		
		return TRUE;
	}

protected:
	CContainedWindowT<CComboBox>m_FindTextCombo;
	CContainedWindowT<CButton>	m_ReHelperBtn;
	CString	m_FindText;
	int		m_Direction;
	BOOL	m_bMatchCase;
	BOOL	m_bMatchWhole;
	BOOL	m_bSearchAll;
	BOOL	m_bRegExp;
};

class CReplaceDlg : public CSearchDlg<CReplaceDlg>,
					public CWinDataExchange<CFindDlg>
{
public:
	enum { IDD = IDD_REPLACE };

	BEGIN_MSG_MAP(CReplaceDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
		COMMAND_HANDLER(IDC_REHELPER_BUTTON, BN_CLICKED, OnReHelperClicked)
		COMMAND_HANDLER(IDC_RHELPER_BUTTON, BN_CLICKED, OnReHelper2Clicked)
		COMMAND_RANGE_HANDLER(ID_REGEXP_ANYCHARACTER, ID_REGEXP_GROUP, OnReHelperMenuItemClicked)
		MESSAGE_HANDLER(WM_SHOWWINDOW, OnShowWindow)
		COMMAND_HANDLER(IDC_FINDNEXT_BUTTON, BN_CLICKED, OnFindNextClicked)
		COMMAND_HANDLER(IDC_REPLACE_BUTTON, BN_CLICKED, OnReplaceClicked)
		COMMAND_HANDLER(IDC_REPLACEALL_BUTTON, BN_CLICKED, OnReplaceAllClicked)
		COMMAND_HANDLER(IDC_REPLACEINSEL_BUTTON, BN_CLICKED, OnReplaceInSelectionClicked)
	END_MSG_MAP()

	
	BEGIN_DDX_MAP(CReplaceDlg)
		DDX_TEXT(IDC_FINDTEXT_COMBO, m_FindText)
		DDX_TEXT(IDC_REPLACETEXT_COMBO, m_ReplaceText)
		DDX_CHECK(IDC_MATCHCASE_CHECK, m_bMatchCase)
		DDX_CHECK(IDC_MATCHWHOLE_CHECK, m_bMatchWhole)
		DDX_CHECK(IDC_REGEXP_CHECK, m_bRegExp)
		DDX_CHECK(IDC_BACKSLASH_CHECK, m_bUseSlashes)
		DDX_RADIO(IDC_UP_RADIO, m_Direction)
		//DDX_CHECK(IDC_SEARCHALL_CHECK, m_bSearchAll)
	END_DDX_MAP()

	CReplaceDlg(CWindow* pParent)
	{
		m_pParent = pParent;
	}

	SReplaceOptions* GetOptions()
	{
		DoDataExchange(TRUE);

		SReplaceOptions* pOptions = theApp.GetReplaceOptions();

		pOptions->FindText			= m_FindText;
		pOptions->ReplaceText		= m_ReplaceText;
		pOptions->Direction			= (m_Direction == 1);
		pOptions->MatchCase			= (m_bMatchCase == TRUE);
		pOptions->MatchWholeWord	= (m_bMatchWhole == TRUE);
		//pOptions->SearchAll			= (m_bSearchAll == TRUE);
		pOptions->UseRegExp			= (m_bRegExp == TRUE);
		pOptions->UseSlashes		= (m_bUseSlashes == TRUE);
		pOptions->InSelection		= false;
		
		///@todo Add a user interface counterpart for the loop search option.
		pOptions->Loop				= true;
		
		return pOptions;
	}

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		m_Direction = 1;
		
		m_ReHelperBtn.Attach(GetDlgItem(IDC_REHELPER_BUTTON));
		m_ReHelperBtn2.Attach(GetDlgItem(IDC_RHELPER_BUTTON));
		m_FindTextCombo.Attach(GetDlgItem(IDC_FINDTEXT_COMBO));
		m_ReplaceTextCombo.Attach(GetDlgItem(IDC_REPLACETEXT_COMBO));

		CenterWindow(GetParent());
		return TRUE;
	}

	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		ShowWindow(SW_HIDE);
		return 0;
	}
	
	LRESULT OnReHelperClicked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CRect rc;
		m_ReHelperBtn.GetWindowRect(&rc);
		DoREHelperMenu(rc);

		return TRUE;
	}
	
	LRESULT OnReHelper2Clicked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		return 0;
	}
	
	LRESULT OnReHelperMenuItemClicked(WORD /*wNotifyCode*/, WORD nID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CString Text;
		int offset = GetRegExpString(nID, Text);

		CString str;
		DoDataExchange(TRUE);
		str = m_FindText;
		int pos = DoRegExpInsert(Text, str, offset);
		m_FindTextCombo.SetWindowText(str);
		m_FindTextCombo.SetFocus();
		m_FindTextCombo.SetEditSel(pos, pos);
		
		return 0;
	}

	LRESULT OnShowWindow(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		if((BOOL) wParam)
		{
			SReplaceOptions* pOptions = theApp.GetReplaceOptions();
			
			m_FindText = pOptions->FindText;
			m_ReplaceText = pOptions->ReplaceText;
			m_Direction = pOptions->Direction;
			m_bMatchCase = pOptions->MatchCase;
			m_bMatchWhole = pOptions->MatchWholeWord;
			m_bRegExp = pOptions->UseRegExp;
			m_bUseSlashes = pOptions->UseSlashes;
			//m_bSearchAll = pOptions->SearchAll;

			DoDataExchange(FALSE);
		}

		return 0;
	}

	LRESULT OnFindNextClicked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CChildFrame* pEditor = GetCurrentEditorWnd();

		if(pEditor)
			pEditor->FindNext(GetOptions());

		return 0;
	}
	LRESULT OnReplaceClicked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CChildFrame* pEditor = GetCurrentEditorWnd();
		
		if(pEditor)
			pEditor->Replace(GetOptions());

		return 0;
	}
	LRESULT OnReplaceAllClicked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CChildFrame* pEditor = GetCurrentEditorWnd();
		
		if(pEditor)
			pEditor->ReplaceAll(GetOptions());

		return 0;
	}
	LRESULT OnReplaceInSelectionClicked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CChildFrame* pEditor = GetCurrentEditorWnd();

		if(pEditor)
		{
			SReplaceOptions* pOptions = theApp.GetReplaceOptions();
			pOptions->InSelection = true;

			pEditor->ReplaceAll(pOptions);
		}

		return 0;
	}

protected:
	CContainedWindowT<CComboBox> m_FindTextCombo;
	CContainedWindowT<CComboBox> m_ReplaceTextCombo;
	CContainedWindowT<CButton> m_ReHelperBtn;
	CContainedWindowT<CButton> m_ReHelperBtn2;

	CString	m_FindText;
	CString m_ReplaceText;
	int		m_Direction;
	BOOL	m_bMatchCase;
	BOOL	m_bMatchWhole;
	BOOL	m_bRegExp;
	BOOL	m_bUseSlashes;
	//BOOL	m_bSearchAll;
};

#endif // !finddlg_h_included