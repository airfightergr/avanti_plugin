/*
Plugin for P-180 Avanti II for X-Plane
ilias@tselios.com - All rights reserved - 2023.

Scope to understand how work with c/c++ and produce a working plugin for X-Plane

*/

// system includes
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


// #define XPLM400 = 1;  // This example requires SDK4.0
#include "XPLMPlugin.h"
#include "XPLMProcessing.h"
#include "XPLMDataAccess.h"
#include "XPLMUtilities.h"
#include "XPLMDisplay.h"

// include main header file
#include "acfutils/log.h"
#include "acfutils/glew.h"
#include "acfutils/core.h"
#include "acfutils/helpers.h"
#include "acfutils/mt_cairo_render.h"
#include "units_conv.h"
#include "pfd.h"
#include "avanti_gui.h"

#if IBM
#include <windows.h>
#endif
#if LIN
#include <GL/glew.h>
#include <GL/gl.h>
#endif
#if __GNUC__ && APL
#include <OpenGL/gl.h>
#endif
#if __GNUC__ && IBM
#include <GL/gl.h>
#endif





// Log buffer
char myValue_buffer[256];
char logbuff[256];


//  Add dataref to DRE message
#define MSG_ADD_DATAREF 0x01000000

//  Our custom dataref
static XPLMDataRef altInMeters = NULL;
static XPLMDataRef pilotAltFeet = NULL;

//  Our custom dataref's value
float   altInMetersValue;
float   altInFeet;

// float   calc_altimeter(float elapsedMe, float elapsedSim, int counter, void* refcon );
float   RegaltInMetersInDRE(float elapsedMe, float elapsedSim, int counter, void * refcon);  //  Declare callback to register dataref

// Test function
float myTimer(float elapsedMe, float elapsedSim, int counter, void * refcon);

float   getAltVal(void* inRefcon);
void    setAltVal(void* inRefcon, float outValue);

time_t now = 0;


/* This will be your custom logging function */
static void my_dbg_logger(const char *str)
{
    XPLMDebugString(str);
}




XPLMCommandRef OpenLink = NULL;

int OpenLinkHandler(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void * inRefcon);


PLUGIN_API int XPluginStart(
         char *        outName,
         char *        outSig,
         char *        outDesc)
{

    XPLMEnableFeature("XPLM_USE_NATIVE_PATHS", 1);

 // Plugin Info
     strcpy(outName, "P-180 Avanti II");
     strcpy(outSig, "tselios.acft.avanti");
     strcpy(outDesc, "Aircraft Plugin for Avanti");

 //  Create our custom integer dataref
 altInMeters = XPLMRegisterDataAccessor("avanti/FM/pilotAltMeters",
                    xplmType_Float,         // The types we support
                    1,                      // Writable
                    NULL, NULL,             // Integer accessors
                    getAltVal, setAltVal,   // Float accessors
                    NULL, NULL,             // Doubles accessors
                    NULL, NULL,             // Int array accessors
                    NULL, NULL,             // Float array accessors
                    NULL, NULL,             // Raw data accessors
                    NULL, NULL);            // Refcons not used

    // Find and intialize our Counter dataref
    altInMeters = XPLMFindDataRef ("avanti/FM/pilotAltMeters");
    pilotAltFeet = XPLMFindDataRef("sim/cockpit2/gauges/indicators/altitude_ft_pilot");
    altInFeet = XPLMGetDataf(pilotAltFeet);
    XPLMSetDataf(altInMeters, altInFeet * 0.3048); 

    //time stamp and print to Log.txt
    struct tm *ltm = localtime(&now);

    snprintf(myValue_buffer, 256, "%d-%02d-%02d %02d:%02d:%02d [P-180 Avanti II]: Plugin loaded!\n", 1900 + ltm->tm_year, 1 + ltm->tm_mon, ltm->tm_mday, ltm->tm_hour, ltm->tm_min, ltm->tm_sec);
    XPLMDebugString(myValue_buffer);

    //register the callback
    XPLMRegisterFlightLoopCallback(RegaltInMetersInDRE, 1, NULL);   // This FLCB will register our custom dataref in DRE
    XPLMRegisterFlightLoopCallback(myTimer, 1, NULL);   // This FLCB will register our custom dataref in DRE
    // XPLMRegisterFlightLoopCallback(calc_altimeter, 1, NULL);
    XPLMPluginID PluginID = XPLMFindPluginBySignature("xplanesdk.examples.DataRefEditor");

    // init units
    units_conv_init();

    // init drawing
    drawing_init();

    // init imgui
    XPLMRegisterDrawCallback((XPLMDrawCallback_f) DrawImGui, xplm_Phase_Window, 0, NULL);
//    ImGui_Start();

    // init command to open manual page
    OpenLink = XPLMCreateCommand("avanti/cmd/openLink", "Open Web Page");
    XPLMRegisterCommandHandler(OpenLink, OpenLinkHandler, 1, (void *) 0);

    log_init(my_dbg_logger, "[ACF UTILS LOG]: ");

if (PluginID != XPLM_NO_PLUGIN_ID)
{
    XPLMSendMessageToPlugin(PluginID, MSG_ADD_DATAREF, (void*)"avanti/FM/pilotAltMeters");
    XPLMSendMessageToPlugin(PluginID, MSG_ADD_DATAREF, (void*)"avanti/gauges/pilot/plt_airspeed_kmh");
}

return 1;

}


