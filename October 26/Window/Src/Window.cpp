/*=============================================================================
	Window.cpp: GUI window management code.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "..\..\Engine\Src\EnginePrivate.h"

/*-----------------------------------------------------------------------------
	Globals.
-----------------------------------------------------------------------------*/

WNDPROC WTabControl::SuperProc;
WNDPROC WLabel::SuperProc;
WNDPROC WGroupBox::SuperProc;
WNDPROC WCustomLabel::SuperProc;
WNDPROC WListView::SuperProc;
WNDPROC WEdit::SuperProc;
WNDPROC WRichEdit::SuperProc;
WNDPROC WListBox::SuperProc;
WNDPROC WCheckListBox::SuperProc;
WNDPROC WTrackBar::SuperProc;
WNDPROC WProgressBar::SuperProc;
WNDPROC WComboBox::SuperProc;
WNDPROC WButton::SuperProc;
WNDPROC WPropertySheet::SuperProc;
WNDPROC WToolTip::SuperProc;
WNDPROC WCoolButton::SuperProc;
WNDPROC WUrlButton::SuperProc;
WNDPROC WCheckBox::SuperProc;
WNDPROC WVScrollBar::SuperProc;
WNDPROC WTreeView::SuperProc;
INT WWindow::ModalCount=0;
TArray<WWindow*> WWindow::_Windows;
TArray<WWindow*> WWindow::_DeleteWindows;
TArray<WProperties*> WProperties::PropertiesWindows;
WINDOW_API WLog* GLogWindow=NULL;
WINDOW_API HBRUSH hBrushBlack;
WINDOW_API HBRUSH hBrushWhite;
WINDOW_API HBRUSH hBrushOffWhite;
WINDOW_API HBRUSH hBrushHeadline;
WINDOW_API HBRUSH hBrushStipple;
WINDOW_API HBRUSH hBrushCurrent;
WINDOW_API HBRUSH hBrushDark;
WINDOW_API HBRUSH hBrushGrey;
WINDOW_API HFONT hFontUrl;
WINDOW_API HFONT hFontText;
WINDOW_API HFONT hFontHeadline;
WINDOW_API HINSTANCE hInstanceWindow;
WINDOW_API UBOOL GNotify=0;
WCoolButton* WCoolButton::GlobalCoolButton=NULL;
WINDOW_API UINT WindowMessageOpen;
WINDOW_API UINT WindowMessageMouseWheel;
WINDOW_API NOTIFYICONDATA NID;
#if UNICODE
WINDOW_API NOTIFYICONDATAA NIDA;
WINDOW_API BOOL (WINAPI* Shell_NotifyIconWX)( DWORD dwMessage, PNOTIFYICONDATAW pnid )=NULL;
WINDOW_API BOOL (WINAPI* SHGetSpecialFolderPathWX)( HWND hwndOwner, LPTSTR lpszPath, INT nFolder, BOOL fCreate );
#endif

/*-----------------------------------------------------------------------------
	Window manager.
-----------------------------------------------------------------------------*/

W_IMPLEMENT_CLASS(WWindow)
W_IMPLEMENT_CLASS(WControl)
W_IMPLEMENT_CLASS(WTabControl)
W_IMPLEMENT_CLASS(WLabel)
W_IMPLEMENT_CLASS(WGroupBox)
W_IMPLEMENT_CLASS(WCustomLabel)
W_IMPLEMENT_CLASS(WListView)
W_IMPLEMENT_CLASS(WButton)
W_IMPLEMENT_CLASS(WPropertySheet)
W_IMPLEMENT_CLASS(WToolTip)
W_IMPLEMENT_CLASS(WCoolButton)
W_IMPLEMENT_CLASS(WUrlButton)
W_IMPLEMENT_CLASS(WComboBox)
W_IMPLEMENT_CLASS(WEdit)
W_IMPLEMENT_CLASS(WRichEdit)
W_IMPLEMENT_CLASS(WTerminalBase)
W_IMPLEMENT_CLASS(WTerminal)
W_IMPLEMENT_CLASS(WLog)
W_IMPLEMENT_CLASS(WDialog)
W_IMPLEMENT_CLASS(WPasswordDialog)
W_IMPLEMENT_CLASS(WTextScrollerDialog)
W_IMPLEMENT_CLASS(WTrackBar)
W_IMPLEMENT_CLASS(WProgressBar)
W_IMPLEMENT_CLASS(WListBox)
W_IMPLEMENT_CLASS(WItemBox)
W_IMPLEMENT_CLASS(WCheckListBox)
W_IMPLEMENT_CLASS(WPropertiesBase)
W_IMPLEMENT_CLASS(WDragInterceptor)
W_IMPLEMENT_CLASS(WPictureButton)
W_IMPLEMENT_CLASS(WPropertyPage)
W_IMPLEMENT_CLASS(WProperties)
W_IMPLEMENT_CLASS(WObjectProperties)
W_IMPLEMENT_CLASS(WClassProperties)
W_IMPLEMENT_CLASS(WConfigProperties)
W_IMPLEMENT_CLASS(WWizardPage)
W_IMPLEMENT_CLASS(WWizardDialog)
W_IMPLEMENT_CLASS(WEditTerminal)
W_IMPLEMENT_CLASS(WCheckBox)
W_IMPLEMENT_CLASS(WVScrollBar)
W_IMPLEMENT_CLASS(WTreeView)

