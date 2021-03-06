
// CollabApp.h : CollabApp アプリケーションのメイン ヘッダー ファイル
//
#pragma once

#ifndef __AFXWIN_H__
	#error "PCH に対してこのファイルをインクルードする前に 'stdafx.h' をインクルードしてください"
#endif

#include "resource.h"       // メイン シンボル


// CCollabAppApp:
// このクラスの実装については、CollabApp.cpp を参照してください
//

class CCollabAppApp : public CWinApp
{
public:
	CCollabAppApp() noexcept;


// オーバーライド
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// 実装
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CCollabAppApp theApp;
