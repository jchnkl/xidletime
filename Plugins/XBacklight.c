#include "XBacklight.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include <xcb/xcb.h>
#include <xcb/randr.h>

#include "XIdleTimer.h"

typedef struct BacklightStateT
    { uint noutputs
    ; double * last, * min, * cur, * new, * set
    ; int steps, total_time
    ; xcb_atom_t backlight
    ; xcb_randr_output_t * outputs
    ; pthread_t tid
    ;
    } BacklightStateT;

static BacklightStateT blState;

xcb_atom_t getBacklightAtom ( xcb_connection_t * c ) {
    xcb_intern_atom_cookie_t blcookie;
    xcb_atom_t backlight = XCB_ATOM_NONE;

    blcookie = xcb_intern_atom ( c
                               , 1
                               , strlen ( "Backlight" )
                               , "Backlight"
                               );

    xcb_intern_atom_reply_t * reply = xcb_intern_atom_reply ( c
                                                            , blcookie
                                                            , NULL
                                                            );

    if ( reply == NULL || reply->atom == XCB_ATOM_NONE ) {
        blcookie = xcb_intern_atom ( c
                                   , 1
                                   , strlen ( "BACKLIGHT" )
                                   , "BACKLIGHT"
                                   );

        free ( reply );
        reply = xcb_intern_atom_reply ( c, blcookie, NULL );
    }

    if ( reply != NULL ) backlight = reply->atom;

    free ( reply );

    return backlight;
}

xcb_randr_get_output_property_reply_t *
getOutputProp ( xcb_connection_t * c, xcb_randr_output_t output ) {
    xcb_randr_get_output_property_reply_t *prop_reply = NULL;
    xcb_randr_get_output_property_cookie_t prop_cookie;

    if ( blState.backlight != XCB_ATOM_NONE ) {
        prop_cookie = xcb_randr_get_output_property ( c
                                                    , output
                                                    , blState.backlight
                                                    , XCB_ATOM_NONE
                                                    , 0
                                                    , 4
                                                    , 0
                                                    , 0
                                                    );
        prop_reply =
            xcb_randr_get_output_property_reply ( c, prop_cookie, NULL );
    }

    return prop_reply;
}

long getBacklight ( xcb_connection_t * c, xcb_randr_output_t output ) {
    long value = 0;
    xcb_randr_get_output_property_reply_t *prop_reply = NULL;

    prop_reply = getOutputProp ( c, output );

    if ( prop_reply == NULL
      || prop_reply->type != XCB_ATOM_INTEGER
      || prop_reply->num_items != 1
      || prop_reply->format != 32
       ) {
        value = -1;
    } else {
        value += *(xcb_randr_get_output_property_data ( prop_reply ));
    }

    free ( prop_reply );

    return value;
}

void setBacklight ( xcb_connection_t * c
                  , xcb_randr_output_t output
                  , long value
                  ) {
    xcb_randr_change_output_property ( c
                                     , output
                                     , blState.backlight
                                     , XCB_ATOM_INTEGER
                                     , 32
                                     , XCB_PROP_MODE_REPLACE
                                     , 1
                                     , (unsigned char *)&value
                                     );
}

static void * dimBacklight ( void * p ) {
    int i, j;
    double step;
    double new[blState.noutputs];

    for ( i = 0; i < blState.noutputs; ++i ) {

        if ( blState.set[i] == blState.min[i] ) {
            new[i] = blState.min[i] + blState.set[i];
            step = (new[i] - blState.cur[i]) / blState.steps;

            for ( j = 0; j < blState.steps && step != 0; ++j ) {

                if (j == blState.steps - 1) {
                    blState.cur[i] = new[i];
                } else {
                    blState.cur[i] += step;
                }

                setBacklight ( (xcb_connection_t *)p
                             , blState.outputs[i]
                             , (long) blState.cur[i]
                             );
                xcb_flush ( (xcb_connection_t *)p );

                // usleep is a cancellation point
                usleep ( blState.total_time * 1000 / blState.steps );
            }

        } else {
            setBacklight ( (xcb_connection_t *)p
                         , blState.outputs[i]
                         , (long)blState.set[i]
                         );
            xcb_flush ( (xcb_connection_t *)p );
        }
    }

    xcb_aux_sync ( (xcb_connection_t *)p );

    return NULL;
}

