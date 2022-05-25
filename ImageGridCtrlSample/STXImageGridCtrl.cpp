#include "STXImageGridCtrl.h"

//////////////////////////////////////////////////////////////////////////

#ifndef GET_X_LPARAM
#define GET_X_LPARAM(lp)                        ((int)(short)LOWORD(lp))
#endif

#ifndef GET_Y_LPARAM
#define GET_Y_LPARAM(lp)                        ((int)(short)HIWORD(lp))
#endif


//////////////////////////////////////////////////////////////////////////

#define STXIGC_TIMER_ID_ANIMATION				1600
#define STXIGC_TIMER_ID_ITEM_FLOAT_CHECK		1601

#define STXIGC_TIMER_INTERVAL_ITEM_FLOAT_CHECK		1000

#define STXIGC_MEASURE_STRING_WIDTH_FIX		10240


#define STXIGC_ITEM_START_OFFSET_FIX_X		12

//////////////////////////////////////////////////////////////////////////


CSTXImageGridNode::CSTXImageGridNode(CSTXImageGridCtrl *pParentControl)
	: m_pParentControl(pParentControl)
{
	m_ptDragOffset.x = m_ptDragOffset.y = 0;
	m_dwNodeData = 0;
}

CSTXImageGridNode::~CSTXImageGridNode()
{

}

void CSTXImageGridNode::DrawItem(Gdiplus::Graphics *pGraphics, int nGlobalOffsetY, BOOL bSelectedItem)
{
	if (pGraphics == NULL)
		return;

	std::shared_ptr<Gdiplus::Image> imgUse = m_pImgImage;

	if (imgUse == NULL)
		return;

	int nItemWidth = m_pParentControl->GetItemWidth();
	int nItemHeight = m_pParentControl->GetItemHeight();

	Gdiplus::RectF rectItem(0, 0, static_cast<Gdiplus::REAL>(nItemWidth), static_cast<Gdiplus::REAL>(nItemHeight));
	DOUBLE fItemRatio = (DOUBLE)nItemWidth / (DOUBLE)nItemHeight;

	DOUBLE nItemOffsetX = 0;
	m_pAVLeftOffset->GetValue(&nItemOffsetX);
	DOUBLE nItemOffsetY = 0;
	m_pAVTopOffset->GetValue(&nItemOffsetY);

	rectItem.Offset(static_cast<Gdiplus::REAL>(nItemOffsetX), static_cast<Gdiplus::REAL>(nItemOffsetY));
	rectItem.Offset(0, -static_cast<Gdiplus::REAL>(nGlobalOffsetY));

	UINT nImgWidth = imgUse->GetWidth();
	UINT nImgHeight = imgUse->GetHeight();
	Gdiplus::RectF rectImg(0, 0, static_cast<Gdiplus::REAL>(nImgWidth), static_cast<Gdiplus::REAL>(nImgHeight));

	DOUBLE fImageRatio = (DOUBLE)nImgWidth / (DOUBLE)nImgHeight;

	int nItemHeightForImage = nItemHeight;

	if (m_strText.size() > 0)
	{
		nItemHeightForImage -= static_cast<int>(m_pParentControl->m_fTextHeight + 2);
	}

	SIZE sizeContainer = { nItemWidth, nItemHeightForImage };
	SIZE sizeImage = { nImgWidth, nImgHeight };
	SIZE sizeImgDest;
	POINT ptImgOffset;
	CSTXImageGridCtrl::CalculateFitSize(sizeContainer, sizeImage, &sizeImgDest, &ptImgOffset);

	Gdiplus::RectF rectImgDest(static_cast<Gdiplus::REAL>(nItemOffsetX + ptImgOffset.x), static_cast<Gdiplus::REAL>(nItemOffsetY + ptImgOffset.y)
		, static_cast<Gdiplus::REAL>(sizeImgDest.cx), static_cast<Gdiplus::REAL>(sizeImgDest.cy));

	DOUBLE fDropOffsetX = 0;
	m_pAVDropDestOffsetX->GetValue(&fDropOffsetX);
	rectImgDest.Offset(static_cast<Gdiplus::REAL>(m_ptDragOffset.x), static_cast<Gdiplus::REAL>(m_ptDragOffset.y));
	rectImgDest.Offset(static_cast<Gdiplus::REAL>(fDropOffsetX), 0);
	rectImgDest.Offset(0, static_cast<Gdiplus::REAL>(-nGlobalOffsetY));

	DOUBLE fImageOpacity = 0;
	m_pAVOpacity->GetValue(&fImageOpacity);
	fImageOpacity = 1;

	Gdiplus::REAL rOpacity = static_cast<Gdiplus::REAL>(fImageOpacity);
	Gdiplus::ColorMatrix *pCMUse = NULL;
	Gdiplus::ColorMatrix cm = {
		1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, rOpacity, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f, 1.0f
	};

	pCMUse = &cm;

	Gdiplus::ImageAttributes ImgAttr;

	ImgAttr.SetColorMatrix(pCMUse, Gdiplus::ColorMatrixFlagsDefault,
		Gdiplus::ColorAdjustTypeBitmap);

	Gdiplus::RectF rectHoverDest(rectItem);
	rectHoverDest.Inflate(2, 2);
	if (bSelectedItem)
	{
		DrawSelection(pGraphics, &rectHoverDest);
	}

	DrawHover(pGraphics, &rectHoverDest);
	BOOL bStringDrawn = DrawString(pGraphics, &rectHoverDest, bSelectedItem);



	//Apply Click Effect Scale
	DOUBLE fScale = 0;
	m_pAVClickScale->GetValue(&fScale);
	rectImgDest.Inflate(-1 * rectImgDest.Width * (1 - static_cast<Gdiplus::REAL>(fScale)), -1 * rectImgDest.Height * (1 - static_cast<Gdiplus::REAL>(fScale)));

	if (!m_pImgCached)
	{
		Gdiplus::Bitmap* pBitmap = new Gdiplus::Bitmap(static_cast<int>(rectImgDest.Width), static_cast<int>(rectImgDest.Height));
		Gdiplus::Graphics graphics(pBitmap);
		Gdiplus::RectF rect(0, 0, rectImgDest.Width, rectImgDest.Height);

		graphics.DrawImage(imgUse.get(), rect, 0, 0, static_cast<Gdiplus::REAL>(nImgWidth), static_cast<Gdiplus::REAL>(nImgHeight)
			, Gdiplus::UnitPixel, &ImgAttr);


		Gdiplus::Bitmap *pBitmapSrc = pBitmap;
		std::shared_ptr<Gdiplus::CachedBitmap> imgCached(new Gdiplus::CachedBitmap(pBitmapSrc, &graphics));
		m_pImgCached = imgCached;

		delete pBitmap;
	}

	if (m_pImgCached)
		pGraphics->DrawCachedBitmap(m_pImgCached.get(), static_cast<int>(rectImgDest.X), static_cast<int>(rectImgDest.Y));
	//else
	//	pGraphics->DrawImage(imgUse.get(), rectImgDest, 0, 0, static_cast<Gdiplus::REAL>(nImgWidth), static_cast<Gdiplus::REAL>(nImgHeight)
	//		, Gdiplus::UnitPixel, &ImgAttr);
}

void CSTXImageGridNode::DrawHover(Gdiplus::Graphics * pGraphics, Gdiplus::RectF *pItemRect)
{
	COLORREF clrHighlight = RGB(232, 240, 255);
	DOUBLE fHoverOpacity = 0;
	m_pAVHoverOpacity->GetValue(&fHoverOpacity);
	Gdiplus::SolidBrush bkBrush(Gdiplus::Color(static_cast<BYTE>(192 * fHoverOpacity), GetRValue(clrHighlight), GetGValue(clrHighlight), GetBValue(clrHighlight)));

	pGraphics->FillRectangle(&bkBrush, *pItemRect);
}

void CSTXImageGridNode::DrawSelection(Gdiplus::Graphics * pGraphics, Gdiplus::RectF *pItemRect)
{
	//COLORREF clrSelection = RGB(192, 204, 248);
	COLORREF clrSelection = GetSysColor(COLOR_HIGHLIGHT);

	Gdiplus::SolidBrush bkBrush(Gdiplus::Color(255, GetRValue(clrSelection), GetGValue(clrSelection), GetBValue(clrSelection)));

	pGraphics->FillRectangle(&bkBrush, *pItemRect);
}

void CSTXImageGridNode::OnMouseEnter()
{
	//m_bHover = TRUE;

	CComPtr<IUIAnimationStoryboard> pStory;
	m_pParentControl->m_AnimationManager->CreateStoryboard(&pStory);

	CComPtr<IUIAnimationTransition> pTrans;
	m_pParentControl->m_AnimationTransitionLibrary->CreateInstantaneousTransition(1.0, &pTrans);
	pStory->AddTransition(m_pAVHoverOpacity, pTrans);

	pStory->Schedule(m_pParentControl->GetCurrentTime(), NULL);
}

void CSTXImageGridNode::OnMouseLeave()
{
	//m_bHover = FALSE;

	m_pParentControl->ApplySmoothStopTransition(m_pAVHoverOpacity, m_pParentControl->m_nDefaultAnimationDuration / 8.0
		, 0.0);
}

