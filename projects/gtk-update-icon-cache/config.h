#pragma once
#include "../../gtk2-ng/config.h.win32.in"

// All this stuff is a workaround for a problem where the icon cache header was getting truncated. 
// Instead using g_printf and using redirection to write the file it will directly write to file using fprintf
// MSBUILD seems to be doing something weird with redirections
#include <stdio.h>
#include <string.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <libintl.h>
#undef fprintf
static FILE* outfile = NULL;
#define g_printf(format, ...) do{											\
if(!outfile) {														\
	char*name = getenv("ICONCACHE_HEADER_NAME");							\
	if(name) outfile = fopen(name, "w");				\
	if(outfile) fprintf(stderr,"Writing header to %s",name);	\
	else outfile = stdout;											\
}																	\
fprintf(outfile, format __VA_OPT__ (,) __VA_ARGS__);								\
if(!strcmp(format,"0x%02x\n};\n")){\
fclose(outfile);\
outfile = NULL;\
}} while(0)

