/////////////////////////////////////////////////////////////////////////////
// Name:        wx/dde.h
// Purpose:     DDE base header
// Author:      Julian Smart
// Modified by:
// Created:
// Copyright:   (c) Julian Smart
// RCS-ID:      $Id$
// Licence:     wxWindows Licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_DDE_H_BASE_
#define _WX_DDE_H_BASE_

#include "wx/list.h"

class  wxDDEClient;
class  wxDDEServer;
class  wxDDEConnection;

WX_DECLARE_USER_EXPORTED_LIST(wxDDEClient, wxDDEClientList, );
WX_DECLARE_USER_EXPORTED_LIST(wxDDEServer, wxDDEServerList, );
WX_DECLARE_USER_EXPORTED_LIST(wxDDEConnection, wxDDEConnectionList, );

#if defined(__WINDOWS__)
    #include "wx/msw/dde.h"
#else
    #error DDE is only supported under Windows
#endif

#endif
    // _WX_DDE_H_BASE_
