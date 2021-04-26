/* naPalm Racing editor
  Copyright (C) 2004
  Author: Alexander Semenov <acmain@gmail.com>
*/
#include <PalmOS.h>
#include <CWCallbackThunks.h>

#include "NREditor.h"
#include "NREditorRsc.h"

#include "cfg.h"
#include "geometric.h"
#include "layer.h"


//=================================================================
db_info	*track_bases[1024];
int		n_tracks=0;
ListType	*tracks;
static	int		current_track=0;


//=================================================================
db_info *select_track(int tr, UInt16 label)
{
	if (n_tracks==0)
		return NULL;

	if (tr>=n_tracks)
		tr=0;
	
	db_info	*base=track_bases[tr];

	FormType	*frmP=FrmGetActiveForm();
	FrmCopyLabel(frmP, label, base->name);
//	CtlSetValue(GetObjectPtr<ControlType>(ManagerLockedCheckbox , frmP), base->creator==TRACK_CREATOR);

	DmOpenRef db=DmOpenDatabase(base->card, DmFindDatabase(base->card, base->name), dmModeReadOnly);

	MemHandle	h;
	UInt16		index=DmNumRecords(db);
	
	// поверхность
	--index;

	// дорога
	h=DmGetRecord(db, --index);

	packed_1bit	*buffer=new packed_1bit[MAP_SIZE*MAP_SIZE/8];

	image		*minimap_img=NULL;
	BitmapType	*minimap_bmp=NULL;
	if (!g_prefs.safe_mode)
	{
		minimap_img=new image(size<>(64, 64));
		minimap_img->draw_type=image::Put;
	}
	else
	{
		Err	error;
		minimap_bmp=BmpCreate(64, 64, COLOR_SIZE, NULL, &error);
	}	

	packed_1bit	*src=buffer;
	MemMove(buffer, MemHandleLock(h), MAP_SIZE*MAP_SIZE/8);
	MemHandleUnlock(h);

	int	x, y;
#ifdef COLOR_GRAY
	#define COLOR(n)	(1<<(src[0]._##n##0+src[0]._##n##1+src[MAP_SIZE/16]._##n##0+src[MAP_SIZE/16]._##n##1))-1;
	packed_4bit	*dst=(packed_4bit*)(g_prefs.safe_mode
		?	BmpGetBits(minimap_bmp)
		:	(color*)*minimap_img);

	for (y=0; y<MAP_SIZE/2; y++, src+=MAP_SIZE/16)
		for (x=0; x<MAP_SIZE/16; src++, dst+=2, x++)
		{
			dst[0]._00=COLOR(0);
			dst[0]._01=COLOR(1);
			dst[0]._10=COLOR(2);
			dst[0]._11=COLOR(3);
			dst[1]._00=COLOR(4);
			dst[1]._01=COLOR(5);
			dst[1]._10=COLOR(6);
			dst[1]._11=COLOR(7);
		}
#endif

#ifdef COLOR_8
	color	colors[]={0x00, 0x25, 0x38, 0x5d, 0x64};
	#define COLOR(n)	colors[src[0]._##n##0+src[0]._##n##1+src[MAP_SIZE/16]._##n##0+src[MAP_SIZE/16]._##n##1];
	color	*dst=(color*)(g_prefs.safe_mode
		?	BmpGetBits(minimap_bmp)
		:	*minimap_img);

	for (y=0; y<MAP_SIZE/2; y++, src+=MAP_SIZE/16)
		for (x=0; x<MAP_SIZE/16; src++, dst+=8, x++)
		{
			dst[0]=COLOR(0);
			dst[1]=COLOR(1);
			dst[2]=COLOR(2);
			dst[3]=COLOR(3);
			dst[4]=COLOR(4);
			dst[5]=COLOR(5);
			dst[6]=COLOR(6);
			dst[7]=COLOR(7);
		}
#endif

	#undef COLOR

	DmReleaseRecord(db, index, true);
	DmCloseDatabase(db);
	
	delete[] buffer;

	if (!g_prefs.safe_mode)
	{
		color	*screen=(color*)BmpGetBits(WinGetBitmap(WinGetDisplayWindow()));
		minimap_img->draw(screen, point<>(96, 15));
		delete minimap_img;
	}
	else
	{
		WinDrawBitmap(minimap_bmp, 96, 15);
		BmpDelete(minimap_bmp);
	}
	
	return base;
}


//=================================================================
static	void insert_track(db_info *base)
{
	*(track_bases[n_tracks++]=new db_info)=*base;
//	StrCopy(names[n_tracks++], name);
	LstSetListChoices(tracks, (char**)track_bases, n_tracks);
	LstSetSelection(tracks, n_tracks-1);
	track_bases[n_tracks]=NULL;
}


