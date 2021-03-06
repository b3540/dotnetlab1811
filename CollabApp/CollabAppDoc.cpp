
// CollabAppDoc.cpp : CCollabAppDoc クラスの実装
//

#include "NonClrPch.h"
// SHARED_HANDLERS は、プレビュー、縮小版、および検索フィルター ハンドラーを実装している ATL プロジェクトで定義でき、
// そのプロジェクトとのドキュメント コードの共有を可能にします。
#ifndef SHARED_HANDLERS
#include "CollabApp.h"
#endif

#include "CollabAppDoc.h"
#include "PipeCommunication.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif






// CCollabAppDoc
IMPLEMENT_DYNCREATE(CCollabAppDoc, CDocument)

BEGIN_MESSAGE_MAP(CCollabAppDoc, CDocument)
END_MESSAGE_MAP()


// CCollabAppDoc コンストラクション/デストラクション
const LPCTSTR	PIPE_FULLNAME	= _T( "\\\\.\\pipe\\DotNetLab\\201811" );	//	パイプ名
const LPCTSTR	PIPE_NAME = _T( "DotNetLab\\201811" );	//	パイプ名
const DWORD		BUFF_SIZE = 4000;


CCollabAppDoc::CCollabAppDoc() noexcept
//	: m_exitThread( false )
//	, m_pThread( nullptr )
{
	
	// TODO: この位置に 1 度だけ呼ばれる構築用のコードを追加してください。

}

CCollabAppDoc::~CCollabAppDoc()
{
	m_pipeCom.reset();
}

BOOL CCollabAppDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	return TRUE;
}

// CCollabAppDoc のシリアル化

void CCollabAppDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: 格納するコードをここに追加してください。
	}
	else
	{
		// TODO: 読み込むコードをここに追加してください。
	}
}

#ifdef SHARED_HANDLERS

//縮小版のサポート
void CCollabAppDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// このコードを変更してドキュメントのデータを描画します
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// 検索ハンドラーのサポート
void CCollabAppDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// ドキュメントのデータから検索コンテンツを設定します。
	// コンテンツの各部分は ";" で区切る必要があります

	// 例:      strSearchContent = _T("point;rectangle;circle;ole object;");
	SetSearchContent(strSearchContent);
}

void CCollabAppDoc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = nullptr;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != nullptr)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// CCollabAppDoc の診断

#ifdef _DEBUG
void CCollabAppDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CCollabAppDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CCollabAppDoc コマンド

void CCollabAppDoc::ExecuteOutProcessWork()
{
	if( !m_pipeCom )
	{
		m_pipeCom = std::make_unique<CPipeCommunication>();
		if( !m_pipeCom->InitializeCommunication() )
		{
			UpdateAllViews( nullptr, UpdateHint::GetPipeMsg );	//	内部エラーメッセージは全部吸い上げてリセットとする
			m_pipeCom.reset();
		}
	}
	//	まぁ
	m_pipeCom->ExecuteOutProcessWork();
}
bool CCollabAppDoc::IsExistPipeMessage()
{
	if( m_pipeCom )
	{
		return m_pipeCom->IsExistPipeMessage();
	}
	return false;
}
bool CCollabAppDoc::GetPipeMessage( CString& msg )
{
	if( m_pipeCom )
	{
		return m_pipeCom->GetPipeMessage( msg );
	}
	return false;
}
#if 0
LPCTSTR	CCollabAppDoc::GetPipeName() const
{
	return PIPE_NAME;
}

void CCollabAppDoc::CreatePipeThread()
{
	if( m_pThread == nullptr )
	{
		//	従来の待機スタイルということにして、スレッドを構築する
		m_pThread = AfxBeginThread( threadProc, this, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED );
		if( m_pThread != nullptr )
		{
			m_pThread->m_bAutoDelete = FALSE;
			m_pThread->ResumeThread();
		}
		else
		{
			auto lastError = GetLastError();
			_com_error err( HRESULT_FROM_WIN32( lastError ) );
			SendPipeMessage( err.ErrorMessage(), true );
		}
	}
}
void CCollabAppDoc::OnUpdateFromThread( WPARAM wParam, LPARAM lParam )
{
	//	ロックタイムは細切れの場合でも最小に!
	{
		CComCritSecLock	lock( m_lock );
		if( m_pipeMsg.empty() )
		{
			return;
		}
	}
	//	シングル通知
	if( wParam != FALSE )
	{
		UpdateAllViews( nullptr, UpdateHint::OneMsg );
	}
	//	全部処理
	else
	{
		UpdateAllViews( nullptr, UpdateHint::AllMsg );
	}
}
void CCollabAppDoc::SendPipeMessage( LPCTSTR message, bool send, BOOL dispOne )
{
	bool pipeIsEmpty = true;
	{
		CComCritSecLock	lock( m_lock );
		if( message != nullptr )
		{
			m_pipeMsg.push_back( message );
		}
		pipeIsEmpty = m_pipeMsg.empty();
	}
	//	メッセージで同期化する仕組みを入れておく
	if( send && !pipeIsEmpty )
	{
		auto pos = GetFirstViewPosition();
		if( pos != nullptr )
		{
			CView* pView = GetNextView( pos );
			if( pView != nullptr )
			{
				if( dispOne )
				{
					pView->SendMessage( WMA_UPDATE_FROM_THREAD, dispOne );
				}
				else
				{
					pView->PostMessage( WMA_UPDATE_FROM_THREAD, dispOne );
				}
			}
		}
	}
}