class UWindowManager : public USubsystem
{
	DECLARE_CLASS(UWindowManager,UObject,CLASS_Transient);

	// Constructor.
	UWindowManager()
	{
		// Init common controls.
		InitCommonControls();

		// Get addresses of procedures that don't exist in Windows 95.
#if UNICODE
		HMODULE hModShell32 = GetModuleHandle( TEXT("SHELL32.DLL") );
		*(FARPROC*)&Shell_NotifyIconWX       = GetProcAddress( hModShell32, "Shell_NotifyIconW" );
		*(FARPROC*)&SHGetSpecialFolderPathWX = GetProcAddress( hModShell32, "SHGetSpecialFolderPathW" );
#endif

		// Save instance.
		hInstanceWindow = hInstance;

		LoadLibraryX(TEXT("RICHED32.DLL"));

		// Implement window classes.
		IMPLEMENT_WINDOWSUBCLASS(WListBox,TEXT("LISTBOX"));
		IMPLEMENT_WINDOWSUBCLASS(WItemBox,TEXT("LISTBOX"));
		IMPLEMENT_WINDOWSUBCLASS(WCheckListBox,TEXT("LISTBOX"));
		IMPLEMENT_WINDOWSUBCLASS(WTabControl,WC_TABCONTROL);
		IMPLEMENT_WINDOWSUBCLASS(WLabel,TEXT("STATIC"));
		IMPLEMENT_WINDOWSUBCLASS(WGroupBox,TEXT("BUTTON"));
		IMPLEMENT_WINDOWSUBCLASS(WCustomLabel,TEXT("STATIC"));
		IMPLEMENT_WINDOWSUBCLASS(WListView,TEXT("SysListView32"));
		IMPLEMENT_WINDOWSUBCLASS(WEdit,TEXT("EDIT"));
		IMPLEMENT_WINDOWSUBCLASS(WRichEdit,TEXT("RICHEDIT"));
		IMPLEMENT_WINDOWSUBCLASS(WComboBox,TEXT("COMBOBOX"));
		IMPLEMENT_WINDOWSUBCLASS(WEditTerminal,TEXT("EDIT"));
		IMPLEMENT_WINDOWSUBCLASS(WButton,TEXT("BUTTON"));
		IMPLEMENT_WINDOWSUBCLASS(WPropertySheet,TEXT("STATIC"));
		IMPLEMENT_WINDOWSUBCLASS(WToolTip,TOOLTIPS_CLASS);
		IMPLEMENT_WINDOWSUBCLASS(WCoolButton,TEXT("BUTTON"));
		IMPLEMENT_WINDOWSUBCLASS(WUrlButton,TEXT("BUTTON"));
		IMPLEMENT_WINDOWSUBCLASS(WCheckBox,TEXT("BUTTON"));
		IMPLEMENT_WINDOWSUBCLASS(WVScrollBar,TEXT("SCROLLBAR"));
		IMPLEMENT_WINDOWSUBCLASS(WTreeView,WC_TREEVIEW);
		IMPLEMENT_WINDOWSUBCLASS(WTrackBar,TRACKBAR_CLASS);
		IMPLEMENT_WINDOWSUBCLASS(WProgressBar,PROGRESS_CLASS);

		IMPLEMENT_WINDOWCLASS(WTerminal,CS_DBLCLKS);
		IMPLEMENT_WINDOWCLASS(WLog,CS_DBLCLKS);
		IMPLEMENT_WINDOWCLASS(WPasswordDialog,CS_DBLCLKS);
		IMPLEMENT_WINDOWCLASS(WTextScrollerDialog,CS_DBLCLKS);
		IMPLEMENT_WINDOWCLASS(WProperties,CS_DBLCLKS);
		IMPLEMENT_WINDOWCLASS(WObjectProperties,CS_DBLCLKS);
		IMPLEMENT_WINDOWCLASS(WConfigProperties,CS_DBLCLKS);
		IMPLEMENT_WINDOWCLASS(WClassProperties,CS_DBLCLKS);
		IMPLEMENT_WINDOWCLASS(WWizardDialog,0);
		IMPLEMENT_WINDOWCLASS(WWizardPage,0);
		IMPLEMENT_WINDOWCLASS(WDragInterceptor,CS_DBLCLKS);
		IMPLEMENT_WINDOWCLASS(WPictureButton,CS_DBLCLKS);
		IMPLEMENT_WINDOWCLASS(WPropertyPage,CS_DBLCLKS);
		//WC_HEADER (InitCommonControls)
		//UPDOWN_CLASS (InitCommonControls)
		//STATUSCLASSNAME (InitCommonControls)
		//TOOLBARCLASSNAME (InitCommonControls)

		// Create brushes.
		hBrushBlack    = CreateSolidBrush( RGB(0,0,0) );
		hBrushWhite    = CreateSolidBrush( RGB(255,255,255) );
		hBrushOffWhite = CreateSolidBrush( RGB(224,224,224) );
		hBrushHeadline = CreateSolidBrush( RGB(200,200,200) );
		hBrushCurrent  = CreateSolidBrush( RGB(0,0,128) );
		hBrushDark     = CreateSolidBrush( RGB(64,64,64) );
		hBrushGrey     = CreateSolidBrush( RGB(128,128,128) );

		// Create stipple brush.
		WORD Pat[8];
		for( INT i = 0; i < 8; i++ )
			Pat[i] = (WORD)(0x5555 << (i & 1));
		HBITMAP Bitmap = CreateBitmap( 8, 8, 1, 1, &Pat );
		check(Bitmap);
		hBrushStipple = CreatePatternBrush(Bitmap);
		DeleteObject(Bitmap);

		// Create fonts.
		HDC hDC       = GetDC( NULL );
#ifndef JAPANESE
		hFontText     = CreateFont( -MulDiv(9/*PointSize*/,  GetDeviceCaps(hDC, LOGPIXELSY), 72), 0, 0, 0, 0, 0, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, TEXT("Arial") );
		hFontUrl      = CreateFont( -MulDiv(9/*PointSize*/,  GetDeviceCaps(hDC, LOGPIXELSY), 72), 0, 0, 0, 0, 0, 1, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, TEXT("Arial") );
		hFontHeadline = CreateFont( -MulDiv(15/*PointSize*/, GetDeviceCaps(hDC, LOGPIXELSY), 72), 0, 0, FW_BOLD, 1, 1, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, TEXT("Arial") );
#else
		hFontText     = (HFONT)( GetStockObject( DEFAULT_GUI_FONT ) );
		hFontUrl      = (HFONT)( GetStockObject( DEFAULT_GUI_FONT ) );
		hFontHeadline = (HFONT)( GetStockObject( DEFAULT_GUI_FONT ) );
#endif
		ReleaseDC( NULL, hDC );

		// Custom window messages.
		WindowMessageOpen       = RegisterWindowMessageX( TEXT("UnrealOpen") );
		WindowMessageMouseWheel = RegisterWindowMessageX( TEXT("MSWHEEL_ROLLMSG") );
	}

