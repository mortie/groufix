#ifndef GFX_GILIA_H
#define GFX_GILIA_H

#include <stdbool.h>
#include <stdio.h>

bool gfx_has_gilia(void);
bool gfx_gilia_execute(const char *str);
bool gfx_gilia_execute_file(FILE *f);

#endif