void CSTXImageGridNode::PerformDeleteEffect()
{

}

BOOL CSTXImageGridNode::DrawString(Gdiplus::Graphics * pGraphics, Gdiplus::RectF *pItemRect, BOOL bSelectedItem)
{
	if (m_strText.size() == 0)
		return FALSE;

	Gdiplus::RectF rectTextMain(pItemRect->X, pItemRect->Y + pItemRect->Height - m_pParentControl->m_fTextHeight - 2, pItemRect->Width, m_pParentControl->m_fTextHeight + 2);
	Gdiplus::StringFormat strFormat;

	strFormat.SetFormatFlags(Gdiplus::StringFormatFlagsNoWrap);
	strFormat.SetTrimming(Gdiplus::StringTrimmingEllipsisWord);
	strFormat.SetAlignment(Gdiplus::StringAlignmentCenter);

	/*
	Gdiplus::RectF rectTextMeasured;
	pGraphics->MeasureString(m_strText.c_str(), -1, m_pParentControl->GetDefaultFont(), rectTextMain, &strFormat, &rectTextMeasured);
	int iTextTopOffsetPatch = static_cast<int>((rectTextMain.Height - rectTextMeasured.Height) / 2);
	if (iTextTopOffsetPatch > 0)
		rectTextMain.Y += iTextTopOffsetPatch;
	*/

	DOUBLE fNodeOpacity = 0;
	m_pAVOpacity->GetValue(&fNodeOpacity);
	COLORREF clrTextColor = GetSysColor(COLOR_WINDOWTEXT);

	if (bSelectedItem)
		clrTextColor = GetSysColor(COLOR_HIGHLIGHTTEXT);

	Gdiplus::SolidBrush textBrush(Gdiplus::Color(static_cast<BYTE>(255 * fNodeOpacity), GetRValue(clrTextColor), GetGValue(clrTextColor), GetBValue(clrTextColor)));

	pGraphics->DrawString(m_strText.c_str(), -1, m_pParentControl->m_pDefaultFont, rectTextMain, &strFormat, &textBrush);

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////

LPCTSTR CSTXImageGridCtrl::s_lpszImageGridCtrlClassName = _T("STXImageGridCtrl");

CSTXImageGridCtrl::CSTXImageGridCtrl()
{
	m_AnimationManager.CoCreateInstance(CLSID_UIAnimationManager);
	m_AnimationTimer.CoCreateInstance(CLSID_UIAnimationTimer);
	m_AnimationTransitionLibrary.CoCreateInstance(CLSID_UIAnimationTransitionLibrary);

	m_nDefaultAnimationDuration = 0.4;
	m_AnimationManager->SetManagerEventHandler(this);
	m_hwndControl = NULL;
	m_pDefaultFont = NULL;
	m_rcMargins.left = m_rcMargins.top = m_rcMargins.right = m_rcMargins.bottom = 4;
	m_nItemWidth = 100;
	m_nItemHeight = 100;
	m_nItemSpacingX = 4;
	m_nItemSpacingY = 8;
	COLORREF clrWindow = GetSysColor(COLOR_WINDOW);
	m_clrBackground = Gdiplus::Color(255, GetRValue(clrWindow), GetGValue(clrWindow), GetBValue(clrWindow));
	m_bLButtonDown = FALSE;
	m_ptLButtonDown.x = m_ptLButtonDown.y = -1;
	m_nCurrentDropTargetIndex = -1;
	m_nIndexItemDragging = -1;
	m_nItemPerLine = 1;
	m_nLineCount = 0;
	m_nSelectedItemIndex = -1;
	m_fTextHeight = 0;
}


CSTXImageGridCtrl::~CSTXImageGridCtrl()
{
}

void CSTXImageGridCtrl::RegisterAnimatedTreeCtrlClass()
{
	WNDCLASS wc;
	memset(&wc, 0, sizeof(wc));
	wc.hInstance = GetModuleHandle(NULL);
	wc.lpfnWndProc = STXImageGridWindowProc;
	wc.lpszClassName = s_lpszImageGridCtrlClassName;
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wc.hCursor = ::LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW));

	RegisterClass(&wc);
}

BOOL CSTXImageGridCtrl::Create(LPCTSTR lpszWindowText, DWORD dwStyle, int x, int y, int cx, int cy, HWND hWndParent, UINT nID)
{
	HWND hWnd = CreateWindow(s_lpszImageGridCtrlClassName, lpszWindowText, dwStyle, x, y, cx, cy, hWndParent, (HMENU)nID, GetModuleHandle(NULL), NULL);
	if (hWnd == NULL)
		return FALSE;

	SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)this);
	m_hwndControl = hWnd;

	//	HFONT hFont = (HFONT)::GetStockObject(DEFAULT_GUI_FONT);
	//	if (hFont != NULL)
	//	{
	//		::SendMessage(m_hwndControl, WM_SETFONT, (WPARAM)hFont, FALSE);
	//	}

	GetDefaultFontInfo();

	return TRUE;
}

void CSTXImageGridCtrl::GetDefaultFontInfo()
{
	HFONT hFont = (HFONT)::GetStockObject(DEFAULT_GUI_FONT);
	if (hFont != NULL)
	{
		::GetObject(hFont, sizeof(LOGFONT), &m_lfDefaultFont);
		::DeleteObject(hFont);
		HDC hDC = ::GetDC(m_hwndControl);
		m_pDefaultFont = new Gdiplus::Font(hDC, &m_lfDefaultFont);

		Gdiplus::Graphics g(hDC);

		Gdiplus::RectF rectTextMain(0, 0, STXIGC_MEASURE_STRING_WIDTH_FIX * 2, STXIGC_MEASURE_STRING_WIDTH_FIX);
		Gdiplus::RectF rectTextMeasured;
		Gdiplus::StringFormat strFormat;
		strFormat.SetFormatFlags(Gdiplus::StringFormatFlagsNoWrap);
		
		g.MeasureString(_T("TextToMeasure"), -1, m_pDefaultFont, rectTextMain, &strFormat, &rectTextMeasured);
		m_fTextHeight = rectTextMeasured.Height;

		g.ReleaseHDC(hDC);
		::ReleaseDC(m_hwndControl, hDC);
	}
}

LRESULT CALLBACK CSTXImageGridCtrl::STXImageGridWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CSTXImageGridCtrl *pThis = (CSTXImageGridCtrl*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if (pThis == NULL)
		return DefWindowProc(hwnd, uMsg, wParam, lParam);

	switch (uMsg)
	{
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);
		pThis->OnPaint(hdc);
		EndPaint(hwnd, &ps);
	}
	break;
 	case WM_TIMER:
 		pThis->OnTimer(static_cast<UINT>(wParam));
 		break;
// 	case WM_CLOSE:
// 		PostQuitMessage(0);
// 		break;
// 	case WM_HSCROLL:
// 		pThis->OnHScroll(LOWORD(wParam), HIWORD(wParam), (HWND)lParam);
// 		break;
 	case WM_VSCROLL:
 		pThis->OnVScroll(LOWORD(wParam), HIWORD(wParam), (HWND)lParam);
 		break;
 	case WM_LBUTTONDOWN:
		pThis->OnLButtonDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), wParam);
 		break;
 	case WM_LBUTTONUP:
		pThis->OnLButtonUp(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), wParam);
 		break;
// 	case WM_RBUTTONDOWN:
// 		pThis->OnRButtonDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), wParam);
// 		break;
// 	case WM_RBUTTONUP:
// 		pThis->OnRButtonUp(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), wParam);
// 		break;
 	case WM_LBUTTONDBLCLK:
		pThis->OnLButtonDblClk(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), wParam);
 		break;
 	case WM_MOUSEWHEEL:
 		pThis->OnMouseWheel(GET_KEYSTATE_WPARAM(wParam), GET_WHEEL_DELTA_WPARAM(wParam), LOWORD(lParam), HIWORD(lParam));
 		break;
 	case WM_KEYDOWN:
 		pThis->OnKeyDown(wParam, LOWORD(lParam), HIWORD(lParam));
 		break;
	case WM_MOUSEMOVE:
		pThis->OnMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), wParam);
		break;
 	case WM_MOUSELEAVE:
 		pThis->OnMouseLeave();
 		break;
// 	case WM_SETFOCUS:
// 		pThis->OnSetFocus((HWND)wParam);
// 		break;
// 	case WM_KILLFOCUS:
// 		pThis->OnKillFocus((HWND)wParam);
// 		break;
 	case WM_DESTROY:
 		pThis->OnDestroy();
 		break;
// 	case WM_GETOBJECT:
// 		return pThis->OnGetObject((DWORD)wParam, (DWORD)lParam);
// 		break;
// 	case WM_SETCURSOR:
// 		if (pThis->OnSetCursor((HWND)wParam, LOWORD(lParam), HIWORD(lParam)))
// 			return TRUE;
// 		break;
// 	case WM_GETDLGCODE:
// 		return pThis->OnGetDlgCode();
 	case WM_SIZE:
 		pThis->OnSize(wParam, LOWORD(lParam), HIWORD(lParam));
 		break;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

