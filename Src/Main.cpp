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

//================================================================
static	db_info	current_track;

enum	draw_state
{
	terrain_draw,
	road_draw,
	waypoints_place
}state=road_draw;

layer	ground(size<>(MAP_SIZE, MAP_SIZE), grass),
		road_mask(size<>(MAP_SIZE, MAP_SIZE), no_road);

point<>	old_p;	// предыдущая позиция пера

Int8	pen_size=1;
color	pen_color=road;

BitmapType	*safe_scr=NULL;

//-----------------------------------------------------------
// Путь
point<>	waypoints[128];
struct
{
	int	n;
}way;
bool	hide_way=false;


//=================================================================
static void	redraw()
{
	if (!safe_scr)
	{
		ground.redraw(false);
		road_mask.redraw(true);
	}
	else
	{
		UInt32	*screen=(UInt32*)BmpGetBits(safe_scr),
				*gr=(UInt32*)(color*)ground,
				*rm=(UInt32*)(color*)road_mask;
		for (int i=0; i<BPP(MAP_SIZE*MAP_SIZE)/4; i++)
			*(screen++)=*(gr++)+*(rm++);
	}

	// отрисока пути
	if (!hide_way)
	{
		layer	*scr;

		if (!safe_scr)
		{
			color	*screen=(color*)BmpGetBits(WinGetBitmap(WinGetActiveWindow()));
			scr=new layer(ground, screen+SCREEN_OFFSET(MAP_LEFT, MAP_TOP), SCREEN_WIDTH-MAP_SIZE);
		}
		else
			scr=new layer(ground, (color*)BmpGetBits(safe_scr));

		waypoints[way.n]=waypoints[0];	// для удобства связки первой и последней точки
		for (int i=0; i<way.n; i++)
		{
			scr->line(waypoints[i], waypoints[i+1], 0, 1);
			if (i>0)
				scr->big_point(waypoints[i], 0, 5);
			else
			{
				scr->big_point(waypoints[i], 0, 5);
				scr->big_point(waypoints[i], 0xff, 3);
			}
		}
		
		delete scr;
	}

	if (safe_scr)
		WinDrawBitmap(safe_scr, MAP_LEFT, MAP_TOP);
}


//=================================================================
static void MainFormInit(FormType *frmP)
{
	pen_size=1;
	pen_color=road;
	state=road_draw;

	CtlSetValue(GetObjectPtr<ControlType>(MainPen1PushButton, frmP), true);
	CtlSetValue(GetObjectPtr<ControlType>(MainRoadPushButton, frmP), true);

	if (g_prefs.safe_mode)
	{
		Err	error;
		safe_scr=BmpCreate(MAP_SIZE, MAP_SIZE, COLOR_SIZE, NULL, &error);
	}

	RectangleType	r={{MAP_TOP-1, MAP_LEFT-1}, {MAP_SIZE+2, MAP_SIZE+2}};
	WinDrawRectangle(&r, 0);
	redraw();
}


//=================================================================
static void MainFormDestroy(FormType * frmP)
{
	if (safe_scr)
	{
		BmpDelete(safe_scr);
		safe_scr=NULL;
	}
}



//=================================================================
void load_track(const db_info &base)
{
//	StrCopy(track_name, name);
	current_track=base;
	
	if (!base.name || base.name[0]=='\0')
		ERROR("No track name");

	DmOpenRef db=DmOpenDatabase(base.card, DmFindDatabase(base.card, base.name), dmModeReadOnly);

	if (!db)
		ERROR("DmOpenDatabase()");
		
	MemHandle	h;
	UInt16		index=DmNumRecords(db);
		
	// поверхность
	h=DmGetRecord(db, --index);
	if (!h)
		ERROR("DmGetRecord()");
	ground.unpack_2bit(h);
	if (DmReleaseRecord(db, index, true)!=errNone)
		ERROR("DmReleaseRecord()");

	// дорога
	h=DmGetRecord(db, --index);
	if (!h)
		ERROR("DmGetRecord()");
	road_mask.unpack_1bit(h);
	if (DmReleaseRecord(db, index, true)!=errNone)
		ERROR("DmReleaseRecord()");
		
	// путь
	h=DmGetRecord(db, --index);
	if (!h)
		ERROR("DmGetRecord()");
	MemPtr	p=MemHandleLock(h);
	if (!p)
		ERROR("MemHandleLock()");
	if (MemMove(&way, p, sizeof(way))!=errNone)
		ERROR("MemMove()");

	if (MemMove(waypoints, (char*)p+sizeof(way), sizeof(point<>)*way.n)!=errNone)
		ERROR("MemMove()");
	if (MemHandleUnlock(h)!=errNone)
		ERROR("MemHandleUnlock()");
	if (DmReleaseRecord(db, index, true)!=errNone)
		ERROR("DmReleaseRecord()");

	if (DmCloseDatabase(db)!=errNone)
		ERROR("DmCloseDatabase()");
		
	// исключение наложения рядом стоящих точек
	for (int i=0; i<way.n-1; i++)
		if (waypoints[i]==waypoints[i+1])
		{
			for (int j=i; j<way.n-1; j++)
				waypoints[j]=waypoints[j+1];
			i--;
			way.n--;
		}
}



