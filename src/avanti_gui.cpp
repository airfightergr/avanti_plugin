//
// Created by ilias on 8/20/23.
//

#include <iostream>

#include "../imgui/imgui.h"
#include "../imgui/backends/imgui_impl_opengl3.h"
//#include "../SDK/CHeaders/XPLM/XPLMDisplay.h"
#include "XPLMPlugin.h"
#include "XPLMProcessing.h"
#include "XPLMDataAccess.h"
#include "XPLMUtilities.h"
#include "XPLMDisplay.h"

#include "avanti_gui.h"



#ifdef __cplusplus
extern "C" {
#endif


/// ImGui Initialize call
void StartImGui()
{
    ImGui::CreateContext();///create ImGui Context
    ImGui_ImplOpenGL3_Init();
}

///ImGui callback
void DrawImGui(XPLMDrawCallback_f inCallback, XPLMDrawingPhase inPhase, int inIsBefore, void *inRefcon)
{

   if (inIsBefore) {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();

        // Create and render your ImGui windows here
        ImGui::Begin("Main Window");
        ImGui::Text("Welcome to ImGui inside X-Plane!");
        ImGui::Text("testing.....");
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
}


///ImGui Destroy ImGui
void DestroyImGui()
{

    ImGui_ImplOpenGL3_Shutdown();
    ImGui::DestroyContext();
}


//}

#ifdef __cplusplus
}
#endif //__cplusplus