
// CollabAppDoc.h : CCollabAppDoc クラスのインターフェイス
//


#pragma once
#include <deque>
#include <atomic>

#define WMA_UPDATE_FROM_THREAD	(WM_APP+1)

class CPipeData : public CObject
{
	DECLARE_DYNAMIC( CPipeData )
public:
	CPipeData( const CPipeData& ) = delete;
	CPipeData& operator=( const CPipeData& ) = delete;

	CPipeData( LPCTSTR lineData )
		: m_lineData( lineData )
	{
	}
public:
	CString m_lineData;
};

class CCollabAppDoc : public CDocument
{
protected: // シリアル化からのみ作成します。
	CCollabAppDoc() noexcept;
	DECLARE_DYNCREATE(CCollabAppDoc)

// 属性
public:
//	パイプ処理関連
private:
	std::atomic_bool	m_exitThread;	//	スレッド終了フラグ
	CWinThread*	m_pThread;
	std::deque<CString>	m_pipeMsg;

	static UINT AFX_CDECL threadProc( LPVOID );
	HANDLE	WaitConnectPipe();
	void	CommunicationToClient( HANDLE hPipe );
	void	SendPipeMessage( LPCTSTR message, bool send, BOOL dispOne = FALSE );
	// 操作
public:
	LPCTSTR	GetPipeName() const;
	void	CreatePipeThread();
	void	OnUpdateFromThread( WPARAM wParam, LPARAM lParam );

// オーバーライド
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

// 実装
public:
	virtual ~CCollabAppDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 生成された、メッセージ割り当て関数
protected:
	DECLARE_MESSAGE_MAP()

#ifdef SHARED_HANDLERS
	// 検索ハンドラーの検索コンテンツを設定するヘルパー関数
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS
};
