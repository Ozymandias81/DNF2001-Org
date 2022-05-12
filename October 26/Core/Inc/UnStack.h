/*=============================================================================
	UnStack.h: UnrealScript execution stack definition.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

class UStruct;

/*-----------------------------------------------------------------------------
	Constants & types.
-----------------------------------------------------------------------------*/

// Sizes.
enum {MAX_STRING_CONST_SIZE		= 256               };
enum {MAX_CONST_SIZE			= 16 *sizeof(TCHAR) };
enum {MAX_FUNC_PARMS			= 16                };

//
// UnrealScript intrinsic return value declaration.
//
#define RESULT_DECL void*const Result

//
// guardexec mechanism for script debugging.
//
#define unguardexecSlow  unguardfSlow(( TEXT("(%s @ %s : %04X)"), Stack.Object->GetFullName(), Stack.Node->GetFullName(), Stack.Code - &Stack.Node->Script(0) ))
#define unguardexec      unguardf    (( TEXT("(%s @ %s : %04X)"), Stack.Object->GetFullName(), Stack.Node->GetFullName(), Stack.Code - &Stack.Node->Script(0) ))

//
// Evaluatable expression item types.
//
enum EExprToken
{
	// Variable references.
	EX_LocalVariable		= 0x00,	// A local variable.
	EX_InstanceVariable		= 0x01,	// An object variable.
	EX_DefaultVariable		= 0x02,	// Default variable for a concrete object.

	// Tokens.
	EX_Return				= 0x04,	// Return from function.
	EX_Switch				= 0x05,	// Switch.
	EX_Jump					= 0x06,	// Goto a local address in code.
	EX_JumpIfNot			= 0x07,	// Goto if not expression.
	EX_Stop					= 0x08,	// Stop executing state code.
	EX_Assert				= 0x09,	// Assertion.
	EX_Case					= 0x0A,	// Case.
	EX_Nothing				= 0x0B,	// No operation.
	EX_LabelTable			= 0x0C,	// Table of labels.
	EX_GotoLabel			= 0x0D,	// Goto a label.
	EX_EatString            = 0x0E, // Ignore a dynamic string.
	EX_Let					= 0x0F,	// Assign an arbitrary size value to a variable.
	EX_DynArrayElement      = 0x10, // Dynamic array element.!!
	EX_New                  = 0x11, // New object allocation.
	EX_ClassContext         = 0x12, // Class default metaobject context.
	EX_MetaCast             = 0x13, // Metaclass cast.
	EX_LetBool				= 0x14, // Let boolean variable.
	EX_LineNumber			= 0x15, // Set current source code line number in stack frame.
	EX_EndFunctionParms		= 0x16,	// End of function call parameters.
	EX_Self					= 0x17,	// Self object.
	EX_Skip					= 0x18,	// Skippable expression.
	EX_Context				= 0x19,	// Call a function through an object context.
	EX_ArrayElement			= 0x1A,	// Array element.
	EX_VirtualFunction		= 0x1B,	// A function call with parameters.
	EX_FinalFunction		= 0x1C,	// A prebound function call with parameters.
	EX_IntConst				= 0x1D,	// Int constant.
	EX_FloatConst			= 0x1E,	// Floating point constant.
	EX_StringConst			= 0x1F,	// String constant.
	EX_ObjectConst		    = 0x20,	// An object constant.
	EX_NameConst			= 0x21,	// A name constant.
	EX_RotationConst		= 0x22,	// A rotation constant.
	EX_VectorConst			= 0x23,	// A vector constant.
	EX_ByteConst			= 0x24,	// A byte constant.
	EX_IntZero				= 0x25,	// Zero.
	EX_IntOne				= 0x26,	// One.
	EX_True					= 0x27,	// Bool True.
	EX_False				= 0x28,	// Bool False.
	EX_NativeParm           = 0x29, // Native function parameter offset.
	EX_NoObject				= 0x2A,	// NoObject.
	EX_IntConstByte			= 0x2C,	// Int constant that requires 1 byte.
	EX_BoolVariable			= 0x2D,	// A bool variable which requires a bitmask.
	EX_DynamicCast			= 0x2E,	// Safe dynamic class casting.
	EX_Iterator             = 0x2F, // Begin an iterator operation.
	EX_IteratorPop          = 0x30, // Pop an iterator level.
	EX_IteratorNext         = 0x31, // Go to next iteration.
	EX_StructCmpEq          = 0x32,	// Struct binary compare-for-equal.
	EX_StructCmpNe          = 0x33,	// Struct binary compare-for-unequal.
	EX_UnicodeStringConst   = 0x34, // Unicode string constant.
	EX_RangeConst			= 0x35,	// A range constant.
	EX_StructMember         = 0x36, // Struct member.
	//
	EX_GlobalFunction		= 0x38, // Call non-state version of a function.

