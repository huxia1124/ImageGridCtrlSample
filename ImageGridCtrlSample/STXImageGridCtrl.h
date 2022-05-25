#pragma once
#include "framework.h"
#include <memory>
#include <gdiplus.h>
#include <tchar.h>
#include <vector>
#include <queue>
#include <string>
#include <atlbase.h>

//////////////////////////////////////////////////////////////////////////

class CSTXImageGridNode;

//////////////////////////////////////////////////////////////////////////

#define  STXIGHIT_FLAG_LEFT_HALF		0x00000001
#define  STXIGHIT_FLAG_RIGHT_HALF		0x00000002

//////////////////////////////////////////////////////////////////////////
#define STXGIVN_ITEMCLICK				1
#define STXGIVN_ITEMDBLCLICK			2
#define STXGIVN_HOVERITEMCHANGED		5
#define STXGIVN_SELECTEDITEMCHANGED		6
#define STXGIVN_PREDELETEITEM			7
#define STXGIVN_POSTDELETEITEM			8
#define STXGIVN_ITEMMOUSEFLOAT			9
//#define STXGIVN_ENDEDIT				10
//#define STXGIVN_ITEMEXPANDING			20
//#define STXGIVN_ITEMEXPANDED			21
#define STXGIVN_CLICK					22


typedef struct tagSTXGIVNITEM
{
	NMHDR hdr;
	DWORD_PTR dwNotifySpec;
	CSTXImageGridNode *pNode;
	DWORD_PTR dwItemData;
}STXGIVNITEM, *LPSTXGIVNITEM;

//////////////////////////////////////////////////////////////////////////

class CSTXImageGridNode
{
	friend class CSTXImageGridCtrl;
protected:
	CSTXImageGridNode(CSTXImageGridCtrl *pParentControl);
public:
	virtual ~CSTXImageGridNode();

protected:
	CComPtr<IUIAnimationVariable> m_pAVLeftOffset;
	CComPtr<IUIAnimationVariable> m_pAVTopOffset;
	CComPtr<IUIAnimationVariable> m_pAVOpacity;			//0.0 - 1.0
	CComPtr<IUIAnimationVariable> m_pAVHoverOpacity;	//0.0 - 1.0
	CComPtr<IUIAnimationVariable> m_pAVClickScale;		//Normally 0.8 - 1.0, used for clicking effect

	CSTXImageGridCtrl *m_pParentControl;

	std::shared_ptr<Gdiplus::Image> m_pImgImage;		//Node Image
	std::shared_ptr<Gdiplus::CachedBitmap> m_pImgCached;

#ifdef UNICODE
	std::wstring m_strText;
#else
	std::string m_strText;
#endif

	POINT m_ptDragOffset;
	CComPtr<IUIAnimationVariable> m_pAVDropDestOffsetX;

protected:
	DWORD_PTR m_dwNodeData;


protected:
	void DrawItem(Gdiplus::Graphics *pGraphics, int nGlobalOffsetY, BOOL bSelectedItem);
	void DrawHover(Gdiplus::Graphics * pGraphics, Gdiplus::RectF *pItemRect);
	void DrawSelection(Gdiplus::Graphics * pGraphics, Gdiplus::RectF *pItemRect);
	BOOL DrawString(Gdiplus::Graphics * pGraphics, Gdiplus::RectF *pItemRect, BOOL bSelectedItem);

	void OnMouseEnter();
	void OnMouseLeave();

	void PerformDeleteEffect();
};

//////////////////////////////////////////////////////////////////////////

class CSTXImageGridCtrl : public IUIAnimationManagerEventHandler
{
	friend class CSTXImageGridNode;

public:
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) { return E_NOTIMPL; }
	virtual ULONG STDMETHODCALLTYPE AddRef(void) { return 1; }
	virtual ULONG STDMETHODCALLTYPE Release(void) { return 1; }

public:
	CSTXImageGridCtrl();
	virtual ~CSTXImageGridCtrl();

protected:

protected:
	CComPtr<IUIAnimationManager> m_AnimationManager;
	CComPtr<IUIAnimationTimer> m_AnimationTimer;
	CComPtr<IUIAnimationTransitionLibrary> m_AnimationTransitionLibrary;

	UI_ANIMATION_SECONDS m_nDefaultAnimationDuration;


protected:
	std::shared_ptr<Gdiplus::Image> m_pImgBackground;		//Control Background Image
	std::shared_ptr<Gdiplus::CachedBitmap> m_pImgBackgroundCached;
	Gdiplus::Color m_clrBackground;
	Gdiplus::REAL m_fTextHeight;

protected:
	std::vector<std::shared_ptr<CSTXImageGridNode> > m_arrNodes;
	std::queue<std::shared_ptr<CSTXImageGridNode> > m_queDeletedNodes;


	int m_nItemWidth;
	int m_nItemHeight;
	int m_nItemSpacingX;
	int m_nItemSpacingY;
	RECT m_rcMargins;
	LOGFONT m_lfDefaultFont;
	Gdiplus::Font *m_pDefaultFont;
	int m_nItemPerLine;
	int m_nLineCount;