//=================================================================
void save_track(const db_info &base)
{
	DmDeleteDatabase(base.card, DmFindDatabase(base.card, base.name));
	DmCreateDatabase(base.card, base.name, TRACK_CREATOR, TRACK_TYPE, false);
	LocalID	id=DmFindDatabase(base.card, base.name);
	DmOpenRef db=DmOpenDatabase(base.card, id, dmModeWrite);

	MemHandle	h;
	UInt16		index;
	
	// поверхность
	h=DmNewRecord(db, &index, MAP_SIZE*MAP_SIZE/4);
	ground.pack_2bit(h);
	DmReleaseRecord(db, index, true);

	// дорога
	h=DmNewRecord(db, &index, MAP_SIZE*MAP_SIZE/8);
	road_mask.pack_1bit(h);
	DmReleaseRecord(db, index, true);

	// путь
	h=DmNewRecord(db, &index, sizeof(way)+way.n*sizeof(point<>));
	MemPtr	p=MemHandleLock(h);
	DmWrite(p, 0, &way, sizeof(way));
	DmWrite(p, sizeof(way), waypoints, sizeof(point<>)*way.n);
	MemHandleUnlock(h);
	DmReleaseRecord(db, index, true);
	
	DmCloseDatabase(db);
	
	UInt16 attributes;
	UInt16 version;

	DmDatabaseInfo(0, id, NULL,
					&attributes, &version, NULL,
					NULL, NULL,
					NULL, NULL,
					NULL, NULL,
					NULL);

	attributes|=dmHdrAttrBackup;
//	version=1;

	DmSetDatabaseInfo(0, id, NULL,
					&attributes, &version, NULL,
					NULL, NULL,
					NULL, NULL,
					NULL, NULL,
					NULL);
}


//=================================================================
void clear_track()
{
	ground.clear(state==terrain_draw ? pen_color : grass);
	road_mask.clear(no_road);
	way.n=0;
}


//=================================================================
static	bool waypoints_form()
{
	FormType *frmP=FrmInitForm(WayForm);

	CtlSetValue(GetObjectPtr<ControlType>(WayHideCheckbox, frmP), hide_way);

	switch (FrmDoDialog(frmP))
	{
	case WayResetButton:
		way.n=0;
	
	case WayDoneButton:
		hide_way=CtlGetValue(GetObjectPtr<ControlType>(WayHideCheckbox, frmP));
		FrmDeleteForm (frmP);
		return true;
		
	case WayDrawRoadButton:
		for (int i=0; i<way.n; i++)
			road_mask.line(waypoints[i], waypoints[i+1], road, pen_size);
		hide_way=CtlGetValue(GetObjectPtr<ControlType>(WayHideCheckbox, frmP));
		FrmDeleteForm (frmP);
		return true;
	}

	FrmDeleteForm (frmP);
	return false;
}





//=================================================================
static Boolean MainFormDoCommand(UInt16 command)
{
    Boolean handled = false;
    FormType * frmP;

    switch (command)
    {
    case TrackManager:
	    FrmGotoForm(ManagerForm);
	    return true;
    
    case TrackClearAll:
		clear_track();
		redraw();
	    return true;
	    
	case TrackUndoAll:
		load_track(current_track);
		redraw();
		return true;
    
    case HelpAbout:
		MenuEraseStatus(0);
        frmP = FrmInitForm (AboutForm);
        FrmCopyLabel(frmP, AboutDateLabel, __DATE__);
        FrmDoDialog (frmP);
        FrmDeleteForm (frmP);
		return true;

    case HelpUseEditor:
    	FrmHelp(UseEditorString);
		return true;
    }
    
    return handled;
}