	// FExec interface.
	UBOOL Exec( const TCHAR* Cmd, FOutputDevice& Ar )
	{
		return 0;
	}

	// UObject interface.
	void Serialize( FArchive& Ar )
	{
		Super::Serialize( Ar );
		for( INT i=0; i<WWindow::_Windows.Num(); i++ )
			WWindow::_Windows(i)->Serialize( Ar );
		for( i=0; i<WWindow::_DeleteWindows.Num(); i++ )
			WWindow::_DeleteWindows(i)->Serialize( Ar );
	}
	void Destroy()
	{
		Super::Destroy();
		check(GWindowManager==this);
		GWindowManager = NULL;
		if( !GIsCriticalError )
			Tick( 0.0 );
		WWindow::_Windows.Empty();
		WWindow::_DeleteWindows.Empty();
		WProperties::PropertiesWindows.Empty();
	}

	// USubsystem interface.
	void __fastcall Tick( FLOAT DeltaTime )
	{
		while( WWindow::_DeleteWindows.Num() )
		{
			WWindow* W = WWindow::_DeleteWindows( 0 );
			delete W;
			check(WWindow::_DeleteWindows.FindItemIndex(W)==INDEX_NONE);
		}
	}
};
IMPLEMENT_CLASS(UWindowManager);

/*-----------------------------------------------------------------------------
	Functions.
-----------------------------------------------------------------------------*/

WINDOW_API HBITMAP LoadFileToBitmap( const TCHAR* Filename, INT& SizeX, INT& SizeY )
{
	TArray<BYTE> Bitmap;
	if( appLoadFileToArray(Bitmap,Filename) )
	{
		HDC              hDC = GetDC(NULL);
		BITMAPFILEHEADER* FH = (BITMAPFILEHEADER*)&Bitmap(0);
		BITMAPINFO*       BM = (BITMAPINFO      *)(FH+1);
		BITMAPINFOHEADER* IH = (BITMAPINFOHEADER*)(FH+1);
		//RGBQUAD*          RQ = (RGBQUAD         *)(IH+1);
		BYTE*             BY = (BYTE            *)(IH+1); //(RQ+(1<<IH->biBitCount));
		SizeX                = IH->biWidth;
		SizeY                = IH->biHeight;
		HBITMAP      hBitmap = CreateDIBitmap( hDC, IH, CBM_INIT, BY, BM, 0/*DIB_RGB_COLORS*/ );
		ReleaseDC( NULL, hDC );
		return hBitmap;
	}
	return NULL;
}

WINDOW_API void InitWindowing()
{
	GWindowManager = new UWindowManager;
	GWindowManager->AddToRoot();
}

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
