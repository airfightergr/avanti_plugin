//
// Created by ilias on 8/20/23.
//

#ifndef AVANTI_UI_H
#define AVANTI_UI_H

#ifdef __cplusplus
extern "C" {
#endif

void DrawImGui(XPLMDrawCallback_f inCallback, XPLMDrawingPhase inPhase, int inIsBefore, void *inRefcon);
void StartImGui();
void DestroyImGui();

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //AVANTI_UI_H
