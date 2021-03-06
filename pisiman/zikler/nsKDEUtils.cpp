/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "nsKDEUtils.h"
#include "nsIWidget.h"
#include "nsISupportsPrimitives.h"
#include "nsIMutableArray.h"
#include "nsComponentManagerUtils.h"
#include "nsArrayUtils.h"

#include <gtk/gtk.h>

#include <limits.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <unistd.h>
#include <X11/Xlib.h>

//#define DEBUG_KDE
#ifdef DEBUG_KDE
#define KMOZILLAHELPER "kmozillahelper"
#else
// not need for lib64, it's a binary
#define KMOZILLAHELPER "/usr/lib/mozilla/kmozillahelper"
#endif

#define KMOZILLAHELPER_VERSION 6
#define MAKE_STR2( n ) #n
#define MAKE_STR( n ) MAKE_STR2( n )

static bool getKdeSession()
    {
    Display* dpy = XOpenDisplay( NULL );
    if( dpy == NULL )
        return false;
    Atom kde_full_session = XInternAtom( dpy, "KDE_FULL_SESSION", True );
    bool kde = false;
    if( kde_full_session != None )
        {
        int cnt;
        if( Atom* props = XListProperties( dpy, DefaultRootWindow( dpy ), &cnt ))
            {
            for( int i = 0;
                 i < cnt;
                 ++i )
                {
                if( props[ i ] == kde_full_session )
                    {
                    kde = true;
#ifdef DEBUG_KDE
                    fprintf( stderr, "KDE SESSION %d\n", kde );
#endif
                    break;
                    }
                }
            XFree( props );
            }
        }
    XCloseDisplay( dpy );
    return kde;
    }

static bool getKdeSupport()
    {
    nsTArray<nsCString> command;
    command.AppendElement( NS_LITERAL_CSTRING( "CHECK" ));
    command.AppendElement( NS_LITERAL_CSTRING( MAKE_STR( KMOZILLAHELPER_VERSION )));
    bool kde = nsKDEUtils::command( command );
#ifdef DEBUG_KDE
    fprintf( stderr, "KDE RUNNING %d\n", kde );
#endif
    return kde;
    }

nsKDEUtils::nsKDEUtils()
    : commandFile( NULL )
    , replyFile( NULL )
    {
    }

nsKDEUtils::~nsKDEUtils()
    {
//    closeHelper(); not actually useful, exiting will close the fd too
    }

nsKDEUtils* nsKDEUtils::self()
    {
    static nsKDEUtils s;
    return &s;
    }

static bool helperRunning = false;
static bool helperFailed = false;

bool nsKDEUtils::kdeSession()
    {
    static bool session = getKdeSession();
    return session;
    }

bool nsKDEUtils::kdeSupport()
    {
    static bool support = kdeSession() && getKdeSupport();
    return support && helperRunning;
    }

struct nsKDECommandData
    {
    FILE* file;
    nsTArray<nsCString>* output;
    GMainLoop* loop;
    bool success;
    };

static gboolean kdeReadFunc( GIOChannel*, GIOCondition, gpointer data )
    {
    nsKDECommandData* p = static_cast< nsKDECommandData* >( data );
    char buf[ 8192 ]; // TODO big enough
    bool command_done = false;
    bool command_failed = false;
    while( !command_done && !command_failed && fgets( buf, 8192, p->file ) != NULL )
        { // TODO what if the kernel splits a line into two chunks?
//#ifdef DEBUG_KDE
//        fprintf( stderr, "READ: %s %d\n", buf, feof( p->file ));
//#endif
        if( char* eol = strchr( buf, '\n' ))
            *eol = '\0';
        command_done = ( strcmp( buf, "\\1" ) == 0 );
        command_failed = ( strcmp( buf, "\\0" ) == 0 );
        nsAutoCString line( buf );
        line.ReplaceSubstring( "\\n", "\n" );
        line.ReplaceSubstring( "\\" "\\", "\\" ); //  \\ -> \ , i.e. unescape
        if( p->output && !( command_done || command_failed ))
            p->output->AppendElement( nsCString( buf )); // TODO utf8?
        }
    bool quit = false;
    if( feof( p->file ) || command_failed )
        {
        quit = true;
        p->success = false;
        }
    if( command_done )
        { // reading one reply finished
        quit = true;
        p->success = true;
        }
    if( quit )
        {
        if( p->loop )
            g_main_loop_quit( p->loop );
        return FALSE;
        }
    return TRUE;
    }

bool nsKDEUtils::command( const nsTArray<nsCString>& command, nsTArray<nsCString>* output )
    {
    return self()->internalCommand( command, NULL, false, output );
    }

bool nsKDEUtils::command( nsIArray* command, nsIArray** output)
    {
    nsTArray<nsCString> in;
    PRUint32 length;
    command->GetLength( &length );
    for ( PRUint32 i = 0; i < length; i++ )
        {
        nsCOMPtr<nsISupportsCString> str = do_QueryElementAt( command, i );
        if( str )
            {
            nsAutoCString s;
            str->GetData( s );
            in.AppendElement( s );
            }
        }

    nsTArray<nsCString> out;
    bool ret = self()->internalCommand( in, NULL, false, &out );

    if ( !output ) return ret;

    nsCOMPtr<nsIMutableArray> result = do_CreateInstance( NS_ARRAY_CONTRACTID );
    if ( !result ) return false;

    for ( PRUint32 i = 0; i < out.Length(); i++ )
        {
        nsCOMPtr<nsISupportsCString> rstr = do_CreateInstance( NS_SUPPORTS_CSTRING_CONTRACTID );
        if ( !rstr ) return false;

        rstr->SetData( out[i] );
        result->AppendElement( rstr, false );
        }

    NS_ADDREF( *output = result);
    return ret;
    }