PLUGIN_API void XPluginStop(void)
{
    XPLMUnregisterDataAccessor(altInMeters);
    XPLMUnregisterFlightLoopCallback(RegaltInMetersInDRE, NULL);	 //  Don't forget to unload this callback.
    XPLMUnregisterCommandHandler(OpenLink, OpenLinkHandler, 0, 0);
    //      XPLMUnregisterFlightLoopCallback(calc_altimeter,  NULL);
    // Unregister acfutils logging
    log_fini();
    
    // uregister drawing
    drawing_disable();

    // unregister OpenGL drawing
//    texture_fini();

    DestroyImGui();

}


PLUGIN_API int XPluginEnable(void)
{

    drawing_enable();

    StartImGui();

    return 1;
}


PLUGIN_API void XPluginDisable(void)
{
//     XPLMUnregisterFlightLoopCallback(calc_altimeter, NULL);	 //  Don't forget to unload this callback.

    DestroyImGui();

}


PLUGIN_API void XPluginReceiveMessage(XPLMPluginID    inFromWho, int inMessage,  void *           inParam)
{

}


float getAltVal(void* inRefcon)
{
    return altInMetersValue;
}


void setAltVal(void* inRefcon, float inValue)
{
    altInMetersValue = inValue;
}


 //  This single shot FLCB registers our custom dataref in DRE
 float RegaltInMetersInDRE(float elapsedMe, float elapsedSim, int counter, void * refcon)
 {
    float p_alt = XPLMGetDataf(pilotAltFeet);
    float alt_conv = (p_alt * 0.3048);
    XPLMSetDataf(altInMeters, alt_conv);
    
    // conversion flightloop
    units_conv_update();
    
    // drawing flightloop

     return 0.1f;

 }


float altNow;
float altPrev;
float myTimer(float elapsedMe, float elapsedSim, int counter, void * refcon)
{

    altNow = XPLMGetDataf(pilotAltFeet);

    if (altNow >= 100 && altPrev < 100)
    {
        snprintf(myValue_buffer, 256, "[P-180 Avanti II]: Passed 100 ft!\n");
        XPLMDebugString(myValue_buffer);
        logMsg("Passed 100 ft!");
    }
    altPrev = altNow;

    return 0.01f;
}


int OpenLinkHandler(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void * inRefcon)
{
    if (inPhase == 0)
    {
        system("open https://airfightergr.github.io/les_dc3_docs");
        sprintf(logbuff, "Open Link has been pressed\n");
        XPLMDebugString(logbuff);
        logMsg("Linked opened!");
    }

    return 0;
}