UINT AFX_CDECL CCollabAppDoc::threadProc( LPVOID pThis )
{
	CCollabAppDoc* pDoc = static_cast<CCollabAppDoc*>(pThis);
	while( pDoc->m_exitThread == false )
	{
		HANDLE pipe = pDoc->WaitConnectPipe();
		if( pipe != INVALID_HANDLE_VALUE )
		{
			pDoc->CommunicationToClient( pipe );
		}
	}
	return ERROR_SUCCESS;
}
HANDLE CCollabAppDoc::WaitConnectPipe()
{
	HANDLE pipe = CreateNamedPipe(
		PIPE_FULLNAME,					// パイプの名前
		PIPE_ACCESS_INBOUND,		// 一方的な受信のみ
		PIPE_TYPE_MESSAGE |			// 書き込みモード(メッセージモードはワード単位)
		PIPE_READMODE_MESSAGE |		// 読み取りモード
		PIPE_WAIT,					// ブロッキングモード(PIPE_NOWAITは今は使わないので必ずブロッキングモードにする)
		PIPE_UNLIMITED_INSTANCES,	// 最大インスタンス数は無制限
		BUFF_SIZE,					// 読み取り用バッファサイズ
		BUFF_SIZE,					// 書き込み用バッファサイズ
		0,							// クライアントがWaitNamedPipe で NMPWAIT_USE_DEFAULT_WAIT を指定した時の待ち時間
		nullptr );					// セキュリティ記述子(SECURITY_ATTRIBUTES*)

	if( pipe == INVALID_HANDLE_VALUE )
	{
		auto lastError = GetLastError();
		_com_error err( HRESULT_FROM_WIN32( lastError ) );
		SendPipeMessage( err.ErrorMessage(), true );
	}
	else
	{
		auto pipeConnected = ConnectNamedPipe( pipe, nullptr );
		//	アプリ終了の設定になっていれば、接続したものを無視してそのまま閉じる
		if( m_exitThread )
		{
			pipeConnected = FALSE;
		}
		else if( !pipeConnected )
		{
			auto errorCode = GetLastError();
			//	CreateNamedPipeした時点でパイプにつながっていた場合(クライアント側が作るほうが先の場合)
			if( errorCode == ERROR_PIPE_CONNECTED )
			{
				pipeConnected = TRUE;
			}
			else
			{
				_com_error err( HRESULT_FROM_WIN32( errorCode ) );
				SendPipeMessage( err.ErrorMessage(), true );
			}
		}
		if( !pipeConnected )
		{
			if( pipe != INVALID_HANDLE_VALUE )
			{
				CloseHandle( pipe );
				pipe = INVALID_HANDLE_VALUE;
			}
		}
	}
	return pipe;
}
void CCollabAppDoc::CommunicationToClient( HANDLE hPipe )
{
	_ASSERTE( hPipe != INVALID_HANDLE_VALUE );
	//	ここで、あらかじめ決めておいた読み書きが行われる。今回は、MSDNのサンプルと同じ段取り
	CAtlFile	pipe( hPipe );
	auto CheckError = []( CCollabAppDoc* pDoc, HRESULT hRes, DWORD readLength ) -> bool
	{
		//	さらにデータがある場合は成功とみなしてよい
		if( HRESULT_CODE( hRes ) == ERROR_MORE_DATA )
		{
			hRes = S_OK;	//	まだデータがある場合は成功とみなす
		}
		if( FAILED( hRes ) || readLength == 0 )
		{
			if( FAILED( hRes ) && HRESULT_CODE( hRes ) != ERROR_BROKEN_PIPE )
			{
				_com_error err( hRes );
				pDoc->SendPipeMessage( err.ErrorMessage(), true );
			}
			return true;
		}
		return false;
	};
	for( ;;)
	{
		wchar_t	buffer[BUFF_SIZE];
		DWORD sendLength;
		DWORD	readed;
		auto result = pipe.Read( &sendLength, sizeof( sendLength ), readed );
		if( CheckError( this, result, readed ) )
		{
			break;
		}
		//	ここは本当は尺取り虫をしないと駄目だけどサンプルなので手を抜く
		result = pipe.Read( buffer, sendLength, readed );
		if( CheckError( this, result, readed ) )
		{
			break;
		}
		buffer[sendLength/sizeof(wchar_t)] = L'\0';
		SendPipeMessage( buffer, true );
	}
	DisconnectNamedPipe( pipe );
}
#endif
