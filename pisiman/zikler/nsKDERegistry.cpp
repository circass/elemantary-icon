/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "nsKDERegistry.h"
#include "prlink.h"
#include "prmem.h"
#include "nsString.h"
#include "nsILocalFile.h"
#include "nsMIMEInfoUnix.h"
#include "nsAutoPtr.h"
#include "nsKDEUtils.h"

/* static */ bool
nsKDERegistry::HandlerExists(const char *aProtocolScheme)
{
    nsTArray<nsCString> command;
    command.AppendElement( NS_LITERAL_CSTRING( "HANDLEREXISTS" ));
    command.AppendElement( nsAutoCString( aProtocolScheme ));
    return nsKDEUtils::command( command );
}

/* static */ nsresult
nsKDERegistry::LoadURL(nsIURI *aURL)
{
    nsTArray<nsCString> command;
    command.AppendElement( NS_LITERAL_CSTRING( "OPEN" ));
    nsCString url;
    aURL->GetSpec( url );
    command.AppendElement( url );
    bool rv = nsKDEUtils::command( command );
    if (!rv)
      return NS_ERROR_FAILURE;

    return NS_OK;
}

/* static */ void
nsKDERegistry::GetAppDescForScheme(const nsACString& aScheme,
                                     nsAString& aDesc)
{
    nsTArray<nsCString> command;
    command.AppendElement( NS_LITERAL_CSTRING( "GETAPPDESCFORSCHEME" ));
    command.AppendElement( aScheme );
    nsTArray<nsCString> output;
    if( nsKDEUtils::command( command, &output ) && output.Length() == 1 )
        CopyUTF8toUTF16( output[ 0 ], aDesc );
}


/* static */ already_AddRefed<nsMIMEInfoBase>
nsKDERegistry::GetFromExtension(const nsACString& aFileExt)
{
    NS_ASSERTION(aFileExt[0] != '.', "aFileExt shouldn't start with a dot");
    nsTArray<nsCString> command;
    command.AppendElement( NS_LITERAL_CSTRING( "GETFROMEXTENSION" ));
    command.AppendElement( aFileExt );
    return GetFromHelper( command );
}

/* static */ already_AddRefed<nsMIMEInfoBase>
nsKDERegistry::GetFromType(const nsACString& aMIMEType)
{
    nsTArray<nsCString> command;
    command.AppendElement( NS_LITERAL_CSTRING( "GETFROMTYPE" ));
    command.AppendElement( aMIMEType );
    return GetFromHelper( command );
}

/* static */ already_AddRefed<nsMIMEInfoBase>
nsKDERegistry::GetFromHelper(const nsTArray<nsCString>& command)
{
    nsTArray<nsCString> output;
    if( nsKDEUtils::command( command, &output ) && output.Length() == 3 )
        {
        nsCString mimetype = output[ 0 ];
        nsRefPtr<nsMIMEInfoUnix> mimeInfo = new nsMIMEInfoUnix( mimetype );
        NS_ENSURE_TRUE(mimeInfo, nullptr);
        nsCString description = output[ 1 ];
        mimeInfo->SetDescription(NS_ConvertUTF8toUTF16(description));
        nsCString handlerAppName = output[ 2 ];
        mimeInfo->SetDefaultDescription(NS_ConvertUTF8toUTF16(handlerAppName));
        mimeInfo->SetPreferredAction(nsIMIMEInfo::useSystemDefault);
        nsMIMEInfoBase* retval;
        NS_ADDREF((retval = mimeInfo));
        return retval;
        }
    return nullptr;
}
