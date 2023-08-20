//
// Created by ilias on 7/12/23.
//


#include "XPLMPlugin.h"
#include "XPLMDisplay.h"
#include "XPLMGraphics.h"
#include "XPLMProcessing.h"
#include "XPLMDataAccess.h"
#include "XPLMMenus.h"
#include "XPLMUtilities.h"
#include "XPWidgets.h"
#include "XPStandardWidgets.h"
#include "XPLMScenery.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

char logbuff[256];

XPLMCommandRef OpenLink = NULL;

int OpenLinkHandler(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void * inRefcon);

void commands_init()
{
    OpenLink = XPLMCreateCommand("avanti/cmd/openLink", "Open Web Page");
}

void commands_disable()
{
    XPLMUnregisterCommandHandler(OpenLink, OpenLinkHandler, 0, 0);

}

void openLink(){
    system("xdg-open https://airfightergr.github.io/les_dc3_docs/");
    sprintf(logbuff, "Open Link has been pressed");
    XPLMDebugString(logbuff);
}
//
//int OpenLinkHandler(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void * inRefcon)
//{
//    if (inPhase == 0)
//    {
//        system("xdg-open https://airfightergr.github.io/les_dc3_docs/");
//    }
//    return 0;
//}