bool nsKDEUtils::commandBlockUi( const nsTArray<nsCString>& command, const GtkWindow* parent, nsTArray<nsCString>* output )
    {
    return self()->internalCommand( command, parent, true, output );
    }

bool nsKDEUtils::internalCommand( const nsTArray<nsCString>& command, const GtkWindow* parent, bool blockUi,
    nsTArray<nsCString>* output )
    {
    if( !startHelper())
        return false;
    feedCommand( command );
    // do not store the data in 'this' but in extra structure, just in case there
    // is reentrancy (can there be? the event loop is re-entered)
    nsKDECommandData data;
    data.file = replyFile;
    data.output = output;
    data.success = false;
    if( blockUi )
        {
        data.loop = g_main_loop_new( NULL, FALSE );
        GtkWidget* window = gtk_window_new( GTK_WINDOW_TOPLEVEL );
        if( parent && parent->group )
            gtk_window_group_add_window( parent->group, GTK_WINDOW( window ));
        gtk_widget_realize( window );
        gtk_widget_set_sensitive( window, TRUE );
        gtk_grab_add( window );
        GIOChannel* channel = g_io_channel_unix_new( fileno( data.file ));
        g_io_add_watch( channel, static_cast< GIOCondition >( G_IO_IN | G_IO_ERR | G_IO_HUP ), kdeReadFunc, &data );
        g_io_channel_unref( channel );
        g_main_loop_run( data.loop );
        g_main_loop_unref( data.loop );
        gtk_grab_remove( window );
        gtk_widget_destroy( window );
        }
    else
        {
        data.loop = NULL;
        while( kdeReadFunc( NULL, static_cast< GIOCondition >( 0 ), &data ))
            ;
        }
    return data.success;
    }

bool nsKDEUtils::startHelper()
    {
    if( helperRunning )
        return true;
    if( helperFailed )
        return false;
    helperFailed = true;
    int fdcommand[ 2 ];
    int fdreply[ 2 ];
    if( pipe( fdcommand ) < 0 )
        return false;
    if( pipe( fdreply ) < 0 )
        {
        close( fdcommand[ 0 ] );
        close( fdcommand[ 1 ] );
        return false;
        }
    char* args[ 2 ] = { const_cast< char* >( KMOZILLAHELPER ), NULL };
    switch( fork())
        {
        case -1:
            {
            close( fdcommand[ 0 ] );
            close( fdcommand[ 1 ] );
            close( fdreply[ 0 ] );
            close( fdreply[ 1 ] );
            return false;
            }
        case 0: // child
            {
            if( dup2( fdcommand[ 0 ], STDIN_FILENO ) < 0 )
                _exit( 1 );
            if( dup2( fdreply[ 1 ], STDOUT_FILENO ) < 0 )
                _exit( 1 );
            int maxfd = 1024; // close all other fds
            struct rlimit rl;
            if( getrlimit( RLIMIT_NOFILE, &rl ) == 0 )
                maxfd = rl.rlim_max;
            for( int i = 3;
                 i < maxfd;
                 ++i )
                close( i );
#ifdef DEBUG_KDE
            execvp( KMOZILLAHELPER, args );
#else
            execv( KMOZILLAHELPER, args );
#endif
            _exit( 1 ); // failed
            }
        default: // parent
            {
            commandFile = fdopen( fdcommand[ 1 ], "w" );
            replyFile = fdopen( fdreply[ 0 ], "r" );
            close( fdcommand[ 0 ] );
            close( fdreply[ 1 ] );
            if( commandFile == NULL || replyFile == NULL )
                {
                closeHelper();
                return false;
                }
            // ok, helper ready, getKdeRunning() will check if it works
            }
        }
    helperFailed = false;
    helperRunning = true;
    return true;
    }

void nsKDEUtils::closeHelper()
    {
    if( commandFile != NULL )
        fclose( commandFile ); // this will also make the helper quit
    if( replyFile != NULL )
        fclose( replyFile );
    helperRunning = false;
    }

void nsKDEUtils::feedCommand( const nsTArray<nsCString>& command )
    {
    for( int i = 0;
         i < command.Length();
         ++i )
        {
        nsCString line = command[ i ];
        line.ReplaceSubstring( "\\", "\\" "\\" ); // \ -> \\ , i.e. escape
        line.ReplaceSubstring( "\n", "\\n" );
#ifdef DEBUG_KDE
        fprintf( stderr, "COMM: %s\n", line.get());
#endif
        fputs( line.get(), commandFile );
        fputs( "\n", commandFile );
        }
    fputs( "\\E\n", commandFile ); // done as \E, so it cannot happen in normal data
    fflush( commandFile );
    }
