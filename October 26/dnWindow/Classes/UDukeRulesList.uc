//=============================================================================
// UDukeRulesList - The rules returned by the server
//=============================================================================
class UDukeRulesList extends UWindowList;

var string			Rule;
var string			Value;

// Sentinel only
var int				SortColumn;
var bool			bDescending;

function SortByColumn(int Column)
{
	if(SortColumn == Column)
	{
		bDescending = !bDescending;
	}
	else
	{
		SortColumn = Column;
		bDescending = False;
	}

	Sort();
}

function int Compare(UWindowList T, UWindowList B)
{
	local int Result;
	local UDukeRulesList RT, RB;

	if(B == None) return -1; 

	if(UDukeRulesList(T).Rule < UDukeRulesList(B).Rule)
		Result = -1;
	else
		Result = 1;

	if(UDukeRulesList(Sentinel).bDescending)
		Result = -Result;

	return Result;
}

defaultproperties
{
	SortColumn=1
	bDescending=False
}
