#include "XConfig.h"

int initXConfig ( XConfig * xconfig ) {

    int ret = 0;

    if ( xconfig == NULL ) { ret = -1; goto exit; }
    if ( NULL == ( xconfig->dpy = XOpenDisplay ("") ) ) { ret = -1; goto exit; }
    xconfig->root = DefaultRootWindow ( xconfig->dpy );

exit:
    return ret;

}

void finalizeXConfig ( XConfig * xconfig ) {
    if ( xconfig != NULL && xconfig->dpy != NULL ) {
        XCloseDisplay ( xconfig->dpy );
    }
}

int checkXConfig ( XConfig * xconfig ) {
    if ( xconfig == NULL ) {
        return -1;
    } else if ( xconfig->dpy == NULL ) {
        return -1;
    } else {
        return 0;
    }
}
