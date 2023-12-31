//
// Created by ilias on 6/25/23.
//

#include <string.h>
#include <stdbool.h>

#include <XPLMUtilities.h>
#include <XPLMGraphics.h>
#include <XPLMDataAccess.h>
#include <XPLMDisplay.h>


#include <acfutils/glew.h>
#include <acfutils/log.h>
#include <acfutils/mt_cairo_render.h>
#include "acfutils/assert.h"
#include <acfutils/core.h>
#include <acfutils/helpers.h>
#include <acfutils/sysmacros.h>

#include "pfd.h"

//#if APL
//#if defined(_MACH_)
//#include <Carbon/Carbon.h>
//#endif
//#endif
//#if IBM
//#include <windows.h>
//#endif

#define PANEL_X 0 //(575)
#define PANEL_Y 0 //(442)

#define DISPLAY_FPS (25)

#define DISPLAY_W (1024)
#define DISPLAY_H (1024)

static mt_cairo_render_t *render = NULL;

// acf relative path
static XPLMDataRef acf_path_dref = NULL;
char acf_dir_path[1024] = {0};
const char *acf_path = acf_dir_path;

// HDG dataref
static XPLMDataRef plt_hdg_ahars;

// The cairo drawing callback. This is called in the background thread by
// mt_cairo_render_t.
static void display_render_cb(cairo_t *cr, unsigned w, unsigned h, void *data) {
    // In a real use scenario, this would be a pointer to whatever data you need to
    // have for drawing. Here, we don't *really* care
    UNUSED(data);

    // This clears the surface, so we have a clean slate to work from
    cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
    cairo_paint(cr);
    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

    // Draw some stuff!

    cairo_set_line_width(cr, 8);

    cairo_set_source_rgb(cr, 1.0, 0.4, 1);      // color
    cairo_arc(cr, 63 + (w/2), 70 + (h/2), 350, 0, 2 * M_PI); //arc
    cairo_stroke(cr);   // use this to paint the arc. If not, will be connected to the next one, by a line

    // draw another one?
    cairo_set_source_rgb(cr, 1.0, 0.4, 1.0);
    cairo_arc(cr, 63 + (w/2), 70 + (h/2), 175, 0.0f, 2 * M_PI);     //angles in randians
    cairo_stroke(cr);

    cairo_move_to(cr, 575, 419); // move to the center of the rose on x->
    cairo_line_to(cr, 575, 395);
    cairo_stroke(cr);

    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, 55.0);

    cairo_text_extents_t extents;   // init extents, the width of the text

    int hdg = (int) XPLMGetDataf(plt_hdg_ahars); // read the heading (float) and cast it to int
    char headg[4];
    snprintf(headg, 4,  "%03d", hdg);

    cairo_text_extents(cr, headg, &extents);    // find the width and store to extents

    cairo_move_to(cr, 575 - extents.width/2, 200);
    cairo_text_path(cr, headg);
    cairo_fill(cr);
}

// Here we draw the cairo-generated image onto the panel texture.
//
// You do not have to have a draw loop -- you could for example draw on a window (using the
// window's draw callback)
static int draw_loop(XPLMDrawingPhase phase, int is_before, void *refcon) {
    UNUSED(phase);
    UNUSED(is_before);
    UNUSED(refcon);

    // This is the coordinates at which we want to draw on the panel texture.
    // !!warning!! This is OpenGL coordinates, so from the bottom of the texture
    vect2_t panel_loc = VECT2(PANEL_X, PANEL_Y);

    mt_cairo_render_draw(render, panel_loc, VECT2(DISPLAY_W, DISPLAY_H));
    return 1;
}


void drawing_init(void)
{
    glewInit();
    mt_cairo_render_glob_init(false);
}


void drawing_enable(void)
{
    // Create our actual renderer
    render = mt_cairo_render_init(
            DISPLAY_W, DISPLAY_H,           // The size of our renderer/display
            DISPLAY_FPS,
            NULL, display_render_cb, NULL,  // callback. init, render, fini. only render is required
            NULL                            // arbitrary data pointer, passed to callbacks
    );

    // Fire up a display loop to draw our rendered image on the panel
    XPLMRegisterDrawCallback(draw_loop, xplm_Phase_Gauges, 1, NULL);

    // testing logging via acfutils
    acf_path_dref = XPLMFindDataRef("sim/aircraft/view/acf_relative_path");
    int ap = XPLMGetDatab(acf_path_dref, acf_dir_path, 0, sizeof (acf_dir_path));
    char *log_path = "resources/data/avanti/log";
    char *path = mkpathname(acf_dir_path, &log_path, NULL);

    logMsg("Aircraft Path: %s", acf_dir_path);

    // get hdg dref
    plt_hdg_ahars = XPLMFindDataRef("sim/cockpit2/gauges/indicators/heading_AHARS_deg_mag_pilot");
}


void drawing_disable(void)
{
    XPLMUnregisterDrawCallback(draw_loop, xplm_Phase_Gauges, 1, NULL);
    mt_cairo_render_fini(render);
    render = NULL;
}


void drawing_receiveMessage(void)
{
//    UNUSED(from);
//    UNUSED(msg);
//    UNUSED(param);
}
