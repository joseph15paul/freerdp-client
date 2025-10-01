#include <iostream>
#include <delta_conversion.hxx>

class IRdpClient
{
public:
    virtual ~IRdpClient() {};
    virtual int Setup(const char *host, const char *username, const char *password) = 0;
    virtual int StartAndConnect() = 0;
    virtual int DispatchRdpEvents() = 0;
    virtual uint8_t* ReadDisplay() = 0;
    virtual int GetDisplayWidth() = 0;
    virtual int GetDisplayHeight() = 0;
    virtual int sendInputs() = 0;
    virtual int Stop() = 0;
};

IRdpClient* createRdpClient();