CSTXImageGridNode* CSTXImageGridCtrl::Internal_InsertItem(LPCTSTR lpszText, IStream *pStream, int nIndex, DWORD_PTR dwItemData /*= 0*/)
{
	if (nIndex < 0 || nIndex > (int)m_arrNodes.size())
	{
		return NULL;
	}

	POINT ptLoc;
	CalculateItemLocation(nIndex, &ptLoc);

	std::shared_ptr<CSTXImageGridNode> pNewNode(new CSTXImageGridNode(this));

	m_AnimationManager->CreateAnimationVariable(ptLoc.x + STXIGC_ITEM_START_OFFSET_FIX_X, &pNewNode->m_pAVLeftOffset);
	m_AnimationManager->CreateAnimationVariable(0, &pNewNode->m_pAVOpacity);
	m_AnimationManager->CreateAnimationVariable(ptLoc.y, &pNewNode->m_pAVTopOffset);
	//m_AnimationManager.CreateAnimationVariable(0, &pNewNode->m_pAVImageOccupy);
	m_AnimationManager->CreateAnimationVariable(0, &pNewNode->m_pAVHoverOpacity);
	m_AnimationManager->CreateAnimationVariable(0, &pNewNode->m_pAVDropDestOffsetX);
	m_AnimationManager->CreateAnimationVariable(1, &pNewNode->m_pAVClickScale);

	CComPtr<IUIAnimationStoryboard> pStory;
	m_AnimationManager->CreateStoryboard(&pStory);

	CComPtr<IUIAnimationTransition> pTrans;
	m_AnimationTransitionLibrary->CreateSmoothStopTransition(m_nDefaultAnimationDuration, ptLoc.x, &pTrans);
	pStory->AddTransition(pNewNode->m_pAVLeftOffset, pTrans);
	//pNewNode->m_nLeftOffsetFix = static_cast<int>(fTargetOffsetX);

	CComPtr<IUIAnimationTransition> pTransOpacity;
	m_AnimationTransitionLibrary->CreateSmoothStopTransition(m_nDefaultAnimationDuration, 1.0, &pTransOpacity);
	pStory->AddTransition(pNewNode->m_pAVOpacity, pTransOpacity);

	if (lpszText != NULL)
		pNewNode->m_strText = lpszText;

	m_arrNodes.insert(m_arrNodes.begin() + nIndex, pNewNode);

	SetItemImage(pStream, nIndex);

	ResetScrollBars();

	AdjustItemsForInsert(nIndex, pStory);

	pStory->Schedule(GetCurrentTime(), NULL);

	return pNewNode.get();
}

void CSTXImageGridCtrl::OnPaint(HDC hDC)
{
	DrawControl(hDC);
}

void CSTXImageGridCtrl::DrawControl(HDC hDC)
{
	RECT rcThis;
	::GetClientRect(m_hwndControl, &rcThis);
	Gdiplus::Rect rectThis(rcThis.left, rcThis.top, rcThis.right - rcThis.left, rcThis.bottom - rcThis.top);

	Gdiplus::Graphics g(hDC);
	Gdiplus::Bitmap bmpMem(rcThis.right - rcThis.left, rcThis.bottom - rcThis.top);
	Gdiplus::Graphics *pMemGraphics = Gdiplus::Graphics::FromImage(&bmpMem);

	DrawBackground(pMemGraphics, &rectThis);
	DrawContent(pMemGraphics, &rectThis);

	delete pMemGraphics;

	Gdiplus::TextureBrush brushContent(&bmpMem);
	g.FillRectangle(&brushContent, rectThis);
	g.ReleaseHDC(hDC);
}

void CSTXImageGridCtrl::DrawBackground(Gdiplus::Graphics *pGraphics, Gdiplus::Rect *rectThis)
{
	if (m_pImgBackground)
	{
		if (!m_pImgBackgroundCached)
		{
			Gdiplus::Bitmap* pBitmap = new Gdiplus::Bitmap(rectThis->Width, rectThis->Height);
			Gdiplus::Graphics graphics(pBitmap);
			graphics.DrawImage(m_pImgBackground.get(), 0, 0, rectThis->Width, rectThis->Height);

			Gdiplus::Bitmap *pBitmapSrc = pBitmap;
			std::shared_ptr<Gdiplus::CachedBitmap> imgCached(new Gdiplus::CachedBitmap(pBitmapSrc, pGraphics));
			m_pImgBackgroundCached = imgCached;

			delete pBitmap;
		}
		if (m_pImgBackgroundCached)
			pGraphics->DrawCachedBitmap(m_pImgBackgroundCached.get(), 0, 0);
		else
			pGraphics->DrawImage(m_pImgBackground.get(), rectThis->X, rectThis->Y, rectThis->Width, rectThis->Height);
	}
	else
	{
		Gdiplus::SolidBrush brushBk(m_clrBackground);
		pGraphics->FillRectangle(&brushBk, rectThis->X, rectThis->Y, rectThis->Width, rectThis->Height);
	}

}

void CSTXImageGridCtrl::DrawContent(Gdiplus::Graphics *pGraphics, Gdiplus::Rect *rectThis)
{
	int iVScrollPos = GetScrollPos(m_hwndControl, SB_VERT);

	RECT rcClient;
	GetClientRect(m_hwndControl, &rcClient);

	std::queue<std::shared_ptr<CSTXImageGridNode> > m_queueAllToDraw;

	//Draw Deleted items
	size_t nDrawDelete = m_queDeletedNodes.size();
	for (size_t i = 0; i < nDrawDelete; i++)
	{
		std::shared_ptr<CSTXImageGridNode> pItem = m_queDeletedNodes.front();
		m_queDeletedNodes.pop();

		DOUBLE fItemOpacity = 0;
		pItem->m_pAVOpacity->GetValue(&fItemOpacity);
		if (fItemOpacity > 0.02f)
		{
			m_queDeletedNodes.push(pItem);
			m_queueAllToDraw.push(pItem);
		}
	}

	std::vector<std::shared_ptr<CSTXImageGridNode> >::iterator it = m_arrNodes.begin();
	for (; it != m_arrNodes.end(); it++)
	{
		m_queueAllToDraw.push(*it);
	}

	if (m_pItemDragging)		//Draw the dragging item to force it to show at top level and not be overlapped
	{
		m_queueAllToDraw.push(m_pItemDragging);
	}

	size_t nItemToDraw = m_queueAllToDraw.size();
	for (size_t i = 0; i < nItemToDraw; i++)
	{
		std::shared_ptr<CSTXImageGridNode> pItem = m_queueAllToDraw.front();
		m_queueAllToDraw.pop();

		DOUBLE fTopOffset = 0;
		pItem->m_pAVTopOffset->GetValue(&fTopOffset);
		DOUBLE fBottomEdgeOffset = fTopOffset + m_nItemHeight;

		//if (!pItem->m_bPreDeleteNotified)
		{
			fTopOffset -= iVScrollPos;
			fBottomEdgeOffset -= iVScrollPos;
		}
		//else
		//{
		//	fTopOffset -= pItem->m_nVOffsetDeleting;
		//	fTopOffsetFinal -= pItem->m_nVOffsetDeleting;
		//}

		if (fTopOffset > rcClient.bottom)
			continue;
		
		if (fBottomEdgeOffset < 0)
			continue;

		//DOUBLE fLeftOffset = pItem->m_pAVLeftOffset->GetValue();
		//DOUBLE fTopOffsetFinal;
		//pItem->m_pAVTopOffset->GetFinalValue(&fTopOffsetFinal);
		//DOUBLE fLeftOffsetFinal;
		//pItem->m_pAVLeftOffset->GetFinalValue(&fLeftOffsetFinal);

		pItem->DrawItem(pGraphics, iVScrollPos, pItem == m_pItemSelected);
	}

}

int CSTXImageGridCtrl::GetItemWidth()
{
	return m_nItemWidth;
}

int CSTXImageGridCtrl::GetItemHeight()
{
	return m_nItemHeight;
}

HRESULT CSTXImageGridCtrl::OnManagerStatusChanged(UI_ANIMATION_MANAGER_STATUS newStatus, UI_ANIMATION_MANAGER_STATUS previousStatus)
{
	if (newStatus == UI_ANIMATION_MANAGER_BUSY)
	{
		//OutputDebugString(_T("Activate...\n"));
		::SetTimer(m_hwndControl, STXIGC_TIMER_ID_ANIMATION, 10, NULL);
	}
	else
	{
		//OutputDebugString(_T("Deactivate...\n"));
		::KillTimer(m_hwndControl, STXIGC_TIMER_ID_ANIMATION);
	}

	return S_OK;
}

void CSTXImageGridCtrl::OnTimer(UINT nIDEvent)
{
	if (nIDEvent == STXIGC_TIMER_ID_ANIMATION)
	{
		UpdateAnimationManager();
		InvalidateRect(m_hwndControl, NULL, TRUE);
	}
	//else if (nIDEvent == STXTC_TIMER_ID_ITEM_FLOAT_CHECK)
	//{
	//	OnItemFloatCheckTriggered();
	//}
}

