#include <iostream>
#include <freerdp/freerdp.h>

#include <freerdp/client/cliprdr.h>
#include <freerdp/client/channels.h>

#include <freerdp/utils/signal.h>
#include <winpr/wlog.h>

#include <freerdp/channels/audin.h>
#include <freerdp/channels/echo.h>
#include <freerdp/channels/disp.h>
#include <freerdp/channels/rdpear.h>
#include <freerdp/channels/remdesk.h>
#include <freerdp/channels/drdynvc.h>
#include <freerdp/channels/rdpsnd.h>

class IRdpClient
{
public:
    virtual ~IRdpClient() {};
    virtual int Setup(std::string host, std::string username, std::string password) = 0;
    virtual int StartAndConnect() = 0;
    virtual int DispatchRdpEvents() = 0;
    virtual int Stop() = 0;
}