//=================================================================
static	void add_to_list(UInt32 creator)
{
//	UInt16	cardNo=0;
	LocalID	dbID;
	bool	newSearch=true;
//	char	name[256];
	db_info	base;
	UInt16	version;
	DmSearchStateType stateInfo;
	
//	base.creator=creator;

	while (!DmGetNextDatabaseByTypeCreator(newSearch, &stateInfo,
		TRACK_TYPE, creator, false, &base.card, &dbID))
	{
		DmDatabaseInfo(base.card, dbID, base.name,
					NULL, &version, NULL,
					NULL, NULL,
					NULL, NULL,
					NULL, NULL,
					NULL);
		insert_track(&base);
		newSearch=false;
	}
}


//=================================================================
static void FormInit(FormType *frmP)
{
	tracks=GetObjectPtr<ListType>(ManagerTracksList, frmP);
	//-----------------------------------------------
	// «аполнение списка
	n_tracks=0;

	add_to_list(TRACK_CREATOR);
//	add_to_list(TRACK_EXTERNAL);
	
	select_track(n_tracks-1, ManagerNameLabel);
	
	CtlSetValue(GetObjectPtr<ControlType>(ManagerSafeModeCheckbox, frmP), g_prefs.safe_mode);
}


//=================================================================
static void FormDestroy(FormType * frmP)
{
	while (n_tracks--)
		delete track_bases[n_tracks];
}


//=================================================================
static	bool enter_name(char *name)
{
	FormType *frmP=FrmInitForm(TrackNameForm);

	FieldType	*fldP=GetObjectPtr<FieldType>(TrackNameTextField, frmP);
	FldDelete(fldP, 0, FldGetMaxChars(fldP));
	FldInsert(fldP, name, StrLen(name));

	if (FrmDoDialog(frmP)==TrackNameOkButton)
	{
		StrCopy(name, FldGetTextPtr(GetObjectPtr<FieldType>(TrackNameTextField, frmP)));
		FrmDeleteForm (frmP);
		return true;
	}

	FrmDeleteForm (frmP);
	return false;
}


//=================================================================
static bool select_control(UInt16 id)
{
	db_info	base,
			new_track={"new track", 0};
	int		select=LstGetSelection(tracks);

//	StrCopy(name, n_tracks ? track_bases[select] : "new track");	
	base=(n_tracks ? *track_bases[select] : new_track);

	switch (id)
	{	
	case ManagerCreateButton:
		if (enter_name(base.name))
		{
			clear_track();
			save_track(base);
			insert_track(&base);
			LstDrawList(tracks);
		}
		return true;

	case ManagerOpenButton:
		if (!n_tracks)
			FrmAlert(NoneTrackAlert);
		else
		{
			load_track(base);
			FrmGotoForm(MainForm);
		}
		return true;

	case ManagerEraseButton:	
		if (!n_tracks)
			FrmAlert(NoneTrackAlert);
		else if (FrmCustomAlert(SubmitEraseAlert, base.name, NULL, NULL)==SubmitEraseOK)
		{
			DmDeleteDatabase(0, DmFindDatabase(0, base.name));
			n_tracks--;
			while (select<n_tracks)
				*track_bases[select++]=*track_bases[select+1];
			delete track_bases[n_tracks];
			track_bases[n_tracks]=NULL;
			LstSetListChoices(tracks, (char**)track_bases, n_tracks);
			LstDrawList(tracks);
			select_track(n_tracks-1, ManagerNameLabel);
		}
		return true;

	case ManagerDuplicateButton:
		if (!n_tracks)
			FrmAlert(NoneTrackAlert);
		else
		{
			load_track(base);
			if (enter_name(base.name))
			{
				save_track(base);
				insert_track(&base);
			}
		}
		return true;

	case  ManagerSafeModeCheckbox:
		g_prefs.safe_mode=!g_prefs.safe_mode;
		return true;
	}

	return false;
}



//=================================================================
Boolean ManagerFormHandleEvent(EventType *eventP)
{
    Boolean handled = false;
	FormType *frmP=NULL;

    switch (eventP->eType) 
        {
        case frmOpenEvent:
            frmP = FrmGetActiveForm();
            FormInit(frmP);
            FrmDrawForm(frmP);
            handled = true;
            break;

		case frmCloseEvent:
			FormDestroy(frmP);
			break;
            
        case frmUpdateEvent:
            // To do any custom drawing here, first call 
            // FrmDrawForm(), then do your drawing, and 
            // then set handled to true.
            break;
 
        case lstSelectEvent:
        	select_track(eventP->data.lstSelect.selection, ManagerNameLabel);
        	return true;
        	       
        case ctlSelectEvent:
            return select_control(eventP->data.ctlSelect.controlID);
		}
    
    return handled;
}