void CSTXImageGridCtrl::UpdateAnimationManager()
{
	UI_ANIMATION_UPDATE_RESULT result;
	UI_ANIMATION_SECONDS secTime;
	m_AnimationTimer->GetTime(&secTime);
	m_AnimationManager->Update(secTime, &result);
}

void CSTXImageGridCtrl::SetItemImage(IStream *pStream, int nIndex, BOOL bResizeImage)
{
	if (nIndex < 0 || nIndex >= (int)m_arrNodes.size())
		return;

	std::shared_ptr<CSTXImageGridNode> pNode = m_arrNodes[nIndex];

	int nResizeImageWidth = -1;		//Use original image
	if (bResizeImage)
	{
		nResizeImageWidth = static_cast<int>(m_nItemHeight);
	}
	std::shared_ptr<Gdiplus::Image> pImg = GetResizedImage(pStream, nResizeImageWidth);
	if (pImg && pImg->GetWidth() == 0)
		pImg.reset();

	pNode->m_pImgImage = pImg;

	if (pImg && pImg->GetWidth() > 0)
	{
	}
}

std::shared_ptr<Gdiplus::Image> CSTXImageGridCtrl::GetResizedImage(IStream *pStream, int nWidthHeight)
{
	std::shared_ptr<Gdiplus::Image> img(new Gdiplus::Bitmap(pStream));
	return GetResizedImage(img, nWidthHeight);
}

std::shared_ptr<Gdiplus::Image> CSTXImageGridCtrl::GetResizedImage(HBITMAP hBitmap, int nWidthHeight)
{
	std::shared_ptr<Gdiplus::Image> img(new Gdiplus::Bitmap(hBitmap, NULL));
	return GetResizedImage(img, nWidthHeight);
}

std::shared_ptr<Gdiplus::Image> CSTXImageGridCtrl::GetResizedImage(LPCTSTR lpszFile, int nWidthHeight)
{
	std::shared_ptr<Gdiplus::Image> img(new Gdiplus::Image(lpszFile));
	return GetResizedImage(img, nWidthHeight);
}

std::shared_ptr<Gdiplus::Image> CSTXImageGridCtrl::GetResizedImage(std::shared_ptr<Gdiplus::Image> pImage, int nWidthHeight)
{
	UINT o_height = pImage->GetHeight();
	UINT o_width = pImage->GetWidth();

	if (nWidthHeight > 0 && (o_height > (UINT)nWidthHeight || o_width > (UINT)nWidthHeight))
	{
		INT n_width = nWidthHeight;
		INT n_height = nWidthHeight;
		double ratio = ((double)o_width) / ((double)o_height);
		if (o_width > o_height)
		{
			// Resize down by width
			n_height = static_cast<UINT>(((double)n_width) / ratio);
		}
		else
		{
			n_width = static_cast<UINT>(n_height * ratio);
		}

		std::shared_ptr<Gdiplus::Image> imgThumbnail(pImage->GetThumbnailImage(n_width, n_height));
		return imgThumbnail;
	}
	else
	{
		return pImage;
	}
}

std::shared_ptr<CSTXImageGridNode> CSTXImageGridCtrl::HitTest(POINT pt, UINT *pHitFlag, int *pIndexHit)
{
	int iHScrollPos = GetScrollPos(m_hwndControl, SB_HORZ);
	int iVScrollPos = GetScrollPos(m_hwndControl, SB_VERT);
	pt.x += iHScrollPos;
	pt.y += iVScrollPos;


	if (pt.x < m_rcMargins.left || pt.y < m_rcMargins.top)
		return NULL;

	int xLoc = (pt.x - m_rcMargins.left) / (m_nItemWidth + m_nItemSpacingX);
	int yLoc = (pt.y - m_rcMargins.top) / (m_nItemHeight + m_nItemSpacingY);

	if (pt.x > m_rcMargins.left + (m_nItemWidth + m_nItemSpacingX) * (xLoc + 1) - m_nItemSpacingX)
		return NULL;

	int nItemsPerLine = CalculateItemPerLine();

	if (xLoc >= nItemsPerLine)
		return NULL;

	int nLineCount = m_arrNodes.size() / nItemsPerLine;
	if (m_arrNodes.size() % nItemsPerLine)
		nLineCount++;

	if (yLoc >= nLineCount)
		return NULL;

	int nIndex = yLoc * nItemsPerLine + xLoc;

	if (pIndexHit)
	{
		*pIndexHit = nIndex;
	}

	if (nIndex >= (int)m_arrNodes.size())
		return NULL;

	if (pHitFlag)
	{
		int nItemLeftEdge = m_rcMargins.left + (m_nItemWidth + m_nItemSpacingX) * xLoc;
		int nItemRightEdge = nItemLeftEdge + m_nItemWidth;
		int nOffsetInItem = pt.x - nItemLeftEdge;
		if (nOffsetInItem < (nItemRightEdge - nItemLeftEdge) / 2)
			*pHitFlag |= STXIGHIT_FLAG_LEFT_HALF;
		else
			*pHitFlag |= STXIGHIT_FLAG_RIGHT_HALF;

	}

	if (pIndexHit)
	{
		*pIndexHit = nIndex;
	}

	return m_arrNodes[nIndex];
}

void CSTXImageGridCtrl::OnMouseMove(int x, int y, UINT nFlags)
{
	//POINT pt = { x, y };
	//CSTXImageGridNode *pHit = HitTest(pt);

	//if (pHit)
	//	OutputDebugStringA("Hit\n");
	//else
	//	OutputDebugStringA("Not Hit\n");

	KillTimer(m_hwndControl, STXIGC_TIMER_ID_ITEM_FLOAT_CHECK);
	if (m_pLastHoverItem)
	{
		SetTimer(m_hwndControl, STXIGC_TIMER_ID_ITEM_FLOAT_CHECK, STXIGC_TIMER_INTERVAL_ITEM_FLOAT_CHECK, NULL);
	}

	TRACKMOUSEEVENT tme;
	tme.cbSize = sizeof(tme);
	tme.hwndTrack = m_hwndControl;
	tme.dwFlags = TME_LEAVE;
	TrackMouseEvent(&tme);

	if (m_bLButtonDown && m_pItemDragging == NULL && (abs(m_ptLButtonDown.y - y) > 5 || abs(m_ptLButtonDown.x - x) > 5))
	{
		m_pItemDragging = m_pItemLButtonDown;
	}

	if (m_bLButtonDown && m_pItemDragging)
	{
		POINT ptOffset = { x - m_ptLButtonDown.x, y - m_ptLButtonDown.y };
		m_pItemDragging->m_ptDragOffset = ptOffset;

		POINT pt = { x, y };
		UINT nHitFlags = 0;
		int nHitIndex = -1;

		//std::shared_ptr<CSTXImageGridNode> pHit = HitTest(pt, &nHitFlags, &nHitIndex);

		//int nDropTargetIndex = -1;

		//if (pHit && pHit != m_pItemDragging)
		//{
		//	if (nHitFlags & STXIGHIT_FLAG_LEFT_HALF)
		//	{
		//		nDropTargetIndex = nHitIndex;
		//		POINT ptOffsetFix = { 8, 0 };
		//		//pHit->m_ptDragOffset = ptOffsetFix;
		//	}
		//	if (nHitFlags & STXIGHIT_FLAG_RIGHT_HALF)
		//	{
		//		nDropTargetIndex = nHitIndex + 1;
		//		POINT ptOffsetFix = { -8, 0 };
		//		//pHit->m_ptDragOffset = ptOffsetFix;
		//	}
		//}

		int nDropTargetIndex = DropIndexHitTest(pt);

		if (m_nCurrentDropTargetIndex != nDropTargetIndex)
		{
			ResetDropTargetOffset(m_nCurrentDropTargetIndex, nDropTargetIndex);

			if (nDropTargetIndex != m_nIndexItemDragging && nDropTargetIndex != m_nIndexItemDragging + 1)
			{
				ApplyDropTargetOffset(nDropTargetIndex);
			}
			m_nCurrentDropTargetIndex = nDropTargetIndex;
		}
		else
		{
		}
		UpdateAnimationManager();
		InvalidateRect(m_hwndControl, NULL, TRUE);

	}
	else
	{
		POINT pt = { x, y };
		std::shared_ptr<CSTXImageGridNode> pHit = HitTest(pt, NULL);
		if (m_pLastHoverItem != pHit)
		{
			if (pHit)
			{
				pHit->OnMouseEnter();
			}

			if (m_pLastHoverItem)
				m_pLastHoverItem->OnMouseLeave();

			m_pLastHoverItem = pHit;

			OnInternalHoverItemChanged(pHit == NULL ? NULL : pHit.get());
			//SendCommonNotifyMessage(STXATVN_HOVERITEMCHANGED, pHit, 0);

			//if (!IsAnimationBusy())
			//	InvalidateRect(m_hwndControl, NULL, FALSE);
		}
		UpdateAnimationManager();
		if (!IsAnimationBusy())
			InvalidateRect(m_hwndControl, NULL, FALSE);
	}

	//InvalidateRect(m_hwndControl, NULL, TRUE);

}

