/*=============================================================================
	Window.h: GUI window management code.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "..\..\core\inc\unmsg.h"
#include <richedit.h>

/*-----------------------------------------------------------------------------
	Defines.
-----------------------------------------------------------------------------*/

#ifndef WINDOW_API
#define WINDOW_API __declspec(dllimport)
#endif

#define WIN_OBJ 0

/*-----------------------------------------------------------------------------
	Unicode support.
-----------------------------------------------------------------------------*/

#define RegSetValueExX(a,b,c,d,e,f)		TCHAR_CALL_OS(RegSetValueExW(a,b,c,d,e,f),RegSetValueExA(a,TCHAR_TO_ANSI(b),c,d,(BYTE*)TCHAR_TO_ANSI((TCHAR*)e),f))
#define RegSetValueX(a,b,c,d,e)			TCHAR_CALL_OS(RegSetValueW(a,b,c,d,e),RegSetValueA(a,TCHAR_TO_ANSI(b),c,TCHAR_TO_ANSI(d),e))
#define RegCreateKeyX(a,b,c)			TCHAR_CALL_OS(RegCreateKeyW(a,b,c),RegCreateKeyA(a,TCHAR_TO_ANSI(b),c))
#define RegQueryValueX(a,b,c,d)			TCHAR_CALL_OS(RegQueryValueW(a,b,c,d),RegQueryValueW(a,TCHAR_TO_ANSI(b),TCHAR_TO_ANSI(c),d))
#define RegOpenKeyX(a,b,c)				TCHAR_CALL_OS(RegOpenKeyW(a,b,c),RegOpenKeyA(a,TCHAR_TO_ANSI(b),c))
#define RegDeleteKeyX(a,b)				TCHAR_CALL_OS(RegDeleteKeyW(a,b),RegDeleteKeyA(a,TCHAR_TO_ANSI(b)))
#define RegDeleteValueX(a,b)			TCHAR_CALL_OS(RegDeleteValueW(a,b),RegDeleteValueA(a,TCHAR_TO_ANSI(b)))
#define RegQueryInfoKeyX(a,b)			TCHAR_CALL_OS(RegQueryInfoKeyW(a,NULL,NULL,NULL,b,NULL,NULL,NULL,NULL,NULL,NULL,NULL),RegQueryInfoKeyA(a,NULL,NULL,NULL,b,NULL,NULL,NULL,NULL,NULL,NULL,NULL))
#define RegOpenKeyExX(a,b,c,d,e)        TCHAR_CALL_OS(RegOpenKeyExW(a,b,c,d,e),RegOpenKeyExA(a,TCHAR_TO_ANSI(b),c,d,e))
#define LookupPrivilegeValueX(a,b,c)	TCHAR_CALL_OS(LookupPrivilegeValueW(a,b,c),LookupPrivilegeValueA(TCHAR_TO_ANSI(a),TCHAR_TO_ANSI(b),c))
#define GetDriveTypeX(a)				TCHAR_CALL_OS(GetDriveTypeW(a),GetDriveTypeA(TCHAR_TO_ANSI(a)))
#define GetDiskFreeSpaceX(a,b,c,d,e)	TCHAR_CALL_OS(GetDiskFreeSpaceW(a,b,c,d,e),GetDiskFreeSpaceA(TCHAR_TO_ANSI(a),b,c,d,e))
#define SetFileAttributesX(a,b)			TCHAR_CALL_OS(SetFileAttributesW(a,b),SetFileAttributesA(TCHAR_TO_ANSI(a),b))
#define DrawTextExX(a,b,c,d,e,f)		TCHAR_CALL_OS(DrawTextExW(a,b,c,d,e,f),DrawTextExA(a,const_cast<ANSICHAR*>(TCHAR_TO_ANSI(b)),c,d,e,f))
#define DrawTextX(a,b,c,d,e)			TCHAR_CALL_OS(DrawTextW(a,b,c,d,e),DrawTextA(a,TCHAR_TO_ANSI(b),c,d,e))
#define GetTextExtentPoint32X(a,b,c,d)  TCHAR_CALL_OS(GetTextExtentPoint32W(a,b,c,d),GetTextExtentPoint32A(a,TCHAR_TO_ANSI(b),c,d))
#define DefMDIChildProcX(a,b,c,d)		TCHAR_CALL_OS(DefMDIChildProcW(a,b,c,d),DefMDIChildProcA(a,b,c,d))
#define SetClassLongX(a,b,c)			TCHAR_CALL_OS(SetClassLongW(a,b,c),SetClassLongA(a,b,c))
#define GetClassLongX(a,b)				TCHAR_CALL_OS(GetClassLongW(a,b),GetClassLongA(a,b))
#define RemovePropX(a,b)				TCHAR_CALL_OS(RemovePropW(a,b),RemovePropA(a,TCHAR_TO_ANSI(b)))
#define GetPropX(a,b)					TCHAR_CALL_OS(GetPropW(a,b),GetPropA(a,TCHAR_TO_ANSI(b)))
#define SetPropX(a,b,c)					TCHAR_CALL_OS(SetPropW(a,b,c),SetPropA(a,TCHAR_TO_ANSI(b),c))
#define ShellExecuteX(a,b,c,d,e,f)      TCHAR_CALL_OS(ShellExecuteW(a,b,c,d,e,f),ShellExecuteA(a,TCHAR_TO_ANSI(b),TCHAR_TO_ANSI(c),TCHAR_TO_ANSI(d),TCHAR_TO_ANSI(e),f))
#define CreateMutexX(a,b,c)				TCHAR_CALL_OS(CreateMutexW(a,b,c),CreateMutexA(a,b,TCHAR_TO_ANSI(c)))
#define DefFrameProcX(a,b,c,d,e)		TCHAR_CALL_OS(DefFrameProcW(a,b,c,d,e),DefFrameProcA(a,b,c,d,e))
#define RegisterWindowMessageX(a)       TCHAR_CALL_OS(RegisterWindowMessageW(a),RegisterWindowMessageA(TCHAR_TO_ANSI(a)))
#define AppendMenuX(a,b,c,d)            TCHAR_CALL_OS(AppendMenuW(a,b,c,d),AppendMenuA(a,b,c,TCHAR_TO_ANSI(d)))
#define LoadLibraryX(a)					TCHAR_CALL_OS(LoadLibraryW(a),LoadLibraryA(TCHAR_TO_ANSI(a)))
#define SystemParametersInfoX(a,b,c,d)	TCHAR_CALL_OS(SystemParametersInfoW(a,b,c,d),SystemParametersInfoA(a,b,c,d))
#define DispatchMessageX(a)				TCHAR_CALL_OS(DispatchMessageW(a),DispatchMessageA(a))
#define PeekMessageX(a,b,c,d,e)			TCHAR_CALL_OS(PeekMessageW(a,b,c,d,e),PeekMessageA(a,b,c,d,e))
#define PostMessageX(a,b,c,d)			TCHAR_CALL_OS(PostMessageW(a,b,c,d),PostMessageA(a,b,c,d))
#define SendMessageX(a,b,c,d)			TCHAR_CALL_OS(SendMessageW(a,b,c,d),SendMessageA(a,b,c,d))
#define SendMessageLX(a,b,c,d)			TCHAR_CALL_OS(SendMessageW(a,b,c,(LPARAM)d),SendMessageA(a,b,c,(LPARAM)TCHAR_TO_ANSI(d)))
#define SendMessageWX(a,b,c,d)			TCHAR_CALL_OS(SendMessageW(a,b,(WPARAM)c,d),SendMessageA(a,b,(WPARAM)TCHAR_TO_ANSI(c),d))
#define DefWindowProcX(a,b,c,d)			TCHAR_CALL_OS(DefWindowProcW(a,b,c,d),DefWindowProcA(a,b,c,d))
#define CallWindowProcX(a,b,c,d,e)		TCHAR_CALL_OS(CallWindowProcW(a,b,c,d,e),CallWindowProcA(a,b,c,d,e))
#define GetWindowLongX(a,b)				TCHAR_CALL_OS(GetWindowLongW(a,b),GetWindowLongA(a,b))
#define SetWindowLongX(a,b,c)			TCHAR_CALL_OS(SetWindowLongW(a,b,c),SetWindowLongA(a,b,c))
#define LoadMenuIdX(i,n)				TCHAR_CALL_OS(LoadMenuW(i,MAKEINTRESOURCEW(n)),LoadMenuA(i,MAKEINTRESOURCEA(n)))
#define LoadCursorIdX(i,n)				TCHAR_CALL_OS(LoadCursorW(i,MAKEINTRESOURCEW(n)),LoadCursorA(i,MAKEINTRESOURCEA(n)))

// For some reason, this call will crash when trying to load icons in DEBUG mode.  So I disabled it for now.
#ifdef _DEBUG
	#define LoadIconIdX(i,n)			NULL
#else
	#define LoadIconIdX(i,n)			TCHAR_CALL_OS(LoadIconW(i,MAKEINTRESOURCEW(n)),LoadIconA(i,MAKEINTRESOURCEA(n)))
#endif

inline DWORD GetTempPathX( DWORD nBufferLength, LPTSTR lpBuffer )
{
	DWORD Result;
#if UNICODE
	if( !GUnicodeOS )
	{
		ANSICHAR ACh[MAX_PATH]="";
		Result = GetTempPathA( ARRAY_COUNT(ACh), ACh );
		appStrncpy( lpBuffer, appFromAnsi(ACh), nBufferLength );
	}
	else
#endif
	{
		Result = GetTempPath( nBufferLength, lpBuffer );
	}
	return Result;
}
inline LONG RegQueryValueExX( HKEY hKey, LPCTSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData )
{
#if UNICODE
	if( !GUnicodeOS )
	{
		ANSICHAR* ACh = (ANSICHAR*)appAlloca(*lpcbData);
		LONG Result = RegQueryValueExA( hKey, TCHAR_TO_ANSI(lpValueName), lpReserved, lpType, (BYTE*)ACh, lpcbData );
		if( Result==ERROR_SUCCESS )
			MultiByteToWideChar( CP_ACP, 0, ACh, -1, (TCHAR*)lpData, *lpcbData );
		return Result;
	}
	else
#endif
	{
		return RegQueryValueEx( hKey, lpValueName, lpReserved, lpType, lpData, lpcbData );
	}
}

#if UNICODE
	#define MAKEINTRESOURCEX(a)  MAKEINTRESOURCEA(a)
	#define OSVERSIONINFOX OSVERSIONINFOA
	#define GetVersionExX GetVersionExA
	extern WINDOW_API BOOL (WINAPI* Shell_NotifyIconWX)( DWORD dwMessage, PNOTIFYICONDATAW pnid );
	extern WINDOW_API BOOL (WINAPI* SHGetSpecialFolderPathWX)( HWND hwndOwner, LPTSTR lpszPath, INT nFolder, BOOL fCreate );
	inline HRESULT SHGetSpecialFolderPathX( HWND hwndOwner, LPTSTR lpszPath, INT nFolder, BOOL fCreate )
	{
		if( !GUnicodeOS || !SHGetSpecialFolderPathWX )
		{
			ANSICHAR ACh[MAX_PATH];
			ITEMIDLIST* IdList=NULL;
#if 1 /* Needed because Windows 95 doesn't export SHGetSpecialFolderPath */
			HRESULT Result = SHGetSpecialFolderLocation( NULL, nFolder, &IdList );
			SHGetPathFromIDListA( IdList, ACh );
#else
			HRESULT Result = SHGetSpecialFolderPathA( hwndOwner, ACh, nFolder, fCreate );
#endif
			MultiByteToWideChar( CP_ACP, 0, ACh, -1, lpszPath, MAX_PATH );
			//MultiByteToWideChar( CP_OCP, 0, ACh, -1, lpszPath, MAX_PATH );//!!OCP experimental!!
			return Result;
		}
		else return SHGetSpecialFolderPathWX( hwndOwner, lpszPath, nFolder, fCreate );
	}
#else
	#define MAKEINTRESOURCEX(a) MAKEINTRESOURCEA(a)
	#define OSVERSIONINFOX OSVERSIONINFOA
	#define GetVersionExX GetVersionExA
	#define GetTempPathX GetTempPathA
	#define SHGetSpecialFolderPathX(a,b,c,d) SHGetSpecialFolderPathA(a,TCHAR_TO_ANSI(b),c,d)
#endif

/*-----------------------------------------------------------------------------
	Globals.
-----------------------------------------------------------------------------*/

// Classes.
class WWindow;
class WControl;
class WWizardDialog;
class WWizardPage;
class WDragInterceptor;

// Global functions.
WINDOW_API void InitWindowing();
WINDOW_API HBITMAP LoadFileToBitmap( const TCHAR* Filename, INT& SizeX, INT& SizeY );

// Global variables.
extern WINDOW_API HBRUSH hBrushWhite;
extern WINDOW_API HBRUSH hBrushOffWhite;
extern WINDOW_API HBRUSH hBrushHeadline;
extern WINDOW_API HBRUSH hBrushBlack;
extern WINDOW_API HBRUSH hBrushStipple;
extern WINDOW_API HBRUSH hBrushCurrent;
extern WINDOW_API HBRUSH hBrushDark;
extern WINDOW_API HBRUSH hBrushGrey;
extern WINDOW_API HFONT  hFontText;
extern WINDOW_API HFONT  hFontUrl;
extern WINDOW_API HFONT  hFontHeadline;
extern WINDOW_API class  WLog* GLogWindow;
extern WINDOW_API HINSTANCE hInstanceWindow;
extern WINDOW_API UBOOL GNotify;
extern WINDOW_API UINT WindowMessageOpen;
extern WINDOW_API UINT WindowMessageMouseWheel;
extern WINDOW_API NOTIFYICONDATA NID;
#if UNICODE
	extern WINDOW_API NOTIFYICONDATAA NIDA;
#else
	#define NIDA NID
#endif

/*-----------------------------------------------------------------------------
	Window class definition macros.
-----------------------------------------------------------------------------*/

inline void MakeWindowClassName( TCHAR* Result, const TCHAR* Base )
{
	appSprintf( Result, TEXT("%sUnreal%s"), appPackage(), Base );
}

