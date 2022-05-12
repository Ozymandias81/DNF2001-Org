class dnMapListExclude expands dnMapListBox;

function bool ExternalDragOver(UWindowDialogControl ExternalControl, float X, float Y)
{
	if(ExternalControl.OwnerWindow != OwnerWindow || dnMapListInclude(ExternalControl) == None)
		return False;
	
	if(Super.ExternalDragOver(ExternalControl, X, Y))
	{
		Sort();
		return True;
	}

	return False;
}

function ReceiveDoubleClickItem(UWindowListBox L, UWindowListBoxItem I)
{
	Super.ReceiveDoubleClickItem(L, I);
	Sort();
	MakeSelectedVisible();
}

defaultproperties
{
	bAcceptExternalDragDrop=True
	bCanDragExternal=True
}