void CSTXImageGridCtrl::OnLButtonDown(int x, int y, UINT nFlags, BOOL bForRButton /*= FALSE*/)
{
	SetFocus(m_hwndControl);
	SetCapture(m_hwndControl);
	m_bLButtonDown = TRUE;
	m_ptLButtonDown.x = x;
	m_ptLButtonDown.y = y;

	UINT nHitFlags = 0;
	POINT pt = { x, y };
	int nIndex = -1;
	std::shared_ptr<CSTXImageGridNode> pHit = HitTest(pt, NULL, &nIndex);
	m_pItemLButtonDown = pHit;
	m_pItemSelected = pHit;
	//m_pItemDragging = pHit;
	m_nIndexItemDragging = nIndex;
	m_nSelectedItemIndex = nIndex;

	ApplyClickDownEffect();
	InvalidateRect(m_hwndControl, NULL, TRUE);
}

void CSTXImageGridCtrl::OnLButtonUp(int x, int y, UINT nFlags, BOOL bForRButton /*= FALSE*/)
{
	ReleaseCapture();

	if (m_bLButtonDown)
	{
		ApplyClickUpEffect();

		if (m_pItemDragging)
		{
			DropDraggingItem();
			//ResetDropTargetOffset(m_nCurrentDropTargetIndex, -1);
			UpdateAnimationManager();

			m_nCurrentDropTargetIndex = -1;

			POINT ptOffset = { 0, 0 };
			m_pItemDragging->m_ptDragOffset = ptOffset;
			//InvalidateRect(m_hwndControl, NULL, TRUE);
		}
		else
		{
			if (abs(m_ptLButtonDown.y - y) < 6)
			{
				POINT pt = { x, y };
				std::shared_ptr<CSTXImageGridNode> pHit = HitTest(pt, NULL);

				SendCommonNotifyMessage(STXGIVN_ITEMCLICK, pHit.get(), bForRButton ? 1 : 0);
			}
		}
	}

	m_nIndexItemDragging = -1;
	m_bLButtonDown = FALSE;
	m_pItemDragging = NULL;
	m_pItemLButtonDown = NULL;
}

void CSTXImageGridCtrl::ResetDropTargetOffset(int nIndexDropTarget, int nNextIndexDropTarget)
{
	if (nIndexDropTarget < 0)
		return;

	if (m_arrNodes.size() == 0)
		return;

	if (nIndexDropTarget - 1 >= 0 && nIndexDropTarget - 1 < (int)m_arrNodes.size())
	{
		std::shared_ptr<CSTXImageGridNode> pNodeLeft = m_arrNodes[nIndexDropTarget - 1];
		if (pNodeLeft != m_pItemDragging)
		{
			CComPtr<IUIAnimationStoryboard> pStory;
			m_AnimationManager->CreateStoryboard(&pStory);

			CComPtr<IUIAnimationTransition> pTrans;
			m_AnimationTransitionLibrary->CreateSmoothStopTransition(m_nDefaultAnimationDuration / 8, 0, &pTrans);
			pStory->AddTransition(pNodeLeft->m_pAVDropDestOffsetX, pTrans);
			pStory->Schedule(GetCurrentTime(), NULL);
		}

	}

	if (nIndexDropTarget < (int)m_arrNodes.size())
	{
		std::shared_ptr<CSTXImageGridNode> pNodeRight = m_arrNodes[nIndexDropTarget];
		if (pNodeRight != m_pItemDragging)
		{
			CComPtr<IUIAnimationStoryboard> pStory;
			m_AnimationManager->CreateStoryboard(&pStory);

			CComPtr<IUIAnimationTransition> pTrans;
			m_AnimationTransitionLibrary->CreateSmoothStopTransition(m_nDefaultAnimationDuration / 8, 0, &pTrans);
			pStory->AddTransition(pNodeRight->m_pAVDropDestOffsetX, pTrans);
			pStory->Schedule(GetCurrentTime(), NULL);
		}
	}
}

void CSTXImageGridCtrl::ApplyDropTargetOffset(int nIndexDropTarget)
{
	if (nIndexDropTarget < 0)
		return;

	if (m_arrNodes.size() == 0)
		return;

	if (nIndexDropTarget - 1 >= 0 && nIndexDropTarget - 1 < (int)m_arrNodes.size())
	{
		std::shared_ptr<CSTXImageGridNode> pNodeLeft = m_arrNodes[nIndexDropTarget - 1];
		if (pNodeLeft != m_pItemDragging)
		{
			CComPtr<IUIAnimationStoryboard> pStory;
			m_AnimationManager->CreateStoryboard(&pStory);

			CComPtr<IUIAnimationTransition> pTrans;
			m_AnimationTransitionLibrary->CreateSmoothStopTransition(m_nDefaultAnimationDuration / 2, -20, &pTrans);
			pStory->AddTransition(pNodeLeft->m_pAVDropDestOffsetX, pTrans);
			pStory->Schedule(GetCurrentTime(), NULL);
		}
	}

	if (nIndexDropTarget < (int)m_arrNodes.size())
	{
		std::shared_ptr<CSTXImageGridNode> pNodeRight = m_arrNodes[nIndexDropTarget];
		if (pNodeRight != m_pItemDragging)
		{
			CComPtr<IUIAnimationStoryboard> pStory;
			m_AnimationManager->CreateStoryboard(&pStory);

			CComPtr<IUIAnimationTransition> pTrans;
			m_AnimationTransitionLibrary->CreateSmoothStopTransition(m_nDefaultAnimationDuration / 2, 20, &pTrans);
			pStory->AddTransition(pNodeRight->m_pAVDropDestOffsetX, pTrans);
			pStory->Schedule(GetCurrentTime(), NULL);
		}
	}
}

int CSTXImageGridCtrl::DropIndexHitTest(POINT pt)
{
	int iHScrollPos = GetScrollPos(m_hwndControl, SB_HORZ);
	int iVScrollPos = GetScrollPos(m_hwndControl, SB_VERT);
	pt.x += iHScrollPos;
	pt.y += iVScrollPos;

	if (pt.y < m_rcMargins.top)
		return -1;

	pt.x += m_nItemWidth / 2;

	int xLoc = (pt.x - m_rcMargins.left) / (m_nItemWidth + m_nItemSpacingX);
	int yLoc = (pt.y - m_rcMargins.top) / (m_nItemHeight + m_nItemSpacingY);

	if (pt.x > m_rcMargins.left + (m_nItemWidth + m_nItemSpacingX) * (xLoc + 1) - m_nItemSpacingX)
		return -1;

	int nItemsPerLine = CalculateItemPerLine();

	if (xLoc >= nItemsPerLine + 1)
		return -1;
	if (xLoc <= -1)
		return -1;

	int nLineCount = m_arrNodes.size() / nItemsPerLine;
	if (m_arrNodes.size() % nItemsPerLine)
		nLineCount++;

	//if (yLoc >= nLineCount)
	//	return -1;

	int nIndex = yLoc * nItemsPerLine + xLoc;


//	if (nIndex >= m_arrNodes.size())
//		return -1;


	return nIndex;
}

void CSTXImageGridCtrl::DropDraggingItem()
{
	int nIndexFrom = m_nIndexItemDragging;
	int nIndexTo = m_nCurrentDropTargetIndex;

	if (nIndexTo < 0 || nIndexFrom < 0 || nIndexFrom == nIndexTo)
		return;

	if (m_nSelectedItemIndex == nIndexFrom && nIndexTo >= 0)
	{
		m_nSelectedItemIndex = nIndexTo;
	}

	std::shared_ptr<CSTXImageGridNode> pItemFrom = m_arrNodes[nIndexFrom];

	//0*00
	if (nIndexTo < (int)m_arrNodes.size())
	{
		if (nIndexTo < nIndexFrom)
		{
			m_arrNodes.erase(m_arrNodes.begin() + nIndexFrom);
			m_arrNodes.insert(m_arrNodes.begin() + nIndexTo, pItemFrom);
		}
		else if (nIndexTo > nIndexFrom)
		{
			m_arrNodes.insert(m_arrNodes.begin() + nIndexTo, pItemFrom);
			m_arrNodes.erase(m_arrNodes.begin() + nIndexFrom);
		}
	}
	else
	{
		m_arrNodes.erase(m_arrNodes.begin() + nIndexFrom);
		m_arrNodes.push_back(pItemFrom);
	}

	for (int i = 0; i < (int)m_arrNodes.size(); i++)
	{
		POINT ptLoc;
		CalculateItemLocation(i, &ptLoc);

		std::shared_ptr<CSTXImageGridNode> pItem = m_arrNodes[i];

		CComPtr<IUIAnimationStoryboard> pStory;
		m_AnimationManager->CreateStoryboard(&pStory);

		CComPtr<IUIAnimationTransition> pTrans;
		m_AnimationTransitionLibrary->CreateSmoothStopTransition(m_nDefaultAnimationDuration, ptLoc.x, &pTrans);

		if (pItem == m_pItemDragging)
		{
			CComPtr<IUIAnimationTransition> pTransOffsetFixX;
			DOUBLE fItemLeftOffset = 0;
			pItem->m_pAVLeftOffset->GetValue(&fItemLeftOffset);
			m_AnimationTransitionLibrary->CreateInstantaneousTransition(fItemLeftOffset + pItem->m_ptDragOffset.x, &pTransOffsetFixX);
			pStory->AddTransition(pItem->m_pAVLeftOffset, pTransOffsetFixX);
		}

		pStory->AddTransition(pItem->m_pAVLeftOffset, pTrans);

		CComPtr<IUIAnimationTransition> pTransV;
		m_AnimationTransitionLibrary->CreateSmoothStopTransition(m_nDefaultAnimationDuration, ptLoc.y, &pTransV);

		if (pItem == m_pItemDragging)
		{
			CComPtr<IUIAnimationTransition> pTransOffsetFixY;
			DOUBLE fItemTopOffset = 0;
			pItem->m_pAVTopOffset->GetValue(&fItemTopOffset);
			m_AnimationTransitionLibrary->CreateInstantaneousTransition(fItemTopOffset + pItem->m_ptDragOffset.y, &pTransOffsetFixY);
			pStory->AddTransition(pItem->m_pAVTopOffset, pTransOffsetFixY);
		}

		pStory->AddTransition(pItem->m_pAVTopOffset, pTransV);

		CComPtr<IUIAnimationTransition> pTransDropFixX;
		m_AnimationTransitionLibrary->CreateSmoothStopTransition(m_nDefaultAnimationDuration, 0, &pTransDropFixX);
		pStory->AddTransition(pItem->m_pAVDropDestOffsetX, pTransDropFixX);

		pStory->Schedule(GetCurrentTime(), NULL);
	}

	//POINT ptLoc;
	//CalculateItemLocation(nIndex, &ptLoc);

}

