
// CollabAppView.h : CCollabAppView クラスのインターフェイス
//

#pragma once
#define USE_FORMS_VIEW
#ifdef USE_FORMS_VIEW
#include <afxwinforms.h>

using CParentView = CWinFormsView;
#else
using CParentView = CView;
#endif

class CCollabAppView : public CParentView
{
protected: // シリアル化からのみ作成します。
	CCollabAppView() noexcept;
	DECLARE_DYNCREATE(CCollabAppView)

// 属性
public:
	CCollabAppDoc* GetDocument() const;

// 操作
public:

// オーバーライド
public:
	virtual void OnDraw(CDC* pDC);  // このビューを描画するためにオーバーライドされます。
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// 実装
public:
	virtual ~CCollabAppView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 生成された、メッセージ割り当て関数
protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual void OnInitialUpdate();
	virtual void OnUpdate( CView* pSender, LPARAM lHint, CObject* pHint );

public:
#ifdef USE_FORMS_VIEW
	//	.NET のイベントを連動マクロ
	BEGIN_DELEGATE_MAP( CCollabAppView )
		EVENT_DELEGATE_ENTRY( OnClick, System::Object^, System::EventArgs^ )
	END_DELEGATE_MAP()
	void OnClick( System::Object^ sender, System::EventArgs^ e );
#endif
	afx_msg void OnTest();
};

#ifndef _DEBUG  // CollabAppView.cpp のデバッグ バージョン
inline CCollabAppDoc* CCollabAppView::GetDocument() const
   { return reinterpret_cast<CCollabAppDoc*>(m_pDocument); }
#endif

