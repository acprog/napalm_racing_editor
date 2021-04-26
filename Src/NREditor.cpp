/* naPalm Racing editor
  Copyright (C) 2004
  Author: Alexander Semenov <acmain@gmail.com>
*/

#include <PalmOS.h>
#include <CWCallbackThunks.h>

#include "NREditor.h"
#include "NREditorRsc.h"
#include "cfg.h"

// ********************************************************************
// Entry Points
// ********************************************************************

// ********************************************************************
// Global variables
// ********************************************************************

// g_prefs
// cache for application preferences during program execution
NREditorPreferenceType g_prefs;
bool	os5_available;

// MainFormHandleEventThunk
//     holds callback thunk for main form event handler

static _CW_CallbackThunk MainFormHandleEventThunk;

// ********************************************************************
// Internal Constants
// ********************************************************************

// Define the minimum OS version we support
#define os5Version    	 sysMakeROMVersion(5,0,0,sysROMStageDevelopment,0)
#define ourMinVersion    sysMakeROMVersion(3,5,0,sysROMStageDevelopment,0)
#define kPalmOS10Version sysMakeROMVersion(1,0,0,sysROMStageRelease,0)

// ********************************************************************
// Internal Functions
// ********************************************************************

// all code from here to end of file should use no global variables
#pragma warn_a5_access on


// FUNCTION: AppHandleEvent
//
// DESCRIPTION: 
//
// This routine loads form resources and set the event handler for
// the form loaded.
//
// PARAMETERS:
//
// event
//     a pointer to an EventType structure
//
// RETURNED:
//     true if the event was handled and should not be passed
//     to a higher level handler.

static Boolean AppHandleEvent(EventType * eventP)
{
    UInt16 formId;
    FormType * frmP;

    if (eventP->eType == frmLoadEvent)
    {
        // Load the form resource.
        formId = eventP->data.frmLoad.formID;
        frmP = FrmInitForm(formId);
        FrmSetActiveForm(frmP);

        // Set the event handler for the form.  The handler of the 
        // currently active form is called by FrmHandleEvent each 
        // time is receives an event.
        switch (formId)
        {
        case MainForm:
            FrmSetEventHandler(frmP, (FormEventHandlerType *)&MainFormHandleEventThunk);
            break;

        case ManagerForm:
            FrmSetEventHandler(frmP, (FormEventHandlerType *)&ManagerFormHandleEvent);
            break;

        default:
            break;

        }
        return true;
    }

    return false;
}

// FUNCTION: AppEventLoop
//
// DESCRIPTION: This routine is the event loop for the application.

static void AppEventLoop(void)
{
    UInt16 error;
    EventType event;

    do {
        // change timeout if you need periodic nilEvents
        EvtGetEvent(&event, evtWaitForever);

        if (! SysHandleEvent(&event))
        {
            if (! MenuHandleEvent(0, &event, &error))
            {
                if (! AppHandleEvent(&event))
                {
                    FrmDispatchEvent(&event);
                }
            }
        }
    } while (event.eType != appStopEvent);
}




//===========================================================
void DefaultCfg()
{
	g_prefs.pen_size=1;
	g_prefs.safe_mode=os5_available;
}





// FUNCTION: AppStart
//
// DESCRIPTION:  Get the current application's preferences.
//
// RETURNED:
//     errNone - if nothing went wrong

static Err AppStart(void)
{
    UInt16 prefsSize;

    // Read the saved preferences / saved-state information.
    prefsSize = sizeof(NREditorPreferenceType);
    if (PrefGetAppPreferences(
        appFileCreator, appPrefID, &g_prefs, &prefsSize, true) == noPreferenceFound
        || g_prefs.size!=sizeof(NREditorPreferenceType))
    {
   		g_prefs.size=sizeof(NREditorPreferenceType);
   		// FIXME: setup g_prefs with default values
   		DefaultCfg();
    }   
    
    // Setup main form event handler callback thunk (needed for "expanded" mode)
    _CW_GenerateCallbackThunk(MainFormHandleEvent, &MainFormHandleEventThunk);

#ifdef COLOR_GRAY
	UInt32 requiredDepth = 4;
#endif

#ifdef COLOR_8
	UInt32 requiredDepth = 8;
#endif

	Err err = WinScreenMode(winScreenModeSet, NULL, NULL, &requiredDepth, NULL);
	if (err)
	{
		FrmAlert(NeedMoreColorsAlert);
		return err;
	}

    return errNone;
}