#if WIN_OBJ
	#define DECLARE_WINDOWCLASS(cls,parentcls,pkg) \
	public: \
		void GetWindowClassName( TCHAR* Result ) {MakeWindowClassName(Result,TEXT(#cls));} \
		void Destroy() \
		{ \
			Super::Destroy(); \
			MaybeDestroy(); \
		} \
		virtual const TCHAR* GetPackageName() { return TEXT(#pkg); }
#else
	#define DECLARE_WINDOWCLASS(cls,parentcls,pkg) \
	public: \
		void GetWindowClassName( TCHAR* Result ) {MakeWindowClassName(Result,TEXT(#cls));} \
		~cls() {MaybeDestroy();} \
		virtual const TCHAR* GetPackageName() { return TEXT(#pkg); }
#endif

#define DECLARE_WINDOWSUBCLASS(cls,parentcls,pkg) \
	DECLARE_WINDOWCLASS(cls,parentcls,pkg) \
	static WNDPROC SuperProc;

#define IMPLEMENT_WINDOWCLASS(cls,clsf) \
{\
	TCHAR Temp[256];\
	MakeWindowClassName(Temp,TEXT(#cls));\
	cls::RegisterWindowClass( Temp, clsf );\
}

#define IMPLEMENT_WINDOWSUBCLASS(cls,wincls) \
	{TCHAR Temp[256]; MakeWindowClassName(Temp,TEXT(#cls)); cls::SuperProc = cls::RegisterWindowClass( Temp, wincls );}

#define FIRST_AUTO_CONTROL 8192

/*-----------------------------------------------------------------------------
	FRect.
-----------------------------------------------------------------------------*/

struct FPoint
{
	INT X, Y;
	FPoint()
	{}
	FPoint( INT InX, INT InY )
	:	X( InX )
	,	Y( InY )
	{}
	static FPoint ZeroValue()
	{
		return FPoint(0,0);
	}
	static FPoint NoneValue()
	{
		return FPoint(INDEX_NONE,INDEX_NONE);
	}
	operator POINT*() const
	{
		return (POINT*)this;
	}
	const INT& operator()( INT i ) const
	{
		return (&X)[i];
	}
	INT& operator()( INT i )
	{
		return (&X)[i];
	}
	static INT Num()
	{
		return 2;
	}
	UBOOL operator==( const FPoint& Other ) const
	{
		return X==Other.X && Y==Other.Y;
	}
	UBOOL operator!=( const FPoint& Other ) const
	{
		return X!=Other.X || Y!=Other.Y;
	}
	FPoint& operator+=( const FPoint& Other )
	{
		X += Other.X;
		Y += Other.Y;
		return *this;
	}
	FPoint& operator-=( const FPoint& Other )
	{
		X -= Other.X;
		Y -= Other.Y;
		return *this;
	}
	FPoint operator+( const FPoint& Other ) const
	{
		return FPoint(*this) += Other;
	}
	FPoint operator-( const FPoint& Other ) const
	{
		return FPoint(*this) -= Other;
	}
};

struct FRect
{
	FPoint Min, Max;
	FRect()
	{}
	FRect( INT X0, INT Y0, INT X1, INT Y1 )
	:	Min( X0, Y0 )
	,	Max( X1, Y1 )
	{}
	FRect( FPoint InMin, FPoint InMax )
	:	Min( InMin )
	,	Max( InMax )
	{}
	FRect( RECT R )
	:	Min( R.left, R.top )
	,	Max( R.right, R.bottom )
	{}
	operator RECT*() const
	{
		return (RECT*)this;
	}
	const FPoint& operator()( INT i ) const
	{
		return (&Min)[i];
	}
	FPoint& operator()( INT i )
	{
		return (&Min)[i];
	}
	static INT Num()
	{
		return 2;
	}
	UBOOL operator==( const FRect& Other ) const
	{
		return Min==Other.Min && Max==Other.Max;
	}
	UBOOL operator!=( const FRect& Other ) const
	{
		return Min!=Other.Min || Max!=Other.Max;
	}
	FRect Right( INT Width )
	{
		return FRect( ::Max(Min.X,Max.X-Width), Min.Y, Max.X, Max.Y );
	}
	FRect Bottom( INT Height )
	{
		return FRect( Min.X, ::Max(Min.Y,Max.Y-Height), Max.X, Max.Y );
	}
	FPoint Size()
	{
		return FPoint( Max.X-Min.X, Max.Y-Min.Y );
	}
	INT Width()
	{
		return Max.X-Min.X;
	}
	INT Height()
	{
		return Max.Y-Min.Y;
	}
	FRect& operator+=( const FPoint& P )
	{
		Min += P;
		Max += P;
		return *this;
	}
	FRect& operator-=( const FPoint& P )
	{
		Min -= P;
		Max -= P;
		return *this;
	}
	FRect operator+( const FPoint& P ) const
	{
		return FRect( Min+P, Max+P );
	}
	FRect operator-( const FPoint& P ) const
	{
		return FRect( Min-P, Max-P );
	}
	FRect operator+( const FRect& R ) const
	{
		return FRect( Min+R.Min, Max+R.Max );
	}
	FRect operator-( const FRect& R ) const
	{
		return FRect( Min-R.Min, Max-R.Max );
	}
	FRect Inner( FPoint P ) const
	{
		return FRect( Min+P, Max-P );
	}
	UBOOL Contains( FPoint P ) const
	{
		return P.X>=Min.X && P.X<Max.X && P.Y>=Min.Y && P.Y<Max.Y;
	}
};

/*-----------------------------------------------------------------------------
	FControlSnoop.
-----------------------------------------------------------------------------*/

// For forwarding interaction with a control to an object.
class WINDOW_API FControlSnoop
{
public:
	// FControlSnoop interface.
	virtual void SnoopChar( WWindow* Src, INT Char ) {}
	virtual void SnoopKeyDown( WWindow* Src, INT Char ) {}
	virtual void SnoopLeftMouseDown( WWindow* Src, FPoint P ) {}
	virtual void SnoopRightMouseDown( WWindow* Src, FPoint P ) {}
};

/*-----------------------------------------------------------------------------
	FCommandTarget.
-----------------------------------------------------------------------------*/

//
// Interface for accepting commands.
//
class WINDOW_API FCommandTarget
{
public:
	virtual void Unused() {}
};

//
// Delegate function pointers.
//
typedef void(FCommandTarget::*TDelegate)();
typedef void(FCommandTarget::*TDelegateInt)(INT);

//
// Simple bindings to an object and a member function of that object.
//
struct WINDOW_API FDelegate
{
	FCommandTarget* TargetObject;
	void (FCommandTarget::*TargetInvoke)();
	FDelegate( FCommandTarget* InTargetObject=NULL, TDelegate InTargetInvoke=NULL )
	: TargetObject( InTargetObject )
	, TargetInvoke( InTargetInvoke )
	{}
	virtual void operator()() { if( TargetObject ) (TargetObject->*TargetInvoke)(); }
};
struct WINDOW_API FDelegateInt
{
	FCommandTarget* TargetObject;
	void (FCommandTarget::*TargetInvoke)(int);
	FDelegateInt( FCommandTarget* InTargetObject=NULL, TDelegateInt InTargetInvoke=NULL )
	: TargetObject( InTargetObject )
	, TargetInvoke( InTargetInvoke )
	{}
	virtual void operator()( int I ) { if( TargetObject ) (TargetObject->*TargetInvoke)(I); }
};

// Text formatting.
inline const TCHAR* LineFormat( const TCHAR* In )
{
	static TCHAR Result[4069];
	TCHAR* Ptr = Result;
	while( *In )
		*Ptr++ = *In++!='\\' ? In[-1] : *In++=='n' ? '\n' : In[-1];
	*Ptr++ = 0;
	return Result;
}

/*-----------------------------------------------------------------------------
	Menu helper functions.
-----------------------------------------------------------------------------*/

//
// Load a menu and localize its text.
//
inline void LocalizeSubMenu( HMENU hMenu, const TCHAR* Name, const TCHAR* Package )
{
	for( INT i=GetMenuItemCount(hMenu)-1; i>=0; i-- )
	{
#if UNICODE
		if( GUnicode && !GUnicodeOS )
		{
			MENUITEMINFOA Info;
			ANSICHAR Buffer[1024];
			appMemzero( &Info, sizeof(Info) );
			Info.cbSize     = sizeof(Info);
			Info.fMask      = MIIM_TYPE | MIIM_SUBMENU;
			Info.cch        = ARRAY_COUNT(Buffer);
			Info.dwTypeData = Buffer;
			GetMenuItemInfoA( hMenu, i, 1, &Info );
			const ANSICHAR* String = (const ANSICHAR*)Info.dwTypeData;
			if( String && String[0]=='I' && String[1]=='D' && String[2]=='_' )
			{
				const_cast<const ANSICHAR*&>(Info.dwTypeData) = TCHAR_TO_ANSI(Localize( Name, appFromAnsi(String), Package ));
				SetMenuItemInfoA( hMenu, i, 1, &Info );
			}
			if( Info.hSubMenu )
				LocalizeSubMenu( Info.hSubMenu, Name, Package );
		}
		else
#endif
		{
			MENUITEMINFO Info;
			TCHAR Buffer[1024];
			appMemzero( &Info, sizeof(Info) );
			Info.cbSize     = sizeof(Info);
			Info.fMask      = MIIM_TYPE | MIIM_SUBMENU;
			Info.cch        = ARRAY_COUNT(Buffer);
			Info.dwTypeData = Buffer;
			GetMenuItemInfo( hMenu, i, 1, &Info );
			const TCHAR* String = (const TCHAR*)Info.dwTypeData;
			if( String && String[0]=='I' && String[1]=='D' && String[2]=='_' )
			{
				const_cast<const TCHAR*&>(Info.dwTypeData) = Localize( Name, String, Package );
				SetMenuItemInfo( hMenu, i, 1, &Info );
			}
			if( Info.hSubMenu )
				LocalizeSubMenu( Info.hSubMenu, Name, Package );
		}
	}
}
inline WINDOW_API HMENU LoadLocalizedMenu( HINSTANCE hInstance, INT Id, const TCHAR* Name, const TCHAR* Package=GPackage )
{
	HMENU hMenu = LoadMenuIdX( hInstance, Id );
	if( !hMenu )
		appErrorf( TEXT("Failed loading menu: %s %s"), Package, Name );
	LocalizeSubMenu( hMenu, Name, Package );
	return hMenu;
}

//
// Toggle a menu item and return 0 if it's now off, 1 if it's now on.
//
inline WINDOW_API UBOOL ToggleMenuItem( HMENU hMenu, UBOOL Item )
{
	if( GetMenuState(hMenu,Item,MF_BYCOMMAND)&MF_CHECKED )
	{
		// Now unchecked.
		CheckMenuItem( hMenu, Item, MF_UNCHECKED );
		return 0;
	}
	else
	{
		// Now checked.
		CheckMenuItem( hMenu, Item, MF_CHECKED );
		return 1;
	}
}

/*-----------------------------------------------------------------------------
	FWindowsBitmap.
-----------------------------------------------------------------------------*/

// A bitmap.
class FWindowsBitmap
{
public:
	INT SizeX, SizeY, Keep;
	FWindowsBitmap( UBOOL InKeep=0 )
	: hBitmap( NULL )
	, SizeX( 0 )
	, SizeY( 0 )
	, Keep( InKeep )
	{}
	~FWindowsBitmap()
	{
		if( hBitmap && !Keep )
			DeleteObject( hBitmap );
	}
	UBOOL LoadFile( const TCHAR* Filename )
	{
		if( hBitmap )
			DeleteObject( hBitmap );
		hBitmap = LoadFileToBitmap( Filename, SizeX, SizeY );
		return hBitmap!=NULL;
	}
	HBITMAP GetBitmapHandle()
	{
		return hBitmap;
	}
private:
	HBITMAP hBitmap;
	void operator=( FWindowsBitmap& ) {}
};

/*-----------------------------------------------------------------------------
	WWindow.
-----------------------------------------------------------------------------*/

#if WIN_OBJ
	#define W_DECLARE_ABSTRACT_CLASS(a,b,c) DECLARE_ABSTRACT_CLASS(a,b,c) 
	#define W_DECLARE_CLASS(a,b,c) DECLARE_CLASS(a,b,c) 
	#define W_IMPLEMENT_CLASS(a) IMPLEMENT_CLASS(a)
#else
	#define W_DECLARE_ABSTRACT_CLASS(a,b,c) public:
	#define W_DECLARE_CLASS(a,b,c) public:
	#define W_IMPLEMENT_CLASS(a)
#endif

// An operating system window.
class WINDOW_API WWindow : 
#if WIN_OBJ
public UObject, 
#endif
public FCommandTarget
{
	W_DECLARE_ABSTRACT_CLASS(WWindow,UObject,CLASS_Transient);

	// Variables.
	HWND					hWnd;
	FName					PersistentName;
	WORD					ControlId, TopControlId;
	BITFIELD				Destroyed:1;
	BITFIELD				MdiChild:1;
	WWindow*				OwnerWindow;
	FNotifyHook*			NotifyHook;
	FControlSnoop*			Snoop;
	TArray<class WControl*>	Controls;
	BOOL m_bShow;
	WPARAM LastwParam;	
	LPARAM LastlParam;

	// Static.
	static INT              ModalCount;
	static TArray<WWindow*> _Windows;
	static TArray<WWindow*> _DeleteWindows;
	// Use this procedure for modeless dialogs.
	static INT_PTR CALLBACK StaticDlgProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
	{
		for( INT i=0; i<_Windows.Num(); i++ )
			if( _Windows(i)->hWnd==hwndDlg )
				break;
		if( i==_Windows.Num() && uMsg==WM_INITDIALOG )
		{
			WWindow* WindowCreate = (WWindow*)lParam;
			check(WindowCreate);
			check(!WindowCreate->hWnd);
			WindowCreate->hWnd = hwndDlg;
			for( i=0; i<_Windows.Num(); i++ )
				if( _Windows(i)==WindowCreate )
					break;
			check(i<_Windows.Num());
		}
		if( i!=_Windows.Num() && !GIsCriticalError )
		{
			_Windows(i)->WndProc( uMsg, wParam, lParam );			
		}

		// Give up cycles.
		//
		::Sleep(0);

		return 0;
	}
	static LONG APIENTRY StaticWndProc( HWND hWnd, UINT Message, UINT wParam, LONG lParam )
	{
		for( INT i=0; i<_Windows.Num(); i++ )
			if( _Windows(i)->hWnd==hWnd )
				break;
		if( i==_Windows.Num() && (Message==WM_NCCREATE || Message==WM_INITDIALOG) )
		{
			WWindow* WindowCreate
			=	Message!=WM_NCCREATE
			?	(WWindow*)lParam
			:	(GetWindowLongX(hWnd,GWL_EXSTYLE) & WS_EX_MDICHILD)
			?	(WWindow*)((MDICREATESTRUCT*)((CREATESTRUCT*)lParam)->lpCreateParams)->lParam
			:	(WWindow*)((CREATESTRUCT*)lParam)->lpCreateParams;
			check(WindowCreate);
			check(!WindowCreate->hWnd);
			WindowCreate->hWnd = hWnd;
			for( i=0; i<_Windows.Num(); i++ )
				if( _Windows(i)==WindowCreate )
					break;
			check(i<_Windows.Num());
		}
		if( i==_Windows.Num() || GIsCriticalError )
		{
			// Gets through before WM_NCCREATE: WM_GETMINMAXINFO
			return DefWindowProcX( hWnd, Message, wParam, lParam );
		}
		else
		{
			check(_Windows(i));		// NJS: Somehow NULL this pointers are filtering through.
			return _Windows(i)->WndProc( Message, wParam, lParam );			
		}
	}
	static WNDPROC RegisterWindowClass( const TCHAR* Name, DWORD Style )
	{
#if UNICODE
		if( GUnicodeOS )
		{
			WNDCLASSEXW Cls;
			appMemzero( &Cls, sizeof(Cls) );
			Cls.cbSize			= sizeof(Cls);
			Cls.style			= Style;
			Cls.lpfnWndProc		= StaticWndProc;
			Cls.hInstance		= hInstanceWindow;
			Cls.hIcon			= LoadIcon(hInstanceWindow,MAKEINTRESOURCE(IDICON_Mainframe/*(GIsEditor?IDICON_Editor:IDICON_Mainframe)*/));
			Cls.lpszClassName	= Name;
			Cls.hIconSm			= Cls.hIcon; //LoadIconIdX(hInstanceWindow,(GIsEditor?IDICON_Editor:IDICON_Mainframe));
			verify(RegisterClassExW( &Cls ));
		}
		else
#endif
		{
			WNDCLASSEXA Cls;
			appMemzero( &Cls, sizeof(Cls) );
			Cls.cbSize			= sizeof(Cls);
			Cls.style			= Style;
			Cls.lpfnWndProc		= StaticWndProc;
			Cls.hInstance		= hInstanceWindow;
			Cls.hIcon			= LoadIconIdX(hInstanceWindow,(GIsEditor?IDICON_Editor:IDICON_Mainframe));
			Cls.lpszClassName	= TCHAR_TO_ANSI(Name);
			Cls.hIconSm			= LoadIconIdX(hInstanceWindow,(GIsEditor?IDICON_Editor:IDICON_Mainframe));
			verify(RegisterClassExA( &Cls ));
		}
		return NULL;
	}

	// Structors.
	WWindow( FName InPersistentName=NAME_None, WWindow* InOwnerWindow=NULL )
#if WIN_OBJ
	:	UObject				( EC_InPlaceConstructor, WWindow::StaticClass(), UObject::GetTransientPackage(), NAME_None, 0 )
	,	hWnd				( NULL )
#else
	:	hWnd				( NULL )
#endif
	,	PersistentName		( InPersistentName )
	,	ControlId			( 0 )
	,	TopControlId		( FIRST_AUTO_CONTROL )
	,	Destroyed			( 0 )
	,   MdiChild            ( 0 )
	,	OwnerWindow			( InOwnerWindow )
	,	NotifyHook			( 0 )
	,   Snoop               ( NULL )
	{}
#if WIN_OBJ
	void Destroy()
	{
		Super::Destroy();
		MaybeDestroy();
		m_bShow = FALSE;
		WWindow::_DeleteWindows.RemoveItem( this );
	}
#else
	virtual ~WWindow()
	{
		MaybeDestroy();
		WWindow::_DeleteWindows.RemoveItem( this );
	}
#endif

	// Accessors.
	FRect GetClientRect() const
	{
		RECT R;
		::GetClientRect( hWnd, &R );
		return FRect( R );
	}
	void MoveWindow( FRect R, UBOOL bRepaint )
	{
		::MoveWindow( hWnd, R.Min.X, R.Min.Y, R.Width(), R.Height(), bRepaint );
	}
	void MoveWindow( int Left, int Top, int Width, int Height, UBOOL bRepaint )
	{
		::MoveWindow( hWnd, Left, Top, Width, Height, bRepaint );
	}
	FRect GetWindowRect() const
	{
		RECT R;
		::GetWindowRect( hWnd, &R );
		return OwnerWindow ? OwnerWindow->ScreenToClient(R) : FRect(R);
	}
	FPoint ClientToScreen( const FPoint& InP )
	{
		POINT P;
		P.x = InP.X;
		P.y = InP.Y;
		::ClientToScreen( hWnd, &P );
		return FPoint( P.x, P.y );
	}
	FPoint ScreenToClient( const FPoint& InP )
	{
		POINT P;
		P.x = InP.X;
		P.y = InP.Y;
		::ScreenToClient( hWnd, &P );
		return FPoint( P.x, P.y );
	}
	FRect ClientToScreen( const FRect& InR )
	{
		return FRect( ClientToScreen(InR.Min), ClientToScreen(InR.Max) );
	}
	FRect ScreenToClient( const FRect& InR )
	{
		return FRect( ScreenToClient(InR.Min), ScreenToClient(InR.Max) );
	}
	FPoint GetCursorPos()
	{
		FPoint Mouse;
		::GetCursorPos( Mouse );
		return ScreenToClient( Mouse );
	}
	void Show( UBOOL Show )
	{
		m_bShow = Show;
		ShowWindow( hWnd, Show ? ::IsIconic(hWnd) ? SW_RESTORE : SW_SHOW : SW_HIDE );
	}

	// WWindow interface.
	virtual void Serialize( FArchive& Ar )
	{
		//!!UObject interface.
		//!!Super::Serialize( Ar );
		Ar << PersistentName;
	}
	virtual const TCHAR* GetPackageName()
	{
		return TEXT("Window");
	}
	virtual void DoDestroy()
	{
		if( NotifyHook )
			NotifyHook->NotifyDestroy( this );
		if( hWnd )
			DestroyWindow( *this );
		_Windows.RemoveItem( this );
	}
	virtual void GetWindowClassName( TCHAR* Result )=0;
	virtual LONG WndProc( UINT Message, UINT wParam, LONG lParam )
	{
		try
		{
			LastwParam = wParam;
			LastlParam = lParam;

			// Message snoop.
			if( Snoop )
			{
				if( Message==WM_CHAR )
					Snoop->SnoopChar( this, wParam );
				else if( Message==WM_KEYDOWN )
					Snoop->SnoopKeyDown( this, wParam );
				else if( Message==WM_LBUTTONDOWN )
					Snoop->SnoopLeftMouseDown( this, FPoint(LOWORD(lParam),HIWORD(lParam)) );
				else if( Message==WM_RBUTTONDOWN )
					Snoop->SnoopRightMouseDown( this, FPoint(LOWORD(lParam),HIWORD(lParam)) );
			}

			// Special multi-window activation handling.
			if( !MdiChild && !ModalCount )
			{
				static UBOOL AppActive=0;
				if( Message==WM_ACTIVATEAPP )
				{
					AppActive = wParam;
					SendMessageX( hWnd, WM_NCACTIVATE, wParam, 0 );
				}
				else if( Message==WM_NCACTIVATE && AppActive && !wParam )
				{
					return 1;
				}
			}

			// Message processing.
			if( Message==WM_DESTROY )
			{
				OnDestroy();
			}
			else if( Message==WM_DRAWITEM )
			{
				DRAWITEMSTRUCT* Info = (DRAWITEMSTRUCT*)lParam;
				for( INT i=0; i<Controls.Num(); i++ )
					if( ((WWindow*)Controls(i))->hWnd==Info->hwndItem )
						{((WWindow*)Controls(i))->OnDrawItem(Info); break;}
				return 1;
			}
			else if( Message==WM_MEASUREITEM )
			{
				MEASUREITEMSTRUCT* Info = (MEASUREITEMSTRUCT*)lParam;
				for( INT i=0; i<Controls.Num(); i++ )
					if( ((WWindow*)Controls(i))->ControlId==Info->CtlID )
						{((WWindow*)Controls(i))->OnMeasureItem(Info); break;}
				return 1;
			}
			else if( Message==WM_CLOSE )
			{
				if (ShouldClose())
					OnClose();
				else
					return 0;
			}
			else if( Message==WM_CHAR )
			{
				OnChar( wParam );
			}
			else if( Message==WM_KEYDOWN )
			{
				OnKeyDown( wParam );
			}
			else if( Message==WM_PAINT )
			{
				OnPaint();
			}
			else if( Message==WM_CREATE )
			{
				OnCreate();
			}
			else if( Message==WM_TIMER )
			{
				OnTimer();
			}
			else if( Message==WM_INITDIALOG )
			{
				OnInitDialog();
			}
			else if( Message==WM_ENTERIDLE )
			{
				OnEnterIdle();
			}
			else if( Message==WM_SETFOCUS )
			{
				OnSetFocus( (HWND)wParam );
			}
			else if( Message==WM_ACTIVATE )
			{
				OnActivate( LOWORD(wParam)!=0 );
			}
			else if( Message==WM_KILLFOCUS )
			{
				OnKillFocus( (HWND)wParam );
			}
			else if( Message==WM_SIZE )
			{
				OnSize( wParam, LOWORD(lParam), HIWORD(lParam) );
			}
			else if( Message==WM_PASTE )
			{
				OnPaste();
			}
			else if( Message==WM_SHOWWINDOW )
			{
				OnShowWindow( wParam );
			}
			else if( Message==WM_COPYDATA )
			{
				OnCopyData( (HWND)wParam, (COPYDATASTRUCT*)lParam );
			}
			else if( Message==WM_CAPTURECHANGED )
			{
				OnReleaseCapture();
			}
			else if( Message==WM_MDIACTIVATE )
			{
				OnMdiActivate( (HWND)lParam==hWnd );
			}
			else if( Message==WM_MOUSEMOVE )
			{
				OnMouseMove( wParam, FPoint(LOWORD(lParam), HIWORD(lParam)) );
			}
			else if( Message==WM_LBUTTONDOWN )
			{
				OnLeftButtonDown();
			}
			else if( Message==WM_RBUTTONDOWN )
			{
				OnRightButtonDown();
			}
			else if( Message==WM_LBUTTONUP )
			{
				OnLeftButtonUp();
			}
			else if( Message==WM_RBUTTONUP )
			{
				OnRightButtonUp();
			}
			else if( Message==WM_CUT )
			{
				OnCut();
			}
			else if( Message==WM_COPY )
			{
				OnCopy();
			}
			else if( Message==WM_UNDO )
			{
				OnUndo();
			}
			else if( Message==WM_SETCURSOR )
			{
				if( OnSetCursor() )
					return 1;
			}
			else if( Message==WM_NOTIFY )
			{
				for( INT i=0; i<Controls.Num(); i++ )
					if(wParam==((WWindow*)Controls(i))->ControlId
							&& ((WWindow*)Controls(i))->InterceptControlCommand(Message,wParam,lParam) )
						return 1;
				OnCommand( wParam );
			}
			else if( Message==WM_VSCROLL )
			{
				OnVScroll( wParam, lParam );
			}
			else if( Message==WM_KEYUP)
			{
				OnKeyUp( wParam, lParam );
			}
			else if( Message==WM_COMMAND || Message==WM_HSCROLL )
			{
				for( INT i=0; i<Controls.Num(); i++ )
					if((HWND)lParam==((WWindow*)Controls(i))->hWnd
							&& ((WWindow*)Controls(i))->InterceptControlCommand(Message,wParam,lParam) )
						return 1;
				OnCommand( wParam );
			}
			else if( Message==WM_SYSCOMMAND )
			{
				if( OnSysCommand( wParam ) )
					return 1;
			}
			return CallDefaultProc( Message, wParam, lParam );
		}
		catch( const TCHAR* )
		{
			// This exception prevents the message from being routed to the default proc.
			return 0;
		}
	}
	virtual INT CallDefaultProc( UINT Message, UINT wParam, LONG lParam )
	{
		if( MdiChild )
			return DefMDIChildProcX( hWnd, Message, wParam, lParam );
		else
			return DefWindowProcX( hWnd, Message, wParam, lParam );
	}
	virtual UBOOL InterceptControlCommand( UINT Message, UINT wParam, LONG lParam )
	{
		return 0;
	}
	virtual FString GetText()
	{
		check(hWnd);
		INT Length = GetLength();
#if UNICODE
		if( GUnicode && !GUnicodeOS )
		{
			ANSICHAR* ACh = (ANSICHAR*)appAlloca((Length+1)*sizeof(ANSICHAR));
			SendMessageA( *this, WM_GETTEXT, Length+1, (LPARAM)ACh );
			return appFromAnsi(ACh);
		}
		else
#endif
		{
			TCHAR* Text = (TCHAR*)appAlloca((Length+1)*sizeof(TCHAR));
			SendMessage( *this, WM_GETTEXT, Length+1, (LPARAM)Text );
			return Text;
		}
	}
	virtual void SetText( const TCHAR* Text )
	{
		check(hWnd);
		SendMessageLX( *this, WM_SETTEXT, 0, Text );
	}
	virtual INT GetLength()
	{
		check(hWnd);
		return SendMessageX( *this, WM_GETTEXTLENGTH, 0, 0 );
	}
	void SetNotifyHook( FNotifyHook* InNotifyHook )
	{
		NotifyHook = InNotifyHook;
	}

	// WWindow notifications.
	virtual void OnCopyData( HWND hWndSender, COPYDATASTRUCT* CD )
	{}
	virtual void OnSetFocus( HWND hWndLosingFocus )
	{}
	virtual void OnKillFocus( HWND hWndGaininFocus )
	{}
	virtual void OnSize( DWORD Flags, INT NewX, INT NewY )
	{}
	virtual void OnCommand( INT Command )
	{}
	virtual int OnSysCommand( INT Command )
	{
		return 0;
	}
	virtual void OnActivate( UBOOL Active )
	{}
	virtual void OnChar( TCHAR Ch )
	{}
	virtual void OnKeyDown( TCHAR Ch )
	{}
	virtual void OnCut()
	{}
	virtual void OnCopy()
	{}
	virtual void OnPaste()
	{}
	virtual void OnShowWindow( UBOOL bShow )
	{}
	virtual void OnUndo()
	{}
	virtual void OnVScroll( WPARAM wParam, LPARAM lParam )
	{}
	virtual void OnKeyUp( WPARAM wParam, LPARAM lParam )
	{}
	virtual void OnPaint()
	{}
	virtual void OnCreate()
	{}
	virtual void OnDrawItem( DRAWITEMSTRUCT* Info )
	{}
	virtual void OnMeasureItem( MEASUREITEMSTRUCT* Info )
	{}
	virtual void OnInitDialog()
	{}
	virtual void OnEnterIdle()
	{}
	virtual void OnMouseEnter()
	{}
	virtual void OnMouseLeave()
	{}
	virtual void OnMouseHover()
	{}
	virtual void OnTimer()
	{}
	virtual void OnReleaseCapture()
	{}
	virtual void OnMdiActivate( UBOOL Active )
	{}
	virtual void OnMouseMove( DWORD Flags, FPoint Location )
	{}
	virtual void OnLeftButtonDown()
	{}
	virtual void OnRightButtonDown()
	{}
	virtual void OnLeftButtonUp()
	{}
	virtual void OnRightButtonUp()
	{}
	virtual void OnFinishSplitterDrag( WDragInterceptor* Drag, UBOOL Success )
	{}
	virtual INT OnSetCursor()
	{
		return 0;
	}
	virtual UBOOL ShouldClose()
	{
		return TRUE;
	}
	virtual void OnClose()
	{
		if( MdiChild )
			SendMessage( OwnerWindow->hWnd, WM_MDIDESTROY, (WPARAM)hWnd, 0 );
		else
			DestroyWindow( *this );
	}
	virtual void OnDestroy()
	{
		if (hWnd)
		{
			if( PersistentName!=NAME_None )
			{
				FRect R = GetWindowRect();
				if( !IsZoomed(hWnd) )
					GConfig->SetString( TEXT("WindowPositions"), *PersistentName, *FString::Printf( TEXT("(X=%i,Y=%i,XL=%i,YL=%i)"), R.Min.X, R.Min.Y, R.Width(), R.Height() ) );
			}
			_Windows.RemoveItem( this );
			hWnd = NULL;
		}
	}

	// WWindow functions.
	void SaveWindowPos()
	{
	}
	void MaybeDestroy()
	{
		if( !Destroyed )
		{
			Destroyed=1;
			DoDestroy();
		}
	}
	void _CloseWindow()
	{
		check(hWnd);
		DestroyWindow( *this );
	}
	operator HWND() const
	{
		return hWnd;
	}
	void SetFont( HFONT hFont )
	{
		SendMessageX( *this, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(0,0) );
	}
	void PerformCreateWindowEx( DWORD dwExStyle, LPCTSTR lpWindowName, DWORD dwStyle, INT x, INT y, INT nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance )
	{
		check(hWnd==NULL);

		// Retrieve remembered position.
		TCHAR Pos[256];
		if
		(	PersistentName!=NAME_None 
		&&	GConfig->GetString( TEXT("WindowPositions"), *PersistentName, Pos, ARRAY_COUNT(Pos) ) )
		{
			// Get saved position.
			Parse( Pos, TEXT("X="), x );
			Parse( Pos, TEXT("Y="), y );
			if( dwStyle & WS_SIZEBOX )
			{
				Parse( Pos, TEXT("XL="), nWidth );
				Parse( Pos, TEXT("YL="), nHeight );
			}

			// Count identical windows already opened.
			INT Count=0;
			for( INT i=0; i<_Windows.Num(); i++ )
			{
				Count += _Windows(i)->PersistentName==PersistentName;
			}
			if( Count )
			{
				// Move away.
				x += Count*16;
				y += Count*16;
			}

			// Clip size to screen.
			RECT Desktop;
			::GetWindowRect( GetDesktopWindow(), &Desktop );
			if( x+nWidth  > Desktop.right  ) x = Desktop.right  - nWidth;
			if( y+nHeight > Desktop.bottom ) y = Desktop.bottom - nHeight;
			if( x<0 )
			{
				if( dwStyle & WS_SIZEBOX )
					nWidth += x;
				x=0;
			}
			if( y<0 )
			{
				if( dwStyle & WS_SIZEBOX )
					nHeight += y;
				y=0;
			}
		}

		// Create window.
		_Windows.AddItem( this );
		TCHAR ClassName[256];
		GetWindowClassName( ClassName );
		//hinstance must match window class hinstance!!
		HWND hWndCreated = TCHAR_CALL_OS(CreateWindowEx(dwExStyle,ClassName,lpWindowName,dwStyle,x,y,nWidth,nHeight,hWndParent,hMenu,hInstanceWindow,this),CreateWindowExA(dwExStyle,TCHAR_TO_ANSI(ClassName),TCHAR_TO_ANSI(lpWindowName),dwStyle,x,y,nWidth,nHeight,hWndParent,hMenu,hInstanceWindow,this));
		if( !hWndCreated )
			appErrorf( TEXT("CreateWindowEx failed: %s"), appGetSystemErrorMessage() );
		check(hWndCreated);
		check(hWndCreated==hWnd);
	}
	void SetRedraw( UBOOL Redraw )
	{
		SendMessageX( *this, WM_SETREDRAW, Redraw, 0 );
	}
	// Used in the editor ... used to draw window edges in custom colors.
	virtual void MyDrawEdge( HDC hdc, LPRECT qrc, UBOOL bRaised )
	{
		HPEN penOld, penRaised = CreatePen( PS_SOLID, 1, RGB(159,159,159) ),
			penSunken = CreatePen( PS_SOLID, 1, RGB(106,106,106) );
		HDC	hDC = GetDC( hWnd );

		RECT rc = *qrc;
		rc.right -= 1;
		rc.bottom -= 1;

		penOld = (HPEN)SelectObject( hDC, (bRaised ? penRaised : penSunken ) );
		::MoveToEx( hDC, rc.left, rc.top, NULL );
		::LineTo( hDC, rc.right, rc.top );
		::MoveToEx( hDC, rc.left, rc.top, NULL );
		::LineTo( hDC, rc.left, rc.bottom);
		SelectObject( hDC, penOld );

		penOld = (HPEN)SelectObject( hDC, (bRaised ? penSunken : penRaised ) );
		::MoveToEx( hDC, rc.right, rc.bottom, NULL );
		::LineTo( hDC, rc.right, rc.top );
		::MoveToEx( hDC, rc.right, rc.bottom, NULL );
		::LineTo( hDC, rc.left, rc.bottom );
		SelectObject( hDC, penOld );

		DeleteObject( penRaised );
		DeleteObject( penSunken );
		::ReleaseDC( hWnd, hDC );
	}
};

/*-----------------------------------------------------------------------------
	WControl.
-----------------------------------------------------------------------------*/

// A control which exists inside an owner window.
class WINDOW_API WControl : public WWindow
{
	W_DECLARE_ABSTRACT_CLASS(WControl,WWindow,CLASS_Transient);

	// Variables.
	WNDPROC WindowDefWndProc;

	// Structors.
	WControl()
	{}
	WControl( WWindow* InOwnerWindow, INT InId, WNDPROC InSuperProc )
	: WWindow( NAME_None, InOwnerWindow )
	{
		check(OwnerWindow);
		WindowDefWndProc = InSuperProc;
		ControlId = InId ? InId : InOwnerWindow->TopControlId++;
		OwnerWindow->Controls.AddItem( this );
	}
#if WIN_OBJ
	void Destroy()
	{
		Super::Destroy();
		check(OwnerWindow);
		OwnerWindow->Controls.RemoveItem( this );
	}
#else
	~WControl()
	{
		if( OwnerWindow )
			OwnerWindow->Controls.RemoveItem( this );
	}
#endif

	// WWindow interface.
	virtual INT CallDefaultProc( UINT Message, UINT wParam, LONG lParam )
	{
		if( ::IsWindow( hWnd ) )
			return CallWindowProcX( WindowDefWndProc, hWnd, Message, wParam, lParam );
		else
			return 1;
	}
	static WNDPROC RegisterWindowClass( const TCHAR* Name, const TCHAR* WinBaseClass )
	{
		WNDPROC SuperProc=NULL;
#if UNICODE
		if( GUnicodeOS )
		{
			WNDCLASSEXW Cls;
			appMemzero( &Cls, sizeof(Cls) );
			Cls.cbSize        = sizeof(Cls);
			verify( GetClassInfoExW( NULL, WinBaseClass, &Cls ) );
			SuperProc         = Cls.lpfnWndProc;
			Cls.lpfnWndProc   = WWindow::StaticWndProc;
			Cls.lpszClassName = Name;
			Cls.hInstance     = hInstanceWindow;
			check(Cls.lpszMenuName==NULL);
			verify(RegisterClassExW( &Cls ));
		}
		else
#endif
		{
			WNDCLASSEXA Cls;
			appMemzero( &Cls, sizeof(Cls) );
			Cls.cbSize        = sizeof(Cls);
			verify( GetClassInfoExA( NULL, TCHAR_TO_ANSI(WinBaseClass), &Cls ) );
			SuperProc         = Cls.lpfnWndProc;
			Cls.lpfnWndProc   = WWindow::StaticWndProc;
			Cls.lpszClassName = TCHAR_TO_ANSI(Name);
			Cls.hInstance     = hInstanceWindow;
			check(Cls.lpszMenuName==NULL);
			verify(RegisterClassExA( &Cls ));
		}
		return SuperProc;
	}
};

/*-----------------------------------------------------------------------------
	WTabControl.
-----------------------------------------------------------------------------*/

class WINDOW_API WTabControl : public WControl
{
	W_DECLARE_CLASS(WTabControl,WControl,CLASS_Transient)
	DECLARE_WINDOWSUBCLASS(WTabControl,WControl,Window)

	FDelegate SelectionChangeDelegate;

	// Constructor.
	WTabControl()
	{}
	WTabControl( WWindow* InOwner, INT InId=0, WNDPROC InSuperProc=NULL )
	: WControl( InOwner, InId, InSuperProc?InSuperProc:SuperProc )
	{}

	// WWindow interface.
	void OpenWindow( UBOOL Visible, DWORD dwExtraStyle = 0 )
	{
		PerformCreateWindowEx
		(
			0,
            NULL,
            WS_CHILD | (Visible?WS_VISIBLE:0) | dwExtraStyle,
            0, 0,
			0, 0,
            *OwnerWindow,
            (HMENU)ControlId,
            hInstance
		);

		SendMessageX( *this, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(0,0) );
	}
	void AddTab( FString InText, int InlParam )
	{
		TCHAR Text[256] = TEXT("");
		appStrcpy( Text, *InText );

		TCITEM tci;

		tci.mask = TCIF_TEXT | TCIF_PARAM;
		tci.pszText = Text;
		tci.lParam = InlParam;

		SendMessageX( *this, TCM_INSERTITEM, GetCount(), (LPARAM)(const LPTCITEM)&tci );
	}
	void Empty()
	{
		SendMessageX( *this, TCM_DELETEALLITEMS, 0, 0 );
	}
	int GetCount()
	{
		return SendMessageX( *this, TCM_GETITEMCOUNT, 0, 0 );
	}
	int SetCurrent( int Index )
	{
		return SendMessageX( *this, TCM_SETCURSEL, Index, 0 );
	}
	int GetCurrent()
	{
		return SendMessageX( *this, TCM_GETCURFOCUS, 0, 0 );
	}
	int GetIndexFromlParam( int InlParam )
	{
		TCITEM tci;
		tci.mask = TCIF_PARAM;

		for( int x = 0 ; x < GetCount() ; x++ )
		{
			SendMessageX( *this, TCM_GETITEM, x, (LPARAM)(LPTCITEM)&tci );
			if( tci.lParam == InlParam )
				return x;
		}

		return -1;
	}
	FString GetString( int Index )
	{
		TCHAR Buffer[256] = TEXT("");

		TCITEM tci;
		tci.mask = TCIF_TEXT;
		tci.pszText = Buffer;

		SendMessageX( *this, TCM_GETITEM, Index, (LPARAM)(LPTCITEM)&tci );

		return FString( Buffer );
	}
	int GetlParam( int Index )
	{
		TCITEM tci;
		tci.mask = TCIF_PARAM;

		SendMessageX( *this, TCM_GETITEM, Index, (LPARAM)(LPTCITEM)&tci );

		return tci.lParam;
	}
	UBOOL InterceptControlCommand( UINT Message, UINT wParam, LONG lParam )
	{
		LastwParam = wParam;
		LastlParam = lParam;

		NMHDR* pnmhdr = (LPNMHDR)lParam;

		if( pnmhdr->code == TCN_SELCHANGE)
			SelectionChangeDelegate();

		return 0;
	}
};

/*-----------------------------------------------------------------------------
	WLabel.
-----------------------------------------------------------------------------*/

// A non-interactive label control.
class WINDOW_API WLabel : public WControl
{
	W_DECLARE_CLASS(WLabel,WControl,CLASS_Transient)
	DECLARE_WINDOWSUBCLASS(WLabel,WControl,Window)

	// Constructor.
	WLabel()
	{}
	WLabel( WWindow* InOwner, INT InId=0, WNDPROC InSuperProc=NULL )
	: WControl( InOwner, InId, InSuperProc?InSuperProc:SuperProc )
	{}

	// WWindow interface.
	void OpenWindow( UBOOL Visible, UBOOL ClientEdge = 1, DWORD dwExtraStyle = 0 )
	{
		PerformCreateWindowEx
		(
			(ClientEdge?WS_EX_CLIENTEDGE:0),
            NULL,
            WS_CHILD | (Visible?WS_VISIBLE:0) | dwExtraStyle,
            0, 0,
			0, 0,
            *OwnerWindow,
            (HMENU)ControlId,
            hInstance
		);

		SendMessageX( *this, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(0,0) );
	}
};

/*-----------------------------------------------------------------------------
	WGroupBox.
-----------------------------------------------------------------------------*/

class WINDOW_API WGroupBox : public WControl
{
	W_DECLARE_CLASS(WGroupBox,WControl,CLASS_Transient)
	DECLARE_WINDOWSUBCLASS(WGroupBox,WControl,Window)

	// Constructor.
	WGroupBox()
	{}
	WGroupBox( WWindow* InOwner, INT InId=0, WNDPROC InSuperProc=NULL )
	: WControl( InOwner, InId, InSuperProc?InSuperProc:SuperProc )
	{}

	// WWindow interface.
	void OpenWindow( UBOOL Visible, DWORD dwExtraStyle = 0 )
	{
		PerformCreateWindowEx
		(
			0,
            NULL,
            BS_GROUPBOX | WS_CHILD | (Visible?WS_VISIBLE:0) | dwExtraStyle,
            0, 0,
			0, 0,
            *OwnerWindow,
            (HMENU)ControlId,
            hInstance
		);

		SendMessageX( *this, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(0,0) );
	}
};

/*-----------------------------------------------------------------------------
	WPropertyPage.

    A WPropertySheet has 1 or more of these on it.  Each tab
	represents one page.  A page uses a dialog box resource as it's
	template to determine where to place child controls.
-----------------------------------------------------------------------------*/

// This is a helper struct that is just used to store the window positions
// of the various controls this page contains.
struct WPropertyPageCtrl
{
	WPropertyPageCtrl( INT InId, RECT InRect, FString InCaption, LONG InStyle, LONG InExStyle )
	{
		id = InId;
		rect = InRect;
		Caption = InCaption;
		Style = InStyle;
		ExStyle = InExStyle;
	}

	INT id;
	RECT rect;
	FString Caption;
	LONG Style;
	LONG ExStyle;
};

class WINDOW_API WPropertyPage : public WWindow
{
	DECLARE_WINDOWCLASS(WPropertyPage,WWindow,Window)

	TArray<WPropertyPageCtrl> Ctrls;
	TArray<WLabel*> Labels;
	FString Caption;
	INT id;

	// Structors.
	WPropertyPage ( WWindow* InOwnerWindow )
	:	WWindow( TEXT("WPropertyPage"), InOwnerWindow )
	{
	}

	// WWindow interface.
	virtual void OpenWindow( INT InDlgId, HMODULE InHMOD )
	{
		check(InHMOD);
		MdiChild = 0;

		id = InDlgId;

		PerformCreateWindowEx
		(
			0,
			NULL,
			WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
			0, 0,
			0, 0,
			OwnerWindow ? OwnerWindow->hWnd : NULL,
			NULL,
			hInstance
		);
		SendMessageX( hWnd, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(0,0) );

		// Create an invisible dialog box from the specified resource id, and read the control
		// positions from it.  Put these positions into an array for future reference.
		char ResName[80] = "";
		strcpy( ResName, appToAnsi( *FString::Printf(TEXT("#%d"), InDlgId) ) );
		HRSRC hrsrc = FindResourceA( InHMOD, ResName, (LPSTR)RT_DIALOG );
		check(hrsrc);

		HGLOBAL hgbl = LoadResource( InHMOD, hrsrc );
		check(hgbl);

		DLGTEMPLATE* dlgtemplate = (DLGTEMPLATE*)LockResource(hgbl);
		check(dlgtemplate);

		HWND hwndDlg = ::CreateDialogIndirectA( hInstance, dlgtemplate, hWnd, NULL );
		check(hwndDlg);	// Specified ID is NOT a valid dialog resource.

		char TempCaption[80] = "";
		GetWindowTextA( hwndDlg, TempCaption, 80 );
		Caption = appFromAnsi( TempCaption );
		for( HWND ChildWnd = ::GetWindow( hwndDlg, GW_CHILD ) ; ChildWnd ; ChildWnd = ::GetNextWindow( ChildWnd, GW_HWNDNEXT ) )
		{
			RECT rect;
			::GetWindowRect( ChildWnd, &rect );
			::ScreenToClient( hwndDlg, (POINT*)&rect.left );
			::ScreenToClient( hwndDlg, (POINT*)&rect.right );

			INT id = GetDlgCtrlID( ChildWnd );

			::GetWindowTextA( ChildWnd, TempCaption, 80 );
			LONG Style, ExStyle;
			Style = GetWindowLong( ChildWnd, GWL_STYLE );
			ExStyle = GetWindowLong( ChildWnd, GWL_EXSTYLE );
			new( Ctrls )WPropertyPageCtrl( id, rect, appFromAnsi( TempCaption ), Style, ExStyle );
		}

		// Get the size of the dialog resource and use this as the default size of the property page.
		RECT rectDlg;
		::GetClientRect( hwndDlg, &rectDlg );
		::MoveWindow( hWnd, 0, 0, rectDlg.right, rectDlg.bottom, 1);

		UnlockResource( dlgtemplate );
	}
	// Places a control based on the position that it previously read from the dialog template
	void PlaceControl( WControl* InControl )
	{
		for( int x = 0 ; x < Ctrls.Num() ; x++ )
			if( Ctrls(x).id  == InControl->ControlId )
			{
				::MoveWindow( InControl->hWnd, Ctrls(x).rect.left, Ctrls(x).rect.top, Ctrls(x).rect.right - Ctrls(x).rect.left, Ctrls(x).rect.bottom - Ctrls(x).rect.top, 1 );
				InControl->SetText( *Ctrls(x).Caption );
				SetWindowLong( InControl->hWnd, GWL_STYLE, Ctrls(x).Style );
				SetWindowLong( InControl->hWnd, GWL_EXSTYLE, Ctrls(x).ExStyle );
				Ctrls.Remove(x);
				break;
			}
	}
	// Loops through the control list and for any controls that are left over, create static 
	// controls for them and place them.
	void Finalize()
	{
		for( int x = 0 ; x < Ctrls.Num() ; x++ )
		{
			WLabel* lbl = new WLabel( this, Ctrls(x).id );
			Labels.AddItem( lbl );
			lbl->OpenWindow( 1, 0 );
			RECT rect = Ctrls(x).rect;
			::MoveWindow( lbl->hWnd, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, 1 );
			lbl->SetText( *Ctrls(x).Caption );
			SetWindowLong( lbl->hWnd, GWL_STYLE, Ctrls(x).Style );
			SetWindowLong( lbl->hWnd, GWL_EXSTYLE, Ctrls(x).ExStyle );
		}
	}
	void Cleanup()
	{
		for( int x = 0 ; x < Labels.Num() ; x++ )
		{
			DestroyWindow( Labels(x)->hWnd );
			delete Labels(x);
		}
	}	FString GetCaption()
	{
		return Caption;
	}
	INT GetID()
	{
		return id;
	}
	virtual void Refresh()
	{
	}
};

/*-----------------------------------------------------------------------------
	WPropertySheet.
-----------------------------------------------------------------------------*/

#define WPS_TABS	19900
class WINDOW_API WPropertySheet : public WControl
{
	W_DECLARE_CLASS(WPropertySheet,WControl,CLASS_Transient)
	DECLARE_WINDOWSUBCLASS(WPropertySheet,WControl,Window)

	WTabControl* Tabs;
	TArray<WPropertyPage*> Pages;

	// Constructor.
	WPropertySheet()
	{
		Tabs = NULL;
	}
	WPropertySheet( WWindow* InOwner, INT InId=0, WNDPROC InSuperProc=NULL )
	: WControl( InOwner, InId, InSuperProc?InSuperProc:SuperProc )
	{}

	// WWindow interface.
	void OpenWindow( UBOOL Visible, DWORD dwExtraStyle = 0 )
	{
		PerformCreateWindowEx
		(
			0,
            NULL,
            WS_CHILD | (Visible?WS_VISIBLE:0) | dwExtraStyle,
            0, 0,
			0, 0,
            *OwnerWindow,
            (HMENU)ControlId,
            hInstance
		);

		SendMessageX( *this, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(0,0) );
	}
	void OnCreate()
	{
		WControl::OnCreate();

		Tabs = new WTabControl( this, WPS_TABS );
		Tabs->OpenWindow( 1 );
		Tabs->SelectionChangeDelegate = FDelegate(this, (TDelegate)OnTabsSelChange);
	}
	void OnSize( DWORD Flags, INT NewX, INT NewY )
	{
		WControl::OnSize(Flags, NewX, NewY);

		RECT rect;
		::GetClientRect( *this, &rect );
		::MoveWindow( Tabs->hWnd, 4, 4, rect.right-8, rect.bottom-8, 1 );

		RefreshPages();

		InvalidateRect( hWnd, NULL, 1 );
	}
	void RefreshPages()
	{
		if( !Pages.Num() ) return;

		INT idx = Tabs->GetCurrent();
		if( idx == -1 )	idx = 0;
		check( idx < Pages.Num() );

		// Hide all pages.
		for( INT x = 0 ; x < Pages.Num() ; x++ )
		{
			if( x == idx )
				Pages(x)->Show( 1 );
			else
				Pages(x)->Show( 0 );
			Pages(x)->Refresh();
		}

		::SetWindowPos( Pages(idx)->hWnd, HWND_TOP, 8, 24, 0, 0, SWP_NOSIZE );
	}
	void Empty()
	{
		Tabs->Empty();
	}
	void AddPage( WPropertyPage* InPage )
	{
		Pages.AddItem( InPage );
		Tabs->AddTab( InPage->GetCaption(), InPage->GetID() );
	}
	int SetCurrent( int Index )
	{
		INT ret = Tabs->SetCurrent( Index );
		RefreshPages();
		return ret;
	}
	void OnTabsSelChange()
	{
		RefreshPages();
	}
};

/*-----------------------------------------------------------------------------
	WCustomLabel.
-----------------------------------------------------------------------------*/

class WINDOW_API WCustomLabel : public WLabel
{
	W_DECLARE_CLASS(WCustomLabel,WLabel,CLASS_Transient)
	DECLARE_WINDOWSUBCLASS(WCustomLabel,WLabel,Window)

	// Constructor.
	WCustomLabel()
	{}
	WCustomLabel( WWindow* InOwner, INT InId=0, WNDPROC InSuperProc=NULL )
		: WLabel( InOwner, InId, InSuperProc?InSuperProc:SuperProc )
	{}

	// WWindow interface.
	void OnPaint()
	{
		PAINTSTRUCT PS;
		HDC hDC = BeginPaint( *this, &PS );
		HBRUSH brushBack = CreateSolidBrush( RGB(128,128,128) );

		FRect Rect = GetClientRect();
		FillRect( hDC, Rect, brushBack );

		HFONT OldFont = (HFONT)SelectObject( hDC, GetStockObject(DEFAULT_GUI_FONT) );

		FString Caption = GetText();
		::SetBkMode( hDC, TRANSPARENT );
		DrawTextX( hDC, *Caption, Caption.Len(), Rect, DT_CENTER | DT_SINGLELINE );

		SelectObject( hDC, OldFont );

		EndPaint( *this, &PS );

		DeleteObject( brushBack );
	}
};

/*-----------------------------------------------------------------------------
	WButton.
-----------------------------------------------------------------------------*/

// A button.
class WINDOW_API WButton : public WControl
{
	W_DECLARE_CLASS(WButton,WControl,CLASS_Transient);
	DECLARE_WINDOWSUBCLASS(WButton,WControl,Window)

	// Delegates.
	FDelegate ClickDelegate;
	FDelegate DoubleClickDelegate;
	FDelegate PushDelegate;
	FDelegate UnPushDelegate;
	FDelegate SetFocusDelegate;
	FDelegate KillFocusDelegate;
	HBITMAP hbm;
	UBOOL bChecked, bOwnerDraw;

	// Constructor.
	WButton()
	{}
	WButton( WWindow* InOwner, INT InId=0, FDelegate InClicked=FDelegate(), WNDPROC InSuperProc=NULL )
	: WControl( InOwner, InId, InSuperProc?InSuperProc:SuperProc )
	, ClickDelegate( InClicked )
	{
		bChecked = 0;
	}

	// WWindow interface.
	void OpenWindow( UBOOL Visible, INT X, INT Y, INT XL, INT YL, const TCHAR* Text, UBOOL bBitmap = 0, DWORD dwExtraStyle = 0 )
	{
		PerformCreateWindowEx
		(
			0,
            NULL,
            WS_CHILD | BS_PUSHBUTTON | (bBitmap?BS_BITMAP:0) | dwExtraStyle,
            X, Y,
			XL, YL,
            *OwnerWindow,
            (HMENU)ControlId,
            hInstance
		);

		SendMessageX( *this, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(0,0) );
		SetText( Text );
		if( Visible )
			ShowWindow( *this, SW_SHOWNOACTIVATE );
		bOwnerDraw = (dwExtraStyle & BS_OWNERDRAW);
	}
	void SetVisibleText( const TCHAR* Text )
	{
		check(hWnd);
		if( Text )
			SetText( Text );
		Show( Text!=NULL );
	}
	void SetBitmap( HBITMAP Inhbm )
	{
		SendMessageX( hWnd, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)Inhbm);
		hbm = Inhbm;
	}
	virtual Clicked(void)
	{
		SendMessageX( OwnerWindow->hWnd, WM_COMMAND, WM_PB_PUSH, ControlId );
	}
	void OnDrawItem( DRAWITEMSTRUCT* Item )
	{
		RECT R=Item->rcItem;
		UBOOL bPressed = (Item->itemState&ODS_SELECTED)!=0 || bChecked;

		HBRUSH brush = CreateSolidBrush( RGB(128,128,128) );
		FillRect( Item->hDC, &Item->rcItem, brush );

		MyDrawEdge( Item->hDC, &R, !bPressed );

		HDC hdcMem;
		HBITMAP hbmOld;

		hdcMem = CreateCompatibleDC(Item->hDC);
		hbmOld = (HBITMAP)SelectObject(hdcMem, hbm);
		BITMAP bitmap;
		GetObjectA( hbm, sizeof(BITMAP), (LPSTR)&bitmap );

		int Adjust = (bPressed) ? 2 : 0;

		BitBlt(Item->hDC,
			(R.right / 2) - (bitmap.bmWidth / 2) + Adjust, (R.bottom / 2) - (bitmap.bmHeight / 2) + Adjust,
			bitmap.bmWidth, bitmap.bmHeight,
			hdcMem,
			0, 0,
			SRCCOPY);

		// If this button is checked, draw a light border around in the inside to make it obvious.
		if( bPressed )
		{
			HPEN penOld, pen = CreatePen( PS_SOLID, 1, RGB(145,210,198) );

			RECT rc = R;
			rc.left++;	rc.top++;	rc.right--;		rc.bottom--;

			penOld = (HPEN)SelectObject( Item->hDC, pen );
			::MoveToEx( Item->hDC, rc.left, rc.top, NULL );
			::LineTo( Item->hDC, rc.right, rc.top );
			::LineTo( Item->hDC, rc.right, rc.bottom );
			::LineTo( Item->hDC, rc.left, rc.bottom );
			::LineTo( Item->hDC, rc.left, rc.top );
			SelectObject( Item->hDC, penOld );

			DeleteObject( pen );
		}

		// Clean up.
		//
		DeleteObject(brush);
		SelectObject(hdcMem, hbmOld);
		DeleteDC(hdcMem);
	}
	BOOL IsChecked( void )
	{
		// Owner draw buttons require us to keep track of the checked status ourselves.
		if( bOwnerDraw )
			return bChecked;
		else
			return ( SendMessageLX( *this, BM_GETCHECK, 0, 0 ) == BST_CHECKED );
	}
	void SetCheck( int iCheck )
	{
		SendMessageLX( *this, BM_SETCHECK, (WPARAM)iCheck, 0 );
		bChecked = (iCheck == BST_CHECKED);
	}

	// WControl interface.
	UBOOL InterceptControlCommand( UINT Message, UINT wParam, LONG lParam )
	{
		if     ( HIWORD(wParam)==BN_CLICKED   )
		{
			Clicked();
			ClickDelegate();
			// This notification returns 0 instead of 1 because the editor sometimes wants
			// to know about clicks without using a delegate (i.e. dynamically created buttons)
			return 0;
		}
		else if( HIWORD(wParam)==BN_DBLCLK    ) {DoubleClickDelegate(); return 1;}
		else if( HIWORD(wParam)==BN_PUSHED    ) {PushDelegate();        return 1;}
		else if( HIWORD(wParam)==BN_UNPUSHED  ) {UnPushDelegate();      return 1;}
		else if( HIWORD(wParam)==BN_SETFOCUS  ) {SetFocusDelegate();    return 1;}
		else if( HIWORD(wParam)==BN_KILLFOCUS ) {UnPushDelegate();      return 1;}
		else return 0;
	}
};

/*-----------------------------------------------------------------------------
	WToolTip
-----------------------------------------------------------------------------*/

// Tooltip window - easy way to create tooltips for standard controls.
class WINDOW_API WToolTip : public WControl
{
	W_DECLARE_CLASS(WToolTip,WControl,CLASS_Transient);
	DECLARE_WINDOWSUBCLASS(WToolTip,WControl,Window)

	// Constructor.
	WToolTip()
	{}
	WToolTip( WWindow* InOwner, INT InId=900, WNDPROC InSuperProc=NULL )
	: WControl( InOwner, InId, InSuperProc?InSuperProc:SuperProc )
	{
		check(OwnerWindow);
	}

	// WWindow interface.
	void OpenWindow()
	{
		PerformCreateWindowEx
		(
			0,
            NULL,
            TTS_ALWAYSTIP,
            0, 0,
			0, 0,
            NULL,
            (HMENU)NULL,
            hInstance
		);

		SendMessageX( *this, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(0,0) );
	}
	void AddTool( HWND InHwnd, FString InToolTip, int InId, RECT* InRect = NULL )
	{
		check(::IsWindow( hWnd ));

		RECT rect;
		::GetClientRect( InHwnd, &rect );

		TOOLINFO ti;
		TCHAR chToolTip[128] = TEXT("");
		appStrcpy( chToolTip, *InToolTip );
		ti.cbSize = sizeof(TOOLINFO);
		ti.uFlags = TTF_SUBCLASS;
		ti.hwnd = InHwnd;
		ti.hinst = hInstance;
		ti.uId = ControlId + InId;
		ti.lpszText = chToolTip;
		if( InRect )
			ti.rect = *InRect;
		else
			ti.rect = rect;

		SendMessageX(hWnd, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);
	}
};

/*-----------------------------------------------------------------------------
	WPictureButton.
-----------------------------------------------------------------------------*/
class WPictureButton : public WWindow
{
	DECLARE_WINDOWCLASS(WPictureButton,WWindow,Window)

	HBITMAP hbmOn, hbmOff;
	RECT ClientPos, BmpOffPos, BmpOnPos;
	FString ToolTipText;
	int ID;
	WToolTip* ToolTipCtrl;
	UBOOL bOn, bHasBeenSetup;

	// Structors.
	WPictureButton( WWindow* InOwnerWindow )
	:	WWindow( TEXT("PictureButton"), InOwnerWindow )
	{
		hbmOn = hbmOff = NULL;
		bOn = bHasBeenSetup = 0;
		ToolTipCtrl = NULL;
	}

	// WWindow interface.
	void OpenWindow()
	{
		MdiChild = 0;

		// HAVE to call "SetUp" before opening the window
		check(bHasBeenSetup);

		PerformCreateWindowEx
		(
			0,
			NULL,
			WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
			ClientPos.left,
			ClientPos.top,
			ClientPos.right,
			ClientPos.bottom,
			OwnerWindow ? OwnerWindow->hWnd : NULL,
			NULL,
			hInstance
		);
		SendMessageX( hWnd, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(0,0) );

		ToolTipCtrl = new WToolTip(this);
		ToolTipCtrl->OpenWindow();
	}
	void OnSize( DWORD Flags, INT NewX, INT NewY )
	{
		WWindow::OnSize(Flags, NewX, NewY);

		RECT rect;
		::GetClientRect( hWnd, &rect );

		if( ToolTipCtrl && ToolTipText.Len() )
			ToolTipCtrl->AddTool( hWnd, ToolTipText, 0 );
	}
	void OnDestroy()
	{
		WWindow::OnDestroy();
		delete ToolTipCtrl;
	}
	void OnPaint()
	{
		PAINTSTRUCT PS;
		HDC hDC = BeginPaint( *this, &PS );
		HBRUSH brushBack = CreateSolidBrush( RGB(255,255,255) );

		RECT rc;
		::GetClientRect( hWnd, &rc );

		FillRect( hDC, &rc, brushBack );

		HDC hdcMem;
		HBITMAP hbmOld;

		hdcMem = CreateCompatibleDC(hDC);
		hbmOld = (HBITMAP)SelectObject(hdcMem, (bOn ? hbmOn : hbmOff));

		RECT BmpPos = (bOn ? BmpOnPos : BmpOffPos);

		BitBlt(hDC,
			(rc.right - BmpPos.right) / 2, (rc.bottom - BmpPos.bottom) / 2,
			BmpPos.right, BmpPos.bottom,
			hdcMem,
			BmpPos.left, BmpPos.top,
			SRCCOPY);

		// Clean up.
		//
		SelectObject(hdcMem, hbmOld);
		DeleteDC(hdcMem);
		DeleteObject( brushBack );

		EndPaint( *this, &PS );
	}
	void OnLeftButtonDown()
	{
		SendMessageX( OwnerWindow->hWnd, WM_COMMAND, WM_PB_PUSH, ID );
	}
	void SetUp( FString InToolTipText, int InID, 
		int InClientLeft, int InClientTop, int InClientRight, int InClientBottom,
		HBITMAP InHbmOff, int InBmpOffLeft, int InBmpOffTop, int InBmpOffRight, int InBmpOffBottom,
		HBITMAP InHbmOn, int InBmpOnLeft, int InBmpOnTop, int InBmpOnRight, int InBmpOnBottom )
	{
		bHasBeenSetup = 1;

		ToolTipText = InToolTipText;
		ID = InID;
		ClientPos.left = InClientLeft;
		ClientPos.top = InClientTop;
		ClientPos.right = InClientRight;
		ClientPos.bottom = InClientBottom;
		hbmOff = InHbmOff;
		BmpOffPos.left = InBmpOffLeft;
		BmpOffPos.top = InBmpOffTop;
		BmpOffPos.right = InBmpOffRight;
		BmpOffPos.bottom = InBmpOffBottom;
		hbmOn = InHbmOn;
		BmpOnPos.left = InBmpOnLeft;
		BmpOnPos.top = InBmpOnTop;
		BmpOnPos.right = InBmpOnRight;
		BmpOnPos.bottom = InBmpOnBottom;
	}
};

/*-----------------------------------------------------------------------------
	WCheckBox.
-----------------------------------------------------------------------------*/

// A checkbox.
class WINDOW_API WCheckBox : public WButton
{
	W_DECLARE_CLASS(WCheckBox,WButton,CLASS_Transient);
	DECLARE_WINDOWSUBCLASS(WCheckBox,WButton,Window)

	UBOOL bAutocheck;

	// Constructor.
	WCheckBox()
	{}
	WCheckBox( WWindow* InOwner, INT InId=0, FDelegate InClicked=FDelegate() )
	: WButton( InOwner, InId, InClicked )
	{}

	// WWindow interface.
	void OpenWindow( UBOOL Visible, INT X, INT Y, INT XL, INT YL, const TCHAR* Text, UBOOL InbAutocheck = 1, UBOOL bBitmap = 0, DWORD dwExtraStyle = 0 )
	{
		bAutocheck = InbAutocheck;
		PerformCreateWindowEx
		(
			0,
            NULL,
            WS_CHILD | BS_CHECKBOX | (bAutocheck?BS_AUTOCHECKBOX:0) | (bBitmap?BS_BITMAP:0) | dwExtraStyle,
            X, Y,
			XL, YL,
            *OwnerWindow,
            (HMENU)ControlId,
            hInstance
		);
		SetText( Text );
		SendMessageX( *this, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(0,0) );
		if( Visible )
			ShowWindow( *this, SW_SHOWNOACTIVATE );
		bOwnerDraw = (dwExtraStyle & BS_OWNERDRAW);
	}
	void OnCreate()
	{
		WButton::OnCreate();
		bOwnerDraw = 0;
	}
	virtual void OnRightButtonDown()
	{
		// This is just a quick hack so the parent window can know when the user right clicks on
		// one of the buttons in the UnrealEd button bar.
		SendMessageX( ::GetParent( hWnd ), WM_COMMAND, WM_BB_RCLICK, ControlId );
	}
	virtual Clicked(void)
	{
		if( bAutocheck )
			bChecked = !bChecked;
		else
			bChecked = FALSE;
	}
};

/*-----------------------------------------------------------------------------
	WVScrollBar.
-----------------------------------------------------------------------------*/

// A vertical scrollbar.
class WINDOW_API WVScrollBar : public WControl
{
	W_DECLARE_CLASS(WVScrollBar,WControl,CLASS_Transient);
	DECLARE_WINDOWSUBCLASS(WVScrollBar,WControl,Window)

	// Constructor.
	WVScrollBar()
	{}
	WVScrollBar( WWindow* InOwner, INT InId=0, WNDPROC InSuperProc=NULL  )
	: WControl( InOwner, InId, InSuperProc?InSuperProc:SuperProc )
	{}

	// WWindow interface.
	void OpenWindow( UBOOL Visible, INT X, INT Y, INT XL, INT YL )
	{
		PerformCreateWindowEx
		(
			0,
            NULL,
            WS_CHILD | SBS_RIGHTALIGN | SBS_VERT,
            X, Y,
			XL, YL,
            *OwnerWindow,
            (HMENU)ControlId,
            hInstance
		);
		SendMessageX( *this, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(0,0) );
		if( Visible )
			ShowWindow( *this, SW_SHOWNOACTIVATE );
	}
};

/*-----------------------------------------------------------------------------
	WTreeView.
-----------------------------------------------------------------------------*/

// A tree control
class WINDOW_API WTreeView : public WControl
{
	W_DECLARE_CLASS(WTreeView,WControl,CLASS_Transient);
	DECLARE_WINDOWSUBCLASS(WTreeView,WControl,Window)

	// Delegates.
	FDelegate ItemExpandingDelegate;
	FDelegate SelChangedDelegate;
	FDelegate DblClkDelegate;

	// Constructor.
	WTreeView()
	{}
	WTreeView( WWindow* InOwner, INT InId=0, WNDPROC InSuperProc=NULL  )
	: WControl( InOwner, InId, InSuperProc?InSuperProc:SuperProc )
	{}

	// WWindow interface.
	void OpenWindow( UBOOL Visible, INT X, INT Y, INT XL, INT YL, DWORD dwExtraStyle = 0 )
	{
		PerformCreateWindowEx
		(
			WS_EX_CLIENTEDGE,
            NULL,
            WS_CHILD | TVS_HASLINES | TVS_HASBUTTONS | TVS_LINESATROOT | TVS_SHOWSELALWAYS | TVS_DISABLEDRAGDROP | dwExtraStyle,
            X, Y,
			XL, YL,
            *OwnerWindow,
            (HMENU)ControlId,
            hInstance
		);
		SendMessageX( *this, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(0,0) );
		if( Visible )
			ShowWindow( *this, SW_SHOWNOACTIVATE );
	}
	void Empty( void )
	{
		SendMessageX( *this, TVM_DELETEITEM, 0, (LPARAM)TVI_ROOT );
	}
	HTREEITEM AddItem( const TCHAR* pName, HTREEITEM hti, BOOL bHasChildren )
	{
		TCHAR chName[256] = TEXT("\0");

		appStrcpy( chName, pName );

		TV_INSERTSTRUCT	InsertInfo;

		InsertInfo.hParent = hti;
		InsertInfo.hInsertAfter = TVI_SORT;

		InsertInfo.item.mask = TVIF_TEXT;
		InsertInfo.item.pszText = chName;

		if( bHasChildren )
		{
			InsertInfo.item.mask |= TVIF_CHILDREN;
			InsertInfo.item.cChildren = 1;
		}

		return (HTREEITEM)SendMessageX( *this, TVM_INSERTITEM, 0, (LPARAM) &InsertInfo );
	}
	virtual void OnRightButtonDown()
	{
		PostMessageX( OwnerWindow->hWnd, WM_COMMAND, WM_TREEVIEW_RIGHT_CLICK, 0 );
	}
	UBOOL InterceptControlCommand( UINT Message, UINT wParam, LONG lParam )
	{
		LastwParam = wParam;
		LastlParam = lParam;

		NMTREEVIEW* pntv = (LPNMTREEVIEW)lParam;

		if( pntv->hdr.code==TVN_SELCHANGEDA
				|| pntv->hdr.code==TVN_SELCHANGEDW )
			SelChangedDelegate();
		if( pntv->hdr.code==TVN_ITEMEXPANDINGA
				|| pntv->hdr.code==TVN_ITEMEXPANDINGW )
			ItemExpandingDelegate();
		if( pntv->hdr.code==NM_DBLCLK )
		{
			DblClkDelegate();
			return 1;
		}

		return 0;
	}
};

/*-----------------------------------------------------------------------------
	WCoolButton.
-----------------------------------------------------------------------------*/

// Frame showing styles.
enum EFrameFlags
{
	CBFF_ShowOver	= 0x01,
	CBFF_ShowAway	= 0x02,
	CBFF_DimAway    = 0x04,
	CBFF_UrlStyle	= 0x08,
	CBFF_NoCenter   = 0x10
};

// A coolbar-style button.
class WINDOW_API WCoolButton : public WButton
{
	W_DECLARE_CLASS(WCoolButton,WButton,CLASS_Transient);
	DECLARE_WINDOWSUBCLASS(WCoolButton,WButton,Window)

	// Variables.
	static WCoolButton* GlobalCoolButton;
	HICON hIcon;
	DWORD FrameFlags;

	// Constructor.
	WCoolButton()
	{}
	WCoolButton( WWindow* InOwner, INT InId=0, FDelegate InClicked=FDelegate(), DWORD InFlags=CBFF_ShowOver|CBFF_DimAway )
	: WButton( InOwner, InId, InClicked )
	, hIcon( NULL )
	, FrameFlags( InFlags )
	{}

	// WWindow interface.
	void OpenWindow( UBOOL Visible, INT X, INT Y, INT XL, INT YL, const TCHAR* Text )
	{
#ifndef JAPANESE
		PerformCreateWindowEx
		(
			0,
            NULL,
            WS_CHILD | BS_OWNERDRAW,
            X, Y,
			XL, YL,
            *OwnerWindow,
            (HMENU)ControlId,
            hInstance
		);
#else
		PerformCreateWindowEx
		(
			0,
            TEXT("BUTTON"),
            WS_CHILD | BS_PUSHBUTTON | WS_VISIBLE,
            X, Y,
			XL, YL,
            *OwnerWindow,
            (HMENU)ControlId,
            hInstance
		);
#endif
		SetText( Text );

		SendMessageX( *this, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(0,0) );
		if( Visible )
			ShowWindow( *this, SW_SHOWNOACTIVATE );
	}
	void OnDestroy()
	{
		if( GlobalCoolButton==this )
			GlobalCoolButton=NULL;
		WButton::OnDestroy();
	}
#ifndef JAPANESE
	void OnCreate()
	{
		WButton::OnCreate();
		SetClassLongX( *this, GCL_STYLE, GetClassLongX(*this,GCL_STYLE) & ~CS_DBLCLKS);
	}

	// WCoolButton interface.
	void UpdateHighlight( UBOOL TurnOff )
	{
		FPoint P;
		FRect R;
		::GetCursorPos((POINT*)&P);
		::GetWindowRect(*this,(RECT*)&R);
		UBOOL ShouldHighlight = (R.Contains(P) || GetCapture()==hWnd) && !TurnOff;
		if( (GlobalCoolButton==this) != (ShouldHighlight) )
		{
			if( GlobalCoolButton==this )
			{
				GlobalCoolButton = NULL;
				KillTimer( hWnd, 0 );
			}
			else
			{
				if( GlobalCoolButton )
					GlobalCoolButton->UpdateHighlight( 1 );
				GlobalCoolButton = this;
				SetTimer( hWnd, 0, 50, NULL );
			}
			InvalidateRect( *this, NULL, 1 );
			UpdateWindow( *this );
		}
	}
	void OnTimer()
	{
		UpdateHighlight(0);
	}
	INT OnSetCursor()
	{
		WButton::OnSetCursor();
		UpdateHighlight(0);
		if( FrameFlags & CBFF_UrlStyle )
			SetCursor(LoadCursorIdX(hInstanceWindow,IDC_Hand));
		return 1;
	}

	// WWindow interface.
	void OnDrawItem( DRAWITEMSTRUCT* Item )
	{
		RECT R=Item->rcItem;
		UBOOL Pressed = (Item->itemState&ODS_SELECTED)!=0;
		FillRect( Item->hDC, &Item->rcItem, (HBRUSH)(COLOR_BTNFACE+1));
		if( GlobalCoolButton==this )
		{
			if( FrameFlags & CBFF_ShowOver )
				DrawEdge( Item->hDC, &Item->rcItem, Pressed?EDGE_SUNKEN:EDGE_RAISED, BF_RECT );
		}
		else
		{
			if( FrameFlags & CBFF_DimAway )
				DrawEdge( Item->hDC, &Item->rcItem, Pressed?BDR_SUNKENINNER:BDR_RAISEDINNER, BF_RECT );
			else if( FrameFlags & CBFF_ShowAway )
				DrawEdge( Item->hDC, &Item->rcItem, Pressed?EDGE_SUNKEN:EDGE_RAISED, BF_RECT );
		}
		R.left += Pressed;
		R.right += Pressed;
		R.top += Pressed;
		R.bottom += Pressed;
		if( hIcon )
		{
			ICONINFO II;
			GetIconInfo( hIcon, &II );
			DrawIcon( Item->hDC, R.left, R.top + (R.bottom-R.top)/2-II.yHotspot, hIcon );
			R.left += II.xHotspot * 2;
		}
		FString Text = GetText();
		DWORD TextFlags
		=	DT_END_ELLIPSIS
		|	DT_VCENTER
		|	DT_SINGLELINE
		|	((FrameFlags & CBFF_NoCenter) ? 0 : DT_CENTER);
		HFONT OldFont;
		if( FrameFlags & CBFF_UrlStyle )
		{
			R.left += 8;
			SetTextColor( Item->hDC, RGB(0,0,255) );
			OldFont = SelectObject( Item->hDC, hFontUrl );
		}
		else
			OldFont = SelectObject( Item->hDC, hFontText );
		SetBkMode( Item->hDC, TRANSPARENT );
		DrawTextX( Item->hDC, *Text, Text.Len(), &R, TextFlags );
		SelectObject( Item->hDC, OldFont );
	}
#endif
};

/*-----------------------------------------------------------------------------
	WUrlButton.
-----------------------------------------------------------------------------*/

// A URL button.
class WINDOW_API WUrlButton : public WCoolButton
{
	W_DECLARE_CLASS(WUrlButton,WCoolButton,CLASS_Transient);
	DECLARE_WINDOWSUBCLASS(WUrlButton,WCoolButton,Window)

	// Variables.
	FString URL;

	// Constructor.
	WUrlButton()
	{}
	WUrlButton( WWindow* InOwner, const TCHAR* InURL, INT InId=0 )
	: WCoolButton( InOwner, InId, FDelegate(this,(TDelegate)OnClick) )
	, URL( InURL )
	{
		FrameFlags = CBFF_ShowOver | CBFF_UrlStyle | CBFF_NoCenter;
	}

	// WUrlButton interface.
	void OnClick()
	{
		ShellExecuteX( GetActiveWindow(), TEXT("open"), *URL, TEXT(""), TEXT(""), SW_SHOWNORMAL );
	}
};

/*-----------------------------------------------------------------------------
	WComboBox.
-----------------------------------------------------------------------------*/

// A combo box control.
class WINDOW_API WComboBox : public WControl
{
	W_DECLARE_CLASS(WComboBox,WControl,CLASS_Transient);
	DECLARE_WINDOWSUBCLASS(WComboBox,WControl,Window)

	// Delegates.
	FDelegate DoubleClickDelegate;
	FDelegate DropDownDelegate;
	FDelegate CloseComboDelegate;
	FDelegate EditChangeDelegate;
	FDelegate EditUpdateDelegate;
	FDelegate SetFocusDelegate;
	FDelegate KillFocusDelegate;
	FDelegate SelectionChangeDelegate;
	FDelegate SelectionEndOkDelegate;
	FDelegate SelectionEndCancelDelegate;
 
	// Constructor.
	WComboBox()
	{}
	WComboBox( WWindow* InOwner, INT InId=0, WNDPROC InSuperProc=NULL )
	: WControl( InOwner, InId, InSuperProc?InSuperProc:SuperProc )
	{}

	// WWindow interface.
	void OpenWindow( UBOOL Visible, UBOOL Sort = FALSE, UINT InListType = CBS_DROPDOWNLIST )
	{
		PerformCreateWindowEx
		(
			0,
            NULL,
            WS_CHILD | WS_VSCROLL | InListType | (Sort?CBS_SORT:0) | (Visible?WS_VISIBLE:0),
            0, 0,
			64, 384,
            *OwnerWindow,
            (HMENU)ControlId,
            hInstance
		);

		SendMessageX( *this, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(0,0) );
	}
	virtual LONG WndProc( UINT Message, UINT wParam, LONG lParam )
	{
		if( Message==WM_KEYDOWN && (wParam==VK_UP || wParam==VK_DOWN) )
		{
			// Suppress arrow keys.
			if( Snoop )
				Snoop->SnoopKeyDown( this, wParam );
			return 1;
		}
		else return WControl::WndProc( Message, wParam, lParam );
	}

	// WControl interface.
	UBOOL InterceptControlCommand( UINT Message, UINT wParam, LONG lParam )
	{
		if     ( HIWORD(wParam)==CBN_DBLCLK         ) {DoubleClickDelegate();        return 1;}
		else if( HIWORD(wParam)==CBN_DROPDOWN       ) {DropDownDelegate();           return 1;}
		else if( HIWORD(wParam)==CBN_CLOSEUP        ) {CloseComboDelegate();         return 1;}
		else if( HIWORD(wParam)==CBN_EDITCHANGE     ) {EditChangeDelegate();         return 1;}
		else if( HIWORD(wParam)==CBN_EDITUPDATE     ) {EditUpdateDelegate();         return 1;}
		else if( HIWORD(wParam)==CBN_SETFOCUS       ) {SetFocusDelegate();           return 1;}
		else if( HIWORD(wParam)==CBN_KILLFOCUS      ) {KillFocusDelegate();          return 1;}
		else if( HIWORD(wParam)==CBN_SELCHANGE      ) {SelectionChangeDelegate();    return 1;}
		else if( HIWORD(wParam)==CBN_SELENDOK       ) {SelectionEndOkDelegate();     return 1;}
		else if( HIWORD(wParam)==CBN_SELENDCANCEL   ) {SelectionEndCancelDelegate(); return 1;}
		else return 0;
	}

	// WComboBox interface.
	virtual void AddString( const TCHAR* Str )
	{
		SendMessageLX( *this, CB_ADDSTRING, 0, Str );
	}
	virtual FString GetString( INT Index )
	{
		INT Length = SendMessageX( *this, CB_GETLBTEXTLEN, Index, 0 );
		if( Length==CB_ERR )
			return TEXT("");
#if UNICODE
		if( GUnicode && !GUnicodeOS )
		{
			ANSICHAR* Text = (ANSICHAR*)appAlloca((Length+1)*sizeof(ANSICHAR));
			SendMessageA( *this, CB_GETLBTEXT, Index, (LPARAM)Text );
			return appFromAnsi( Text );
		}
		else
#endif
		{
			TCHAR* Text = (TCHAR*)appAlloca((Length+1)*sizeof(TCHAR));
			SendMessage( *this, CB_GETLBTEXT, Index, (LPARAM)Text );
			return Text;
		}
	}
	virtual INT GetCount()
	{
		return SendMessageX( *this, CB_GETCOUNT, 0, 0 );
	}
	virtual void SetCurrent( INT Index )
	{
		SendMessageX( *this, CB_SETCURSEL, Index, 0 );
	}
	virtual INT GetCurrent()
	{
		return SendMessageX( *this, CB_GETCURSEL, 0, 0 );
	}
	virtual INT FindString( const TCHAR* String )
	{
		INT Index = SendMessageLX( *this, CB_FINDSTRING, -1, String );
		return Index!=CB_ERR ? Index : -1;
	}
	virtual INT FindStringExact( const TCHAR* String )
	{
		INT Index = SendMessageLX( *this, CB_FINDSTRINGEXACT, -1, String );
		return Index!=CB_ERR ? Index : -1;
	}
	void Empty()
	{
		SendMessageLX( *this, CB_RESETCONTENT, 0, 0 );
	}
};

/*-----------------------------------------------------------------------------
	WEdit.
-----------------------------------------------------------------------------*/

// A single-line or multiline edit control.
class WINDOW_API WEdit : public WControl
{
	W_DECLARE_CLASS(WEdit,WControl,CLASS_Transient);
	DECLARE_WINDOWSUBCLASS(WEdit,WControl,Window)

	// Variables.
	FDelegate ChangeDelegate;

	// Constructor.
	WEdit()
	{}
	WEdit( WWindow* InOwner, INT InId=0, WNDPROC InSuperProc=NULL )
	: WControl( InOwner, InId, InSuperProc?InSuperProc:SuperProc )
	{}

	// WWindow interface.
	void OpenWindow( UBOOL Visible, UBOOL Multiline, UBOOL ReadOnly, UBOOL HorizScroll = FALSE, UBOOL NoHideSel = FALSE )
	{
		PerformCreateWindowEx
		(
			WS_EX_CLIENTEDGE,
            NULL,
            WS_CHILD | (HorizScroll?WS_HSCROLL:0) | (Visible?WS_VISIBLE:0) | ES_LEFT | (Multiline?(ES_MULTILINE|WS_VSCROLL):0) | ES_AUTOVSCROLL | ES_AUTOHSCROLL | (ReadOnly?ES_READONLY:0) | (NoHideSel?ES_NOHIDESEL:0),
            0, 0,
			0, 0,
            *OwnerWindow,
            (HMENU)ControlId,
            hInstance
		);

		SendMessageX( *this, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(0,0) );
	}
	UBOOL InterceptControlCommand( UINT Message, UINT wParam, LONG lParam )
	{
		if( HIWORD(wParam)==EN_CHANGE )
		{
			ChangeDelegate();
			return 1;
		}
		else return 0;
	}

	// WEdit interface.
	UBOOL GetReadOnly()
	{
		check(hWnd);
		return (GetWindowLongX( *this, GWL_STYLE )&ES_READONLY)!=0;
	}
	void SetReadOnly( UBOOL ReadOnly )
	{
		check(hWnd);
		SendMessageX( *this, EM_SETREADONLY, ReadOnly, 0 );
	}
	INT GetLineCount()
	{
		check(hWnd);
		return SendMessageX( *this, EM_GETLINECOUNT, 0, 0 );
	}
	INT GetLineIndex( INT Line )
	{
		check(hWnd);
		return SendMessageX( *this, EM_LINEINDEX, Line, 0 );
	}
	void GetSelection( INT& Start, INT& End )
	{
		check(hWnd);
		SendMessageX( *this, EM_GETSEL, (WPARAM)&Start, (LPARAM)&End );
	}
	void SetSelection( INT Start, INT End )
	{
		check(hWnd);
		SendMessageX( *this, EM_SETSEL, Start, End );
	}
	void SetSelectedText( const TCHAR* Text )
	{
		check(hWnd);
		SendMessageLX( *this, EM_REPLACESEL, 1, Text );
	}
	UBOOL GetModify()
	{
		return SendMessageX( *this, EM_GETMODIFY, 0, 0 )!=0;
	}
	void SetModify( UBOOL Modified )
	{
		SendMessageX( *this, EM_SETMODIFY, Modified, 0 );
	}
	void ScrollCaret()
	{
		SendMessageX( *this, EM_SCROLLCARET, 0, 0 );
	}
	virtual FString GetText()
	{
		FString Str;
		Str = WWindow::GetText();

		// If the string begins with an "=" sign, we will evaluate it as a mathematical expresssion
		if( Str.Left(1) == TEXT("=") )
		{
			Str = Str.Mid(1);	// Strip off the "=" sign

			float Result;
			if( !Eval( Str, &Result ) )
				Result = 0;

			Str = FString::Printf(TEXT("%f"), Result );
		}

		return Str;
	}
	// Evaluate a numerical expression.
	// Returns 1 if ok, 0 if error.
	// Sets Result, or 0.0 if error.
	//
	// Operators and precedence: 1:+- 2:/% 3:* 4:^ 5:&|
	// Unary: -
	// Types: Numbers (0-9.), Hex ($0-$f)
	// Grouping: ( )
	//
	virtual UBOOL Eval( FString Str, float* pResult )
	{
		float Result;

		// Check for a matching number of brackets right up front.
		int Brackets = 0;
		for( int x = 0 ; x < Str.Len() ; x++ )
		{
			if( Str.Mid(x,1) == TEXT("(") )
				Brackets++;
			else
				if( Str.Mid(x,1) == TEXT(")") )
					Brackets--;
		}
		if( Brackets != 0 )
		{
			appMsgf(TEXT("Mismatched brackets."));
			Result = 0;
		}
		else
			if( !SubEval( &Str, &Result, 0 ) )
			{
				appMsgf(TEXT("Error in expression."));
				Result = 0;
			}

		*pResult = Result;

		return 1;
	}
	virtual UBOOL SubEval( FString* pStr, float* pResult, int Prec )
	{
		FString c;
		float V, W, N;

		V = W = N = 0.0f;

		c = GrabChar(pStr);

		if((c >= TEXT("0") && c <= TEXT("9")) || c == TEXT(".")) // Number
		{
			V = 0;
			while(c >= TEXT("0") && c <= TEXT("9"))
			{
				V = V * 10 + Val(c);
				c = GrabChar(pStr);
			}
			if(c == TEXT("."))
			{
				N = 0.1f;
				c = GrabChar(pStr);
				while(c >= TEXT("0") && c <= TEXT("9"))
				{
					V = V + N * Val(c);
					N = N / 10.0f;
					c = GrabChar(pStr);
				}
			}
		}
		else if( c == TEXT("(")) // Opening parenthesis
		{
			if( !SubEval(pStr, &V, 0) )
				return 0;
			c = GrabChar(pStr);
		}
		else if( c == TEXT("-") ) // Negation
		{
			if( !SubEval(pStr, &V, 1000) )
				return 0;
			V = -V;
			c = GrabChar(pStr);
		}
		else if( c == TEXT("+")) // Positive
		{
			if( !SubEval(pStr, &V, 1000) )
				return 0;
			c = GrabChar(pStr);
		}
		else if( c == TEXT("@") ) // Square root
		{
			if( !SubEval(pStr, &V, 1000) )
				return 0;
			if( V < 0 )
			{
				appMsgf(TEXT("Can't take square root of negative number"));
				return 0;
			}
			else
				V = appSqrt(V);
			c = GrabChar(pStr);
		}
		else // Error
		{
			appMsgf(TEXT("No value recognized"));
			return 0;
		}
PrecLoop:
		if( c == TEXT("") )
		{
			*pResult = V;
			return 1;
		}
		else if( c == TEXT(")") )
		{
			*pStr = TEXT(")") + *pStr;
			*pResult = V;
			return 1;
		}
		else if( c == TEXT("+") )
		{
			if( Prec > 1 )
			{
				*pResult = V;
				*pStr = c + *pStr;
				return 1;
			}
			else
				if( SubEval(pStr, &W, 2) )
				{
					V = V + W;
					c = GrabChar(pStr);
					goto PrecLoop;
				}
		}
		else if( c == TEXT("-") )
		{
			if( Prec > 1 )
			{
				*pResult = V;
				*pStr = c + *pStr;
				return 1;
			}
			else
				if( SubEval(pStr, &W, 2) )
				{
					V = V - W;
					c = GrabChar(pStr);
					goto PrecLoop;
				}
		}
		else if( c == TEXT("/") )
		{
			if( Prec > 2 )
			{
				*pResult = V;
				*pStr = c + *pStr;
				return 1;
			}
			else
				if( SubEval(pStr, &W, 3) )
				{
					if( W == 0 )
					{
						appMsgf(TEXT("Division by zero isn't allowed"));
						return 0;
					}
					else
					{
						V = V / W;
						c = GrabChar(pStr);
						goto PrecLoop;
					}
				}
		}
		else if( c == TEXT("%") )
		{
			if( Prec > 2 )
			{
				*pResult = V;
				*pStr = c + *pStr;
				return 1;
			}
			else 
				if( SubEval(pStr, &W, 3) )
				{
					if( W == 0 )
					{
						appMsgf(TEXT("Modulo zero isn't allowed"));
						return 0;
					}
					else
					{
						V = (int)V % (int)W;
						c = GrabChar(pStr);
						goto PrecLoop;
					}
				}
		}
		else if( c == TEXT("*") )
		{
			if( Prec > 3 )
			{
				*pResult = V;
				*pStr = c + *pStr;
				return 1;
			}
			else
				if( SubEval(pStr, &W, 4) )
				{
					V = V * W;
					c = GrabChar(pStr);
					goto PrecLoop;
				}
		}
		else
		{
			appMsgf(TEXT("Unrecognized Operator"));
		}

		*pResult = V;
		return 1;
	}
	FString GrabChar( FString* pStr )
	{
		FString GrabChar;
		if( pStr->Len() )
			do {
				GrabChar = pStr->Left(1);
				*pStr = pStr->Mid(1);
			} while( GrabChar == TEXT(" ") );
		else
			GrabChar = TEXT("");

		return GrabChar;
	}
	// Converts a string to it's numeric equivalent, ignoring whitespace.
	// "123  45" - becomes 12,345
	FLOAT Val( FString Value )
	{
		FLOAT RetValue = 0;

		for( int x = 0 ; x < Value.Len() ; x++ )
		{
			FString Char = Value.Mid(x, 1);

			if( Char >= TEXT("0") && Char <= TEXT("9") )
			{
				RetValue *= 10;
				RetValue += appAtoi( *Char );
			}
			else 
				if( Char != TEXT(" ") )
					break;
		}

		return RetValue;
	}
};

/*-----------------------------------------------------------------------------
	WRichEdit.
-----------------------------------------------------------------------------*/

static char *GStreamPos;	// Pointer to the current position we are reading text from
static int GiStreamSz, GiStreamPos;
static DWORD CALLBACK RichEdit_StreamIn(DWORD dwCookie, LPBYTE pbBuffer, LONG cb, LONG *pcb)
{
	if( GiStreamSz - GiStreamPos < cb )
	{
		::strncpy( (char*)pbBuffer, GStreamPos, GiStreamSz - GiStreamPos );
		*pcb = GiStreamSz - GiStreamPos;
	}
	else
	{
		::strncpy( (char*)pbBuffer, GStreamPos, cb );
		GStreamPos += cb;
		*pcb = cb;
	}

	return 0;
}

static DWORD CALLBACK RichEdit_StreamOut(DWORD dwCookie, LPBYTE pbBuffer, LONG cb, LONG *pcb)
{
	::strncat( GStreamPos, (char*)pbBuffer, cb );
	*pcb = cb;
	
	return 0;
}

// A single-line or multiline edit control.
class WINDOW_API WRichEdit : public WControl
{
	W_DECLARE_CLASS(WRichEdit,WControl,CLASS_Transient);
	DECLARE_WINDOWSUBCLASS(WRichEdit,WControl,Window)

	// Variables.
	FDelegate ChangeDelegate;

	// Constructor.
	WRichEdit()
	{}
	WRichEdit( WWindow* InOwner, INT InId=0, WNDPROC InSuperProc=NULL )
	: WControl( InOwner, InId, InSuperProc?InSuperProc:SuperProc )
	{}

	void StreamTextIn( char* _StreamSrc, int _iSz )
	{
		if(_iSz > 25000)		GWarn->BeginSlowTask( TEXT("Loading text..."), 1, 0 );
		GStreamPos = _StreamSrc;
		GiStreamSz = _iSz;
		GiStreamPos = 0;

		EDITSTREAM es;
		es.dwCookie = 0;
		es.dwError = 0;
		es.pfnCallback = RichEdit_StreamIn;
		SendMessageX(hWnd, EM_STREAMIN, SF_RTF, (LPARAM) &es);
		if(_iSz > 25000)		GWarn->EndSlowTask();
	}

	void StreamTextOut( char* _StreamDst, int _iSz )
	{
		if( _iSz > 25000 )	GWarn->BeginSlowTask( TEXT("Saving text..."), 1, 0 );
		GStreamPos = _StreamDst;
		GiStreamSz = _iSz;
		GiStreamPos = 0;

		EDITSTREAM es;
		es.dwCookie = 0;
		es.dwError = 0;
		es.pfnCallback = RichEdit_StreamOut;
		SendMessageX(hWnd, EM_STREAMOUT, SF_TEXT, (LPARAM) &es);
		if( _iSz > 25000 )	GWarn->EndSlowTask();
	}

	// WWindow interface.
	void OpenWindow( UBOOL Visible, UBOOL ReadOnly )
	{
		PerformCreateWindowEx
		(
			WS_EX_CLIENTEDGE,
			NULL,
			WS_CHILD | (Visible?WS_VISIBLE:0) | ES_MULTILINE | ES_NOHIDESEL | ES_SUNKEN | ES_SAVESEL | WS_HSCROLL | WS_VSCROLL,
			0, 0,
			0, 0,
            *OwnerWindow,
            (HMENU)ControlId,
            hInstance
		);
		SendMessageX( *this, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(0,0) );
	}
	UBOOL InterceptControlCommand( UINT Message, UINT wParam, LONG lParam )
	{
		if( HIWORD(wParam)==EN_CHANGE )
		{
			ChangeDelegate();
			return 1;
		}
		else return 0;
	}

	// WRichEdit interface.
	void SetTextLimit( int _Limit )
	{
		SendMessageX( *this, EM_EXLIMITTEXT, _Limit, 0 );
	}
	void SetReadOnly( UBOOL ReadOnly )
	{
		check(hWnd);
		SendMessageX( *this, EM_SETREADONLY, ReadOnly, 0 );
	}
};

/*-----------------------------------------------------------------------------
	WTerminal.
-----------------------------------------------------------------------------*/

// Base class of terminal edit windows.
class WINDOW_API WTerminalBase : public WWindow
{
	W_DECLARE_ABSTRACT_CLASS(WTerminalBase,WWindow,CLASS_Transient);
	DECLARE_WINDOWCLASS(WTerminalBase,WWindow,Window)

	// Constructor.
	WTerminalBase()
	{}
	WTerminalBase( FName InPersistentName, WWindow* InOwnerWindow )
	: WWindow( InPersistentName, InOwnerWindow )
	{}

	// WTerminalBase interface.
	virtual void TypeChar( TCHAR Ch )=0;
	virtual void Paste()=0;
};

// A terminal edit window.
class WINDOW_API WEditTerminal : public WEdit
{
	W_DECLARE_ABSTRACT_CLASS(WEditTerminal,WEdit,CLASS_Transient)
	DECLARE_WINDOWCLASS(WEditTerminal,WEdit,Window)

	// Variables.
	WTerminalBase* OwnerTerminal;

	// Constructor.
	WEditTerminal( WTerminalBase* InOwner=NULL )
	: WEdit( InOwner )
	, OwnerTerminal( InOwner )
	{}

	// WWindow interface.
	void OnChar( TCHAR Ch )
	{
		if( Ch!=('C'-'@') )
		{
			OwnerTerminal->TypeChar( Ch );
			throw TEXT("NoRoute");
		}
	}
	void OnRightButtonDown()
	{
		throw TEXT("NoRoute");
	}
	void OnPaste()
	{
		OwnerTerminal->Paste();
		throw TEXT("NoRoute");
	}
	void OnUndo()
	{
		throw TEXT("NoRoute");
	}
};

// A terminal window.
class WINDOW_API WTerminal : public WTerminalBase, public FOutputDevice
{
	W_DECLARE_CLASS(WTerminal,WTerminalBase,CLASS_Transient);
	DECLARE_WINDOWCLASS(WTerminal,WTerminalBase,Window)

	// Variables.
	WEditTerminal Display;
	FExec* Exec;
	INT MaxLines, SlackLines;
	TCHAR Typing[256];
	UBOOL Shown;

	// Structors.
	WTerminal()
	{}
	WTerminal( FName InPersistentName, WWindow* InOwnerWindow )
	:	WTerminalBase	( InPersistentName, InOwnerWindow )
	,	Display			( this )
	,	Exec			( NULL )
	,	MaxLines		( 256 )
	,	SlackLines		( 64 )
	,	Shown			( 0 )
	{
		appStrcpy( Typing, TEXT(">") );
	}

	// FOutputDevice interface.
	void Serialize( const TCHAR* Data, EName MsgType )
	{
		if( MsgType==NAME_Title )
		{
			SetText( Data );
			return;
		}
		else if( Shown )
		{
			Display.SetRedraw( 0 );
			INT LineCount = Display.GetLineCount();
			if( LineCount > MaxLines )
			{
				INT NewLineCount = Max(LineCount-SlackLines,0);
				INT Index = Display.GetLineIndex( LineCount-NewLineCount );
				Display.SetSelection( 0, Index );
				Display.SetSelectedText( TEXT("") );
				INT Length = Display.GetLength();
				Display.SetSelection( Length, Length );
				Display.ScrollCaret();
			}
			TCHAR Temp[1024]=TEXT("");
			appStrncat( Temp, *FName(MsgType), ARRAY_COUNT(Temp) );
			appStrncat( Temp, TEXT(": "), ARRAY_COUNT(Temp) );
			appStrncat( Temp, (TCHAR*)Data, ARRAY_COUNT(Temp) );
			appStrncat( Temp, TEXT("\r\n"), ARRAY_COUNT(Temp) );
			appStrncat( Temp, Typing, ARRAY_COUNT(Temp) );
			Temp[ARRAY_COUNT(Temp)-1] = 0;
			SelectTyping();
			Display.SetRedraw( 1 );
			Display.SetSelectedText( Temp );
		}
	}

	// WWindow interface.
	void OnShowWindow( UBOOL bShow )
	{
		Shown = bShow;
	}
	void OnCreate()
	{
		WWindow::OnCreate();
		Display.OpenWindow( 1, 1, 1 );

		Display.SetFont( (HFONT)GetStockObject(DEFAULT_GUI_FONT) );
		Display.SetText( Typing );
	}
	void OpenWindow( UBOOL bMdi=0, UBOOL AppWindow=0 )
	{
		MdiChild = bMdi;
		PerformCreateWindowEx
		(
			MdiChild
			?	(WS_EX_MDICHILD)
			:	(AppWindow?WS_EX_APPWINDOW:0),
			*FString::Printf( LocalizeGeneral("LogWindow",TEXT("Window")), LocalizeGeneral("Product",TEXT("Core")) ),
			MdiChild
			?	(WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_SYSMENU | WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX)
			:	(WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_SIZEBOX),
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			512,
			256,
			OwnerWindow ? OwnerWindow->hWnd : NULL,
			NULL,
			hInstance
		);
	}
	void OnSetFocus( HWND hWndLoser )
	{
		WWindow::OnSetFocus( hWndLoser );
		SetFocus( Display );
	}
	void OnSize( DWORD Flags, INT NewX, INT NewY )
	{
		WWindow::OnSize( Flags, NewX, NewY );
		Display.MoveWindow( FRect(0,0,NewX,NewY), TRUE );
		Display.ScrollCaret();
	}

	// WTerminalBase interface.
	void Paste()
	{
		SelectTyping();
		FString Str = appClipboardPaste();
		appStrncat( Typing, *Str, ARRAY_COUNT(Typing) );
		Typing[ARRAY_COUNT(Typing)-1]=0;
		for( INT i=0; Typing[i]; i++ )
			if( Typing[i]<32 || Typing[i]>=127 )
				Typing[i] = 0;
		UpdateTyping();
	}
	void TypeChar( TCHAR Ch )
	{
		SelectTyping();
		INT Length = appStrlen(Typing);
		if( Ch>=32 )
		{
			if( Length<ARRAY_COUNT(Typing)-1 )
			{
				Typing[Length]=Ch;
				Typing[Length+1]=0;
			}
		}
		else if( Ch==13 && Length>1 )
		{
			UpdateTyping();
			Display.SetSelectedText( TEXT("\r\n>") );
			TCHAR Temp[ARRAY_COUNT(Typing)];
			appStrcpy( Temp, Typing+1 );
			appStrcpy( Typing, TEXT(">") );
			if( Exec )
				if( !Exec->Exec( Temp, *GLog ) )
					Log( LocalizeError("Exec",TEXT("Core")) );
			SelectTyping();
		}
		else if( (Ch==8 || Ch==127) && Length>1 )
		{
			Typing[Length-1] = 0;
		}
		else if( Ch==27 )
		{
			appStrcpy( Typing, TEXT(">") );
		}
		UpdateTyping();
		if( Ch==22 )
		{
			Paste();
		}
	}

	// WTerminal interface.
	void SelectTyping()
	{
		INT Length = Display.GetLength();
		Display.SetSelection( Max(Length-appStrlen(Typing),0), Length );
	}
	void UpdateTyping()
	{
		Display.SetSelectedText( Typing );
	}
	void SetExec( FExec* InExec )
	{
		Exec = InExec;
	}
};

/*-----------------------------------------------------------------------------
	WLog.
-----------------------------------------------------------------------------*/

// A log window.
static void GNotifyExit()
{
	if( GNotify )
		TCHAR_CALL_OS(Shell_NotifyIconWX(NIM_DELETE,&NID),Shell_NotifyIconA(NIM_DELETE,&NIDA));
}
class WINDOW_API WLog : public WTerminal
{
	W_DECLARE_CLASS(WLog,WTerminal,CLASS_Transient);
	DECLARE_WINDOWCLASS(WLog,WTerminal,Window)

	// Variables.
	UINT NidMessage;
	FArchive*& LogAr;
	FString LogFilename;

	// Functions.
	WLog()
	:LogAr(LogAr)
	{}
	WLog( const TCHAR* InLogFilename, FArchive*& InLogAr, FName InPersistentName, WWindow* InOwnerWindow=NULL )
	: WTerminal( InPersistentName, InOwnerWindow )
	, NidMessage( RegisterWindowMessageX( TEXT("UnrealNidMessage")) )
	, LogAr( InLogAr )
	, LogFilename( InLogFilename )
	{}
	void OnCreate()
	{
		WWindow::OnCreate();
		Display.OpenWindow( 1, 1, 1 );
		Display.SetFont( (HFONT)GetStockObject(ANSI_FIXED_FONT) );
		Display.SetText( Typing );
	}
	void SetText( const TCHAR* Text )
	{
		WWindow::SetText( Text );
		if( GNotify )
		{
#if UNICODE
			if( GUnicode && !GUnicodeOS )
			{
				appMemcpy( NIDA.szTip, TCHAR_TO_ANSI(Text), Min<INT>(ARRAY_COUNT(NIDA.szTip),appStrlen(Text)+1) );
				NIDA.szTip[ARRAY_COUNT(NIDA.szTip)-1]=0;
				Shell_NotifyIconA( NIM_MODIFY, &NIDA );
			}
			else
#endif
			{
				appStrncpy( NID.szTip, Text, ARRAY_COUNT(NID.szTip) );
#if UNICODE
				Shell_NotifyIconWX(NIM_MODIFY,&NID);
#else
				Shell_NotifyIconA(NIM_MODIFY,&NID);
#endif
			}
		}
	}
	void OnShowWindow( UBOOL bShow )
	{
		WTerminal::OnShowWindow( bShow );
		if( bShow )
		{
			// Load log file.
			if( LogAr )
			{
				delete LogAr;
				FArchive* Reader = GFileManager->CreateFileReader( *LogFilename );
				if( Reader )
				{
#if FORCE_ANSI_LOG
					TArray<ANSICHAR> AnsiText( Reader->TotalSize() );
					Reader->Serialize( &AnsiText(0), AnsiText.Num() );
					delete Reader;
					INT CrCount=0;
					for( INT Ofs=AnsiText.Num()-1; Ofs>0 && CrCount<MaxLines; Ofs-- )
						CrCount += (AnsiText(Ofs)=='\n');
					while( Ofs<AnsiText.Num() && AnsiText(Ofs)=='\n' )
						Ofs++;
					AnsiText.AddItem( '>' );
					AnsiText.AddItem( 0 );
					TArray<TCHAR> Text( AnsiText.Num() );
					for( INT i=0; i<AnsiText.Num(); i++ )
						Text( i ) = FromAnsi( AnsiText(i) );
					Display.SetText( &Text(Ofs) );
#else
					TArray<TCHAR> Text( Reader->TotalSize() );
					Reader->Serialize( &Text(0), Text.Num()/sizeof(TCHAR)-GUnicode*2 );
					delete Reader;
					INT CrCount=0;
					for( INT Ofs=Text.Num()-1; Ofs>0 && CrCount<MaxLines; Ofs-- )
						CrCount += (Text(Ofs)=='\n');
					while( Ofs<Text.Num() && Text(Ofs)=='\n' )
						Ofs++;
					Text.AddItem( '>' );
					Text.AddItem( 0 );
					Display.SetText( &Text(Ofs) );
#endif
				}
				LogAr = GFileManager->CreateFileWriter( *LogFilename, FILEWRITE_Unbuffered|FILEWRITE_Append );
			}
			INT Length = Display.GetLength();
			Display.SetSelection( Length, Length );
			Display.ScrollCaret();
		}
	}
	void OpenWindow( UBOOL bShow, UBOOL bMdi )
	{
		WTerminal::OpenWindow( bMdi, 0 );
		Show( bShow );
		UpdateWindow( *this );
		GLogHook = this;

		// Show dedicated server in tray.
		if( !GIsClient && !GIsEditor )
		{
			NID.cbSize           = sizeof(NID);
			NID.hWnd             = hWnd;
			NID.uID              = 0;
			NID.uFlags           = NIF_ICON | NIF_TIP | NIF_MESSAGE;
			NID.uCallbackMessage = NidMessage;
			NID.hIcon            = LoadIconIdX(hInstanceWindow,(GIsEditor?IDICON_Editor:IDICON_Mainframe));
			NID.szTip[0]         = 0;
#if UNICODE
			if( GUnicode && !GUnicodeOS )
			{
				NIDA.cbSize           = sizeof(NIDA);
				NIDA.hWnd             = hWnd;
				NIDA.uID              = 0;
				NIDA.uFlags           = NIF_ICON | NIF_TIP | NIF_MESSAGE;
				NIDA.uCallbackMessage = NidMessage;
				NIDA.hIcon            = LoadIconIdX(hInstanceWindow,(GIsEditor?IDICON_Editor:IDICON_Mainframe));
				NIDA.szTip[0]         = 0;
				Shell_NotifyIconA(NIM_ADD,&NIDA);
			}
			else
#endif
			{
#if UNICODE
				Shell_NotifyIconWX(NIM_ADD,&NID);
#else
				Shell_NotifyIconA(NIM_ADD,&NID);
#endif
			}
			GNotify = 1;
			atexit( GNotifyExit );
		}
	}
	void OnDestroy()
	{
		GLogHook = NULL;
		WTerminal::OnDestroy();
	}
	void OnCopyData( HWND hWndSender, COPYDATASTRUCT* CD )
	{
		if( Exec )
		{
			debugf( TEXT("WM_COPYDATA: %s"), (TCHAR*)CD->lpData );
			Exec->Exec( TEXT("TakeFocus"), *GLogWindow );
			TCHAR NewURL[1024];
			if
			(	ParseToken(*(TCHAR**)&CD->lpData,NewURL,ARRAY_COUNT(NewURL),0)
			&&	NewURL[0]!='-')
				Exec->Exec( *(US+TEXT("Open ")+NewURL),*GLogWindow );
		}
	}
	void OnClose()
	{
		Show( 0 );
		throw TEXT("NoRoute");
	}
	void OnCommand( INT Command )
	{
		if( Command==ID_LogFileExit || Command==ID_NotifyExit )
		{
			// Exit.
			debugf( TEXT("WLog::OnCommand %s"), Command==ID_LogFileExit ? TEXT("ID_LogFileExit") : TEXT("ID_NotifyExit") );
			appRequestExit( 0 );
		}
		else if( Command==ID_LogAdvancedOptions || Command==ID_NotifyAdvancedOptions )
		{
			// Advanced options.
			if( Exec )
				Exec->Exec( TEXT("PREFERENCES"), *GLogWindow );
		}
		else if( Command==ID_NotifyShowLog )
		{
			// Show log window.
			ShowWindow( hWnd, SW_SHOWNORMAL );
			SetForegroundWindow( hWnd );
		}
	}
	LONG WndProc( UINT Message, UINT wParam, LONG lParam )
	{
		if( Message==NidMessage )
		{
			if( lParam==WM_RBUTTONDOWN || lParam==WM_LBUTTONDOWN )
			{
				// Options.
				POINT P;
				::GetCursorPos( &P );
				HMENU hMenu = LoadLocalizedMenu( hInstanceWindow, IDMENU_NotifyIcon, TEXT("IDMENU_NotifyIcon"), TEXT("Window") );
				SetForegroundWindow( hWnd );
				TrackPopupMenu( GetSubMenu(hMenu,0), lParam==WM_LBUTTONDOWN ? TPM_LEFTBUTTON : TPM_RIGHTBUTTON, P.x, P.y, 0, hWnd, NULL );
				PostMessageX( hWnd, WM_NULL, 0, 0 );
			}
			return 1;
		}
		else return WWindow::WndProc( Message, wParam, lParam );
	}
};

/*-----------------------------------------------------------------------------
	WDialog.
-----------------------------------------------------------------------------*/

// A dialog window, always based on a Visual C++ dialog template.
class WINDOW_API WDialog : public WWindow
{
	W_DECLARE_ABSTRACT_CLASS(WDialog,WWindow,CLASS_Transient);

	// Constructors.
	WDialog()
	{}
	WDialog( FName InPersistentName, INT InDialogId, WWindow* InOwnerWindow=NULL )
	: WWindow( InPersistentName, InOwnerWindow )
	{
		ControlId = InDialogId;
	}

	// WDialog interface.
	INT CallDefaultProc( UINT Message, UINT wParam, LONG lParam )
	{
		return 0;
	}
	virtual INT DoModal( HINSTANCE hInst=hInstanceWindow )
	{
		check(hWnd==NULL);
		_Windows.AddItem( this );
		ModalCount++;
		INT Result = TCHAR_CALL_OS(DialogBoxParamW(hInst/*!!*/,MAKEINTRESOURCEW(ControlId),OwnerWindow?OwnerWindow->hWnd:NULL,(INT(APIENTRY*)(HWND,UINT,WPARAM,LPARAM))StaticWndProc,(LPARAM)this),DialogBoxParamA(hInst/*!!*/,MAKEINTRESOURCEA(ControlId),OwnerWindow?OwnerWindow->hWnd:NULL,(INT(APIENTRY*)(HWND,UINT,WPARAM,LPARAM))StaticWndProc,(LPARAM)this));
		ModalCount--;
		return Result;
	}
	void OpenChildWindow( INT InControlId, UBOOL Visible )
	{
		check(!hWnd);
		_Windows.AddItem( this );
		HWND hWndParent = InControlId ? GetDlgItem(OwnerWindow->hWnd,InControlId) : OwnerWindow ? OwnerWindow->hWnd : NULL;
		HWND hWndCreated = TCHAR_CALL_OS(CreateDialogParamW(hInstanceWindow/*!!*/,MAKEINTRESOURCEW(ControlId),hWndParent,(INT(APIENTRY*)(HWND,UINT,WPARAM,LPARAM))StaticWndProc,(LPARAM)this),CreateDialogParamA(hInstanceWindow/*!!*/,MAKEINTRESOURCEA(ControlId),hWndParent,(INT(APIENTRY*)(HWND,UINT,WPARAM,LPARAM))StaticWndProc,(LPARAM)this));
		check(hWndCreated);
		check(hWndCreated==hWnd);
		Show( Visible );
	}
	static BOOL CALLBACK LocalizeTextEnumProc( HWND hInWmd, LPARAM lParam )
	{
		FString String;
		TCHAR** Temp = (TCHAR**)lParam;
#if UNICODE
		if( !GUnicodeOS )
		{
			ANSICHAR ACh[1024]="";
			SendMessageA( hInWmd, WM_GETTEXT, ARRAY_COUNT(ACh), (LPARAM)ACh );
			String = appFromAnsi(ACh);
		}
		else
#endif
		{
			TCHAR Ch[1024]=TEXT("");
			SendMessage( hInWmd, WM_GETTEXT, ARRAY_COUNT(Ch), (LPARAM)Ch );
			String = Ch;
		}
		if( FString(String).Left(4)==TEXT("IDC_") )
			SendMessageLX( hInWmd, WM_SETTEXT, 0, LineFormat(Localize(Temp[0],*String,Temp[1])) );
		else if( String==TEXT("IDOK") )
			SendMessageLX( hInWmd, WM_SETTEXT, 0, LineFormat(LocalizeGeneral("OkButton",TEXT("Window"))) );
		else if( String==TEXT("IDCANCEL") )
			SendMessageLX( hInWmd, WM_SETTEXT, 0, LineFormat(LocalizeGeneral("CancelButton",TEXT("Window"))) );
		SendMessageX( hInWmd, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(1,0) );
		return 1;
	}
	virtual void LocalizeText( const TCHAR* Section, const TCHAR* Package=GPackage )
	{
		const TCHAR* Temp[3];
		Temp[0] = Section;
		Temp[1] = Package;
		Temp[2] = (TCHAR*)this;
		EnumChildWindows( *this, LocalizeTextEnumProc, (LPARAM)Temp );
		LocalizeTextEnumProc( hWnd, (LPARAM)Temp );
	}
	virtual void OnInitDialog()
	{
		WWindow::OnInitDialog();
		
		SendMessageX( hWnd, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(1,0) );
		for( INT i=0; i<Controls.Num(); i++ )
		{
			// Bind all child controls.
			WControl* Control = Controls(i);
			check(!Control->hWnd);
			Control->hWnd = GetDlgItem( *this, Control->ControlId );
			check(Control->hWnd);
			_Windows.AddItem(Control);
			Control->WindowDefWndProc = (WNDPROC)GetWindowLongX( Control->hWnd, GWL_WNDPROC );
			SetWindowLongX( Control->hWnd, GWL_WNDPROC, (LONG)WWindow::StaticWndProc );
			//warning: Don't set GWL_HINSTANCE, it screws up subclassed edit controls in Win95.
		}
		for( i=0; i<Controls.Num(); i++ )
		{
			// Send create to all controls.
			Controls(i)->OnCreate();
		}
		TCHAR Temp[256];
		appSprintf( Temp, TEXT("IDDIALOG_%s"), *PersistentName );
		LocalizeText( Temp, GetPackageName() );
	}
	void EndDialog( INT Result )
	{
		::EndDialog( hWnd, Result );
	}
	void EndDialogTrue()
	{
		EndDialog( 1 );
	}
	void EndDialogFalse()
	{
		EndDialog( 0 );
	}
};

/*-----------------------------------------------------------------------------
	WPasswordDialog.
-----------------------------------------------------------------------------*/

// A password dialog box.
class WINDOW_API WPasswordDialog : public WDialog
{
	W_DECLARE_CLASS(WPasswordDialog,WDialog,CLASS_Transient);
	DECLARE_WINDOWCLASS(WPasswordDialog,WDialog,Window)

	// Controls.
	WCoolButton OkButton;
	WCoolButton CancelButton;
	WEdit Name;
	WEdit Password;
	WLabel Prompt;

	// Output.
	FString ResultName;
	FString ResultPassword;

	// Constructor.
	WPasswordDialog()
	: WDialog	  ( TEXT("PasswordDialog"), IDDIALOG_Password )
	, OkButton    ( this, IDOK,     FDelegate(this,(TDelegate)EndDialogTrue) )
	, CancelButton( this, IDCANCEL, FDelegate(this,(TDelegate)EndDialogFalse) )
	, Name		  ( this, IDEDIT_Name )
	, Password	  ( this, IDEDIT_Password )
	, Prompt      ( this, IDLABEL_Prompt )
	{}

	// WDialog interface.
	void OnInitDialog()
	{
		WDialog::OnInitDialog();
		SetText( LocalizeQuery("PassDlg",TEXT("Core")) );
		Prompt.SetText( LocalizeQuery("PassPrompt",TEXT("Core")) );
		Name.SetText( TEXT("") );
		Password.SetText( TEXT("") );
		SetFocus( Name );
	}
	void OnDestroy()
	{
		ResultName     = Name.GetText();
		ResultPassword = Password.GetText();
		WDialog::OnDestroy();
	}
};

/*-----------------------------------------------------------------------------
	WTextScrollerDialog.
-----------------------------------------------------------------------------*/

// A text scroller dialog box.
class WINDOW_API WTextScrollerDialog : public WDialog
{
	W_DECLARE_CLASS(WTextScrollerDialog,WDialog,CLASS_Transient);
	DECLARE_WINDOWCLASS(WTextScrollerDialog,WDialog,Window)

	// Controls.
	WEdit TextEdit;
	WCoolButton OkButton;
	FString Caption, Message;

	// Constructor.
	WTextScrollerDialog()
	{}
	WTextScrollerDialog( const TCHAR* InCaption, const TCHAR* InMessage )
	: WDialog	( TEXT("TextScrollerDialog"), IDDIALOG_TextScroller )
	, TextEdit  ( this, IDEDIT_TextEdit )
	, OkButton  ( this, IDOK, FDelegate(this,(TDelegate)EndDialogTrue) )
	, Caption   ( InCaption )
	, Message   ( InMessage )
	{}

	// WDialog interface.
	void OnInitDialog()
	{
		WDialog::OnInitDialog();
		HDC hDC = GetDC(hWnd);
		ReleaseDC(hWnd,hDC);
		TextEdit.SetFont( hFontText );
		SetText( *Caption );
		TextEdit.SetText( *Message );
		SetFocus( OkButton );
	}
};

/*-----------------------------------------------------------------------------
	WTrackBar.
-----------------------------------------------------------------------------*/

// A non-interactive label control.
class WINDOW_API WTrackBar : public WControl
{
	W_DECLARE_CLASS(WTrackBar,WControl,CLASS_Transient)
	DECLARE_WINDOWSUBCLASS(WTrackBar,WControl,Window)

	// Delegates.
	FDelegate ThumbTrackDelegate;
	FDelegate ThumbPositionDelegate;

	// Constructor.
	WTrackBar()
	{}
	WTrackBar( WWindow* InOwner, INT InId=0, WNDPROC InSuperProc=NULL )
	: WControl( InOwner, InId, InSuperProc?InSuperProc:SuperProc )
	{}

	// WWindow interface.
	void OpenWindow( UBOOL Visible, UBOOL InClientEdge = 1 )
	{
		PerformCreateWindowEx
		(
			(InClientEdge?WS_EX_CLIENTEDGE:0),
            NULL,
            WS_CHILD | TBS_HORZ | TBS_AUTOTICKS | TBS_BOTTOM | (Visible?WS_VISIBLE:0),
            0, 0,
			0, 0,
            *OwnerWindow,
            (HMENU)ControlId,
            hInstance
		);
	}

	// WControl interface.
	UBOOL InterceptControlCommand( UINT Message, UINT wParam, LONG lParam )
	{
		if     ( Message==WM_HSCROLL && LOWORD(wParam)==TB_THUMBTRACK ) {ThumbTrackDelegate();    return 1;}
		else if( Message==WM_HSCROLL                                  ) {ThumbPositionDelegate(); return 1;}
		else return 0;
	}

	// WTrackBar interface.
	void SetTicFreq( INT TicFreq )
	{
		SendMessageX( *this, TBM_SETTICFREQ, TicFreq, 0 );
	}
	void SetRange( INT Min, INT Max )
	{
		SendMessageX( *this, TBM_SETRANGE, 1, MAKELONG(Min,Max) );
	}
	void SetPos( INT Pos )
	{
		SendMessageX( *this, TBM_SETPOS, 1, Pos );
	}
	INT GetPos()
	{
		return SendMessageX( *this, TBM_GETPOS, 0, 0 );
	}
};

/*-----------------------------------------------------------------------------
	WTrackBar.
-----------------------------------------------------------------------------*/

// A non-interactive label control.
class WINDOW_API WProgressBar : public WControl
{
	W_DECLARE_CLASS(WProgressBar,WControl,CLASS_Transient);
	DECLARE_WINDOWSUBCLASS(WProgressBar,WControl,Window)

	// Variables.
	INT Percent;

	// Constructor.
	WProgressBar()
	{}
	WProgressBar( WWindow* InOwner, INT InId=0, WNDPROC InSuperProc=NULL )
	: WControl( InOwner, InId, InSuperProc?InSuperProc:SuperProc )
	, Percent( 0 )
	{}

	// WWindow interface.
	void OpenWindow( UBOOL Visible )
	{
		PerformCreateWindowEx
		(
			WS_EX_CLIENTEDGE,
            NULL,
            WS_CHILD | (Visible?WS_VISIBLE:0),
            0, 0,
			0, 0,
            *OwnerWindow,
            (HMENU)ControlId,
            hInstance
		);
		SendMessageX( *this, PBM_SETRANGE, 0, 100 );
	}

	// WProgressBar interface.
	void SetProgress( INT InCurrent, INT InMax )
	{
		INT InPercent = (SQWORD)InCurrent*100/Max(InMax,1);
		if( InPercent!=Percent )
			SendMessageX( *this, PBM_SETPOS, InPercent, 0 );
		Percent = InPercent;
	}
};

/*-----------------------------------------------------------------------------
	WListBox.
-----------------------------------------------------------------------------*/

// A list box.
class WINDOW_API WListBox : public WControl
{
	W_DECLARE_CLASS(WListBox,WControl,CLASS_Transient);
	DECLARE_WINDOWSUBCLASS(WListBox,WControl,Window)

	// Delegates.
	FDelegate DoubleClickDelegate;
	FDelegate SelectionChangeDelegate;
	FDelegate SelectionCancelDelegate;
	FDelegate SetFocusDelegate;
	FDelegate KillFocusDelegate;

	BOOL m_bMultiSel;

	// Constructor.
	WListBox()
	{}
	WListBox( WWindow* InOwner, INT InId=0, WNDPROC InSuperProc=NULL )
	: WControl( InOwner, InId, InSuperProc?InSuperProc:SuperProc )
	{
		check(OwnerWindow);
		m_bMultiSel = FALSE;
	}

	// WWindow interface.
	void OpenWindow( UBOOL Visible, UBOOL Integral, UBOOL MultiSel, UBOOL OwnerDrawVariable, UBOOL Sort = 0, DWORD dwExtraStyle = 0  )
	{
		m_bMultiSel = MultiSel;
		PerformCreateWindowEx
		(
			WS_EX_CLIENTEDGE,
            NULL,
            WS_CHILD | WS_BORDER | WS_VSCROLL | WS_CLIPCHILDREN | LBS_NOTIFY | (Visible?WS_VISIBLE:0) | (Integral?0:LBS_NOINTEGRALHEIGHT) 
			| (MultiSel?(LBS_EXTENDEDSEL|LBS_MULTIPLESEL):0) | (OwnerDrawVariable?LBS_OWNERDRAWVARIABLE:0) | (Sort?LBS_SORT:0) | dwExtraStyle,
            0, 0,
			0, 0,
            *OwnerWindow,
            (HMENU)ControlId,
            hInstance
		);

		SendMessageX( *this, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(0,0) );
	}

	// WControl interface.
	UBOOL InterceptControlCommand( UINT Message, UINT wParam, LONG lParam )
	{
		if     ( HIWORD(wParam)==LBN_DBLCLK   ) {DoubleClickDelegate();     return 1;}
		else if( HIWORD(wParam)==LBN_SELCHANGE) {SelectionChangeDelegate(); return 1;}
		else if( HIWORD(wParam)==LBN_SELCANCEL) {SelectionCancelDelegate(); return 1;}
		else if( HIWORD(wParam)==LBN_SETFOCUS)  {SetFocusDelegate();        return 1;}
		else if( HIWORD(wParam)==LBN_KILLFOCUS) {KillFocusDelegate();       return 1;}
		else return 0;
	}

	// WListBox interface.
	FString GetString( INT Index )
	{
		INT Length = SendMessageX(*this,LB_GETTEXTLEN,Index,0);
#if UNICODE
		if( GUnicode && !GUnicodeOS )
		{
			ANSICHAR* ACh = (ANSICHAR*)appAlloca((Length+1)*sizeof(ANSICHAR));
			SendMessageA( *this, LB_GETTEXT, Index, (LPARAM)ACh );
			ACh[Length] = 0;
			return appFromAnsi(ACh);
		}
		else
#endif
		{
			TCHAR* Ch = (TCHAR*)appAlloca((Length+1)*sizeof(TCHAR));
			SendMessage( *this, LB_GETTEXT, Index, (LPARAM)Ch );
			Ch[Length] = 0;
			return Ch;
		}
	}
	void* GetItemData( INT Index )
	{
		return (void*)SendMessageX( *this, LB_GETITEMDATA, Index, 0 );
	}
	void SetItemData( INT Index, void* Value )
	{
		SendMessageX( *this, LB_SETITEMDATA, Index, (LPARAM)Value );
	}
	void SetItemData( INT Index, INT Value )
	{
		SendMessageX( *this, LB_SETITEMDATA, Index, (LPARAM)Value );
	}
	INT GetCurrent()
	{
		return SendMessageX( *this, LB_GETCARETINDEX, 0, 0 );
	}
	void ClearSel()
	{
		SendMessageX( *this, LB_SETSEL, FALSE, -1 );
	}
	void SetCurrent( INT Index, UBOOL bScrollIntoView = 1 )
	{
		if( m_bMultiSel )
		{
			ClearSel();
			SendMessageX( *this, LB_SETSEL, TRUE, Index );
			SendMessageX( *this, LB_SETCARETINDEX, Index, bScrollIntoView );
		}
		else
		{
			SendMessageX( *this, LB_SETCURSEL, Index, 0 );
			SendMessageX( *this, LB_SETCARETINDEX, Index, bScrollIntoView );
		}
	}
	INT GetTop()
	{
		return SendMessageX( *this, LB_GETTOPINDEX, 0, 0 );
	}
	void SetTop( INT Index )
	{
		SendMessageX( *this, LB_SETTOPINDEX, Index, 0 );
	}
	void DeleteString( INT Index )
	{
		SendMessageX( *this, LB_DELETESTRING, Index, 0 );
	}
	INT GetCount()
	{
		return SendMessageX( *this, LB_GETCOUNT, 0, 0 );
	}
	INT GetItemHeight( INT Index )
	{
		return SendMessageX( *this, LB_GETITEMHEIGHT, Index, 0 );
	}
	INT ItemFromPoint( FPoint P )
	{
		DWORD Result=SendMessageX( *this, LB_ITEMFROMPOINT, 0, MAKELPARAM(P.X,P.Y) );
		return HIWORD(Result) ? -1 : LOWORD(Result);
	}
	FRect GetItemRect( INT Index )
	{
		RECT R; R.left=R.right=R.top=R.bottom=0;
		SendMessageX( *this, LB_GETITEMRECT, Index, (LPARAM)&R );
		return R;
	}
	void Empty()
	{
		SendMessageX( *this, LB_RESETCONTENT, 0, 0 );
	}
	UBOOL GetSelected( INT Index )
	{
		return SendMessageX( *this, LB_GETSEL, Index, 0 );
	}
	INT GetSelectedItems( INT Count, INT* Buffer )
	{
		return SendMessageX( *this, LB_GETSELITEMS, (WPARAM)Count, (LPARAM)Buffer );
	}
	INT GetSelectedCount()
	{
		return SendMessageX( *this, LB_GETSELCOUNT, 0, 0 );
	}

	void SetMultiSelection(int index,UBOOL value)
	{
		SendMessageX(*this,LB_SETSEL,value,index);
	}

	void InvertSelection()
	{
		int SelCount=GetSelectedCount();
		int TotalCount=GetCount();

		int *Buffer=NULL;
		
		if(SelCount)
		{
			Buffer = new int[SelCount];
		}
		
		for(int i=0;i<TotalCount;i++)
		{
			UBOOL value=true;

			if(Buffer)
				for(int j=0;j<SelCount;j++)
				{
					if(i==j)
					{
						value=false;
						break;
					}
				}

			SetMultiSelection(i,value);
		}

		if(Buffer) { delete [] Buffer; Buffer=NULL; }
	}

	// Accessing as strings.
	INT AddString( const TCHAR* C )
	{
		return SendMessageLX( *this, LB_ADDSTRING, 0, C );
	}
	void InsertString( INT Index, const TCHAR* C )
	{
		SendMessageLX( *this, LB_INSERTSTRING, Index, C );
	}
	INT FindStringExact( const TCHAR* C )
	{
		return SendMessageLX( *this, LB_FINDSTRINGEXACT, -1, C );
	}
	INT FindString( const TCHAR* C )
	{
		return SendMessageLX( *this, LB_FINDSTRING, -1, C );
	}
	INT FindStringChecked( const TCHAR* C )
	{
		INT Result = SendMessageLX( *this, LB_FINDSTRING, -1, C );
		check(Result!=LB_ERR);
		return Result;
	}
	void InsertStringAfter( const TCHAR* Existing, const TCHAR* New )
	{
		InsertString( FindStringChecked(Existing)+1, New );
	}

	// Accessing as pointers.
	INT AddItem( const void* C )
	{
		return SendMessageX( *this, LB_ADDSTRING, 0, (LPARAM)C );
	}
	void InsertItem( INT Index, const void* C )
	{
		SendMessageX( *this, LB_INSERTSTRING, Index, (LPARAM)C );
	}
	INT FindItem( const void* C )
	{
		return SendMessageX( *this, LB_FINDSTRING, -1, (LPARAM)C );
	}
	INT FindItemChecked( const void* C )
	{
		INT Result = SendMessageX( *this, LB_FINDSTRING, -1, (LPARAM)C );
		check(Result!=LB_ERR);
		return Result;
	}
	void InsertItemAfter( const void* Existing, const void* New )
	{
		InsertItem( FindItemChecked(Existing)+1, New );
	}
};

/*-----------------------------------------------------------------------------
	WCheckListBox.
-----------------------------------------------------------------------------*/

// A list box where each item has a checkbox beside it.
class WINDOW_API WCheckListBox : public WListBox
{
	W_DECLARE_CLASS(WCheckListBox,WListBox,CLASS_Transient);
	DECLARE_WINDOWSUBCLASS(WCheckListBox,WListBox,Window)

	HBITMAP hbmOff, hbmOn;
	int bOn;

	// Constructor.
	WCheckListBox()
	{}
	WCheckListBox( WWindow* InOwner, INT InId=0, WNDPROC InSuperProc=NULL )
		: WListBox( InOwner, InId )
	{
		check(OwnerWindow);
		hbmOff = (HBITMAP)LoadImageA( hInstanceWindow, MAKEINTRESOURCEA(IDBM_CHECKBOX_OFF), IMAGE_BITMAP, 0, 0, LR_LOADMAP3DCOLORS );	check(hbmOff);
		hbmOn = (HBITMAP)LoadImageA( hInstanceWindow, MAKEINTRESOURCEA(IDBM_CHECKBOX_ON), IMAGE_BITMAP, 0, 0, LR_LOADMAP3DCOLORS );	check(hbmOn);
		bOn = 0;
	}

	// WWindow interface.
	void OpenWindow( UBOOL Visible, UBOOL Integral, UBOOL MultiSel, UBOOL Sort = FALSE, DWORD dwExtraStyle = 0 )
	{
		m_bMultiSel = MultiSel;
		PerformCreateWindowEx
		(
			WS_EX_CLIENTEDGE,
            NULL,
            WS_CHILD | WS_BORDER | WS_VSCROLL | WS_CLIPCHILDREN | LBS_NOTIFY | (Visible?WS_VISIBLE:0) | (Integral?0:LBS_NOINTEGRALHEIGHT) 
				| (MultiSel?(LBS_EXTENDEDSEL|LBS_MULTIPLESEL):0) | LBS_OWNERDRAWFIXED | LBS_HASSTRINGS | (Sort?LBS_SORT:0) | dwExtraStyle,
            0, 0,
			0, 0,
            *OwnerWindow,
            (HMENU)ControlId,
            hInstance
		);

		SendMessageX( *this, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(0,0) );
	}
	void OnDestroy()
	{
		WListBox::OnDestroy();
		DeleteObject(hbmOff);
		DeleteObject(hbmOn);
	}
	void OnDrawItem( DRAWITEMSTRUCT* Item )
	{
		if(((LONG)(Item->itemID) >= 0)
			&& (Item->itemAction & (ODA_DRAWENTIRE | ODA_SELECT)))
		{
			BOOL fDisabled = !IsWindowEnabled(hWnd);

			COLORREF newTextColor = fDisabled ? RGB(0x80, 0x80, 0x80) : GetSysColor(COLOR_WINDOWTEXT);  // light gray
			COLORREF oldTextColor = SetTextColor(Item->hDC, newTextColor);

			COLORREF newBkColor = GetSysColor(COLOR_WINDOW);
			COLORREF oldBkColor = SetBkColor(Item->hDC, newBkColor);

			if (newTextColor == newBkColor)
				newTextColor = RGB(0xC0, 0xC0, 0xC0);   // dark gray

			if (!fDisabled && ((Item->itemState & ODS_SELECTED) != 0))
			{
				SetTextColor(Item->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
				SetBkColor(Item->hDC, GetSysColor(COLOR_HIGHLIGHT));
			}

			FString strText = GetString(Item->itemID);
			ExtTextOutA(Item->hDC, Item->rcItem.left + 18,
				Item->rcItem.top + 2,
				ETO_OPAQUE, &(Item->rcItem), appToAnsi( *strText ), strText.Len(), NULL);

			SetTextColor(Item->hDC, oldTextColor);
			SetBkColor(Item->hDC, oldBkColor);

			// BITMAP
			//
			HDC hdcMem = CreateCompatibleDC(Item->hDC);
			HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, ( (int)GetItemData( Item->itemID ) ? hbmOn : hbmOff));

			BitBlt(Item->hDC,
				Item->rcItem.left + 2, Item->rcItem.top + 3,
				13, 13,
				hdcMem,
				0, 0,
				SRCCOPY);

			SelectObject(hdcMem, hbmOld);
			DeleteDC(hdcMem);
		}

		if((Item->itemAction & ODA_FOCUS) != 0)
			DrawFocusRect(Item->hDC, &(Item->rcItem));
	}

	void InvertSelection()
	{
		int Count=GetCount();
		for(int i=0;i<Count;i++)
		{
			int ItemData=(int)GetItemData(i);
			SetItemData(i,ItemData^true);
		}

		check(OwnerWindow);
		PostMessageX( OwnerWindow->hWnd, WM_COMMAND, WM_WCLB_UPDATE_VISIBILITY, 0L );
		InvalidateRect( hWnd, NULL, FALSE );
	}
	void OnLeftButtonDown()
	{
		WListBox::OnLeftButtonDown();

		FPoint pt;
		::GetCursorPos((POINT*)pt);
		::ScreenToClient( hWnd, (POINT*)pt );

		if( pt.X <= 16 )
		{
			int Item = ItemFromPoint( pt );
			int Data = (int)GetItemData( Item );
			Data = Data ? 0 : 1;
			SetItemData( Item, Data );
		}

		check(OwnerWindow);
		PostMessageX( OwnerWindow->hWnd, WM_COMMAND, WM_WCLB_UPDATE_VISIBILITY, 0L );
		InvalidateRect( hWnd, NULL, FALSE );
	}
};

/*-----------------------------------------------------------------------------
	FTreeItemBase.
-----------------------------------------------------------------------------*/

class WINDOW_API FTreeItemBase : public FCommandTarget, public FControlSnoop
{
public:
	virtual void Draw( HDC hDC )=0;
	virtual INT GetHeight()=0;
	virtual void SetSelected( UBOOL NewSelected )=0;
};

/*-----------------------------------------------------------------------------
	WItemBox.
-----------------------------------------------------------------------------*/

// A list box contaning list items.
class WINDOW_API WItemBox : public WListBox
{
	W_DECLARE_CLASS(WItemBox,WListBox,CLASS_Transient);
	DECLARE_WINDOWCLASS(WItemBox,WListBox,Window)

	// Constructors.
	WItemBox()
	{}
	WItemBox( WWindow* InOwner, INT InId=0)
	: WListBox( InOwner, InId )
	{
		check(OwnerWindow);
	}

	void OnDrawItem( DRAWITEMSTRUCT* Info )
	{
		if( Info->itemData )
		{
			((FTreeItemBase*)Info->itemData)->SetSelected( (Info->itemState & ODS_SELECTED)!=0 );
			((FTreeItemBase*)Info->itemData)->Draw( Info->hDC );
		}
	}
	void OnMeasureItem( MEASUREITEMSTRUCT* Info )
	{
		if( Info->itemData )
			Info->itemHeight = ((FTreeItemBase*)Info->itemData)->GetHeight();
	}
};

/*-----------------------------------------------------------------------------
	WListView.
-----------------------------------------------------------------------------*/

// A list view.
class WINDOW_API WListView : public WControl
{
	W_DECLARE_CLASS(WListView,WControl,CLASS_Transient)
	DECLARE_WINDOWSUBCLASS(WListView,WControl,Window)

	FDelegate DblClkDelegate; 

	// Constructor.
	WListView()
	{}
	WListView( WWindow* InOwner, INT InId=0, WNDPROC InSuperProc=NULL )
	: WControl( InOwner, InId, InSuperProc?InSuperProc:SuperProc )
	{}

	// WWindow interface.
	void OpenWindow( UBOOL Visible, DWORD dwExtraStyle = 0 )
	{
		PerformCreateWindowEx
		(
			WS_EX_CLIENTEDGE,
            NULL,
            WS_CHILD | (Visible?WS_VISIBLE:0) | dwExtraStyle,
            0, 0,
			0, 0,
            *OwnerWindow,
            (HMENU)ControlId,
            hInstance
		);

		SendMessageX( *this, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(0,0) );
	}
	void Empty()
	{
		SendMessageX(hWnd, LVM_DELETEALLITEMS, 0, 0L);
	}
	UBOOL InterceptControlCommand( UINT Message, UINT wParam, LONG lParam )
	{
		LastwParam = wParam;
		LastlParam = lParam;

		NMTREEVIEW* pntv = (LPNMTREEVIEW)lParam;

		if( pntv->hdr.code==NM_DBLCLK )
		{
			DblClkDelegate();
			return 1;
		}

		return 0;
	}
	int GetCurrent()
	{
		return SendMessageX( hWnd, LVM_GETNEXTITEM, -1, LVNI_ALL | LVNI_SELECTED | LVNI_FOCUSED );
	}
};

/*-----------------------------------------------------------------------------
	WPropertiesBase.
-----------------------------------------------------------------------------*/

class WINDOW_API WPropertiesBase : public WWindow, public FControlSnoop
{
	W_DECLARE_ABSTRACT_CLASS(WPropertiesBase,WWindow,CLASS_Transient);

	// Variables.
	UBOOL ShowTreeLines;
	WItemBox List;
	class FTreeItem* FocusItem;

	// Structors.
	WPropertiesBase()
	{}
	WPropertiesBase( FName InPersistentName, WWindow* InOwnerWindow )
	:	WWindow			( InPersistentName, InOwnerWindow )
	,	List			( this )
	,	FocusItem		( NULL )
	,	ShowTreeLines	( 1 )
	{
		List.Snoop = this;
	}

	// WPropertiesBase interface.
	virtual FTreeItem* GetRoot()=0;
	virtual INT GetDividerWidth()=0;
	virtual void ResizeList()=0;
	virtual void SetItemFocus( UBOOL FocusCurrent )=0;
	virtual void ForceRefresh()=0;
	virtual void BeginSplitterDrag()=0;
	virtual class FTreeItem* GetListItem( INT i )
	{
		FTreeItem* Result = (FTreeItem*)List.GetItemData(i);
		check(Result);
		return Result;
	}
};

/*-----------------------------------------------------------------------------
	WDragInterceptor.
-----------------------------------------------------------------------------*/

// Splitter drag handler.
class WINDOW_API WDragInterceptor : public WWindow
{
	W_DECLARE_CLASS(WDragInterceptor,WWindow,CLASS_Transient);
	DECLARE_WINDOWCLASS(WDragInterceptor,WWindow,Window)

	// Variables.
	FPoint		OldMouseLocation;
	FPoint		DragIndices;
	FPoint		DragPos;
	FPoint		DragStart;
	FPoint		DrawWidth;
	FRect		DragClamp;
	UBOOL		Success;

	// Constructor.
	WDragInterceptor()
	{}
	WDragInterceptor( WWindow* InOwner, FPoint InDragIndices, FRect InDragClamp, FPoint InDrawWidth )
	:	WWindow			( NAME_None, InOwner )
	,	DragIndices		( InDragIndices )
	,	DragPos			( FPoint::ZeroValue() )
	,	DragClamp		( InDragClamp )
	,	DrawWidth		( InDrawWidth )
	,	Success			( 1 )
	{}

	// Functions.
	virtual void OpenWindow()
	{
		PerformCreateWindowEx( 0, NULL, WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, *OwnerWindow, NULL, hInstance );
		OldMouseLocation = OwnerWindow->GetCursorPos();
		DragStart = DragPos;
		SetCapture( *this );
		SetFocus( *this );
		ClipCursor( ClientToScreen(DragClamp-DragPos+OwnerWindow->GetCursorPos()) );
		ToggleDraw( NULL );
	}
	virtual void ToggleDraw( HDC hInDC )
	{
		HDC hDC = hInDC ? hInDC : GetDC(*OwnerWindow);
		HBRUSH OldBrush = (HBRUSH)SelectObject( hDC, hBrushStipple );
		if( DragIndices.X!=INDEX_NONE )
			PatBlt( hDC, DragPos.X, 0, DrawWidth.X, OwnerWindow->GetClientRect().Height(), PATINVERT );
		if( DragIndices.Y!=INDEX_NONE )
			PatBlt( hDC, 0, DragPos.Y, OwnerWindow->GetClientRect().Width(), DrawWidth.Y, PATINVERT );
		if( !hInDC )
			ReleaseDC( hWnd, hDC );
		SelectObject( hDC, OldBrush );
	}
	void OnKeyDown( TCHAR Ch )
	{
		if( Ch==VK_ESCAPE )
		{
			Success = 0;
			ReleaseCapture();
		}
	}
	void OnMouseMove( DWORD Flags, FPoint MouseLocation )
	{
		ToggleDraw( NULL );
		for( INT i=0; i<FPoint::Num(); i++ )
			if( DragIndices(i)!=INDEX_NONE )
				DragPos(i) = Clamp( DragPos(i) + MouseLocation(i) - OldMouseLocation(i), DragClamp.Min(i), DragClamp.Max(i) );
		ToggleDraw( NULL );
		OldMouseLocation = MouseLocation;
	}
	void OnReleaseCapture()
	{
		ClipCursor( NULL );
		ToggleDraw( NULL );
		OwnerWindow->OnFinishSplitterDrag( this, Success );
		DestroyWindow( *this );
	}
	void OnLeftButtonUp()
	{
		ReleaseCapture();
	}
};

/*-----------------------------------------------------------------------------
	FTreeItem.
-----------------------------------------------------------------------------*/

// QSort comparator.
static INT Compare( const class FTreeItem* T1, const class FTreeItem* T2 );

// Base class of list items.
class WINDOW_API FTreeItem : public FTreeItemBase
{
public:
	// Variables.
	class WPropertiesBase*	OwnerProperties;
	FTreeItem*				Parent;
	UBOOL					Expandable;
	UBOOL					Expanded;
	UBOOL					Sorted;
	UBOOL					Selected;
	INT						ButtonWidth;
	TArray<WCoolButton*>	Buttons;
	TArray<FTreeItem*>		Children;

	// Structors.
	FTreeItem()
	{}
	FTreeItem( WPropertiesBase* InOwnerProperties, FTreeItem* InParent, UBOOL InExpandable )
	:	OwnerProperties	( InOwnerProperties )
	,	Parent			( InParent )
	,	Expandable		( InExpandable )
	,	Expanded		( 0 )
	,	Sorted			( 1 )
	,	ButtonWidth		( 0 )
	,	Selected		( 0 )
	,	Buttons			()
	,	Children		()
	{}
	virtual ~FTreeItem()
	{
		EmptyChildren();
	}

	// FTreeItem interface.
	virtual HBRUSH GetBackgroundBrush( UBOOL Selected )
	{
		return Selected ? hBrushCurrent : hBrushOffWhite;
	}
	virtual COLORREF GetTextColor( UBOOL Selected )
	{
		return Selected ? RGB(255,255,255) : RGB(0,0,0);
	}
	virtual void Serialize( FArchive& Ar )
	{
		//!!Super::Serialize( Ar );
		for( INT i=0; i<Children.Num(); i++ )
			Children(i)->Serialize( Ar );
	}
	virtual INT OnSetCursor()
	{
		return 0;
	}
	void EmptyChildren()
	{
		for( INT i=0; i<Children.Num(); i++ )
			delete Children(i);
		Children.Empty();
	}
	virtual FRect GetRect()
	{
		return OwnerProperties->List.GetItemRect(OwnerProperties->List.FindItemChecked(this));
	}
	virtual void Redraw()
	{
		InvalidateRect( OwnerProperties->List, GetRect(), 0 );
		UpdateWindow( OwnerProperties->List );
		if( Parent!=OwnerProperties->GetRoot() )
			Parent->Redraw();
	}
	virtual void OnItemSetFocus()
	{
		if( Parent && Parent!=OwnerProperties->GetRoot() )
			Parent->OnItemSetFocus();
	}
	virtual void OnItemKillFocus( UBOOL Abort )
	{
		for( INT i=0; i<Buttons.Num(); i++ )
			delete Buttons(i);
		Buttons.Empty();
		ButtonWidth = 0;
		Redraw();
		if( Parent && Parent!=OwnerProperties->GetRoot() )
			Parent->OnItemKillFocus( Abort );
	}
	virtual void AddButton( const TCHAR* Text, FDelegate Action, UBOOL AtLeft = 0 )
	{
		FRect Rect=GetRect();
		Rect.Max.X -= ButtonWidth;
		SIZE Size;
		HDC hDC = GetDC(*OwnerProperties);
		GetTextExtentPoint32X( hDC, Text, appStrlen(Text), &Size ); 
		ReleaseDC(*OwnerProperties,hDC);
		INT Width = Size.cx + 2;
		if (!AtLeft)
			Width += 2;
		WCoolButton* Button = new WCoolButton(&OwnerProperties->List,0,Action);
		Buttons.AddItem( Button );
		if (!AtLeft)
		{
			Button->OpenWindow( 1, Rect.Max.X-Width, Rect.Min.Y, Width, Rect.Max.Y-Rect.Min.Y, Text );
			ButtonWidth += Width;
		}
		else
		{
			Button->OpenWindow( 1, 0, Rect.Min.Y, Width, Rect.Max.Y-Rect.Min.Y, Text );
		}
	}
	virtual void OnItemLeftMouseDown( FPoint P )
	{
		if( Expandable && GetExpanderRect().Inner(FPoint(-1,-1)).Contains(P) )
			ToggleExpansion();
	}
	virtual void OnItemRightMouseDown( FPoint P )
	{
	}
	INT GetIndent()
	{
		INT Result=0;
		for( FTreeItem* Test=Parent; Test!=OwnerProperties->GetRoot(); Test=Test->Parent )
			Result++;
		return Result;
	}
	INT GetUnitIndentPixels()
	{
		return OwnerProperties->ShowTreeLines ? 12 : 8;
	}
	virtual INT GetIndentPixels( UBOOL Text )
	{
		return GetUnitIndentPixels()*GetIndent() + (Text?OwnerProperties->ShowTreeLines?20:16:0);
	}
	virtual FRect GetExpanderRect()
	{
		return FRect( GetIndentPixels(0) + 4, 4, GetIndentPixels(0) + 13, 13 );
	}
	virtual UBOOL GetSelected()
	{
		return Selected;
	}
	void SetSelected( UBOOL InSelected )
	{
		Selected = InSelected;
	}
	virtual void DrawTreeLines( HDC hDC, FRect Rect )
	{
		SetBkMode( hDC, TRANSPARENT );
		SetTextColor( hDC, RGB(0,0,0) );
		if( OwnerProperties->ShowTreeLines )
		{
			FTreeItem* Prev = this;
			for( INT i=GetIndent(); i>=0; i-- )
			{
				check(Prev->Parent);
				UBOOL FromAbove = (Prev->Parent->Children.Last()!=Prev || Prev->Parent->Children.Last()==this) && (i!=0 || Prev->Parent->Children(0)!=this);
				UBOOL ToBelow   = (Prev->Parent->Children.Last()!=Prev);
				FPoint P( Rect.Min.X + GetUnitIndentPixels()*i, Rect.Min.Y );
				if( FromAbove || ToBelow )
					FillRect( hDC, FRect(P+FPoint(8,FromAbove?0:8),P+FPoint(9,ToBelow?16:8)), hBrushStipple );
				if( i==GetIndent() )
					FillRect( hDC, FRect(P+FPoint(8,8),P+FPoint(20,9)), hBrushStipple );
				Prev = Prev->Parent;
			}
			check(Prev->Parent==NULL);
		}
		if( Expandable )
		{
			FRect R = GetExpanderRect() + GetRect().Min;
			FillRect( hDC, R, hBrushGrey );
			FillRect( hDC, R.Inner(FPoint(1,1)), GetBackgroundBrush(0) );
			FillRect( hDC, R.Inner(FPoint(2,4)), hBrushBlack );
			if( !Expanded )
				FillRect( hDC, R.Inner(FPoint(4,2)), hBrushBlack );
		}
	}
	virtual void Collapse()
	{
		OwnerProperties->SetItemFocus( 0 );
		INT Index = OwnerProperties->List.FindItemChecked( this );
		INT Count = OwnerProperties->List.GetCount();
		while( Index+1<Count )
		{
			FTreeItem* NextItem = OwnerProperties->GetListItem(Index+1);
			for( FTreeItem* Check=NextItem->Parent; Check && Check!=this; Check=Check->Parent );
			if( !Check )
				break;
			NextItem->Expanded = 0;
			OwnerProperties->List.DeleteString( Index+1 );
			Count--;
		}
		Expanded=0;
		OwnerProperties->ResizeList();
	}
	virtual void Expand()
	{
		if( Sorted )
		{
			Sort( &Children(0), Children.Num() );
		}
		if( this==OwnerProperties->GetRoot() )
		{
			for( INT i=0; i<Children.Num(); i++ )
				OwnerProperties->List.AddItem( Children(i) );
		}
		else
		{
			for( INT i=Children.Num()-1; i>=0; i-- )
				OwnerProperties->List.InsertItemAfter( this, Children(i) );
		}
		OwnerProperties->SetItemFocus( 0 );
		OwnerProperties->ResizeList();
		Expanded=1;
	}
	virtual void ToggleExpansion()
	{
		if( Expandable )
		{
			OwnerProperties->List.SetRedraw( 0 );
			if( Expanded )
				Collapse();
			else
				Expand();
			OwnerProperties->List.SetRedraw( 1 );
			UpdateWindow( OwnerProperties->List );
		}
		OwnerProperties->SetItemFocus( 1 );
	}
	virtual void OnItemDoubleClick()
	{
		ToggleExpansion();
	}
	virtual BYTE* GetReadAddress( UProperty* Property, INT Offset )
	{
		return Parent ? Parent->GetReadAddress(Property,Offset) : NULL;
	}
	virtual void SetProperty( UProperty* Property, INT Offset, const TCHAR* Value )
	{
		if( Parent )
			Parent->SetProperty( Property, Offset, Value );
	}
	virtual void GetStates( TArray<FName>& States, FName StateCategory, UClass* StateClass )
	{
		if( Parent ) Parent->GetStates( States, StateCategory, StateClass );
	}
	virtual UBOOL AcceptFlags( DWORD InFlags )
	{
		return Parent ? Parent->AcceptFlags( InFlags ) : 0;
	}
	virtual QWORD GetId() const=0;
	virtual FString GetCaption() const=0;
	virtual void OnItemHelp() {}
	virtual void SetFocusToItem() {}
	virtual void SetValue( const TCHAR* Value ) {}

	// FControlSnoop interface.
	void SnoopChar( WWindow* Src, INT Char )
	{
		if( Char==13 && Expandable )
			ToggleExpansion();
	}
	void SnoopKeyDown( WWindow* Src, INT Char )
	{
		if( Char==VK_UP || Char==VK_DOWN )
			PostMessageX( OwnerProperties->List, WM_KEYDOWN, Char, 0 );
		if( Char==9 )
			PostMessageX( OwnerProperties->List, WM_KEYDOWN, (GetKeyState(16)&0x8000)?VK_UP:VK_DOWN, 0 );
	}
};

// QSort comparator.
static INT Compare( const class FTreeItem* T1, const class FTreeItem* T2 )
{
	return appStricmp( *T1->GetCaption(), *T2->GetCaption() );
}

// Property list item.
class WINDOW_API FPropertyItem : public FTreeItem
{
public:
	// Variables.
	UProperty*      Property;
	INT				Offset;
	INT				ArrayIndex;
	WEdit*			EditControl;
	WTrackBar*		TrackControl;
	WComboBox*		ComboControl;
	WLabel*			HolderControl;
	UBOOL			ComboChanged;
	FName			Name;

	// Constructors.
	FPropertyItem( WPropertiesBase* InOwnerProperties, FTreeItem* InParent, UProperty* InProperty, FName InName, INT InOffset, INT InArrayIndex )
	:	FTreeItem	    ( InOwnerProperties, InParent, 0 )
	,	Property		( InProperty )
	,	Name			( InName )
	,	Offset			( InOffset )
	,	ArrayIndex		( InArrayIndex )
	,	EditControl		( NULL )
	,	TrackControl	( NULL )
	,	ComboControl	( NULL )
	,	HolderControl	( NULL )
	,	ComboChanged	( 0 )
	{
		if
		(	(Cast<UStructProperty>(InProperty) && Cast<UStructProperty>(InProperty)->Struct->GetFName()!=NAME_Color)
		||	(Cast<UArrayProperty>(InProperty) && GetReadAddress(Property,Offset))
		||	(InProperty->ArrayDim>1 && InArrayIndex==-1) )
			Expandable = 1;
	}

	// FTreeItem interface.
	void Serialize( FArchive& Ar )
	{
		FTreeItem::Serialize( Ar );
		Ar << Property << Name;
	}
	QWORD GetId() const
	{
		return Name.GetIndex() + ((QWORD)1<<32);
	}
	FString GetCaption() const
	{
		return *Name;
	}
	virtual INT OnSetCursor()
	{
		check(OwnerProperties);	// WDM
		FPoint P = OwnerProperties->GetCursorPos() - GetRect().Min;
		if( Abs(P.X-OwnerProperties->GetDividerWidth())<=2 )
		{
			SetCursor(LoadCursorIdX(hInstanceWindow,IDC_SplitWE));
			return 1;
		}
		return 0;
	}
	virtual void OnItemLeftMouseDown( FPoint P )
	{
		P = OwnerProperties->GetCursorPos() - GetRect().Min;
		if( Abs(P.X-OwnerProperties->GetDividerWidth())<=2 )
		{
			OwnerProperties->BeginSplitterDrag();
			throw TEXT("NoRoute");
		}
		else FTreeItem::OnItemLeftMouseDown( P );
	}
	void GetPropertyText( TCHAR* Str, BYTE* ReadValue )
	{
		if( Cast<UClassProperty>(Property) && appStricmp(*Property->Category,TEXT("Drivers"))==0 )
		{
			// Class config.
			FString Path, Left, Right;
			GConfig->GetString( Property->GetOwnerClass()->GetPathName(), Property->GetName(), Path );
			if( Path.Split(TEXT("."),&Left,&Right) )
				appStrcpy( Str, Localize(*Right,TEXT("ClassCaption"),*Left) );
			else
				appStrcpy( Str, Localize("Language","Language",TEXT("Core"),*Path) );
		}
		else
		{
			// Regular property.
			Property->ExportText( 0, Str, ReadValue-Property->Offset, ReadValue-Property->Offset, PPF_Localized );
		}
	}
	void SetValue( const TCHAR* Value )
	{
		SetProperty( Property, Offset, Value );
		ReceiveFromControl();
		Redraw();
	}
	void Draw( HDC hDC )
	{
		FRect Rect = GetRect();
		TCHAR Str[32768];
		// CDH: Between 220 and 400 this was changed from 256 to 4096, so obviously epic found this bug too; 
		// I'm just upping the size a bit

		// Draw background.
		FillRect( hDC, Rect, hBrushWhite ); 

		// Draw left background.
		FRect LeftRect=Rect;
		LeftRect.Max.X = OwnerProperties->GetDividerWidth();
		FillRect( hDC, LeftRect+FRect(0,1-GetSelected(),-1,0), GetBackgroundBrush(GetSelected()) );
		LeftRect.Min.X += GetIndentPixels(1);

		// Draw tree lines.
		DrawTreeLines( hDC, Rect );

		// Setup text.
		SetBkMode( hDC, TRANSPARENT );

		// Draw left text.
		if( ArrayIndex==-1 ) appStrcpy( Str, *Name );
		else appSprintf( Str, TEXT("[%i]"), ArrayIndex );
		SetTextColor( hDC, GetTextColor(GetSelected()) );
		// CDH... put statics in red to distinguish them
		if (Property->PropertyFlags & CPF_Static)
			SetTextColor(hDC, RGB(255,0,0));
		// ...CDH
		DrawTextExX( hDC, Str, appStrlen(Str), LeftRect + FRect(0,1,0,0), DT_END_ELLIPSIS | DT_LEFT | DT_SINGLELINE | DT_VCENTER, NULL );

		// Draw right background.
		FRect RightRect = Rect;
		RightRect.Min.X = OwnerProperties->GetDividerWidth();
		FillRect( hDC, RightRect+FRect(0,1,0,0), GetBackgroundBrush(0) );

		// Draw right text.
		RightRect.Max.X -= ButtonWidth;		
		BYTE* ReadValue = GetReadAddress( Property, Offset );
		SetTextColor( hDC, GetTextColor(0) );
		if( (Property->ArrayDim!=1 && ArrayIndex==-1) || Cast<UArrayProperty>(Property) )
		{
			// Array expander.
			TCHAR* Str=TEXT("...");
			DrawTextExX( hDC, Str, appStrlen(Str), RightRect + FRect(4,0,0,1), DT_END_ELLIPSIS | DT_LEFT | DT_SINGLELINE | DT_VCENTER, NULL );
		}
		else if( ReadValue && Cast<UStructProperty>(Property) && Cast<UStructProperty>(Property)->Struct->GetFName()==NAME_Color )
		{
			// Color.
			FillRect( hDC, RightRect + FRect(4,4,-4,-4), hBrushBlack ); 
			HBRUSH hBrush = CreateSolidBrush(COLORREF(*(DWORD*)ReadValue));
			FillRect( hDC, RightRect + FRect(5,5,-5,-5), hBrush ); 
			DeleteObject( hBrush );
		}
		else if( ReadValue )
		{
			// Text.
			*Str=0;
			GetPropertyText( Str, ReadValue );
			DrawTextExX( hDC, Str, appStrlen(Str), RightRect + FRect(4,1,0,0), DT_END_ELLIPSIS | DT_LEFT | DT_SINGLELINE | DT_VCENTER, NULL );
		}

	}
	INT GetHeight()
	{
		return 16;
	}
	void SetFocusToItem()
	{
		if( EditControl )
			SetFocus( *EditControl );
		else if( TrackControl )
			SetFocus( *TrackControl );
		else if( ComboControl )
			SetFocus( *ComboControl );
	}
	void OnItemDoubleClick()
	{
		Advance();
		FTreeItem::OnItemDoubleClick();
	}
	void OnItemSetFocus()
	{
		check(!EditControl);
		check(!TrackControl);
		FTreeItem::OnItemSetFocus();
		if
		(	(Property->ArrayDim==1 || ArrayIndex!=-1)
		&&	!(Property->PropertyFlags & CPF_EditConst) )
		{
			// CDH... display comment text if any
			/*
			if (Property->PropertyFlags & CPF_Comment)
			{
				GWarn->BeginSlowTask(TEXT(""), 1, 0);
				GWarn->StatusUpdatef(0, 0, TEXT("%s: %s"), Property->GetName(), *Property->CommentString);
				GWarn->EndSlowTask();
			}
			*/
			AddButton( TEXT(" ? "), FDelegate(this,(TDelegate)OnHelpButton), true );
			// ...CDH

			if( Property->IsA(UByteProperty::StaticClass()) && !Cast<UByteProperty>(Property)->Enum )
			{
				// Slider.
				FRect Rect = GetRect();
				Rect.Min.X = 28+OwnerProperties->GetDividerWidth();
				Rect.Min.Y--;
				TrackControl = new WTrackBar( &OwnerProperties->List );
				TrackControl->ThumbTrackDelegate    = FDelegate(this,(TDelegate)OnTrackBarThumbTrack);
				TrackControl->ThumbPositionDelegate = FDelegate(this,(TDelegate)OnTrackBarThumbPosition);
				TrackControl->OpenWindow( 0 );
				TrackControl->SetTicFreq( 32 );
				TrackControl->SetRange( 0, 255 );
				TrackControl->MoveWindow( Rect, 0 );
			}
			if
			(	(Property->IsA(UBoolProperty::StaticClass()))
			||	(Property->IsA(UByteProperty::StaticClass()) && Cast<UByteProperty>(Property)->Enum)
			||	(Property->IsA(UNameProperty::StaticClass()) && (Property->PropertyFlags & CPF_StateName))
			||  (Cast<UClassProperty>(Property) && appStricmp(*Property->Category,TEXT("Drivers"))==0) )
			{
				// Combo box.
				FRect Rect = GetRect() + FRect(0,0,-1,-1);
				Rect.Min.X = OwnerProperties->GetDividerWidth();

				HolderControl = new WLabel( &OwnerProperties->List );
				HolderControl->Snoop = this;
				HolderControl->OpenWindow( 0 );
				FRect HolderRect = Rect.Right(20) + FRect(0,0,0,1);
				HolderControl->MoveWindow( HolderRect, 0 );

				Rect = Rect + FRect(-2,-6,-2,0);

				ComboControl = new WComboBox( HolderControl );
				ComboControl->Snoop = this;
				ComboControl->SelectionEndOkDelegate     = FDelegate(this,(TDelegate)ComboSelectionEndOk);
				ComboControl->SelectionEndCancelDelegate = FDelegate(this,(TDelegate)ComboSelectionEndCancel);
				ComboControl->OpenWindow( 0 );
				ComboControl->MoveWindow( Rect-HolderRect.Min, 1 );

				if( Property->IsA(UBoolProperty::StaticClass()) )
				{
					ComboControl->AddString( GFalse );
					ComboControl->AddString( GTrue );
				}
				else if( Property->IsA(UByteProperty::StaticClass()) )
				{
					for( INT i=0; i<Cast<UByteProperty>(Property)->Enum->Names.Num(); i++ )
						ComboControl->AddString( *Cast<UByteProperty>(Property)->Enum->Names(i) );
				}
				else if( Property->IsA(UNameProperty::StaticClass()) && (Property->PropertyFlags & CPF_StateName) )
				{
					TArray<FName> States;
					GetStates( States, Cast<UNameProperty>(Property)->NameOptionNameA, (UClass*)(Cast<UNameProperty>(Property)->NameOptionObjectA) );
					ComboControl->AddString( *FName(NAME_None) );
					for( INT i=0; i<States.Num(); i++ )
						ComboControl->AddString( *States(i) );
				}
				else if( Cast<UClassProperty>(Property) && appStricmp(*Property->Category,TEXT("Drivers"))==0 )
				{
					UClassProperty* ClassProp = CastChecked<UClassProperty>(Property);
					TArray<FRegistryObjectInfo> Classes;
					UObject::GetRegistryObjects( Classes, UClass::StaticClass(), ClassProp->MetaClass, 0 );
					for( INT i=0; i<Classes.Num(); i++ )
					{
						FString Path=Classes(i).Object, Left, Right;
						if( Path.Split(TEXT("."),&Left,&Right) )
							ComboControl->AddString( Localize(*Right,TEXT("ClassCaption"),*Left) );
						else
							ComboControl->AddString( Localize("Language","Language",TEXT("Core"),*Path) );
					}
					goto SkipTheRest;
				}
			}
			if( Property->IsA(UArrayProperty::StaticClass()) )
			{
				if( Expandable )
				{
					AddButton( LocalizeGeneral("AddButton",TEXT("Window")), FDelegate(this,(TDelegate)OnArrayAdd) );
				}
				AddButton( LocalizeGeneral("EmptyButton",TEXT("Window")), FDelegate(this,(TDelegate)OnArrayEmpty) );
			}
			if( Cast<UArrayProperty>(Property->GetOuter()) )
			{
				if( Parent->Expandable )
				{
					AddButton( LocalizeGeneral("InsertButton",TEXT("Window")), FDelegate(this,(TDelegate)OnArrayInsert) );
					AddButton( LocalizeGeneral("DeleteButton",TEXT("Window")), FDelegate(this,(TDelegate)OnArrayDelete) );
				}
			}
			if( Property->IsA(UStructProperty::StaticClass()) && Cast<UStructProperty>(Property)->Struct->GetFName()==NAME_Color )
			{
				// Color.
				AddButton( LocalizeGeneral("BrowseButton",TEXT("Window")), FDelegate(this,(TDelegate)OnChooseColorButton) );

				// CDH: Edit control for alpha
				FRect Rect = GetRect();
				if (Rect.Max.X > OwnerProperties->GetDividerWidth() + ButtonWidth + 28)
				{
					Rect.Max.X -= ButtonWidth;
					Rect.Min.X = Rect.Max.X - 28;
					Rect.Min.Y--;
					EditControl = new WEdit( &OwnerProperties->List );
					EditControl->Snoop = this;
					EditControl->OpenWindow( 0, 0, 0 );
					EditControl->MoveWindow( Rect+FRect(0,1,0,1), 0 );
				}
			}
			else if( Property->IsA(UObjectProperty::StaticClass()) )
			{
				// Class.
				AddButton( LocalizeGeneral("BrowseButton",TEXT("Window")), FDelegate(this,(TDelegate)OnBrowseButton) );
				AddButton( LocalizeGeneral("UseButton",   TEXT("Window")), FDelegate(this,(TDelegate)OnUseCurrentButton) );
				AddButton( LocalizeGeneral("ClearButton", TEXT("Window")), FDelegate(this,(TDelegate)OnClearButton) );
			}
			if
			(	(Property->IsA(UFloatProperty ::StaticClass()))
			||	(Property->IsA(UIntProperty   ::StaticClass()))
			||	(Property->IsA(UNameProperty  ::StaticClass()) && (!(Property->PropertyFlags & CPF_StateName)))
			||	(Property->IsA(UStrProperty   ::StaticClass()))
			||	(Property->IsA(UObjectProperty::StaticClass()))
			||	(Property->IsA(UByteProperty  ::StaticClass()) && Cast<UByteProperty>(Property)->Enum==NULL) )
			{
				// Edit control.
				FRect Rect = GetRect();
				Rect.Min.X = 1+OwnerProperties->GetDividerWidth();
				Rect.Min.Y--;
				if( Property->IsA(UByteProperty::StaticClass()) )
					Rect.Max.X = Rect.Min.X + 28;
				else
					Rect.Max.X -= ButtonWidth;
				EditControl = new WEdit( &OwnerProperties->List );
				EditControl->Snoop = this;
				EditControl->OpenWindow( 0, 0, 0 );
				EditControl->MoveWindow( Rect+FRect(0,1,0,1), 0 );
			}
			SkipTheRest:
			ReceiveFromControl();
			Redraw();
			if( EditControl )
				EditControl->Show(1);
			if( TrackControl )
				TrackControl->Show(1);
			if( ComboControl )
				ComboControl->Show(1);
			if( HolderControl )
				HolderControl->Show(1);
			SetFocusToItem();
		}
	}
	void OnItemKillFocus( UBOOL Abort )
	{
		if( !Abort )
			SendToControl();
		if( EditControl )
			delete EditControl;
		EditControl=NULL;
		if( TrackControl )
			delete TrackControl;
		TrackControl=NULL;
		if( ComboControl )
			delete ComboControl;
		ComboControl=NULL;
		if( HolderControl )
			delete HolderControl;
		HolderControl=NULL;
		FTreeItem::OnItemKillFocus( Abort );
	}
	void Expand()
	{
		UStructProperty* StructProperty;
		UArrayProperty* ArrayProperty;
		if( Property->ArrayDim>1 && ArrayIndex==-1 )
		{
			// Expand array.
			Sorted=0;
			for( INT i=0; i<Property->ArrayDim; i++ )
                Children.AddItem( new(TEXT("FPropertyItem"))FPropertyItem( OwnerProperties, this, Property, Name, Offset + i*Property->ElementSize, i ) );
		}
		else if( (ArrayProperty=Cast<UArrayProperty>(Property))!=NULL )
		{
			// Expand array.
			Sorted=0;
			FArray* Array = (FArray*)GetReadAddress( ArrayProperty, Offset );
			if( Array )
				for( INT i=0; i<Array->Num(); i++ )
					Children.AddItem( new(TEXT("FPropertyItem"))FPropertyItem( OwnerProperties, this, ArrayProperty->Inner, Name, i*ArrayProperty->Inner->ElementSize, i ) );
        }
		else if( (StructProperty=Cast<UStructProperty>(Property))!=NULL )
		{
			// Expand struct.
			for( TFieldIterator<UProperty> It(StructProperty->Struct); It; ++It )
				if( AcceptFlags( It->PropertyFlags ) )
					Children.AddItem( new(TEXT("FPropertyItem"))FPropertyItem( OwnerProperties, this, *It, It->GetFName(), Offset + It->Offset, -1 ) );
		}
		FTreeItem::Expand();
	}
	void Collapse()
	{
		FTreeItem::Collapse();
		EmptyChildren();
	}

	// FControlSnoop interface.
	void SnoopChar( WWindow* Src, INT Char )
	{
		if( Char==13 )
			Advance();
		else if( Char==27 )
			ReceiveFromControl();
		FTreeItem::SnoopChar( Src, Char );
	}
	void ComboSelectionEndCancel()
	{
		ReceiveFromControl();
	}
	void ComboSelectionEndOk()
	{
		ComboChanged=1;
		SendToControl();
		ReceiveFromControl();
		Redraw();
	}
	void OnTrackBarThumbTrack()
	{
		if( TrackControl && EditControl )
		{
			TCHAR Tmp[256];
			appSprintf( Tmp, TEXT("%i"), TrackControl->GetPos() );
			EditControl->SetText( Tmp );
			EditControl->SetModify( 1 );
			UpdateWindow( *EditControl );
		}
	}
	void OnTrackBarThumbPosition()
	{
		OnTrackBarThumbTrack();
		SendToControl();
		if( EditControl )
		{
			SetFocus( *EditControl );
			EditControl->SetSelection( 0, EditControl->GetText().Len() );
			Redraw();
		}
	}
	void OnChooseColorButton()
	{
		BYTE* ReadValue = GetReadAddress( Property, Offset );
		CHOOSECOLORA cc;
		static COLORREF acrCustClr[16];
		appMemzero( &cc, sizeof(cc) );
		cc.lStructSize  = sizeof(cc);
		cc.hwndOwner    = OwnerProperties->List;
		cc.lpCustColors = (LPDWORD)acrCustClr;
		cc.rgbResult    = ReadValue ? *(DWORD*)ReadValue : 0;
		cc.Flags        = CC_FULLOPEN | CC_RGBINIT;
		SetCapture(OwnerProperties->List);
		if( ChooseColorA(&cc)==TRUE )
		{
			TCHAR Str[256];
			appSprintf( Str, TEXT("(R=%i,G=%i,B=%i)"), GetRValue(cc.rgbResult), GetGValue(cc.rgbResult), GetBValue(cc.rgbResult) );
			SetValue( Str );
			InvalidateRect( OwnerProperties->List, NULL, 0 );
			UpdateWindow( OwnerProperties->List );
		}
		ReleaseCapture();
		Redraw();

	}
	void OnArrayAdd()
	{
		//!!only works with single selection
		BYTE* Addr = GetReadAddress( Property, Offset );
		if( Addr )
		{
			UArrayProperty* Array = CastChecked<UArrayProperty>( Property );
			Collapse();
			((FArray*)Addr)->AddZeroed( Array->Inner->ElementSize, 1 );
			Expand();
		}
	}
	void OnArrayEmpty()
	{
		//!!only works with single selection
		BYTE* Addr = GetReadAddress( Property, Offset );
		if( Addr )
		{
			UArrayProperty* Array = CastChecked<UArrayProperty>( Property );
			Collapse();
			((FArray*)Addr)->Empty( Array->Inner->ElementSize );
			Expand();
		}
	}
	void OnArrayInsert()
	{
		//!!only works with single selection
		UArrayProperty* Array = CastChecked<UArrayProperty>( Property->GetOuter() );
		BYTE* Addr = GetReadAddress( Array, Array->Offset );
		if( Addr )
		{
			INT Index = Offset/Array->ElementSize;
			((FArray*)Addr)->Insert( Index, 1, Array->ElementSize );
			appMemzero( (BYTE*)((FArray*)Addr)->GetData() + Index*Array->ElementSize, Array->ElementSize );
			Parent->Collapse();
			Parent->Expand();
		}
	}
	void OnArrayDelete()
	{
		//!!only works with single selection
		UArrayProperty* Array = CastChecked<UArrayProperty>( Property->GetOuter() );
		BYTE* Addr = GetReadAddress( Array, Array->Offset );
		if( Addr )
		{
			((FArray*)Addr)->Remove( Offset/Array->ElementSize, 1, Array->ElementSize );
			Parent->Collapse();
			Parent->Expand();
		}
	}
	void OnBrowseButton()
	{
		UObjectProperty* ReferenceProperty = CastChecked<UObjectProperty>(Property);
		TCHAR Temp[256];
		appSprintf( Temp, TEXT("BROWSECLASS CLASS=%s"), ReferenceProperty->PropertyClass->GetName() );
		if( OwnerProperties->NotifyHook )
			OwnerProperties->NotifyHook->NotifyExec( OwnerProperties, Temp );
		Redraw();
	}
	void OnUseCurrentButton()
	{
		UObjectProperty* ReferenceProperty = CastChecked<UObjectProperty>(Property);
		TCHAR Temp[256];
		appSprintf( Temp, TEXT("USECURRENT CLASS=%s"), ReferenceProperty->PropertyClass->GetName() );
		if( OwnerProperties->NotifyHook )
			OwnerProperties->NotifyHook->NotifyExec( OwnerProperties, Temp );
		Redraw();
	}
	void OnClearButton()
	{
		SetValue( TEXT("None") );
		Redraw();
	}
	// CDH...
	void OnHelpButton()
	{
		TCHAR BoxText[4096];

		TCHAR OwnerType[256], Temp[256];
		OwnerType[0] = 0;
		for (UObject* OwnerObj = Property->GetOuter(); OwnerObj && OwnerObj->IsA(UStruct::StaticClass()); OwnerObj = OwnerObj->GetOuter())
		{
			appStrcpy(Temp, OwnerType);
			appStrcpy(OwnerType, OwnerObj->GetName());
			if (Temp[0])
				appSprintf(OwnerType+appStrlen(OwnerType), TEXT(".%s"), Temp);
		}

		TCHAR PropType[256];
		appStrcpy(PropType, Property->GetClass()->GetName());
		if (Property->IsA(UStructProperty::StaticClass()))
			appSprintf(PropType+appStrlen(PropType), TEXT(" %s"), ((UStructProperty*)Property)->Struct->GetName());

		appSprintf(BoxText,
			TEXT("Name: %s (%s)\n")
			TEXT("Within: %s\n\n")
			TEXT("%s\n"),
			Property->GetName(),
			PropType,
			OwnerType,
			(Property->PropertyFlags & CPF_Comment) ? *Property->CommentString : TEXT("No description"));

		MessageBox(NULL, BoxText, TEXT("Property Information"), MB_OK | MB_TASKMODAL);

	}
	// ...CDH

	// FPropertyItem interface.
	virtual void Advance()
	{
		if( ComboControl && ComboControl->GetCurrent()>=0 )
		{
			ComboControl->SetCurrent( (ComboControl->GetCurrent()+1) % ComboControl->GetCount() );
			ComboChanged=1;
		}
		SendToControl();
		ReceiveFromControl();
		Redraw();
	}
	virtual void SendToControl()
	{
		if( EditControl )
		{
			if( EditControl->GetModify() )
			{
				if ( Property->IsA(UStructProperty::StaticClass()) && Cast<UStructProperty>(Property)->Struct->GetFName()==NAME_Color )
				{
					BYTE* ReadValue = GetReadAddress( Property, Offset );
					if (ReadValue)
						ReadValue[3] = appAtoi(*EditControl->GetText());
				}
				else
					SetValue( *EditControl->GetText() );
			}
		}
		else if( ComboControl )
		{
			if( ComboChanged )
				SetValue( *ComboControl->GetString(ComboControl->GetCurrent()) );
			ComboChanged=0;
		}
	}
	virtual void ReceiveFromControl()
	{
		ComboChanged=0;
		BYTE* ReadValue = GetReadAddress( Property, Offset );
		if( EditControl )
		{
			TCHAR Str[4096]=TEXT("");
			if( ReadValue )
			{
				if ( Property->IsA(UStructProperty::StaticClass()) && Cast<UStructProperty>(Property)->Struct->GetFName()==NAME_Color )
					appSprintf(Str, TEXT("%i"), ReadValue[3]);
				else
					GetPropertyText( Str, ReadValue );
			}
			EditControl->SetText( Str );
			EditControl->SetSelection( 0, appStrlen(Str) );
		}
		if( TrackControl )
		{
			if( ReadValue )
				TrackControl->SetPos( *(BYTE*)ReadValue );
		}
		if( ComboControl )
		{
			UBoolProperty* BoolProperty;
			if( (BoolProperty=Cast<UBoolProperty>(Property))!=NULL )
			{
				ComboControl->SetCurrent( ReadValue ? (*(BITFIELD*)ReadValue&BoolProperty->BitMask)!=0 : -1 );
			}
			else if( Property->IsA(UByteProperty::StaticClass()) )
			{
				ComboControl->SetCurrent( ReadValue ? *(BYTE*)ReadValue : -1 );
			}
			else if( Property->IsA(UNameProperty::StaticClass()) && (Property->PropertyFlags & CPF_StateName) )
			{
				INT Index=ReadValue ? ComboControl->FindString( **(FName*)ReadValue ) : 0;
				ComboControl->SetCurrent( Index>=0 ? Index : 0 );
			}
			ComboChanged=0;
		}
	}
};

// An abstract list header.
class WINDOW_API FHeaderItem : public FTreeItem
{
public:
	// Constructors.
	FHeaderItem()
	{}
	FHeaderItem( WPropertiesBase* InOwnerProperties, FTreeItem* InParent, UBOOL InExpandable )
	: FTreeItem( InOwnerProperties, InParent, InExpandable )
	{}

	// FTreeItem interface.
	void Draw( HDC hDC )
	{
		FRect Rect = GetRect();

		// Draw background.
		FillRect( hDC, Rect, hBrushWhite ); 
		FillRect( hDC, Rect+FRect(0,1-GetSelected(),0,0), GetBackgroundBrush(GetSelected()) );

		// Draw tree.
		DrawTreeLines( hDC, Rect );

		// Prep text.
		SetTextColor( hDC, GetTextColor(GetSelected()) );
		SetBkMode( hDC, TRANSPARENT );

		// Draw name.
		FString C = GetCaption();

		if( !appStricmp( UObject::GetLanguage(), TEXT("jpt") ) )
			TextOutW( hDC, Rect.Min.X, Rect.Min.Y,  const_cast<TCHAR*>(*C), C.Len() );
		else
			DrawTextExX( hDC, const_cast<TCHAR*>(*C), C.Len(), FRect(Rect) + FRect(GetIndentPixels(1),1,-1,0), DT_END_ELLIPSIS | DT_LEFT | DT_SINGLELINE | DT_VCENTER, NULL );
	}
	// Converts an RGB value to HSL format
	//
	// RGB : X = Red,	Y = Green,		Z = Blue
	// HSL : X = Hue,	Y = Saturation,	Z = Luminance
	//
	FVector ToHSL( FVector RGB )
	{
		FVector HSL(0,0,0);

		unsigned char minval = min(RGB.X, min(RGB.Y, RGB.Z));
		unsigned char maxval = max(RGB.X, max(RGB.Y, RGB.Z));
		float mdiff  = float(maxval) - float(minval);
		float msum   = float(maxval) + float(minval);

		HSL.Z = msum / 510.0f;

		if (maxval == minval) 
		{
			HSL.Y = 0.0f;
			HSL.X = 0.0f; 
		}   
		else 
		{ 
			float rnorm = (maxval - RGB.X  ) / mdiff;      
			float gnorm = (maxval - RGB.Y) / mdiff;
			float bnorm = (maxval - RGB.Z ) / mdiff;   

			HSL.Y = (HSL.Z <= 0.5f) ? (mdiff / msum) : (mdiff / (510.0f - msum));

			if (RGB.X   == maxval) HSL.X = 60.0f * (6.0f + bnorm - gnorm);
			if (RGB.Y == maxval) HSL.X = 60.0f * (2.0f + rnorm - bnorm);
			if (RGB.Z  == maxval) HSL.X = 60.0f * (4.0f + gnorm - rnorm);
			if (HSL.X > 360.0f) HSL.X = HSL.X - 360.0f;
		}

		HSL.X = 255.0f * (HSL.X / 360.0f);
		HSL.Y *= 255.0f;
		HSL.Z *= 255.0f;

		return HSL;
	}
	void OnChooseHSLColorButton()
	{

		// Get the current color in HSL format and convert it to RGB.
		FVector _RGB(0,0,0);

		// Open the color dialog.
		CHOOSECOLORA cc;
		static COLORREF acrCustClr[16];
		appMemzero( &cc, sizeof(cc) );
		cc.lStructSize  = sizeof(cc);
		cc.hwndOwner    = OwnerProperties->List;
		cc.lpCustColors = (LPDWORD)acrCustClr;
		cc.rgbResult    = RGB( _RGB.X, _RGB.Y, _RGB.Z );
		cc.Flags        = CC_FULLOPEN | CC_RGBINIT;
		SetCapture(OwnerProperties->List);
		if( ChooseColorA(&cc)==TRUE )
		{
			FVector HSL = ToHSL( FVector( GetRValue(cc.rgbResult), GetGValue(cc.rgbResult), GetBValue(cc.rgbResult) ));
			TCHAR Str[256];

			// Once the user chooses a color, loop through the child items of this header item, and set
			// the appropriate values.  We have to do this, since I don't see how to get access to the
			// actual actors themselves.  NOTE : You have to have this header item expanded before this will work.
			for( int x = 0 ; x < Children.Num() ; x++ )
			{
				FTreeItem* Item = Children(x);

				if( Item->GetCaption() == TEXT("LightHue") )
				{
					appSprintf( Str, TEXT("%d"), (int)HSL.X );
					Item->SetValue( Str );
				}
				if( Item->GetCaption() == TEXT("LightSaturation") )
				{
					appSprintf( Str, TEXT("%d"), (int)HSL.Z );
					Item->SetValue( Str );
				}
			}

			InvalidateRect( OwnerProperties->List, NULL, 0 );
			UpdateWindow( OwnerProperties->List );
		}
		ReleaseCapture();

		Redraw();
	}
	void OnItemSetFocus()
	{
		if( GetCaption() == TEXT("LightColor") )
			AddButton( TEXT("Color"), FDelegate(this,(TDelegate)OnChooseHSLColorButton) );
	}
	virtual void OnItemKillFocus( UBOOL Abort )
	{
		for( INT i=0; i<Buttons.Num(); i++ )
			delete Buttons(i);
		Buttons.Empty();
		ButtonWidth = 0;
		Redraw();
	}
	INT GetHeight()
	{
		return 16;
	}
};

// An category header list item.
class WINDOW_API FCategoryItem : public FHeaderItem
{
public:
	// Variables.
	FName Category;
	UClass* BaseClass;

	// Constructors.
	FCategoryItem( WPropertiesBase* InOwnerProperties, FTreeItem* InParent, UClass* InBaseClass, FName InCategory, UBOOL InExpandable )
	:	FHeaderItem( InOwnerProperties, InParent, InExpandable )
	,	Category    ( InCategory )
	,	BaseClass	( InBaseClass )
	{
		check(BaseClass);
	}

	// FTreeItem interface.
	void Serialize( FArchive& Ar )
	{
		FHeaderItem::Serialize( Ar );
		Ar << Category << BaseClass;
	}
	QWORD GetId() const
	{
		return Category.GetIndex() + ((QWORD)2<<32);
	}
	virtual FString GetCaption() const
	{
		return *Category;
	}
	void Expand()
	{
		for( TFieldIterator<UProperty> It(BaseClass); It; ++It )
			if( It->Category==Category && AcceptFlags(It->PropertyFlags) )
				Children.AddItem( new(TEXT("FPropertyItem"))FPropertyItem( OwnerProperties, this, *It, It->GetFName(), It->Offset, -1 ) );
		FTreeItem::Expand();
	}
	void Collapse()
	{
		FTreeItem::Collapse();
		EmptyChildren();
	}
};

/*-----------------------------------------------------------------------------
	WProperties.
-----------------------------------------------------------------------------*/

// General property editing control.
class WINDOW_API WProperties : public WPropertiesBase
{
	W_DECLARE_ABSTRACT_CLASS(WProperties,WPropertiesBase,CLASS_Transient);
	DECLARE_WINDOWCLASS(WProperties,WWindow,Window)

	// Variables.
	TArray<QWORD>		Remembered;
	QWORD				SavedTop, SavedCurrent;
	WDragInterceptor*	DragInterceptor;
	INT					DividerWidth;
	static TArray<WProperties*> PropertiesWindows;
	UBOOL bAllowForceRefresh;

	// Structors.
	WProperties()
	{}
	WProperties( FName InPersistentName, WWindow* InOwnerWindow=NULL )
	:	WPropertiesBase	( InPersistentName, InOwnerWindow )
	,	DragInterceptor	( NULL )
	,	DividerWidth	( 128 )
	{
		if( PersistentName!=NAME_None )
			GConfig->GetInt( TEXT("WindowPositions"), *(FString(*PersistentName)+TEXT(".Split")), DividerWidth );
		PropertiesWindows.AddItem( this );
		List.DoubleClickDelegate     = FDelegate(this,(TDelegate)OnListDoubleClick);
		List.SelectionChangeDelegate = FDelegate(this,(TDelegate)OnListSelectionChange);
		bAllowForceRefresh = 1;
	}

	// WWindow interface.
	void Serialize( FArchive& Ar )
	{
		WPropertiesBase::Serialize( Ar );
		GetRoot()->Serialize( Ar );
	}
	void DoDestroy()
	{
		PropertiesWindows.RemoveItem( this );
		WWindow::DoDestroy();
	}
	INT OnSetCursor()
	{
		if( ::IsWindow(List.hWnd) && List.GetCount() )
		{
			FPoint P = GetCursorPos();
			INT Index = List.ItemFromPoint( P );
			return Index>0 ? GetListItem( Index )->OnSetCursor() : 0;
		}
		return 0;
	}
	void OnDestroy()
	{
		WWindow::OnDestroy();
		_DeleteWindows.AddItem( this );
	}
	void OpenChildWindow( INT InControlId )
	{
		HWND hWndParent = InControlId ? GetDlgItem(OwnerWindow->hWnd,InControlId) : OwnerWindow->hWnd;
		check(hWndParent);
		FRect R;
		::GetClientRect( hWndParent, R );
		PerformCreateWindowEx
		(
			0, NULL, WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
			0, 0, R.Width(), R.Height(),
			hWndParent, NULL, hInstance
		);
		List.OpenWindow( 1, 0, 0, 1 );
		Show(1);
	}
	void OpenWindow( HWND hWndParent=NULL )
	{
		PerformCreateWindowEx
		(
			WS_EX_TOOLWINDOW,
			*GetRoot()->GetCaption(),
			WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_SIZEBOX | WS_MINIMIZEBOX,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			256+64+32,
			512,
			hWndParent ? hWndParent : OwnerWindow ? OwnerWindow->hWnd : NULL,
			NULL,
			hInstance
		);
		List.OpenWindow( 1, 1, 0, 1 );
	}
	void OnActivate( UBOOL Active )
	{
		if( Active==1 )
		{
			SetFocus( List );
			if( !FocusItem )
				SetItemFocus( 1 );
		}
		else
		{
			SetItemFocus( 0 );
		}
	}
	void OnSize( DWORD Flags, INT NewX, INT NewY )
	{
		WWindow::OnSize( Flags, NewX, NewY );
		if( List.hWnd )
		{
			SetItemFocus( 0 );
			InvalidateRect( List, NULL, FALSE );
			ResizeList();
			SetItemFocus( 1 );
		}
	}
	void OnPaint()
	{
		if( GetUpdateRect( *this, NULL, 0 ) )
		{
			PAINTSTRUCT PS;
			HDC   hDC      = BeginPaint( *this, &PS );
			FRect Rect     = GetClientRect();
			FRect ListRect = List.GetClientRect();
			Rect.Min.Y     = ListRect.Max.Y;
			FillRect( hDC, Rect, (HBRUSH)(COLOR_BTNFACE+1) ); 
			EndPaint( *this, &PS );
		}
	}

	// Delegates.
	void OnListDoubleClick()
	{
		if( FocusItem )
			FocusItem->OnItemDoubleClick();
	}
	void OnListSelectionChange()
	{
		SetItemFocus( 1 );
	}

	// FControlSnoop interface.
	void SnoopLeftMouseDown( WWindow* Src, FPoint P )
	{
		if( Src==&List )
		{
			INT Index = List.ItemFromPoint( P );
			if( Index>=0 )
			{
				List.SetCurrent( Index, 0 );
				FTreeItem* Item = GetListItem( Index );
				Item->OnItemLeftMouseDown( P-Item->GetRect().Min );
			}
		}
	}
	void SnoopRightMouseDown( WWindow* Src, FPoint P )
	{
		if( Src==&List )
		{
			INT Index = List.ItemFromPoint( P );
			if( Index>=0 )
			{
				List.SetCurrent( Index, 0 );
				FTreeItem* Item = GetListItem( Index );
				Item->OnItemRightMouseDown( P-Item->GetRect().Min );
			}
		}
	}
	void SnoopChar( WWindow* Src, INT Char )
	{
		if( FocusItem )
			FocusItem->SnoopChar( Src, Char );
	}
	void SnoopKeyDown( WWindow* Src, INT Char )
	{
		if( Char==9 )
			PostMessageX( List, WM_KEYDOWN, (GetKeyState(16)&0x8000)?VK_UP:VK_DOWN, 0 );
		WPropertiesBase::SnoopKeyDown( Src, Char );
	}

	// WPropertiesBase interface.
	INT GetDividerWidth()
	{
		return DividerWidth;
	}

	// WProperties interface.
	virtual void BeginSplitterDrag()
	{
		SetItemFocus( NULL );
		DragInterceptor            = new WDragInterceptor( this, FPoint(0,INDEX_NONE), GetClientRect(), FPoint(3,3) );	
		DragInterceptor->DragPos   = FPoint(GetDividerWidth(),GetCursorPos().Y);
		DragInterceptor->DragClamp = FRect(GetClientRect().Inner(FPoint(64,0)));
		DragInterceptor->OpenWindow();
	}
	void OnFinishSplitterDrag( WDragInterceptor* Drag, UBOOL Success )
	{
		if( Success )
		{
			DividerWidth += Drag->DragPos.X - Drag->DragStart.X;
			if( PersistentName!=NAME_None )
				GConfig->SetInt( TEXT("WindowPositions"), *(FString(*PersistentName)+TEXT(".Split")), DividerWidth );
			InvalidateRect( *this, NULL, 0 );
			UpdateWindow( *this );
		}
		DragInterceptor = NULL;
	}
	virtual void SetValue( const TCHAR* Value )
	{
		if( FocusItem )
			FocusItem->SetValue( Value );
	}
	virtual void SetItemFocus( UBOOL FocusCurrent )
	{
		if( FocusItem )
			FocusItem->OnItemKillFocus( 0 );
		FocusItem = NULL;
		if( FocusCurrent && List.GetCount()>0 )
			FocusItem = GetListItem( List.GetCurrent() );
		if( FocusItem )
			FocusItem->OnItemSetFocus();
	}
	virtual void ResizeList()
	{
		FRect ClientRect = GetClientRect();
		FRect R(0,0,0,4);//!!why?
		for( INT i=List.GetCount()-1; i>=0; i-- )
			R.Max.Y += List.GetItemHeight( i );
		AdjustWindowRect( R, GetWindowLongX(List,GWL_STYLE), 0 );
		List.MoveWindow( FRect(0,0,ClientRect.Width(),Min(ClientRect.Height(),R.Height())), 1 );
	}
	virtual void ForceRefresh()
	{

		if( !bAllowForceRefresh ) return;

		// Disable editing.
		SetItemFocus( 0 );

		// Remember which items were expanded.
		if( List.GetCount() )
		{
			Remembered.Empty();
			SavedTop     = GetListItem(List.GetTop())->GetId();
			SavedCurrent = List.GetSelected(List.GetCurrent()) ? GetListItem(List.GetCurrent())->GetId() : 0;
			for( INT i=0; i<List.GetCount(); i++ )
			{
				FTreeItem* Item = GetListItem(i);
				if( Item->Expanded )
					Remembered.AddItem( Item->GetId() );
			}
		}

		// Empty it and add root items.
		List.Empty();
		GetRoot()->EmptyChildren();
		GetRoot()->Expanded=0;
		GetRoot()->Expand();

		// Restore expansion state of the items.
		INT CurrentIndex=-1, TopIndex=-1;
		for( INT i=0; i<List.GetCount(); i++ )
		{
			FTreeItem* Item = GetListItem(i);
			QWORD      Id   = Item->GetId();
			if( Item->Expandable && !Item->Expanded )
			{
				for( INT j=0; j<Remembered.Num(); j++ )
					if( Remembered(j)==Id )
					{
						Item->Expand();
						break;
					}
			}
			if( Id==SavedTop     ) TopIndex     = i;
			if( Id==SavedCurrent ) CurrentIndex = i;
		}

		// Adjust list size.
		ResizeList();

		// Set indices.
		if( TopIndex>=0 ) List.SetTop( TopIndex );
		if( CurrentIndex>=0 ) List.SetCurrent( CurrentIndex, 1 );

	}
};

/*-----------------------------------------------------------------------------
	FPropertyItemBase.
-----------------------------------------------------------------------------*/

class WINDOW_API FPropertyItemBase : public FHeaderItem
{
public:
	// Variables.
	FString Caption;
	DWORD FlagMask;
	UClass* BaseClass;

	// Structors.
	FPropertyItemBase()
	{}
	FPropertyItemBase( WPropertiesBase* InOwnerProperties, FTreeItem* InParent, DWORD InFlagMask, const TCHAR* InCaption )
	:	FHeaderItem	( InOwnerProperties, InParent, 1 )
	,	Caption		( InCaption )
	,	FlagMask	( InFlagMask )
	,	BaseClass	( NULL )
	{}

	// FTreeItem interface.
	void Serialize( FArchive& Ar )
	{
		FHeaderItem::Serialize( Ar );
		Ar << BaseClass;
	}

	// FHeaderItem interface.
	UBOOL AcceptFlags( DWORD InFlags )
	{
		return (InFlags&FlagMask)==FlagMask;
	}
	void GetStates( TArray<FName>& States, FName StateCategory, UClass* StateClass )
	{
		if (!BaseClass)
			return;
		
		if (!StateClass)
			StateClass = BaseClass;

		for( TFieldIterator<UState> StateIt(StateClass); StateIt; ++StateIt )
		{
			if (!(StateIt->StateFlags & STATE_Editable))
				continue;
			if ((StateCategory == NAME_None)
			 || (StateCategory == StateIt->StructCategory))
			{
				States.AddUniqueItem( StateIt->GetFName() );
			}
		}
		/*
		if( BaseClass )
			for( TFieldIterator<UState> StateIt(BaseClass); StateIt; ++StateIt )
				if( StateIt->StateFlags & STATE_Editable )
					States.AddUniqueItem( StateIt->GetFName() );
		*/
	}
	void Collapse()
	{
		FTreeItem::Collapse();
		EmptyChildren();
	}
	FString GetCaption() const
	{
		return Caption;
	}
	QWORD GetId() const
	{
		return (QWORD)BaseClass + (QWORD)4;
	}
};

/*-----------------------------------------------------------------------------
	WObjectProperties.
-----------------------------------------------------------------------------*/

// Object properties root.
class WINDOW_API FObjectsItem : public FPropertyItemBase
{
public:
	// Variables.
	UBOOL ByCategory;
	TArray<UObject*> _Objects;

	// Structors.
	FObjectsItem()
	{}
	FObjectsItem( WPropertiesBase* InOwnerProperties, FTreeItem* InParent, DWORD InFlagMask, const TCHAR* InCaption, UBOOL InByCategory )
	:	FPropertyItemBase( InOwnerProperties, InParent, InFlagMask, InCaption )
	,	ByCategory( InByCategory )
	{}

	// FTreeItem interface.
	void Serialize( FArchive& Ar )
	{
		FPropertyItemBase::Serialize( Ar );
		Ar << _Objects;
	}
	BYTE* GetAddress( UProperty* Property, BYTE* Base, INT Offset )
	{
		UArrayProperty* Array = Cast<UArrayProperty>(Property->GetOuter());
		if( Array )
			Base = (BYTE*)((FArray*)GetAddress(Array,Base,Array->Offset))->GetData();
		return Base + Offset;
	}
	BYTE* GetReadAddress( UProperty* Property, INT Offset )
	{
		if( !_Objects.Num() )
			return NULL;
		UArrayProperty* Array = Cast<UArrayProperty>(Property->GetOuter());
		if( Array )
		{
			INT Num0 = ((FArray*)GetAddress(Array,(BYTE*)_Objects(0),Array->Offset))->Num();
			for( INT i=1; i<_Objects.Num(); i++ )
				if( Num0 != ((FArray*)GetAddress(Array,(BYTE*)_Objects(i),Array->Offset))->Num() )
					return NULL;
		}
		BYTE* Base0 = GetAddress( Property, (BYTE*)_Objects(0), Offset );
		for( INT i=1; i<_Objects.Num(); i++ )
			if( !Property->Identical( Base0, GetAddress(Property,(BYTE*)_Objects(i),Offset) ) )
				return NULL;
		return Base0;
	}
	void SetProperty( UProperty* Property, INT Offset, const TCHAR* Value )
	{
		if( OwnerProperties->NotifyHook )
			OwnerProperties->NotifyHook->NotifyPreChange( OwnerProperties );
		for( INT i=0; i<_Objects.Num(); i++ )
		{
			Property->ImportText( Value, GetAddress(Property,(BYTE*)_Objects(i),Offset), PPF_Localized );
			_Objects(i)->PostEditChange();
		}
		if( OwnerProperties->NotifyHook )
			OwnerProperties->NotifyHook->NotifyPostChange( OwnerProperties );
	}
	void Expand()
	{
		if( ByCategory )
		{
			// Expand to show categories.
			TArray<FName> Categories;
			for( TFieldIterator<UProperty> It(BaseClass); It; ++It )
				if( AcceptFlags( It->PropertyFlags ) )
					Categories.AddUniqueItem( It->Category );
			for( INT i=0; i<Categories.Num(); i++ )
				Children.AddItem( new(TEXT("FCategoryItem"))FCategoryItem(OwnerProperties,this,BaseClass,Categories(i),1) );
		}
		else
		{
			// Expand to show individual items.
			for( TFieldIterator<UProperty> It(BaseClass); It; ++It )
				if( AcceptFlags(It->PropertyFlags) && It->GetOwnerClass()!=UObject::StaticClass() )//hack for ufactory display!!
					Children.AddItem( new(TEXT("FPropertyItem"))FPropertyItem( OwnerProperties, this, *It, It->GetFName(), It->Offset, -1 ) );
		}
		FTreeItem::Expand();
	}

	// FHeaderItem interface.
	FString GetCaption() const
	{
		if( Caption.Len() )
			return Caption;			
		else if( !BaseClass )
			return LocalizeGeneral("PropNone",TEXT("Window"));
		else if( _Objects.Num()==1 )
			return FString::Printf( LocalizeGeneral("PropSingle",TEXT("Window")), BaseClass->GetName() );
		else
			return FString::Printf( LocalizeGeneral("PropMulti",TEXT("Window")), BaseClass->GetName(), _Objects.Num() );
	}

	// FObjectsItem interface.
	virtual void SetObjects( UObject** InObjects, INT Count )
	{
		// Disable editing, to prevent crash due to edit-in-progress after empty objects list.
		OwnerProperties->SetItemFocus( 0 );

		// Add objects and find lowest common base class.
		_Objects.Empty();
		BaseClass = NULL;
		for( INT i=0; i<Count; i++ )
		{
			if( InObjects[i] )
			{
				check(InObjects[i]->GetClass());
				if( BaseClass==NULL )	
					BaseClass=InObjects[i]->GetClass();
				while( !InObjects[i]->GetClass()->IsChildOf(BaseClass) )
					BaseClass = BaseClass->GetSuperClass();
				_Objects.AddItem( InObjects[i] );
			}
		}

		// Automatically title the window.
		OwnerProperties->SetText( *GetCaption() );

		// Refresh all properties.
		if( Expanded || this==OwnerProperties->GetRoot() )
			OwnerProperties->ForceRefresh();
	}
};

// Multiple selection object properties.
class WINDOW_API WObjectProperties : public WProperties
{
	W_DECLARE_CLASS(WObjectProperties,WProperties,CLASS_Transient);
	DECLARE_WINDOWCLASS(WObjectProperties,WProperties,Window)

	// Variables.
	FObjectsItem Root;

	// Structors.
	WObjectProperties()
	{}
	WObjectProperties( FName InPersistentName, DWORD InFlagMask, const TCHAR* InCaption, WWindow* InOwnerWindow, UBOOL InByCategory )
	:	WProperties	( InPersistentName, InOwnerWindow )
	,	Root		( this, NULL, InFlagMask, InCaption, InByCategory )
	{}

	// WPropertiesBase interface.
	FTreeItem* GetRoot()
	{
		return &Root;
	}
};

/*-----------------------------------------------------------------------------
	WClassProperties.
-----------------------------------------------------------------------------*/

// Object properties root.
class WINDOW_API FClassItem : public FPropertyItemBase
{
public:
	// Structors.
	FClassItem()
	{}
	FClassItem( WPropertiesBase* InOwnerProperties, FTreeItem* InParent, DWORD InFlagMask, const TCHAR* InCaption, UClass* InBaseClass )
	:	FPropertyItemBase( InOwnerProperties, InParent, InFlagMask, InCaption )
	{
		BaseClass = InBaseClass;
	}

	// FTreeItem interface.
	BYTE* GetReadAddress( UProperty* Property, INT Offset )
	{
		BYTE* Base = &BaseClass->Defaults[CPD_Normal](0);
		UArrayProperty* Array = Cast<UArrayProperty>(Property->GetOuter());
		if( Array )
			Base = (BYTE*)((FArray*)GetReadAddress(Array,Array->Offset))->GetData();
		return Base + Offset;
	}
	void SetProperty( UProperty* Property, INT Offset, const TCHAR* Value )
	{
		Property->ImportText( Value, GetReadAddress(Property,Offset), PPF_Localized );
		BaseClass->SetFlags( RF_SourceModified );
	}
	void Expand()
	{
		TArray<FName> Categories;
		for( TFieldIterator<UProperty> It(BaseClass); It; ++It )
			if( AcceptFlags( It->PropertyFlags ) )
				Categories.AddUniqueItem( It->Category );
		for( INT i=0; i<Categories.Num(); i++ )
			Children.AddItem( new(TEXT("FCategoryItem"))FCategoryItem(OwnerProperties,this,BaseClass,Categories(i),1) );
		FTreeItem::Expand();
	}
};

// Multiple selection object properties.
class WINDOW_API WClassProperties : public WProperties
{
	W_DECLARE_CLASS(WClassProperties,WProperties,CLASS_Transient);
	DECLARE_WINDOWCLASS(WClassProperties,WProperties,Window)

	// Variables.
	FClassItem Root;

	// Structors.
	WClassProperties()
	{}
	WClassProperties( FName InPersistentName, DWORD InFlagMask, const TCHAR* InCaption, UClass* InBaseClass )
	:	WProperties	( InPersistentName )
	,	Root		( this, NULL, InFlagMask, InCaption, InBaseClass )
	{}

	// WPropertiesBase interface.
	FTreeItem* GetRoot()
	{
		return &Root;
	}
};

/*-----------------------------------------------------------------------------
	WConfigProperties.
-----------------------------------------------------------------------------*/

// Object configuration header.
class WINDOW_API FObjectConfigItem : public FPropertyItemBase
{
public:
	// Variables.
	FString  ClassName;
	FName    CategoryFilter;
	UClass*  Class;
	UBOOL	 Failed;
	UBOOL    Immediate;

	// Structors.
	FObjectConfigItem( WPropertiesBase* InOwnerProperties, FTreeItem* InParent, const TCHAR* InCaption, const TCHAR* InClass, UBOOL InImmediate, FName InCategoryFilter )
	:	FPropertyItemBase( InOwnerProperties, InParent, CPF_Config, InCaption )
	,	ClassName		( InClass )
	,	Class			( NULL )
	,	Failed			( 0 )
	,	Immediate		( InImmediate )
	,	CategoryFilter	( InCategoryFilter )
	{}

	// FTreeItem interface.
	BYTE* GetReadAddress( UProperty* Property, INT Offset )
	{
        BYTE* Base = &Class->Defaults[CPD_Normal](0);
		UArrayProperty* Array = Cast<UArrayProperty>(Property->GetOuter());
		if( Array )
			Base = (BYTE*)((FArray*)GetReadAddress(Array,Array->Offset))->GetData();
		return Base + Offset;
	}
	void SetProperty( UProperty* Property, INT Offset, const TCHAR* Value )
	{
		check(Class);
		if( OwnerProperties->NotifyHook )
			OwnerProperties->NotifyHook->NotifyPreChange( OwnerProperties );
		if( Cast<UClassProperty>(Property) && appStricmp(*Property->Category,TEXT("Drivers"))==0 )
		{
			// Save it.
			UClassProperty* ClassProp = CastChecked<UClassProperty>( Property );
			TArray<FRegistryObjectInfo> Classes;
			UObject::GetRegistryObjects( Classes, UClass::StaticClass(), ClassProp->MetaClass, 0 );
			for( INT i=0; i<Classes.Num(); i++ )
			{
				TCHAR Path[4096], *Str;
				appStrcpy( Path, *Classes(i).Object );
				Str = appStrstr(Path,TEXT("."));
				const TCHAR* Text = Str ? (*Str++=0,Localize(Str,TEXT("ClassCaption"),Path)) : Localize("Language","Language",TEXT("Core"),Path);
					if( appStricmp( Text, Value )==0 )
						GConfig->SetString( Property->GetOwnerClass()->GetPathName(), Property->GetName(), *Classes(i).Object );
			}
		}
		else if( Cast<UArrayProperty>(Property->GetOuter()) )
		{
			// Arrays.
			Property->ImportText( Value, GetReadAddress(Property,Offset), PPF_Localized );
			Class->GetDefaultObject()->SaveConfig();
		}
		else
		{
			// Regular property.
			UObject::GlobalSetProperty( Value, Class, Property, Offset, Immediate );
		}
		if( OwnerProperties->NotifyHook )
			OwnerProperties->NotifyHook->NotifyPostChange( OwnerProperties );
	}
	void OnResetToDefaultsButton()
	{
		LazyLoadClass();
		if( Class )
		{
			UObject::ResetConfig( Class );
			InvalidateRect( OwnerProperties->List, NULL, 1 );
			UpdateWindow( OwnerProperties->List );
		}
		Redraw();
	}
	void OnItemSetFocus()
	{
		FPropertyItemBase::OnItemSetFocus();
		AddButton( LocalizeGeneral("DefaultsButton",TEXT("Window")), FDelegate(this,(TDelegate)OnResetToDefaultsButton) );
	}
	void Expand()
	{
		LazyLoadClass();
		if( Class )
		{
			if( Children.Num()==0 )
			{
				Class->GetDefaultObject()->LoadConfig( 1 );//!!
				for( TFieldIterator<UProperty> It(Class); It; ++It )
				{
					if
					(	(AcceptFlags(It->PropertyFlags))
					&&	(Class==It->GetOwnerClass() || !(It->PropertyFlags&CPF_GlobalConfig) )
					&&	(CategoryFilter==NAME_None || It->Category==CategoryFilter) )
                    	Children.AddItem( new(TEXT("FPropertyItem"))FPropertyItem( OwnerProperties, this, *It, It->GetFName(), It->Offset, -1 ) );						
				}
			}
			FTreeItem::Expand();
		}
		else
		{
			Expandable = 0;
			Redraw();
		}
	}

	// FObjectConfigItem interface.
	void LazyLoadClass()
	{
		if( !Class && !Failed )
		{
			Class = UObject::StaticLoadClass( UObject::StaticClass(), NULL, *ClassName, NULL, LOAD_NoWarn, NULL );
			if( !Class )
			{
				Failed = 1;
				Caption = FString::Printf( LocalizeError("FailedConfigLoad",TEXT("Window")), ClassName );
			}
		}
	}

	// FTreeItem interface.
	void Serialize( FArchive& Ar )
	{
		FPropertyItemBase::Serialize( Ar );
		Ar << CategoryFilter << Class;
	}
};

// An configuration list item.
class WINDOW_API FConfigItem : public FHeaderItem
{
public:
	// Variables.
	FPreferencesInfo Prefs;

	// Constructors.
	FConfigItem()
	{}
	FConfigItem( const FPreferencesInfo& InPrefs, WPropertiesBase* InOwnerProperties, FTreeItem* InParent )
	: FHeaderItem( InOwnerProperties, InParent, 1 )
	, Prefs( InPrefs )
	{}

	// FTreeItem interface.
	QWORD GetId() const
	{
		return (INT)this + ((QWORD)3<<32);
	}
	virtual FString GetCaption() const
	{
		return Prefs.Caption;
	}
	void Expand()
	{
		TArray<FPreferencesInfo> NewPrefs;
		UObject::GetPreferences( NewPrefs, *Prefs.Caption, 0 );
		for( INT i=0; i<NewPrefs.Num(); i++ )
		{
			for( INT j=0; j<Children.Num(); j++ )
			{
				if( appStricmp( *Children(j)->GetCaption(), *NewPrefs(i).Caption )==0 )
					break;
			}
			if( j==Children.Num() )
			{
				if( NewPrefs(i).Class!=TEXT("") )
					Children.AddItem( new(TEXT("FObjectConfigItem"))FObjectConfigItem( OwnerProperties, this, *NewPrefs(i).Caption, *NewPrefs(i).Class, NewPrefs(i).Immediate, NewPrefs(i).Category ) );
				else
					Children.AddItem( new(TEXT("FConfigItem"))FConfigItem( NewPrefs(i), OwnerProperties, this ) );
			}
		}
		FTreeItem::Expand();
	}
	void Collapse()
	{
		FTreeItem::Collapse();
		EmptyChildren();
	}
};

// Configuration properties.
class WINDOW_API WConfigProperties : public WProperties
{
	W_DECLARE_CLASS(WConfigProperties,WProperties,CLASS_Transient);
	DECLARE_WINDOWCLASS(WConfigProperties,WProperties,Window)

	// Variables.
	FConfigItem Root;

	// Structors.
	WConfigProperties()
	{}
	WConfigProperties( FName InPersistentName, const TCHAR* InTitle )
	:	WProperties	( InPersistentName )
	,	Root		( FPreferencesInfo(), this, NULL )
	{
		Root.Prefs.Caption = InTitle;
	}

	// WPropertiesBase interface.
	FTreeItem* GetRoot()
	{
		return &Root;
	}
};

/*-----------------------------------------------------------------------------
	WWizardPage.
-----------------------------------------------------------------------------*/

// A wizard page.
class WINDOW_API WWizardPage : public WDialog
{
	W_DECLARE_ABSTRACT_CLASS(WWizardPage,WDialog,CLASS_Transient);
	DECLARE_WINDOWCLASS(WWizardPage,WDialog,Window)

	// Variables.
	WWizardDialog* Owner;

	// Constructor.
	WWizardPage()
	{}
	WWizardPage( const TCHAR* PageName, INT ControlId, WWizardDialog* InOwner )
	: WDialog	    ( PageName, ControlId, (WWindow*)InOwner )
	, Owner         ( InOwner )
	{}

	// WWizardPage interface.
	virtual void OnCurrent()
	{}
	virtual WWizardPage* GetNext()
	{
		return NULL;
	}
	virtual const TCHAR* GetBackText()
	{
		return LocalizeGeneral("BackButton",TEXT("Window"));
	}
	virtual const TCHAR* GetNextText()
	{
		return LocalizeGeneral("NextButton",TEXT("Window"));
	}
	virtual const TCHAR* GetFinishText()
	{
		return NULL;
	}
	virtual const TCHAR* GetCancelText()
	{
		return LocalizeGeneral("CancelButton",TEXT("Window"));
	}
	virtual UBOOL GetShow()
	{
		return 1;
	}
	virtual void OnCancel()
	{
		((WDialog*)Owner)->EndDialog( 0 );
	}
};

/*-----------------------------------------------------------------------------
	WWizardDialog.
-----------------------------------------------------------------------------*/

// The wizard frame dialog.
class WINDOW_API WWizardDialog : public WDialog
{
	W_DECLARE_CLASS(WWizardDialog,WDialog,CLASS_Transient);
	DECLARE_WINDOWCLASS(WWizardDialog,WDialog,Window)

	// Variables.
	WCoolButton BackButton;
	WCoolButton NextButton;
	WCoolButton FinishButton;
	WCoolButton CancelButton;
	WLabel PageHolder;
	TArray<WWizardPage*> Pages;
	WWizardPage* CurrentPage;

	// Constructor.
	WWizardDialog()
	: WDialog	    ( TEXT("WizardDialog"), IDDIALOG_WizardDialog )
	, PageHolder    ( this, IDC_PageHolder )
	, BackButton    ( this, IDC_Back,   FDelegate(this,(TDelegate)OnBack  ) )
	, NextButton    ( this, IDC_Next,   FDelegate(this,(TDelegate)OnNext  ) )
	, FinishButton  ( this, IDC_Finish, FDelegate(this,(TDelegate)OnFinish) )
	, CancelButton  ( this, IDCANCEL,   FDelegate(this,(TDelegate)OnCancel) )
	, Pages         ()
	, CurrentPage   ( NULL )
	{}

	// WDialog interface.
	void OnInitDialog()
	{
		WDialog::OnInitDialog();
		SendMessageX( *this, WM_SETICON, ICON_BIG, (WPARAM)LoadIconIdX(hInstance,(GIsEditor?IDICON_Editor:IDICON_Mainframe)) );
		RefreshPage();
	}

	// WWizardDialog interface.
	virtual void Advance( WWizardPage* NewPage )
	{
		check(NewPage);
		Pages.AddItem( NewPage );
		if( hWnd )
			RefreshPage();
		if( !Pages.Last()->GetShow() )
			OnNext();
	}
	virtual void RefreshPage()
	{
		if( Pages.Num() )
		{
			WWizardPage* Page = Pages.Last();
			if( !Page->hWnd )
				Page->OpenChildWindow( IDC_PageHolder, 1 );
			BackButton  .SetVisibleText( Pages.Num()>1 ? Page->GetBackText() : NULL );
			NextButton  .SetVisibleText( Page->GetNextText  () );
			FinishButton.SetVisibleText( Page->GetFinishText() );
			CancelButton.SetVisibleText( Page->GetCancelText() );
			if( Pages.Num()>1 )
				Pages(Pages.Num()-2)->Show(0);
			Pages.Last()->OnCurrent();
		}
	}
	virtual void OnDestroy()
	{
		for( INT i=0; i<Pages.Num(); i++ )
			delete Pages(i);
		WDialog::OnDestroy();
	}
	virtual void OnBack()
	{
		if( Pages.Num()>1 )
		{
			Pages(Pages.Num()-2)->Show(1);
			delete Pages.Pop();
			RefreshPage();
			if( !Pages.Last()->GetShow() )
				OnBack();
		}
	}
	virtual void OnNext()
	{
		if( Pages.Num() && Pages.Last()->GetNextText() )
		{
			WWizardPage* GotNext = Pages.Last()->GetNext();
			if( GotNext )
				Advance( GotNext );
		}
	}
	virtual void OnFinish()
	{
		EndDialog( 1 );
	}
	virtual void OnCancel()
	{
		Pages.Last()->OnCancel();
	}
	void OnClose()
	{
		Pages.Last()->OnCancel();
		throw TEXT("NoRoute");
	}
};

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
