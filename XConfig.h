#ifndef __XCONFIG_H
#define __XCONFIG_H

#include <X11/Xlib.h>

typedef struct
    { Display * dpy
    ; Window    root
    ;
    } XConfig;

int initXConfig ( XConfig * xconfig );

void finalizeXConfig ( XConfig * xconfig );

int checkXConfig ( XConfig * xconfig );

#endif
