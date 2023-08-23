//
// Created by ilias on 7/29/23.
//

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "acfutils/log.h"

#include "XPLMUtilities.h"

static char xpPath[512];
static char logfile[1024];

char avantiLog[256];

void appendLog(void)
{

    FILE *fp;

    XPLMGetSystemPath(xpPath);

    strcpy(logfile, "Aircraft/X-Aviation/P180_Avanti_II/resources/data/avanti.log");

    strcat(logfile, xpPath);

    assert(logfile == NULL);

    fp = fopen(logfile, "a");

    fprintf(fp, "[FILE_IO]: Opened online manual");

    fclose(fp);

}
