
// CollabAppView.cpp : CCollabAppView クラスの実装
//

#include "stdafx.h"
// SHARED_HANDLERS は、プレビュー、縮小版、および検索フィルター ハンドラーを実装している ATL プロジェクトで定義でき、
// そのプロジェクトとのドキュメント コードの共有を可能にします。
#ifndef SHARED_HANDLERS
#include "CollabApp.h"
#endif

#include "CollabAppDoc.h"
#include "CollabAppView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CCollabAppView

IMPLEMENT_DYNCREATE(CCollabAppView, CParentView )

BEGIN_MESSAGE_MAP(CCollabAppView, CParentView )
	// 標準印刷コマンド
	ON_COMMAND(ID_FILE_PRINT, &CParentView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CParentView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CParentView::OnFilePrintPreview)
	ON_COMMAND( IDM_TEST, &CCollabAppView::OnTest )
	ON_MESSAGE( WMA_UPDATE_FROM_THREAD, &CCollabAppView::OnUpdateFromThread)
END_MESSAGE_MAP()


// CCollabAppView コンストラクション/デストラクション

CCollabAppView::CCollabAppView() noexcept
#ifdef USE_FORMS_VIEW
: CParentView( CollabLib::CsView::typeid )
#endif
{
	// TODO: 構築コードをここに追加します。

}

CCollabAppView::~CCollabAppView()
{
}

BOOL CCollabAppView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: この位置で CREATESTRUCT cs を修正して Window クラスまたはスタイルを
	//  修正してください。

	return CParentView::PreCreateWindow(cs);
}

// CCollabAppView 描画

void CCollabAppView::OnDraw(CDC* pDC)
{
	CCollabAppDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	CRect	rc;
	GetClientRect( &rc );
	CString msg( _T( "C++で直接描画。昔ながらのセンタリング" ) );
	CSize sizeText = pDC->GetTextExtent( msg );
	pDC->TextOut( rc.left+(rc.Width()-sizeText.cx)/2, rc.top + (rc.Height()-sizeText.cy)/2, msg );
}


// CCollabAppView の印刷

BOOL CCollabAppView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// 既定の印刷準備
	return DoPreparePrinting(pInfo);
}

void CCollabAppView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 印刷前の特別な初期化処理を追加してください。
}

void CCollabAppView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 印刷後の後処理を追加してください。
}


// CCollabAppView の診断

#ifdef _DEBUG
void CCollabAppView::AssertValid() const
{
	CParentView::AssertValid();
}

void CCollabAppView::Dump(CDumpContext& dc) const
{
	CParentView::Dump(dc);
}

CCollabAppDoc* CCollabAppView::GetDocument() const // デバッグ以外のバージョンはインラインです。
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CCollabAppDoc)));
	return (CCollabAppDoc*)m_pDocument;
}
#endif //_DEBUG


// CCollabAppView メッセージ ハンドラー
void CCollabAppView::OnInitialUpdate()
{
	//	CWinFormsView の OnInitialUpdateは呼び出さないようにして、C++側のサイズを強制させないように修正
	//	UserControlは、ScrollableControl の派生クラスなのでAutoScrollMinSizeを設定してしまうので設定しないように修正。
	//	そういう制御は、View本体にやらせるほうがいい。
	//	詳細は、 src\mfcm\wfrmview.cpp を参照
	auto pCtrl = GetControl();	//	System::Windows::Forms::Control^ pCtrl = *this;
	auto pIView = dynamic_cast<Microsoft::VisualC::MFC::IView^>(pCtrl);
	if( pIView != nullptr )
	{
		pIView->OnInitialUpdate();
	}
	CView::OnInitialUpdate();

	//	ボタンのクリックイベントを受け取る
	auto pView = dynamic_cast<CollabLib::CsView^>(pCtrl);
	pView->button1->Click += MAKE_DELEGATE( System::EventHandler, OnClick );
	//	パイプ待機スレッドを起動する
	GetDocument()->CreatePipeThread();
	pView->AddMsg( L"ボタンを押すとパイプ通信します。押しちゃだめなんだよ？" );
}

#include <msclr/marshal_atl.h>
inline System::String^ FromString( const TCHAR* const& _from_object )
{
	return msclr::interop::marshal_as<System::String^>( _from_object );
//	return gcnew System::String(_from_object);
}
inline CString ToString( System::String^ const& _from_object )
{
	return msclr::interop::marshal_as<CString>( _from_object );
	//cli::pin_ptr<const TCHAR> _pinned_ptr = PtrToStringChars( _from_object );
	//return CString( static_cast<const TCHAR *>(_pinned_ptr), _from_object->Length );
}
void CCollabAppView::OnClick( System::Object^ sender, System::EventArgs^ e )
{
	//	タスクを立ち上げてバックグラウンドでスレッド通信する。
	TCHAR	execPath[MAX_PATH];
	GetModuleFileName( nullptr, execPath, MAX_PATH );
	lstrcpy( PathFindFileName( execPath ), _T( "PipeClientApp.exe" ) );
	ShellExecute( *this, _T( "open" ), execPath, GetDocument()->GetPipeName(), nullptr, SW_SHOWNORMAL );
}
void CCollabAppView::OnTest()
{
	auto wrapWpf = gcnew CollabWpf::WrapWpf();
	if( wrapWpf->CallDlg() )
	{
		CString msg;
		msg = _T( "Succeeded : " );
		msg += ToString( wrapWpf->TelNumber );
		AfxMessageBox( msg );
	}
}
void CCollabAppView::OnUpdate( CView* pSender, LPARAM lHint, CObject* pHint )
{
	CParentView::OnUpdate( pSender, lHint, pHint );
	//
	//	パイプのデータが送られてきてるのでそれをさらに流し込む
	//
	if( lHint == 1 && pHint != nullptr )
	{
		CPipeData* pData = dynamic_cast<CPipeData*>(pHint);
		if( pData != nullptr )
		{
			auto pView = dynamic_cast<CollabLib::CsView^>(GetControl());
			pView->AddMsg( FromString( pData->m_lineData ) );
		}
	}
}
//	ドキュメントはウィンドウメッセージを受け取れないのでビューに肩代わりしてもらう
LRESULT CCollabAppView::OnUpdateFromThread( WPARAM wParam, LPARAM lParam )
{
	GetDocument()->OnUpdateFromThread( wParam, lParam );
	return 0;
}