void CSTXImageGridCtrl::CalculateItemLocation(int nIndex, LPPOINT ptLoc)
{
	if (ptLoc == NULL)
		return;

	if (nIndex < 0)
		return;

	int nItemsPerLine = CalculateItemPerLine();

	if (nIndex > (int)m_arrNodes.size())
		nIndex = (int)m_arrNodes.size();

	int nLineIndex = nIndex / nItemsPerLine;

	int fTargetOffsetX = m_rcMargins.left;
	fTargetOffsetX += (nIndex % nItemsPerLine) * (m_nItemWidth + m_nItemSpacingX);

	int fTargetOffsetY = m_rcMargins.top;
	fTargetOffsetY += nLineIndex * (m_nItemHeight + m_nItemSpacingY);

	ptLoc->x = fTargetOffsetX;
	ptLoc->y = fTargetOffsetY;
}

UI_ANIMATION_SECONDS CSTXImageGridCtrl::GetCurrentTime()
{
	UI_ANIMATION_SECONDS secTime;
	m_AnimationTimer->GetTime(&secTime);
	return secTime;
}

void CSTXImageGridCtrl::OnInternalHoverItemChanged(CSTXImageGridNode *pItem)
{
	if (pItem)
	{
		SetTimer(m_hwndControl, STXIGC_TIMER_ID_ITEM_FLOAT_CHECK, STXIGC_TIMER_INTERVAL_ITEM_FLOAT_CHECK, NULL);
	}
	else
	{
		KillTimer(m_hwndControl, STXIGC_TIMER_ID_ITEM_FLOAT_CHECK);
	}
}

BOOL CSTXImageGridCtrl::IsAnimationBusy()
{
	UI_ANIMATION_MANAGER_STATUS st;
	m_AnimationManager->GetStatus(&st);
	return st == UI_ANIMATION_MANAGER_BUSY;
}

void CSTXImageGridCtrl::ApplySmoothStopTransition(IUIAnimationVariable *pVar, UI_ANIMATION_SECONDS duration, DOUBLE fTargetValue, IUIAnimationStoryboard *pStoryboard, BOOL bResetVelocity)
{
	CComPtr<IUIAnimationStoryboard> pStory;
	if (pStoryboard)
		pStory = pStoryboard;
	else
		m_AnimationManager->CreateStoryboard(&pStory);

	CComPtr<IUIAnimationTransition> pTrans;
	m_AnimationTransitionLibrary->CreateSmoothStopTransition(duration, fTargetValue, &pTrans);
	if (bResetVelocity)
		pTrans->SetInitialVelocity(0.0);
	pStory->AddTransition(pVar, pTrans);

	if (pStoryboard == NULL)
	{
		pStory->Schedule(GetCurrentTime(), NULL);
	}
}

void CSTXImageGridCtrl::OnMouseLeave()
{
	if (m_pLastHoverItem)
	{
		OnInternalHoverItemChanged(NULL);
		//SendCommonNotifyMessage(STXATVN_HOVERITEMCHANGED, NULL, 0);

		//if (IsValidItem(m_hLastHoverNode))
		{
			m_pLastHoverItem->OnMouseLeave();
			InvalidateRect(m_hwndControl, NULL, FALSE);
		}
	}
	m_pLastHoverItem = NULL;
	//m_hLastFloatTriggeredNode = NULL;

}

LRESULT CSTXImageGridCtrl::SendCommonNotifyMessage(UINT nNotifyCode, CSTXImageGridNode *pNode, DWORD_PTR dwNotifySpec)
{
	STXGIVNITEM nm;
	nm.hdr.hwndFrom = m_hwndControl;
	nm.hdr.idFrom = GetDlgCtrlID(m_hwndControl);
	nm.hdr.code = nNotifyCode;

	if (pNode)
		nm.dwItemData = pNode->m_dwNodeData;
	else
		nm.dwItemData = 0;

	nm.dwNotifySpec = dwNotifySpec;
	nm.pNode = pNode;
	
	return ::SendMessage(GetParent(m_hwndControl), WM_NOTIFY, GetDlgCtrlID(m_hwndControl), (LPARAM)&nm);
}

void CSTXImageGridCtrl::OnLButtonDblClk(int x, int y, UINT nFlags)
{
	UINT nHitFlags = 0;
	POINT pt = { x, y };
	int nIndexHit = -1;
	std::shared_ptr<CSTXImageGridNode> pNode = HitTest(pt, NULL, &nIndexHit);
	if (pNode)
	{
		SendCommonNotifyMessage(STXGIVN_ITEMDBLCLICK, pNode.get() , 0);
	}
}

void CSTXImageGridCtrl::OnMouseWheel(UINT nFlags, short zDelta, int x, int y)
{
	if (!((GetWindowLong(m_hwndControl, GWL_STYLE) & WS_VSCROLL) == WS_VSCROLL))
	{
		return;
	}

	int minpos;
	int maxpos;
	GetScrollRange(m_hwndControl, SB_VERT, &minpos, &maxpos);
	maxpos = GetScrollLimit(SB_VERT);

	// Get the current position of scroll box.
	int curpos = GetScrollPos(m_hwndControl, SB_VERT);
	int newpos = curpos - zDelta;
	newpos = min(newpos, maxpos);
	newpos = max(newpos, minpos);

	SetScrollPos(m_hwndControl, SB_VERT, newpos, TRUE);
	InvalidateRect(m_hwndControl, NULL, TRUE);
}

void CSTXImageGridCtrl::AdjustItemsForInsert(int nInsertIndex, IUIAnimationStoryboard *pStory)
{
	CComPtr<IUIAnimationStoryboard> spStory;
	if (pStory)
	{
		// TODO
	}
	else
	{
		m_AnimationManager->CreateStoryboard(&spStory);
		pStory = spStory;
	}

	for (int i = nInsertIndex + 1; i < (int)m_arrNodes.size(); i++)
	{
		POINT ptLoc;
		CalculateItemLocation(i, &ptLoc);

		std::shared_ptr<CSTXImageGridNode> pItem = m_arrNodes[i];


		CComPtr<IUIAnimationTransition> pTrans;
		m_AnimationTransitionLibrary->CreateSmoothStopTransition(m_nDefaultAnimationDuration, ptLoc.x, &pTrans);

		if (pItem == m_pItemDragging)
		{
			CComPtr<IUIAnimationTransition> pTransOffsetFixX;
			DOUBLE fItemLeftOffset = 0;
			pItem->m_pAVLeftOffset->GetValue(&fItemLeftOffset);
			m_AnimationTransitionLibrary->CreateInstantaneousTransition(fItemLeftOffset + pItem->m_ptDragOffset.x, &pTransOffsetFixX);
			pStory->AddTransition(pItem->m_pAVLeftOffset, pTransOffsetFixX);
		}

		pStory->AddTransition(pItem->m_pAVLeftOffset, pTrans);

		CComPtr<IUIAnimationTransition> pTransV;
		m_AnimationTransitionLibrary->CreateSmoothStopTransition(m_nDefaultAnimationDuration, ptLoc.y, &pTransV);

		if (pItem == m_pItemDragging)
		{
			CComPtr<IUIAnimationTransition> pTransOffsetFixY;
			DOUBLE fItemTopOffset = 0;
			pItem->m_pAVTopOffset->GetValue(&fItemTopOffset);
			m_AnimationTransitionLibrary->CreateInstantaneousTransition(fItemTopOffset + pItem->m_ptDragOffset.y, &pTransOffsetFixY);
			pStory->AddTransition(pItem->m_pAVTopOffset, pTransOffsetFixY);
		}

		pStory->AddTransition(pItem->m_pAVTopOffset, pTransV);

		CComPtr<IUIAnimationTransition> pTransDropFixX;
		m_AnimationTransitionLibrary->CreateSmoothStopTransition(m_nDefaultAnimationDuration, 0, &pTransDropFixX);
		pStory->AddTransition(pItem->m_pAVDropDestOffsetX, pTransDropFixX);
	}

	pStory->Schedule(GetCurrentTime(), NULL);
}