protected:
	HWND m_hwndControl;
	BOOL m_bMouseInControl;
	BOOL m_bLButtonDown;
	POINT m_ptLButtonDown;
	std::shared_ptr<CSTXImageGridNode> m_pItemLButtonDown;
	std::shared_ptr<CSTXImageGridNode> m_pItemDragging;
	int m_nIndexItemDragging;
	int m_nCurrentDropTargetIndex;		//Index of drop target. the dragging item will be drop to the new Index
	std::shared_ptr<CSTXImageGridNode> m_pLastHoverItem;
	std::shared_ptr<CSTXImageGridNode> m_pItemSelected;
	int m_nSelectedItemIndex;


protected:
	static LPCTSTR s_lpszImageGridCtrlClassName;
	static LRESULT CALLBACK STXImageGridWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static void CalculateFitSize(SIZE sizeContainer, SIZE sizeContained, SIZE *pContainedAdjusted, POINT *ptOffset);

protected:
	void OnTimer(UINT nIDEvent);
	void OnPaint(HDC hDC);
	void OnMouseMove(int x, int y, UINT nFlags);
	void OnLButtonDown(int x, int y, UINT nFlags, BOOL bForRButton = FALSE);
	void OnLButtonUp(int x, int y, UINT nFlags, BOOL bForRButton = FALSE);
	void OnMouseLeave();
	void OnLButtonDblClk(int x, int y, UINT nFlags);
	void OnMouseWheel(UINT nFlags, short zDelta, int x, int y);
	void OnDestroy();
	void OnVScroll(UINT nSBCode, UINT nPos, HWND hWndScrollBar);
	void OnSize(UINT nType, int cx, int cy);
	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

	HRESULT OnManagerStatusChanged(UI_ANIMATION_MANAGER_STATUS newStatus, UI_ANIMATION_MANAGER_STATUS previousStatus);
	void OnInternalHoverItemChanged(CSTXImageGridNode *pItem);


protected:
	static std::shared_ptr<Gdiplus::Image> GetResizedImage(IStream *pStream, int nWidthHeight);
	static std::shared_ptr<Gdiplus::Image> GetResizedImage(HBITMAP hBitmap, int nWidthHeight);
	static std::shared_ptr<Gdiplus::Image> GetResizedImage(LPCTSTR lpszFile, int nWidthHeight);
	static std::shared_ptr<Gdiplus::Image> GetResizedImage(std::shared_ptr<Gdiplus::Image> pImage, int nWidthHeight);

protected:
	void UpdateAnimationManager();
	UI_ANIMATION_SECONDS GetCurrentTime();
	void GetDefaultFontInfo();
	void DrawControl(HDC hDC);
	void DrawBackground(Gdiplus::Graphics *pGraphics, Gdiplus::Rect *rectThis);
	void DrawContent(Gdiplus::Graphics *pGraphics, Gdiplus::Rect *rectThis);
	std::shared_ptr<CSTXImageGridNode> HitTest(POINT pt, UINT *pHitFlag, int *pIndexHit = NULL);
	int DropIndexHitTest(POINT pt);
	void ResetDropTargetOffset(int nIndexDropTarget, int nNextIndexDropTarget);
	void ApplyDropTargetOffset(int nIndexDropTarget);
	void CalculateItemLocation(int nIndex, LPPOINT ptLoc);
	void ApplySmoothStopTransition(IUIAnimationVariable *pVar, UI_ANIMATION_SECONDS duration, DOUBLE fTargetValue, IUIAnimationStoryboard *pStoryboard = NULL, BOOL bResetVelocity = FALSE);
	LRESULT SendCommonNotifyMessage(UINT nNotifyCode, CSTXImageGridNode *pNode, DWORD_PTR dwNotifySpec);
	void AdjustItemsForInsert(int nInsertIndex, IUIAnimationStoryboard *pStory);
	void ResetScrollBars();
	BOOL ModifyStyle(int nStyleOffset, DWORD dwRemove, DWORD dwAdd, UINT nFlags);
	void ModifyStyle(DWORD dwRemove, DWORD dwAdd);
	int GetScrollLimit(int nBar);
	int CalculateItemPerLine();
	void ApplyClickDownEffect();
	void ApplyClickUpEffect();
	void SelectNextItem(int nIndexDelta);
	void SelectNextLine(int nLineDelta);

public:
	static void RegisterAnimatedTreeCtrlClass();
	BOOL Create(LPCTSTR lpszWindowText, DWORD dwStyle, int x, int y, int cx, int cy, HWND hWndParent, UINT nID);

	CSTXImageGridNode* Internal_InsertItem(LPCTSTR lpszText, IStream *pStream, int nIndex, DWORD_PTR dwItemData = 0);
	BOOL Internal_DeleteItem(int nIndex);
	BOOL Internal_SetItemData(int nIndex, DWORD_PTR dwItemData);
	int GetCurrentSelectedIndex();

	void SetItemImage(IStream *pStream, int nIndex, BOOL bResizeImage = TRUE);
	int GetItemWidth();
	int GetItemHeight();
	void DropDraggingItem();
	BOOL IsAnimationBusy();
	HWND GetSafeHwnd();
	void SetItemWidth(int nWidth);
	void SetItemHeight(int nHeight);
	int GetCurrentTotalHeight();
	int GetItemCountPerLine();
	void EnsureVisible(int nIndex);
	int GetItemCount();

};

