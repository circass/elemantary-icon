/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "nsCommonRegistry.h"

#include "nsGNOMERegistry.h"
#include "nsKDERegistry.h"
#include "nsString.h"
#include "nsVoidArray.h"
#include "nsKDEUtils.h"

/* static */ bool
nsCommonRegistry::HandlerExists(const char *aProtocolScheme)
{
    if( nsKDEUtils::kdeSupport())
        return nsKDERegistry::HandlerExists( aProtocolScheme );
    return nsGNOMERegistry::HandlerExists( aProtocolScheme );
}

/* static */ nsresult
nsCommonRegistry::LoadURL(nsIURI *aURL)
{
    if( nsKDEUtils::kdeSupport())
        return nsKDERegistry::LoadURL( aURL );
    return nsGNOMERegistry::LoadURL( aURL );
}

/* static */ void
nsCommonRegistry::GetAppDescForScheme(const nsACString& aScheme,
                                     nsAString& aDesc)
{
    if( nsKDEUtils::kdeSupport())
        return nsKDERegistry::GetAppDescForScheme( aScheme, aDesc );
    return nsGNOMERegistry::GetAppDescForScheme( aScheme, aDesc );
}


/* static */ already_AddRefed<nsMIMEInfoBase>
nsCommonRegistry::GetFromExtension(const nsACString& aFileExt)
{
    if( nsKDEUtils::kdeSupport())
        return nsKDERegistry::GetFromExtension( aFileExt );
    return nsGNOMERegistry::GetFromExtension( aFileExt );
}

/* static */ already_AddRefed<nsMIMEInfoBase>
nsCommonRegistry::GetFromType(const nsACString& aMIMEType)
{
    if( nsKDEUtils::kdeSupport())
        return nsKDERegistry::GetFromType( aMIMEType );
    return nsGNOMERegistry::GetFromType( aMIMEType );
}