void CSTXImageGridCtrl::OnDestroy()
{
	m_arrNodes.clear();
	while (m_queDeletedNodes.size() > 0)
		m_queDeletedNodes.pop();

	if (m_pDefaultFont)
	{
		delete m_pDefaultFont;
		m_pDefaultFont = NULL;
	}

	m_pItemDragging = NULL;
	m_pItemLButtonDown = NULL;
	m_pLastHoverItem = NULL;
	m_pItemSelected = NULL;
}

HWND CSTXImageGridCtrl::GetSafeHwnd()
{
	return m_hwndControl;
}

void CSTXImageGridCtrl::SetItemWidth(int nWidth)
{
	m_nItemWidth = nWidth;
}

void CSTXImageGridCtrl::SetItemHeight(int nHeight)
{
	m_nItemHeight = nHeight;

}

void CSTXImageGridCtrl::ResetScrollBars()
{
	if (!IsWindow(m_hwndControl))
		return;

	RECT rcClient;
	::GetClientRect(m_hwndControl, &rcClient);

	//Vertical Scroll Bar
	int iTotalHeightAvailable = rcClient.bottom - rcClient.top;

	int iCurPos = 0;
	BOOL bVScrollExist;
	if ((GetWindowLong(m_hwndControl, GWL_STYLE) & WS_VSCROLL) == WS_VSCROLL)
	{
		iCurPos = ::GetScrollPos(m_hwndControl, SB_VERT);
		bVScrollExist = TRUE;
	}
	else
		bVScrollExist = FALSE;

	int iOldPos = ::GetScrollPos(m_hwndControl, SB_VERT);

	int nTotalHeight = GetCurrentTotalHeight();
	if (m_arrNodes.size() > 0 && nTotalHeight > iTotalHeightAvailable)	//Need H-ScrollBar
	{
		OutputDebugString(_T("Need V-Scrol Bar\n"));
		SCROLLINFO si;
		si.cbSize = sizeof(si);
		si.fMask = SIF_PAGE | SIF_POS | SIF_RANGE;
		si.nPage = iTotalHeightAvailable;
		si.nMin = 0;
		si.nMax = nTotalHeight;
		si.nPos = min(iCurPos, si.nMax);

		SetScrollPos(m_hwndControl, SB_VERT, si.nPos, FALSE);
		SetScrollInfo(m_hwndControl, SB_VERT, &si, TRUE);
		::ShowScrollBar(m_hwndControl, SB_VERT, TRUE);
		ModifyStyle(GWL_STYLE, 0, WS_VSCROLL, 0);

		AdjustItemsForInsert(0, NULL);
	}
	else
	{
		int iCurPos = GetScrollPos(m_hwndControl, SB_VERT);
		//ScrollWindow(m_hwndControl, 0, iCurPos, NULL, NULL);
		SetScrollPos(m_hwndControl, SB_VERT, 0, TRUE);
		::ShowScrollBar(m_hwndControl, SB_VERT, FALSE);
		ModifyStyle(GWL_STYLE, WS_VSCROLL, 0, 0);
	}

	//Horizontal Scroll Bar
	//ResetHorizontalScrollBar();

	InvalidateRect(m_hwndControl, NULL, FALSE);
}

BOOL CSTXImageGridCtrl::ModifyStyle(int nStyleOffset, DWORD dwRemove, DWORD dwAdd, UINT nFlags)
{
	DWORD dwStyle = ::GetWindowLong(m_hwndControl, nStyleOffset);
	DWORD dwNewStyle = (dwStyle & ~dwRemove) | dwAdd;
	if (dwStyle == dwNewStyle)
		return FALSE;

	::SetWindowLong(m_hwndControl, nStyleOffset, dwNewStyle);
	if (nFlags != 0)
	{
		::SetWindowPos(m_hwndControl, NULL, 0, 0, 0, 0,
			SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | nFlags);
	}
	return TRUE;
}

void CSTXImageGridCtrl::ModifyStyle(DWORD dwRemove, DWORD dwAdd)
{
	if (GetSafeHwnd() == NULL)
		return;

	ModifyStyle(GWL_STYLE, dwRemove, dwAdd, 0);
	InvalidateRect(m_hwndControl, NULL, FALSE);
}

int CSTXImageGridCtrl::GetCurrentTotalHeight()
{
	int nItemsPerLine = CalculateItemPerLine();

	int nLineCount = m_arrNodes.size() / nItemsPerLine;
	if (m_arrNodes.size() % nItemsPerLine)
		nLineCount++;

	return m_rcMargins.top + nLineCount * (m_nItemHeight + m_nItemSpacingY) + m_rcMargins.bottom - m_nItemSpacingY;
}

void CSTXImageGridCtrl::OnVScroll(UINT nSBCode, UINT nPos, HWND hWndScrollBar)
{
	int minpos;
	int maxpos;
	GetScrollRange(m_hwndControl, SB_VERT, &minpos, &maxpos);
	maxpos = GetScrollLimit(SB_VERT);

	// Get the current position of scroll box.
	int curpos = GetScrollPos(m_hwndControl, SB_VERT);
	int oldpos = curpos;

	// Determine the new position of scroll box.
	switch (nSBCode)
	{
	case SB_LEFT:      // Scroll to far left.
		curpos = minpos;
		break;

	case SB_RIGHT:      // Scroll to far right.
		curpos = maxpos;
		break;

	case SB_ENDSCROLL:   // End scroll.
		break;

	case SB_LINELEFT:      // Scroll left.
		if (curpos > minpos)
			curpos -= 5;
		break;

	case SB_LINERIGHT:   // Scroll right.
		if (curpos < maxpos)
			curpos += 5;
		break;

	case SB_PAGELEFT:    // Scroll one page left.
	{
		// Get the page size. 
		SCROLLINFO   info;
		info.fMask = SIF_ALL;
		GetScrollInfo(m_hwndControl, SB_VERT, &info);

		if (curpos > minpos)
			curpos = max(minpos, curpos - (int)info.nPage);
	}
	break;

	case SB_PAGERIGHT:      // Scroll one page right.
	{
		// Get the page size. 
		SCROLLINFO   info;
		info.fMask = SIF_ALL;
		GetScrollInfo(m_hwndControl, SB_VERT, &info);

		if (curpos < maxpos)
			curpos = min(maxpos, curpos + (int)info.nPage);
	}
	break;

	case SB_THUMBPOSITION: // Scroll to absolute position. nPos is the position
		curpos = nPos;      // of the scroll box at the end of the drag operation.
		break;

	case SB_THUMBTRACK:   // Drag scroll box to specified position. nPos is the
		curpos = nPos;     // position that the scroll box has been dragged to.
		break;
	}

	// Set the new position of the thumb (scroll box).
	SetScrollPos(m_hwndControl, SB_VERT, curpos, TRUE);

	InvalidateRect(m_hwndControl, NULL, TRUE);
}

int CSTXImageGridCtrl::GetScrollLimit(int nBar)
{
	int nMin, nMax;
	GetScrollRange(m_hwndControl, nBar, &nMin, &nMax);
	SCROLLINFO info;
	info.fMask = SIF_PAGE;
	if (GetScrollInfo(m_hwndControl, nBar, &info))
	{
		nMax -= __max(info.nPage - 1, 0);
	}
	return nMax;
}

void CSTXImageGridCtrl::OnSize(UINT nType, int cx, int cy)
{
	int nItemPerLineOld = GetItemCountPerLine();
	int nItemPerLineNew = CalculateItemPerLine();

	if (nItemPerLineOld != nItemPerLineNew)
	{
		AdjustItemsForInsert(0, NULL);
	}
	ResetScrollBars();
	UpdateAnimationManager();
}

int CSTXImageGridCtrl::CalculateItemPerLine()
{
	RECT rcThis;
	::GetClientRect(m_hwndControl, &rcThis);

	int nControlWidth = rcThis.right - rcThis.left;			//Control Total Width

	int nAvailableWidth = nControlWidth - m_rcMargins.left - m_rcMargins.right;		//Total Width for Items

	int nItemsPerLine = 1;
	int nBestFitItemPerLine = 1;
	if ((nBestFitItemPerLine = (nAvailableWidth + m_nItemSpacingX) / (m_nItemWidth + m_nItemSpacingX)) >= 2)
	{
		nItemsPerLine = nBestFitItemPerLine;
	}

	m_nItemPerLine = nItemsPerLine;
	return nItemsPerLine;
}

int CSTXImageGridCtrl::GetItemCountPerLine()
{
	return m_nItemPerLine;
}

void CSTXImageGridCtrl::ApplyClickDownEffect()
{
	if (m_pItemLButtonDown == NULL)
		return;

	CComPtr<IUIAnimationStoryboard> pStory;
	m_AnimationManager->CreateStoryboard(&pStory);

	CComPtr<IUIAnimationTransition> pTrans;
	m_AnimationTransitionLibrary->CreateSmoothStopTransition(m_nDefaultAnimationDuration / 8, 0.95, &pTrans);
	pStory->AddTransition(m_pItemLButtonDown->m_pAVClickScale, pTrans);

	pStory->Schedule(GetCurrentTime(), NULL);
}