	// Native conversions.
	EX_MinConversion		= 0x39,	// Minimum conversion token.
	EX_RotatorToVector		= 0x39,
	EX_ByteToInt			= 0x3A,
	EX_ByteToBool			= 0x3B,
	EX_ByteToFloat			= 0x3C,
	EX_IntToByte			= 0x3D,
	EX_IntToBool			= 0x3E,
	EX_IntToFloat			= 0x3F,
	EX_BoolToByte			= 0x40,
	EX_BoolToInt			= 0x41,
	EX_BoolToFloat			= 0x42,
	EX_FloatToByte			= 0x43,
	EX_FloatToInt			= 0x44,
	EX_FloatToBool			= 0x45,
	//
	EX_ObjectToBool			= 0x47,
	EX_NameToBool			= 0x48,
	EX_StringToByte			= 0x49,
	EX_StringToInt			= 0x4A,
	EX_StringToBool			= 0x4B,
	EX_StringToFloat		= 0x4C,
	EX_StringToVector		= 0x4D,
	EX_StringToRotator		= 0x4E,
	EX_VectorToBool			= 0x4F,
	EX_VectorToRotator		= 0x50,
	EX_RotatorToBool		= 0x51,
	EX_ByteToString			= 0x52,
	EX_IntToString			= 0x53,
	EX_BoolToString			= 0x54,
	EX_FloatToString		= 0x55,
	EX_ObjectToString		= 0x56,
	EX_NameToString			= 0x57,
	EX_VectorToString		= 0x58,
	EX_RotatorToString		= 0x59,
	EX_MaxConversion		= 0x60,	// Maximum conversion token.

	// Natives.
	EX_ExtendedNative		= 0x60,
	EX_FirstNative			= 0x70,

	EX_Max					= 0x1000,
};

//
// Latent functions.
//
enum EPollSlowFuncs
{
	EPOLL_Sleep			      = 384,
	EPOLL_FinishAnim	      = 385,
	EPOLL_FinishInterpolation = 302,
};

//
// Native function table.
//
struct FFrame;
typedef void (__fastcall UObject::*Native)( FFrame& TheStack, RESULT_DECL );
extern CORE_API Native GNatives[];
BYTE CORE_API GRegisterNative( INT iNative, const Native& Func );

/*-----------------------------------------------------------------------------
	Execution stack helpers.
-----------------------------------------------------------------------------*/

//
// Information about script execution at one stack level.
//
struct CORE_API FFrame : public FOutputDevice
{	
	// Variables.
	UStruct*	Node;
	UObject*	Object;
	BYTE*		Code;
	BYTE*		Locals;

	// Constructors.
	FFrame( UObject* InObject );
	FFrame( UObject* InObject, UStruct* InNode, INT CodeOffset, void* InLocals );
	inline ~FFrame() {}

	//__forceinline void Step( UObject* Context, RESULT_DECL );
	__forceinline void Step( UObject* Context, RESULT_DECL )
	{
		INT B = *Code++;
		(Context->*GNatives[B])( *this, Result );	// NJS: Primary script function dispatch point:
	}

	void Serialize( const TCHAR* V, enum EName Event );

	__forceinline INT ReadInt()
	{
		INT Result = *(INT*)Code;
		Code += sizeof(INT);
		return Result;
	}
	__forceinline UObject* ReadObject()
	{
		UObject* Result = *(UObject**)Code;
		Code += sizeof(INT);
		return Result;
	}
	__forceinline FLOAT ReadFloat()
	{
		FLOAT Result = *(FLOAT*)Code;
		Code += sizeof(FLOAT);
		return Result;
	}
	__forceinline INT ReadWord()
	{
		INT Result = *(_WORD*)Code;
		Code += sizeof(_WORD);
		return Result;
	}
	__forceinline FName ReadName()
	{
		FName Result = *(FName*)Code;
		Code += sizeof(FName);
		return Result;
	}

/*
	INT ReadInt();
	UObject* ReadObject();
	FLOAT ReadFloat();
	INT ReadWord();
	FName ReadName();
*/
};

//
// Information about script execution at the main stack level.
// This part of an actor's script state is saveable at any time.
//
struct CORE_API FStateFrame : public FFrame
{
	// Variables.
	FStateFrame*	StateStack;
	UState*			StateNode;
	QWORD			ProbeMask;
	INT				LatentAction;

	// Functions.
	FStateFrame( UObject* InObject );
	~FStateFrame();
	const TCHAR* Describe();
};

/*-----------------------------------------------------------------------------
	Script execution helpers.
-----------------------------------------------------------------------------*/

//
// Base class for UnrealScript iterator lists.
//
struct FIteratorList
{
	FIteratorList* Next;
	FIteratorList() {}
	FIteratorList( FIteratorList* InNext ) : Next( InNext ) {}
	FIteratorList* GetNext() { return (FIteratorList*)Next; }
};

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