void xBacklightSink ( EventSinkT * snk, EventSourceT * src ) {

    int i;

    if ( src->id != 1 ) return;

    if ( snk->private == NULL ) {
        memset ( &blState, 0, sizeof ( BacklightStateT ) );
        snk->private = (void *)&blState;

        blState.steps = 15;
        blState.total_time = 1000;

        blState.backlight = getBacklightAtom ( snk->public->c );

        xcb_screen_iterator_t iter =
            xcb_setup_roots_iterator ( xcb_get_setup ( snk->public->c ) );

        while ( iter.rem ) {
            xcb_screen_t * screen = iter.data;
            xcb_window_t root = screen->root;
            xcb_randr_output_t * outputs;

            xcb_randr_get_screen_resources_cookie_t  resources_cookie;
            xcb_randr_get_screen_resources_reply_t * resources_reply;

            resources_cookie =
                xcb_randr_get_screen_resources ( snk->public->c, root );
            resources_reply =
                xcb_randr_get_screen_resources_reply ( snk->public->c
                                                     , resources_cookie
                                                     , NULL
                                                     );

            if (resources_reply == NULL) continue;

            outputs = xcb_randr_get_screen_resources_outputs ( resources_reply );

            for ( i = 0; i < resources_reply->num_outputs; i++ ) {
                double cur = getBacklight ( snk->public->c, outputs[i] );
                if (cur != -1) {
                    ++(blState.noutputs);
                    blState.outputs = realloc ( blState.outputs
                                              , sizeof ( xcb_randr_output_t )
                                                  * blState.noutputs
                                              );
                    blState.outputs[blState.noutputs - 1] = outputs[i];

                    blState.last = realloc ( blState.last
                                           , sizeof ( double )
                                               * blState.noutputs
                                           );
                    blState.last[blState.noutputs - 1] = cur;

                    blState.cur = realloc ( blState.cur
                                           , sizeof ( pthread_t )
                                               * blState.noutputs
                                           );

                    blState.new = realloc ( blState.new
                                           , sizeof ( pthread_t )
                                               * blState.noutputs
                                           );

                    blState.set = realloc ( blState.set
                                           , sizeof ( pthread_t )
                                               * blState.noutputs
                                           );

                    blState.min = realloc ( blState.min
                                           , sizeof ( pthread_t )
                                               * blState.noutputs
                                           );

                    xcb_randr_query_output_property_cookie_t  prop_cookie;
                    xcb_randr_query_output_property_reply_t * prop_reply;

                    prop_cookie =
                        xcb_randr_query_output_property ( snk->public->c
                                                        , blState.outputs[i]
                                                        , blState.backlight
                                                        );

                    prop_reply =
                        xcb_randr_query_output_property_reply ( snk->public->c
                                                              , prop_cookie
                                                              , NULL
                                                              );

                    if ( prop_reply == NULL )
                        blState.min[blState.noutputs - 1] = 0;

                    if ( prop_reply->range
                            && 2 == xcb_randr_query_output_property_valid_values_length ( prop_reply )
                       ) {
                        int32_t * values =
                            xcb_randr_query_output_property_valid_values ( prop_reply );
                        blState.min[blState.noutputs - 1] = values[0];
                    }

                    free ( prop_reply );

                }
            }

            free ( resources_reply );
            xcb_screen_next (&iter);
        }
        xcb_aux_sync ( snk->public->c );

    } else {
        for ( i = 0; i < blState.noutputs; ++i ) {
            if ( 0 == pthread_cancel ( blState.tid ) ) {
                pthread_join ( blState.tid, NULL );
            }
        }

        for ( i = 0; i < blState.noutputs; ++i ) {
            setBacklight ( snk->public->c
                         , blState.outputs[i]
                         , (long) blState.last[i]
                         );
        }
        xcb_flush ( snk->public->c );
    }

    for ( i = 0; i < blState.noutputs; ++i ) {

        blState.cur[i] = getBacklight ( snk->public->c, blState.outputs[i] );

        if ( Idle == ((XTimerT *)src->private)->status ) {
            blState.last[i] = blState.cur[i];
            blState.set[i] = blState.min[i];
        } else if ( blState.cur[i] > blState.min[i] ) { // && Reset
            blState.set[i] = blState.cur[i];
        } else {
            blState.set[i] = blState.last[i];
        }

    }

    pthread_create ( &(blState.tid)
                   , NULL
                   , dimBacklight
                   , (void *)(snk->public->c)
                   );

    xcb_aux_sync ( snk->public->c );
}
