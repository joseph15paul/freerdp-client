#include <cstring>
#include <iostream>
#include <freerdp/freerdp.h>
#include <freerdp/client/channels.h>
#include <ostream>
#include <winpr/wtypes.h>
#include <winpr/crt.h>
#include <winpr/wlog.h>
#include <freerdp/settings.h>

BOOL MyAuthenticateEx(freerdp* inst,
                    char** username, char** password, char** domain,
                    rdp_auth_reason reason){
    std::cout<<*username<<"  "<<*password<<" \tex called: reason"<<" \n";
    return TRUE;
}

static DWORD MyVerifyCertEx(freerdp* inst,
                           const char* host, UINT16 port,
                           const char* common_name, const char* subject,
                           const char* issuer, const char* fingerprint,
                           DWORD flags)
{
    // Automatically accept the certificate (no UI prompt)
    return 1;  // Accept permanently
}

int main(int argc, char* argv[]) {
    wLog* logger = WLog_Get("my.client");
    WLog_SetLogLevel(logger, WLOG_TRACE);
    WLog_SetLogLevel(WLog_Get("com.freerdp.client"), WLOG_TRACE);
    WLog_SetLogLevel(WLog_Get("com.freerdp.core"), WLOG_TRACE);
    WLog_SetLogLevel(WLog_Get("com.winpr"), WLOG_TRACE);
    WLog_SetLogLevel(WLog_Get("com.freerdp.core.transport"), WLOG_TRACE);
    WLog_SetLogLevel(WLog_Get("com.freerdp.core.nego"), WLOG_TRACE);
    WLog_SetLogLevel(WLog_Get("com.freerdp.crypto"), WLOG_TRACE);

    std::cout<<"starting client"<<std::endl;
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <host> <username> [password]\n";
        return 1;
    }

    const char* server = argv[1];
    const char* username = argv[2];
    const char* password = (argc >= 4) ? argv[3] : "";

    freerdp* instance = freerdp_new();
    if (!instance) {
        std::cerr << "Failed to create freerdp instance\n";
        return 1;
    }

    instance->ContextSize = sizeof(rdpContext);
    instance->ContextNew = [](freerdp* inst, rdpContext* ctx) -> BOOL {
        std::cout << "Context initialized\n";
        return TRUE;
    };
    instance->ContextFree = [](freerdp* inst, rdpContext* ctx) {
        std::cout << "Context freed\n";
    };

    if (!freerdp_context_new(instance)) {
        std::cerr << "freerdp_context_new failed\n";
        freerdp_free(instance);
        return 1;
    }
    
    rdpContext* cont = NULL;
    cont = instance->context;
    cont->instance = instance;

    // Set connection parameters
    rdpSettings* settings = instance->context->settings;
    settings->ServerHostname = strdup("127.0.0.1");
    settings->Username = strdup("user");
    settings->Password = strdup("root");
    settings->ServerPort = 3389;
    settings->Domain = strdup("");

    settings->RdpSecurity = FALSE;
    settings->TlsSecurity = TRUE;
    settings->NlaSecurity = FALSE;
    settings->SendPreconnectionPdu = TRUE;
    settings->UseRdpSecurityLayer = TRUE;
    settings->AutoDenyCertificate = FALSE;
    settings->AutoAcceptCertificate = TRUE;
    settings->SmartcardLogon = FALSE;
    settings->NegotiateSecurityLayer = FALSE;
    settings->RemoteAssistanceRequestControl = FALSE;
    settings->PasswordIsSmartcardPin = FALSE;
    settings->RedirectSmartCards = TRUE;
    settings->DeviceRedirection = TRUE;
    instance->AuthenticateEx        = MyAuthenticateEx;
    instance->VerifyCertificateEx   = MyVerifyCertEx;
    settings->ExternalCertificateManagement = TRUE;
    settings->Authentication = FALSE;
    settings->AutoLogonEnabled = TRUE;
    settings->IgnoreCertificate = TRUE;  // Don't do this in production

    // Connect
    if (!freerdp_connect(instance)) {
        std::cerr << "Connection failed.\n";
        std::cerr << "Connection failed: " << " [" << std::hex << freerdp_get_last_error(instance->context) << "]\n";
        
        freerdp_context_free(instance);
        freerdp_free(instance);
        return 1;
    }

    std::cout << "Connected successfully to " << server << "\n";

    while (freerdp_check_fds(instance)) {
        // In a real app, you would handle RDP output (draw callbacks, etc.) here.
        // This loop continues until the connection closes or an error occurs.
        std::cout<< "checking fds \n";
    }

    // Run main loop (normally you'd drive the session here)
    freerdp_disconnect(instance);

    // Cleanup
    freerdp_context_free(instance);
    freerdp_free(instance);
    return 0;
}