void CSTXImageGridCtrl::ApplyClickUpEffect()
{
	if (m_pItemLButtonDown == NULL)
		return;

	CComPtr<IUIAnimationStoryboard> pStory;
	m_AnimationManager->CreateStoryboard(&pStory);

	CComPtr<IUIAnimationTransition> pTrans;
	m_AnimationTransitionLibrary->CreateSmoothStopTransition(m_nDefaultAnimationDuration / 8, 1.0, &pTrans);
	pStory->AddTransition(m_pItemLButtonDown->m_pAVClickScale, pTrans);

	pStory->Schedule(GetCurrentTime(), NULL);
}

BOOL CSTXImageGridCtrl::Internal_DeleteItem(int nIndex)
{
	if (nIndex < 0 || nIndex >= (int)m_arrNodes.size())
		return FALSE;

	std::shared_ptr<CSTXImageGridNode> pItemToDelete = m_arrNodes[nIndex];

	m_queDeletedNodes.push(pItemToDelete);

	pItemToDelete->PerformDeleteEffect();

	CComPtr<IUIAnimationStoryboard> pStory;
	m_AnimationManager->CreateStoryboard(&pStory);

	CComPtr<IUIAnimationTransition> pTrans;
	m_AnimationTransitionLibrary->CreateSmoothStopTransition(m_nDefaultAnimationDuration, 0, &pTrans);
	pStory->AddTransition(pItemToDelete->m_pAVOpacity, pTrans);

	m_arrNodes.erase(m_arrNodes.begin() + nIndex);

	AdjustItemsForInsert(nIndex - 1, pStory);

	pStory->Schedule(GetCurrentTime(), NULL);

	if (nIndex == m_nSelectedItemIndex)
	{
		m_nSelectedItemIndex = -1;
		m_pItemSelected = NULL;
	}

	return TRUE;
}

BOOL CSTXImageGridCtrl::Internal_SetItemData(int nIndex, DWORD_PTR dwItemData)
{
	if (nIndex < 0 || nIndex >= (int)m_arrNodes.size())
		return FALSE;

	std::shared_ptr<CSTXImageGridNode> pItem = m_arrNodes[nIndex];
	pItem->m_dwNodeData = dwItemData;

	return TRUE;
}

int CSTXImageGridCtrl::GetCurrentSelectedIndex()
{
	return m_nSelectedItemIndex;
}

void CSTXImageGridCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{

	if (nChar == VK_DOWN)
	{
		SelectNextLine(1);
	}
	else if (nChar == VK_UP)
	{
		SelectNextLine(-1);
	}
	else if (nChar == VK_LEFT)
	{
		SelectNextItem(-1);
	}
	else if (nChar == VK_RIGHT)
	{
		SelectNextItem(1);
	}

}

void CSTXImageGridCtrl::SelectNextItem(int nIndexDelta)
{
	if (m_arrNodes.size() == 0)
		return;

	if (m_nSelectedItemIndex < 0)
	{
		m_nSelectedItemIndex = 0;
		m_pItemSelected = m_arrNodes[m_nSelectedItemIndex];
		EnsureVisible(m_nSelectedItemIndex);
		InvalidateRect(m_hwndControl, NULL, TRUE);
		return;
	}

	int nItemsPerLine = GetItemCountPerLine();
	int nCurrentIndexInLine = m_nSelectedItemIndex % nItemsPerLine;

	if ((nCurrentIndexInLine == 0 && nIndexDelta < 0) || (nCurrentIndexInLine >= nItemsPerLine - 1 && nIndexDelta > 0))
		return;

	m_nSelectedItemIndex += nIndexDelta;
	if (m_nSelectedItemIndex < 0)
	{
		m_nSelectedItemIndex = 0;
		m_pItemSelected = m_arrNodes[m_nSelectedItemIndex];
		EnsureVisible(m_nSelectedItemIndex);
		InvalidateRect(m_hwndControl, NULL, TRUE);
		return;
	}

	if (m_nSelectedItemIndex >= (int)m_arrNodes.size())
	{
		m_nSelectedItemIndex = (int)m_arrNodes.size() - 1;
		m_pItemSelected = m_arrNodes[m_nSelectedItemIndex];
		EnsureVisible(m_nSelectedItemIndex);
		InvalidateRect(m_hwndControl, NULL, TRUE);
		return;
	}
	m_pItemSelected = m_arrNodes[m_nSelectedItemIndex];
	EnsureVisible(m_nSelectedItemIndex);
	InvalidateRect(m_hwndControl, NULL, TRUE);
}

void CSTXImageGridCtrl::EnsureVisible(int nIndex)
{
	if (nIndex < 0 || nIndex >= (int)m_arrNodes.size())
		return;

	POINT ptLoc;
	CalculateItemLocation(nIndex, &ptLoc);

	RECT rcClient;
	GetClientRect(m_hwndControl, &rcClient);
	int iVScrollPos = GetScrollPos(m_hwndControl, SB_VERT);

	if (ptLoc.y - iVScrollPos < 0)
	{
		SetScrollPos(m_hwndControl, SB_VERT, ptLoc.y, TRUE);
	}
	else if (ptLoc.y + m_nItemHeight - iVScrollPos > rcClient.bottom - rcClient.top)
	{
		SetScrollPos(m_hwndControl, SB_VERT, ptLoc.y + m_nItemHeight - (rcClient.bottom - rcClient.top), TRUE);
	}
}

void CSTXImageGridCtrl::SelectNextLine(int nLineDelta)
{
	if (m_arrNodes.size() == 0)
		return;

	if (m_nSelectedItemIndex < 0)
	{
		m_nSelectedItemIndex = 0;
		m_pItemSelected = m_arrNodes[m_nSelectedItemIndex];
		EnsureVisible(m_nSelectedItemIndex);
		InvalidateRect(m_hwndControl, NULL, TRUE);
		return;
	}

	int nItemsPerLine = GetItemCountPerLine();
	int nLineCount = m_arrNodes.size() / nItemsPerLine;
	if (m_arrNodes.size() % nItemsPerLine)
		nLineCount++;

	int nCurrentLine = m_nSelectedItemIndex / nItemsPerLine;
	if ((nCurrentLine == 0 && nLineDelta < 0) || (nCurrentLine >= nLineCount - 1 && nLineDelta > 0))
		return;

	m_nSelectedItemIndex += nLineDelta * nItemsPerLine;
	if (m_nSelectedItemIndex < 0)
	{
		m_nSelectedItemIndex = 0;
		m_pItemSelected = m_arrNodes[m_nSelectedItemIndex];
		EnsureVisible(m_nSelectedItemIndex);
		InvalidateRect(m_hwndControl, NULL, TRUE);
		return;
	}

	if (m_nSelectedItemIndex >= (int)m_arrNodes.size())
	{
		m_nSelectedItemIndex = (int)m_arrNodes.size() - 1;
		m_pItemSelected = m_arrNodes[m_nSelectedItemIndex];
		EnsureVisible(m_nSelectedItemIndex);
		InvalidateRect(m_hwndControl, NULL, TRUE);
		return;
	}
	m_pItemSelected = m_arrNodes[m_nSelectedItemIndex];
	EnsureVisible(m_nSelectedItemIndex);
	InvalidateRect(m_hwndControl, NULL, TRUE);

}

void CSTXImageGridCtrl::CalculateFitSize(SIZE sizeContainer, SIZE sizeContained, SIZE *pContainedAdjusted, POINT *ptOffset)
{
	int offsetX = 0;
	int offsetY = 0;
	SIZE sizeDest;

	if (sizeContained.cx <= sizeContainer.cx && sizeContained.cy <= sizeContainer.cy)
	{
		offsetX = (sizeContainer.cx - sizeContained.cx) / 2;
		offsetY = (sizeContainer.cy - sizeContained.cy) / 2;
		sizeDest = sizeContained;
	}
	else
	{
		double fRatioContained = (double)sizeContained.cx / (double)sizeContained.cy;
		double fRatioContainer = (double)sizeContainer.cx / (double)sizeContainer.cy;

		if (fRatioContainer > fRatioContained)		//Container is wider than object
		{
			sizeDest.cy = sizeContainer.cy;
			sizeDest.cx = (int)(sizeContainer.cy * fRatioContained);
			offsetX = (sizeContainer.cx - sizeDest.cx) / 2;
		}
		else
		{
			sizeDest.cx = sizeContainer.cx;
			sizeDest.cy = (int)(sizeContainer.cx / fRatioContained);
			offsetY = (sizeContainer.cy - sizeDest.cy) / 2;
		}
	}

	if (ptOffset)
	{
		ptOffset->x = offsetX;
		ptOffset->y = offsetY;
	}

	if (pContainedAdjusted)
	{
		*pContainedAdjusted = sizeDest;
	}
}

int CSTXImageGridCtrl::GetItemCount()
{
	return static_cast<int>(m_arrNodes.size());
}
