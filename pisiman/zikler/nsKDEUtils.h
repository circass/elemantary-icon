/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef nsKDEUtils_h__
#define nsKDEUtils_h__

#include "nsStringGlue.h"
#include "nsTArray.h"
#include <stdio.h>

typedef struct _GtkWindow GtkWindow;

class nsIArray;

class NS_EXPORT nsKDEUtils
    {
    public:
        /* Returns true if running inside a KDE session (regardless of whether there is KDE
           support available for Firefox). This should be used e.g. when determining
           dialog button order but not for code that requires the KDE support. */
        static bool kdeSession();
        /* Returns true if running inside a KDE session and KDE support is available
           for Firefox. This should be used everywhere where the external helper is needed. */
        static bool kdeSupport();
        /* Executes the given helper command, returns true if helper returned success. */
        static bool command( const nsTArray<nsCString>& command, nsTArray<nsCString>* output = NULL );
        static bool command( nsIArray* command, nsIArray** output = NULL );
        /* Like command(), but additionally blocks the parent widget like if there was
           a modal dialog shown and enters the event loop (i.e. there are still paint updates,
           this is for commands that take long). */
        static bool commandBlockUi( const nsTArray<nsCString>& command, const GtkWindow* parent, nsTArray<nsCString>* output = NULL );

    private:
        nsKDEUtils();
        ~nsKDEUtils();
        static nsKDEUtils* self();
        bool startHelper();
        void closeHelper();
        void feedCommand( const nsTArray<nsCString>& command );
        bool internalCommand( const nsTArray<nsCString>& command, const GtkWindow* parent, bool isParent,
            nsTArray<nsCString>* output );
        FILE* commandFile;
        FILE* replyFile;
    };

#endif // nsKDEUtils
