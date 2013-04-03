/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "nsIURI.h"
#include "nsCOMPtr.h"
#include "nsTArray.h"

class nsMIMEInfoBase;
class nsAutoCString;
class nsCString;

class nsKDERegistry
{
 public:
  static bool HandlerExists(const char *aProtocolScheme);

  static nsresult LoadURL(nsIURI *aURL);

  static void GetAppDescForScheme(const nsACString& aScheme,
                                  nsAString& aDesc);

  static already_AddRefed<nsMIMEInfoBase> GetFromExtension(const nsACString& aFileExt);

  static already_AddRefed<nsMIMEInfoBase> GetFromType(const nsACString& aMIMEType);
 private:
  static already_AddRefed<nsMIMEInfoBase> GetFromHelper(const nsTArray<nsCString>& command);

};