//=================================================================
static bool select_control(UInt16 id)
{
	switch (id)
	{
	//--------------------------------------------------------
	case MainPen1PushButton:
		pen_size=1;
		return true;
			
	case MainPen3PushButton:
		pen_size=3;
		return true;
			
	case MainPen5PushButton:
		pen_size=5;
		return true;
		
	//--------------------------------------------------------
	case MainWaterPushButton:
		pen_color=water;
		state=terrain_draw;
		return true;

	case MainGrassPushButton:
		pen_color=grass;
		state=terrain_draw;
		return true;

	case MainSandPushButton:
		pen_color=sand;
		state=terrain_draw;
		return true;

	case MainBlockPushButton:
		pen_color=block;
		state=terrain_draw;
		return true;
		
	//--------------------------------------------------------
	case MainRoadPushButton:
		pen_color=road;
		state=road_draw;
		return true;
		
	case MainClearPushButton:
		pen_color=no_road;
		state=road_draw;
		return true;

	//--------------------------------------------------------
	case MainWayPushButton:
		state=waypoints_place;
		waypoints_form();
		return true;
    }

	return false;
}


//=================================================================
// корректировка точки (что-бы не выходила за пределы карты)
static	void correct_pen(point<> &p, int size)
{
#ifdef COLOR_GRAY
	if (p.x<size/2-1)
		p.x=size/2-1;
	if (p.x>MAP_SIZE-size/2-(size>1 ? 2 : 1))
		p.x=MAP_SIZE-size/2-(size>1 ? 2 : 1);
#endif

#ifndef COLOR_GRAY
	if (p.x<size/2)
		p.x=size/2;
	if (p.x>MAP_SIZE-size/2-1)
		p.x=MAP_SIZE-size/2-1;
#endif

	if (p.y<size/2)
		p.y=size/2;
	if (p.y>MAP_SIZE-size/2-1)
		p.y=MAP_SIZE-size/2-1;
}


//=================================================================
static	void pen_move(point<> &p)
{
	switch (state)
	{
	case terrain_draw:
		correct_pen(p, pen_size);
		ground.line(old_p, p, pen_color, pen_size);
		break;
		
	case road_draw:
		correct_pen(p, pen_size);
		road_mask.line(old_p, p, pen_color, pen_size);
		break;
		
	case waypoints_place:
		correct_pen(p, 5);
		if (!hide_way)
			waypoints[way.n-1]=p;
		break;
	}
		
	redraw();
	old_p=p;
}


//=================================================================
static	void pen_down(point<> &p)
{
	correct_pen(p, state==waypoints_place ? 5 : pen_size);
	old_p=p;
	if (state==waypoints_place && !hide_way)
		way.n++;
	pen_move(p);
}


//=================================================================
static	void pen_up(const point<> &p)
{
	// проверка на совпадение точек пути
	if (state==waypoints_place && way.n>1 && waypoints[way.n-1]==waypoints[way.n-2])
		way.n--;
	redraw();
}



//=================================================================
Boolean MainFormHandleEvent(EventType * eventP)
{
    Boolean handled = false;
	FormType *frmP=NULL;
	static	rect<>	ground_frame(point<>(MAP_LEFT, MAP_TOP), size<>(MAP_SIZE, MAP_SIZE));
	static	bool	drawing=false,
					initialized=false;
	point<>	p(eventP->screenX, eventP->screenY);
	static	WinHandle	win=(WinHandle)FrmGetFormPtr(MainForm);

    switch (eventP->eType) 
        {
        case menuEvent:
            return MainFormDoCommand(eventP->data.menu.itemID);

        case frmOpenEvent:
            frmP = FrmGetActiveForm();
            MainFormInit(frmP);
            FrmDrawForm(frmP);
            handled = true;
            initialized=true;
            break;

		case penDownEvent:
			if (ground_frame.order(p)==point<>::inside)
			{
				drawing=true;
				p-=ground_frame;
				pen_down(p);
			}
			break;

		case penMoveEvent:
			if (drawing && ground_frame.order(p)==point<>::inside)
			{
				p-=ground_frame;
				pen_move(p);
			}
			break;
			
		case penUpEvent:
			if (drawing)
			{
				drawing=false;
				p-=ground_frame;
				pen_up(p);
			}
			break;			

		case frmCloseEvent:
	    	save_track(current_track);
			MainFormDestroy(frmP);
			initialized=false;
			break;
            
        case frmUpdateEvent:
        	redraw();
            // To do any custom drawing here, first call 
            // FrmDrawForm(), then do your drawing, and 
            // then set handled to true.
            break;
            
		case	winEnterEvent:
			if (eventP->data.winEnter.enterWindow==win && initialized)
				redraw();
			break;
        
        case ctlSelectEvent:
            return select_control(eventP->data.ctlSelect.controlID);
		}
    
    return handled;
}