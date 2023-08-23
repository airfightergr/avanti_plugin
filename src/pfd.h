//
// Created by ilias on 6/25/23.
//

#ifndef AVANTI_PFD_H
#define AVANTI_PFD_H


void drawing_init(void);
void drawing_enable(void);
void drawing_disable(void);
static void display_render_cb(cairo_t *cr, unsigned w, unsigned h, void *data);
static int draw_loop(XPLMDrawingPhase phase, int is_before, void *refcon);
void drawing_receiveMessage(void);

extern char acf_dir_path[1024];

#endif //AVANTI_PFD_H