// FUNCTION: AppStop
//
// DESCRIPTION: Save the current state of the application.

static void AppStop(void)
{
    // Write the saved preferences / saved-state information.  This 
    // data will be saved during a HotSync backup.
    PrefSetAppPreferences(
        appFileCreator, appPrefID, appPrefVersionNum, 
        &g_prefs, sizeof(NREditorPreferenceType), true);
        
    // Close all the open forms.
    FrmCloseAllForms();
}


// FUNCTION: RomVersionCompatible
//
// DESCRIPTION: 
//
// This routine checks that a ROM version is meet your minimum 
// requirement.
//
// PARAMETERS:
//
// requiredVersion
//     minimum rom version required
//     (see sysFtrNumROMVersion in SystemMgr.h for format)
//
// launchFlags
//     flags that indicate if the application UI is initialized
//     These flags are one of the parameters to your app's PilotMain
//
// RETURNED:
//     error code or zero if ROM version is compatible

static Err RomVersionCompatible(UInt32 requiredVersion, UInt16 launchFlags)
{
    UInt32 romVersion;

    // See if we're on in minimum required version of the ROM or later.
    FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);
    if (romVersion < requiredVersion)
    {
        if ((launchFlags & 
            (sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp)) ==
            (sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp))
        {
            // Palm OS 1.0 will continuously relaunch this app unless 
            // we switch to another safe one.
            if (romVersion <= kPalmOS10Version)
            {
                AppLaunchWithCommand(
                    sysFileCDefaultApp, 
                    sysAppLaunchCmdNormalLaunch, NULL);
            }
        }

        return sysErrRomIncompatible;
    }

    return errNone;
}

// FUNCTION: NREditorPalmMain
//
// DESCRIPTION: This is the main entry point for the application.
//
// PARAMETERS:
//
// cmd
//     word value specifying the launch code. 
//
// cmdPB
//     pointer to a structure that is associated with the launch code
//
// launchFlags
//     word value providing extra information about the launch
//
// RETURNED:
//     Result of launch, errNone if all went OK

static UInt32 NREditorPalmMain(
    UInt16 cmd, 
    MemPtr cmdPBP, 
    UInt16 launchFlags)
{
    Err error;

    error = RomVersionCompatible (ourMinVersion, launchFlags);
    if (error)
    {
		FrmAlert(RomIncompatibleAlert);
    	return (error);
    }

	UInt32 romVersion;
    switch (cmd)
    {
    case sysAppLaunchCmdNormalLaunch:
        FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);
        os5_available=romVersion>=os5Version;

        error = AppStart();
        if (error) 
            return error;

        // start application by opening the main form
        // and then entering the main event loop
        FrmGotoForm(ManagerForm);
        AppEventLoop();
        
        AppStop();
        break;

    default:
        break;
    }

    return errNone;
}
// FUNCTION: PilotMain
//
// DESCRIPTION: This is the main entry point for the application.
// 
// PARAMETERS:
//
// cmd
//     word value specifying the launch code. 
//
// cmdPB
//     pointer to a structure that is associated with the launch code
//
// launchFlags
//     word value providing extra information about the launch.
//
// RETURNED:
//     Result of launch, errNone if all went OK

UInt32 PilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags)
{
	try
	{
		return NREditorPalmMain(cmd, cmdPBP, launchFlags);
	}

	catch(...)
	{
	}
	return appErrorClass;
}


//=============================================================
void error(const char *str, const char *module, int line)
{
	char	s[5];
	FrmCustomAlert(RuntimeErrorAlert, str, module, StrIToA(s, line));
	throw -1;
}


// turn a5 warning off to prevent it being set off by C++
// static initializer code generation
#pragma warn_a5_access reset
