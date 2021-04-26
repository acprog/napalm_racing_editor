/* naPalm Racing editor
  Copyright (C) 2004
  Author: Alexander Semenov <acmain@gmail.com>
*/

#ifndef NREDITOR_H_
#define NREDITOR_H_


// ********************************************************************
// Internal Structures
// ********************************************************************
struct db_info
{
	char	name[40];
	UInt16	card;
//	UInt32	creator;
};


typedef struct NREditorPreferenceType
{
	UInt32	size;
	Int8	pen_size;
	bool	safe_mode;
} NREditorPreferenceType;


// ********************************************************************
// Internal funcs
// ********************************************************************
Boolean MainFormHandleEvent(EventType * eventP);
Boolean ManagerFormHandleEvent(EventType *eventP);

void load_track(const db_info &base);
void save_track(const db_info &base);
void clear_track();
db_info *select_track(int tr, UInt16 label);


// ********************************************************************
// Global variables
// ********************************************************************

extern NREditorPreferenceType g_prefs;

// ********************************************************************
// Internal Constants
// ********************************************************************

#define TRACK_CREATOR			'ACnr'
//#define TRACK_EXTERNAL			'ACxt'
#define TRACK_TYPE				'Trek'

#define appFileCreator			'ACre'
#define appFileCreator			'ACre'
#define appName					"NR Editor"
#define appVersionNum			0x01
#define appPrefID				0x00
#define appPrefVersionNum		0x01

#define TrackManager                         1000	// Menu Text: Менеджер
#define TrackClearAll                         1001	// Menu Text: Очистить все
#define TrackUndoAll                         1002	// Menu Text: Отменить все

#define HelpUseEditor                          1100	// Menu Text: Использование Редактора
#define HelpAbout                          1101	// Menu Text: О Программе

// ********************************************************************
// Helper template functions
// ********************************************************************
template <class T>
typename T * GetObjectPtr(UInt16 id, FormType *frmP=NULL)
{
	if (!frmP)
		frmP = FrmGetActiveForm();
	return static_cast<T *>(
		FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, id)));
}

#endif // NREDITOR_H_
