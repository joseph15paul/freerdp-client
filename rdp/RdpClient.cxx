#include "RdpClient.hxx"
#include <unordered_map>
#define TAG CLIENT_TAG("Rdp-client")
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
typedef struct
{
    rdpClientContext common;

    /* Channels */
} tfContext;

typedef struct
{
    FreeRDP_Settings_Keys_Bool settingId;
    const char *channelName;
    void *args;
} ChannelToLoad;

UINT16 ascii_to_scancode(const char c)
{
    switch (tolower(c))
    {
    case 'a':
        return 0x1E;
    case 'b':
        return 0x30;
    case 'c':
        return 0x2E;
    case 'd':
        return 0x20;
    case 'e':
        return 0x12;
    case 'f':
        return 0x21;
    case 'g':
        return 0x22;
    case 'h':
        return 0x23;
    case 'i':
        return 0x17;
    case 'j':
        return 0x24;
    case 'k':
        return 0x25;
    case 'l':
        return 0x26;
    case 'm':
        return 0x32;
    case 'n':
        return 0x31;
    case 'o':
        return 0x18;
    case 'p':
        return 0x19;
    case 'q':
        return 0x10;
    case 'r':
        return 0x13;
    case 's':
        return 0x1F;
    case 't':
        return 0x14;
    case 'u':
        return 0x16;
    case 'v':
        return 0x2F;
    case 'w':
        return 0x11;
    case 'x':
        return 0x2D;
    case 'y':
        return 0x15;
    case 'z':
        return 0x2C;
    case '0':
        return 0x0B;
    case '1':
        return 0x02;
    case '2':
        return 0x03;
    case '3':
        return 0x04;
    case '4':
        return 0x05;
    case '5':
        return 0x06;
    case '6':
        return 0x07;
    case '7':
        return 0x08;
    case '8':
        return 0x09;
    case '9':
        return 0x0A;
    case '\n':
        return RDP_SCANCODE_RETURN; // Enter
    case '\t':
        return RDP_SCANCODE_TAB; // TAB
    default:
        return 0; // unsupported char
    }
}

// void handleMousePayload(rdpInput *input, const nlohmann::json &data)
// {
// if (!input)
// 	return;

// auto type = static_cast<MouseEventType>(data["eventType"].get<int>());
// auto button = static_cast<MouseButton>(data["button"].get<int>());
// int x = data["x"];
// int y = data["y"];

// UINT16 flags = 0;

// switch (type)
// {
// case MouseEventType::Move:
// 	flags = PTR_FLAGS_MOVE;
// 	break;

// case MouseEventType::Down:
// 	switch (button)
// 	{
// 	case MouseButton::Left:
// 		flags = PTR_FLAGS_DOWN | PTR_FLAGS_BUTTON1;
// 		break;
// 	case MouseButton::Right:
// 		flags = PTR_FLAGS_DOWN | PTR_FLAGS_BUTTON2;
// 		break;
// 	case MouseButton::Middle:
// 		flags = PTR_FLAGS_DOWN | PTR_FLAGS_BUTTON3;
// 		break;
// 	default:
// 		return;
// 	}
// 	break;

// case MouseEventType::Up:
// 	switch (button)
// 	{
// 	case MouseButton::Left:
// 		flags = PTR_FLAGS_BUTTON1;
// 		break;
// 	case MouseButton::Right:
// 		flags = PTR_FLAGS_BUTTON2;
// 		break;
// 	case MouseButton::Middle:
// 		flags = PTR_FLAGS_BUTTON3;
// 		break;
// 	default:
// 		return;
// 	}
// 	break;

// case MouseEventType::Wheel:
// {
// 	// int delta = data.value("delta", 0);
// 	// flags = PTR_FLAGS_WHEEL;
// 	// delta = std::clamp(delta, -255, 255);
// 	// freerdp_input_send_mouse_event_ex(input, flags, x, y, delta);
// 	return;
// }
// }

// freerdp_input_send_mouse_event(input, flags, x, y);
// }
std::unordered_map<int, UINT16> keyCodeToScanCode = {
    {8, 0x0E},  // Backspace
    {9, 0x0F},  // Tab
    {13, 0x1C}, // Enter
    {16, 0x2A}, // Shift
    {17, 0x1D}, // Ctrl
    {18, 0x38}, // Alt
    {27, 0x01}, // Escape
    {32, 0x39}, // Space

    {37, 0x4B}, // Left Arrow
    {38, 0x48}, // Up Arrow
    {39, 0x4D}, // Right Arrow
    {40, 0x50}, // Down Arrow

    // Letters A–Z
    {65, 0x1E}, // A
    {66, 0x30}, // B
    {67, 0x2E}, // C
    {68, 0x20}, // D
    {69, 0x12}, // E
    {70, 0x21}, // F
    {71, 0x22}, // G
    {72, 0x23}, // H
    {73, 0x17}, // I
    {74, 0x24}, // J
    {75, 0x25}, // K
    {76, 0x26}, // L
    {77, 0x32}, // M
    {78, 0x31}, // N
    {79, 0x18}, // O
    {80, 0x19}, // P
    {81, 0x10}, // Q
    {82, 0x13}, // R
    {83, 0x1F}, // S
    {84, 0x14}, // T
    {85, 0x16}, // U
    {86, 0x2F}, // V
    {87, 0x11}, // W
    {88, 0x2D}, // X
    {89, 0x15}, // Y
    {90, 0x2C}, // Z

    // Digits 0–9
    {48, 0x0B}, // 0
    {49, 0x02}, // 1
    {50, 0x03}, // 2
    {51, 0x04}, // 3
    {52, 0x05}, // 4
    {53, 0x06}, // 5
    {54, 0x07}, // 6
    {55, 0x08}, // 7
    {56, 0x09}, // 8
    {57, 0x0A}, // 9
};

static BOOL flag = FALSE;
#define RDP_SCANCODE_EXTENDED(_rdp_scancode) (((_rdp_scancode) & KBDEXT) ? TRUE : FALSE)

// void handleKeyboardPayload(rdpInput *input, const nlohmann::json &data)
// {
// 	// KeyboardEventType type = static_cast<KeyboardEventType>(data["eventType"].get<int>());
// 	// int keyCode = data["keyCode"]; // Browser keyCode (NOT scan code)

// 	// // Map browser keyCode to RDP scan code (example uses 1:1)
// 	// // You should create a proper lookup table for full compatibility
// 	// UINT16 scancode = keyCodeToScanCode[keyCode];

// 	// UINT16 flags = (RDP_SCANCODE_EXTENDED(scancode) ? KBD_FLAGS_EXTENDED : 0);
// 	// char a = 'd';
// 	// if (type == KeyboardEventType::KeyUp)
// 	// {
// 	// 	flags |= KBD_FLAGS_RELEASE;
// 	// 	a = 'u';
// 	// }
// 	// else
// 	// {
// 	// 	a = 'd';
// 	// }
// 	// WLog_DBG(TAG, "sending keystroke %d : %c", keyCode, a);
// 	// freerdp_input_send_keyboard_event(input, flags, scancode);
// }
class RdpClient;
struct MyContext : public rdpContext {
    RdpClient* rdpClient; 
};

class RdpClient : public IRdpClient
{
private:
    int rc = -1;
    RDP_CLIENT_ENTRY_POINTS clientEntryPoints;
    rdpContext *context = nullptr;
    rdpSettings *settings = nullptr;
    wLog *root = nullptr;
    uint8_t *rgba_data = nullptr;
    bool display_updated = false;

public:
    explicit RdpClient(wLog *root = nullptr)
    {
        if (!root)
            root = WLog_GetRoot();
    }

    virtual ~RdpClient()
    {
    }

    virtual int Setup(const char *host, const char *username, const char *password)
    {
        WLog_SetLogLevel(root, WLOG_DEBUG);
        RdpClientEntry(&clientEntryPoints);
        
        context = freerdp_client_context_new(&clientEntryPoints);
        if (!context)
            return FreeAndReturn();

        if (!stream_dump_register_handlers(context, CONNECTION_STATE_MCS_CREATE_REQUEST, FALSE))
            return FreeAndReturn();

        settings = context->settings;

        freerdp_settings_set_string(settings, FreeRDP_ServerHostname, host);
        freerdp_settings_set_string(settings, FreeRDP_Username, username);
        freerdp_settings_set_string(settings, FreeRDP_Password, password);

        freerdp_settings_set_bool(settings, FreeRDP_IgnoreCertificate, TRUE);
        freerdp_settings_set_uint32(settings, FreeRDP_ConnectionType, CONNECTION_TYPE_AUTODETECT);
        freerdp_settings_set_bool(settings, FreeRDP_SupportGraphicsPipeline, TRUE);

    return 0;
        // settings->FastPathOutput = FALSE;
        // context->update->SurfaceBits = surface_bits_callback;
        // settings->AllowUnanouncedOrdersFromServer = TRUE;
    }

    virtual int StartAndConnect()
    {
        if (freerdp_client_start(context) != 0)
        {
            WLog_ERR(TAG, "Could not start rdp client");
            return FreeAndReturn();
        }

        freerdp *instance = context->instance;
        DWORD result = 0;
        BOOL rc = freerdp_connect(instance);

        WINPR_ASSERT(instance->context);
        WINPR_ASSERT(instance->context->settings);
        if (freerdp_settings_get_bool(instance->context->settings, FreeRDP_AuthenticationOnly))
        {
            result = freerdp_get_last_error(instance->context);
            freerdp_abort_connect_context(instance->context);
            WLog_ERR(TAG, "Authentication only, exit status 0x%08" PRIx32 "", result);
            freerdp_disconnect(instance);
            return result;
        }
        else if (!rc)
        {
            result = freerdp_get_last_error(instance->context);
            WLog_ERR(TAG, "connection failure 0x%08" PRIx32, result);
            return result;
        }
    }

    virtual int DispatchRdpEvents()
    {
        freerdp *instance = context->instance;
        HANDLE handles[MAXIMUM_WAIT_OBJECTS] = {nullptr};
        DWORD status = 0;
        DWORD nCount = 0;
        while (!freerdp_shall_disconnect_context(instance->context))
        {
            nCount = freerdp_get_event_handles(instance->context, handles, ARRAYSIZE(handles));

            if (nCount == 0)
            {
                WLog_ERR(TAG, "freerdp_get_event_handles failed");
                break;
            }

            status = WaitForMultipleObjects(nCount, handles, FALSE, INFINITE);

            if (status == WAIT_FAILED)
            {
                WLog_ERR(TAG, "WaitForMultipleObjects failed with %" PRIu32 "", status);
                break;
            }

            if (!freerdp_check_event_handles(instance->context))
            {
                if (freerdp_get_last_error(instance->context) == FREERDP_ERROR_SUCCESS)
                    WLog_ERR(TAG, "Failed to check FreeRDP event handles");

                break;
            }
            if (display_updated && rgba_data)
            {
                return 1; // new display data available
            }
        }
    }

    virtual uint8_t* ReadDisplay(){
        if (display_updated && rgba_data)
        {
            display_updated = false;
            return rgba_data;
        }
        return nullptr;
    }

    virtual int GetDisplayWidth()
    {
        WINPR_ASSERT(context);
        return context->gdi->primary->bitmap->scanline;
    }

    virtual int GetDisplayHeight()
    {
        WINPR_ASSERT(context);
        return context->gdi->primary->bitmap->height;
    }
    virtual int Stop()
    {
        if (freerdp_client_stop(context) != 0)
        {
            rc = -1;
            return FreeAndReturn();
        }
    }

    virtual int sendInputs()
    {
        rc = 0;
        return FreeAndReturn();
    }
private:
    int FreeAndReturn()
    {
        freerdp_client_context_free(context);
        return rc;
    }

    static void OnChannelConnectedEventHandler(void *context, const ChannelConnectedEventArgs *e)
    {

        auto *tf = static_cast<tfContext *>(context);

        WINPR_ASSERT(tf);
        WINPR_ASSERT(e);

        if (strcmp(e->name, RAIL_SVC_CHANNEL_NAME) == 0)
        {
        }
        else if (strcmp(e->name, CLIPRDR_SVC_CHANNEL_NAME) == 0)
        {
            auto *clip = static_cast<CliprdrClientContext *>(e->pInterface);
            WINPR_ASSERT(clip);
            clip->custom = context;
        }
        else
            freerdp_client_OnChannelConnectedEventHandler(&tf->common, e);
    }

    static void OnChannelDisconnectedEventHandler(void *context, const ChannelDisconnectedEventArgs *e)
    {

        auto *tf = static_cast<tfContext *>(context);

        WINPR_ASSERT(tf);
        WINPR_ASSERT(e);

        if (strcmp(e->name, RAIL_SVC_CHANNEL_NAME) == 0)
        {
        }
        else if (strcmp(e->name, CLIPRDR_SVC_CHANNEL_NAME) == 0)
        {
            auto *clip = static_cast<CliprdrClientContext *>(e->pInterface);
            WINPR_ASSERT(clip);
            clip->custom = nullptr;
        }
        else
            freerdp_client_OnChannelDisconnectedEventHandler(&tf->common, e);
    }

    static BOOL freerdp_client_add_dynamic_channel(rdpSettings *settings, const size_t count,
                                                   const char *const *params)
    {
        ADDIN_ARGV *_args = nullptr;

        if (!settings || !params || !params[0] || (count > INT_MAX))
            return FALSE;

        if (freerdp_dynamic_channel_collection_find(settings, params[0]))
            return TRUE;

        _args = freerdp_addin_argv_new(count, params);

        if (!_args)
            return FALSE;

        if (!freerdp_dynamic_channel_collection_add(settings, _args))
            goto fail;

        return TRUE;

    fail:
        freerdp_addin_argv_free(_args);
        return FALSE;
    }

    static BOOL freerdp_client_load_static_channel_addin(rdpChannels *channels, rdpSettings *settings,
                                                         const char *name, void *data)
    {
        PVIRTUALCHANNELENTRY entry = nullptr;
        PVIRTUALCHANNELENTRY pvce = freerdp_load_channel_addin_entry(
            name, nullptr, nullptr, FREERDP_ADDIN_CHANNEL_STATIC | FREERDP_ADDIN_CHANNEL_ENTRYEX);
        PVIRTUALCHANNELENTRYEX pvceex = WINPR_FUNC_PTR_CAST(pvce, PVIRTUALCHANNELENTRYEX);

        if (!pvceex)
            entry = freerdp_load_channel_addin_entry(name, nullptr, nullptr, FREERDP_ADDIN_CHANNEL_STATIC);

        if (pvceex)
        {
            if (freerdp_channels_client_load_ex(channels, settings, pvceex, data) == 0)
            {
                WLog_DBG(TAG, "loading channelEx %s", name);
                return TRUE;
            }
        }
        else if (entry)
        {
            if (freerdp_channels_client_load(channels, settings, entry, data) == 0)
            {
                WLog_DBG(TAG, "loading channel %s", name);
                return TRUE;
            }
        }

        return FALSE;
    }

    static BOOL freerdp_client_load_addins(rdpChannels *channels, rdpSettings *settings)
    {
        ChannelToLoad dynChannels[] = {
#if defined(CHANNEL_AINPUT_CLIENT)
            {FreeRDP_BOOL_UNUSED, AINPUT_CHANNEL_NAME, nullptr}, /* always loaded */
#endif
            {FreeRDP_AudioCapture, AUDIN_CHANNEL_NAME, nullptr},
            {FreeRDP_AudioPlayback, RDPSND_CHANNEL_NAME, nullptr},
#ifdef CHANNEL_RDPEI_CLIENT
            {FreeRDP_MultiTouchInput, RDPEI_CHANNEL_NAME, nullptr},
#endif
            {FreeRDP_SupportGraphicsPipeline, RDPGFX_CHANNEL_NAME, nullptr},
            {FreeRDP_SupportEchoChannel, ECHO_CHANNEL_NAME, nullptr},
            {FreeRDP_SupportSSHAgentChannel, "sshagent", nullptr},
            {FreeRDP_SupportDisplayControl, DISP_CHANNEL_NAME, nullptr},
            {FreeRDP_SupportGeometryTracking, GEOMETRY_CHANNEL_NAME, nullptr},
            {FreeRDP_SupportVideoOptimized, VIDEO_CHANNEL_NAME, nullptr},
            {FreeRDP_RemoteCredentialGuard, RDPEAR_CHANNEL_NAME, nullptr},
        };

        ChannelToLoad staticChannels[] = {
            {FreeRDP_AudioPlayback, RDPSND_CHANNEL_NAME, nullptr},
            {FreeRDP_RedirectClipboard, CLIPRDR_SVC_CHANNEL_NAME, nullptr},
#if defined(CHANNEL_ENCOMSP_CLIENT)
            {FreeRDP_EncomspVirtualChannel, ENCOMSP_SVC_CHANNEL_NAME, settings},
#endif
            {FreeRDP_RemdeskVirtualChannel, REMDESK_SVC_CHANNEL_NAME, settings},
            {FreeRDP_RemoteApplicationMode, RAIL_SVC_CHANNEL_NAME, settings}};

        /**
         * Step 1: first load dynamic channels according to the settings
         */
        for (auto &dynChannel : dynChannels)
        {
            if ((dynChannel.settingId == FreeRDP_BOOL_UNUSED) ||
                freerdp_settings_get_bool(settings, dynChannel.settingId))
            {
                const char *const p[] = {dynChannel.channelName};

                printf("Loading dynamic channel: %s\n", dynChannel.channelName);
                if (!freerdp_client_add_dynamic_channel(settings, ARRAYSIZE(p), p))
                    return FALSE;
            }
        }

        /* step 4: do the static channels loading and init */
        for (UINT32 i = 0; i < freerdp_settings_get_uint32(settings, FreeRDP_StaticChannelCount); i++)
        {
            auto *_args = static_cast<ADDIN_ARGV *>(freerdp_settings_get_pointer_array_writable(
                settings, FreeRDP_StaticChannelArray, i));

            printf("Loading static channel: %s\n", _args->argv[0]);
            if (!freerdp_client_load_static_channel_addin(channels, settings, _args->argv[0], _args))
                return FALSE;
        }

        printf("Static channels loaded successfully.\n");
        if (freerdp_settings_get_uint32(settings, FreeRDP_DynamicChannelCount) > 0)
        {
            if (!freerdp_settings_set_bool(settings, FreeRDP_SupportDynamicChannels, TRUE))
                return FALSE;
        }

        if (freerdp_settings_get_bool(settings, FreeRDP_SupportDynamicChannels))
        {
            if (!freerdp_client_load_static_channel_addin(channels, settings, DRDYNVC_SVC_CHANNEL_NAME,
                                                          settings))
                return FALSE;
        }

        return TRUE;
    }

    static BOOL freerdp_client_load_channels(freerdp *instance)
    {
        WINPR_ASSERT(instance);
        WINPR_ASSERT(instance->context);

        if (!freerdp_client_load_addins(instance->context->channels, instance->context->settings))
        {
            WLog_ERR(TAG, "Failed to load addins [%08" PRIx32 "]", GetLastError());
            return FALSE;
        }
        return TRUE;
    }

    static SSIZE_T client_common_retry_dialog(freerdp *instance, const char *what, size_t current,
                                              void *userarg)
    {
        WINPR_UNUSED(instance);
        WINPR_ASSERT(instance->context);
        WINPR_UNUSED(userarg);
        WINPR_ASSERT(instance);
        WINPR_ASSERT(what);
        const size_t delay = freerdp_settings_get_uint32(instance->context->settings, FreeRDP_TcpConnectTimeout);
        return WINPR_ASSERTING_INT_CAST(SSIZE_T, delay);
    }

    static void set_default_callbacks(freerdp *instance)
    {
        WINPR_ASSERT(instance);
        instance->RetryDialog = client_common_retry_dialog;

        // TODO more
    }

    static BOOL freerdp_client_common_new(freerdp *instance, rdpContext *context)
    {
        RDP_CLIENT_ENTRY_POINTS *pEntryPoints = nullptr;

        WINPR_ASSERT(instance);
        WINPR_ASSERT(context);

        instance->LoadChannels = freerdp_client_load_channels;
        set_default_callbacks(instance);

        pEntryPoints = instance->pClientEntryPoints;
        WINPR_ASSERT(pEntryPoints);
        return IFCALLRESULT(TRUE, pEntryPoints->ClientNew, instance, context);
    }

    static void freerdp_client_common_free(freerdp *instance, rdpContext *context)
    {
        RDP_CLIENT_ENTRY_POINTS *pEntryPoints = nullptr;

        WINPR_ASSERT(instance);
        WINPR_ASSERT(context);

        pEntryPoints = instance->pClientEntryPoints;
        WINPR_ASSERT(pEntryPoints);
        IFCALL(pEntryPoints->ClientFree, instance, context);
    }

    rdpContext *freerdp_client_context_new(const RDP_CLIENT_ENTRY_POINTS *pEntryPoints)
    {
        freerdp *instance = nullptr;
        rdpContext *context = nullptr;

        if (!pEntryPoints)
            return nullptr;

        IFCALL(pEntryPoints->GlobalInit);

        instance = freerdp_new();

        if (!instance)
            return nullptr;

        instance->ContextSize = sizeof(MyContext);
        instance->ContextNew = freerdp_client_common_new;
        instance->ContextFree = freerdp_client_common_free;
        instance->pClientEntryPoints = static_cast<RDP_CLIENT_ENTRY_POINTS *>(malloc(pEntryPoints->Size));
        MyContext* ctx = nullptr;

        if (!instance->pClientEntryPoints)
            goto out_fail;

        CopyMemory(instance->pClientEntryPoints, pEntryPoints, pEntryPoints->Size);

        if (!freerdp_context_new_ex(instance, pEntryPoints->settings))
            goto out_fail2;

        context = instance->context;
        context->instance = instance;
        ctx = static_cast<MyContext*>(instance->context);
        ctx->rdpClient = this;

        if (freerdp_register_addin_provider(freerdp_channels_load_static_addin_entry, 0) !=
            CHANNEL_RC_OK)
            goto out_fail2;
        return context;
    out_fail2:
        free(instance->pClientEntryPoints);
    out_fail:
        freerdp_free(instance);
        return nullptr;
    }

    void freerdp_client_context_free(rdpContext *context)
    {
        freerdp *instance = nullptr;

        if (!context)
            return;

        instance = context->instance;

        if (instance)
        {
            const RDP_CLIENT_ENTRY_POINTS *pEntryPoints = instance->pClientEntryPoints;
            freerdp_context_free(instance);

            if (pEntryPoints)
                IFCALL(pEntryPoints->GlobalUninit);

            free(instance->pClientEntryPoints);
            freerdp_free(instance);
        }
    }

    int freerdp_client_start(rdpContext *context)
    {
        RDP_CLIENT_ENTRY_POINTS *pEntryPoints = nullptr;

        if (!context || !context->instance || !context->instance->pClientEntryPoints)
            return ERROR_BAD_ARGUMENTS;

        if (freerdp_settings_get_bool(context->settings, FreeRDP_UseCommonStdioCallbacks))
            set_default_callbacks(context->instance);

#ifdef WITH_SSO_MIB
        rdpClientContext *client_context = (rdpClientContext *)context;
        client_context->mibClientWrapper = sso_mib_new(context);
        if (!client_context->mibClientWrapper)
            return ERROR_INTERNAL_ERROR;
#endif

        pEntryPoints = context->instance->pClientEntryPoints;
        int res = IFCALLRESULT(CHANNEL_RC_OK, pEntryPoints->ClientStart, context);
        return res;
    }

    int freerdp_client_stop(rdpContext *context)
    {
        RDP_CLIENT_ENTRY_POINTS *pEntryPoints = nullptr;

        if (!context || !context->instance || !context->instance->pClientEntryPoints)
            return ERROR_BAD_ARGUMENTS;

        pEntryPoints = context->instance->pClientEntryPoints;
        const int rc = IFCALLRESULT(CHANNEL_RC_OK, pEntryPoints->ClientStop, context);

#ifdef WITH_SSO_MIB
        rdpClientContext *client_context = (rdpClientContext *)context;
        sso_mib_free(client_context->mibClientWrapper);
        client_context->mibClientWrapper = nullptr;
#endif // WITH_SSO_MIB
        return rc;
    }

    static BOOL begin_paint(rdpContext *context)
    {
        rdpGdi *gdi = nullptr;

        WINPR_ASSERT(context);

        gdi = context->gdi;
        WINPR_ASSERT(gdi);
        WINPR_ASSERT(gdi->primary);
        WINPR_ASSERT(gdi->primary->hdc);
        WINPR_ASSERT(gdi->primary->hdc->hwnd);
        WINPR_ASSERT(gdi->primary->hdc->hwnd->invalid);
        gdi->primary->hdc->hwnd->invalid->null = TRUE;
        printf("BeginPaint: invalid area reset\n");
        return TRUE;
    }

    static BOOL end_paint(rdpContext *context) {
        auto myctx = (MyContext*)context;
        auto client = reinterpret_cast<RdpClient*>(myctx->rdpClient);
        return client->on_end_paint(context); // call member logic
    }


    BOOL on_end_paint(rdpContext *context)
    {
        rdpGdi *gdi = nullptr;

        WINPR_ASSERT(context);

        gdi = context->gdi;
        WINPR_ASSERT(gdi);
        WINPR_ASSERT(gdi->primary);
        WINPR_ASSERT(gdi->primary->hdc);
        WINPR_ASSERT(gdi->primary->hdc->hwnd);
        WINPR_ASSERT(gdi->primary->hdc->hwnd->invalid);
        const unsigned int width = context->settings->DesktopWidth;
        const unsigned int height = context->settings->DesktopHeight;

        char filename[64];
        rgba_data = static_cast<uint8_t *>(malloc(width * height * 4));
        agbr_to_rgba(rgba_data, gdi->primary->bitmap->data, width, height);
        printf("Image size is w %u  h %u\n", gdi->primary->bitmap->width, gdi->primary->bitmap->height);
        // broadcast(ws_server, rgba_data, gdi->primary->bitmap->scanline * gdi->primary->bitmap->height);
        display_updated = true;
        rdpInput *input = context->input;
        if (gdi->primary->hdc->hwnd->invalid->null)
            return TRUE;

        return TRUE;
    }

    static BOOL desktop_resize(rdpContext *context)
    {
        rdpGdi *gdi = nullptr;
        rdpSettings *settings = nullptr;

        WINPR_ASSERT(context);

        settings = context->settings;
        WINPR_ASSERT(settings);

        gdi = context->gdi;
        return gdi_resize(gdi, freerdp_settings_get_uint32(settings, FreeRDP_DesktopWidth),
                          freerdp_settings_get_uint32(settings, FreeRDP_DesktopHeight));
    }

    /* This function is called to output a System BEEP */
    static BOOL play_sound(rdpContext *context, const PLAY_SOUND_UPDATE *play_sound)
    {
        /* TODO: Implement */
        WINPR_UNUSED(context);
        WINPR_UNUSED(play_sound);
        return TRUE;
    }

    /* This function is called to update the keyboard indocator LED */
    static BOOL keyboard_set_indicators(rdpContext *context, UINT16 led_flags)
    {
        /* TODO: Set local keyboard indicator LED status */
        WINPR_UNUSED(context);
        WINPR_UNUSED(led_flags);
        return TRUE;
    }

    /* This function is called to set the IME state */
    static BOOL keyboard_set_ime_status(rdpContext *context, UINT16 imeId, UINT32 imeState,
                                        UINT32 imeConvMode)
    {
        if (!context)
            return FALSE;

        WLog_WARN(TAG,
                  "KeyboardSetImeStatus(unitId=%04" PRIx16 ", imeState=%08" PRIx32
                  ", imeConvMode=%08" PRIx32 ") ignored",
                  imeId, imeState, imeConvMode);
        return TRUE;
    }

    /* Called before a connection is established.
     * Set all configuration options to support and load channels here. */
    static BOOL pre_connect(freerdp *instance)
    {
        rdpSettings *settings = nullptr;

        WINPR_ASSERT(instance);
        WINPR_ASSERT(instance->context);

        settings = instance->context->settings;
        WINPR_ASSERT(settings);

        /* If the callbacks provide the PEM all certificate options can be extracted, otherwise
         * only the certificate fingerprint is available. */
        if (!freerdp_settings_set_bool(settings, FreeRDP_CertificateCallbackPreferPEM, TRUE))
            return FALSE;

        /* Optional OS identifier sent to server */
        if (!freerdp_settings_set_uint32(settings, FreeRDP_OsMajorType, OSMAJORTYPE_UNIX))
            return FALSE;
        if (!freerdp_settings_set_uint32(settings, FreeRDP_OsMinorType, OSMINORTYPE_NATIVE_XSERVER))
            return FALSE;
        /* OrderSupport is initialized at this point.
         * Only override it if you plan to implement custom order
         * callbacks or deactivate certain features. */
        /* Register the channel listeners.
         * They are required to set up / tear down channels if they are loaded. */
        PubSub_SubscribeChannelConnected(instance->context->pubSub, OnChannelConnectedEventHandler);
        PubSub_SubscribeChannelDisconnected(instance->context->pubSub,
                                            OnChannelDisconnectedEventHandler);

        /* TODO: Any code your client requires */
        return TRUE;
    }

    /* Called after an RDP connection was successfully established.
     * Settings might have changed during negotiation of client / server feature
     * support.
     *
     * Set up local framebuffers and paint callbacks.
     * If required, register pointer callbacks to change the local mouse cursor
     * when hovering over the RDP window
     */
    static BOOL post_connect(freerdp *instance)
    {
        rdpContext *context = nullptr;

        if (!gdi_init(instance, PIXEL_FORMAT_XRGB32))
            return FALSE;

        context = instance->context;
        WINPR_ASSERT(context);
        WINPR_ASSERT(context->update);

        /* With this setting we disable all graphics processing in the library.
         *
         * This allows low resource (client) protocol parsing.
         */
        if (!freerdp_settings_set_bool(context->settings, FreeRDP_DeactivateClientDecoding, TRUE))
            return FALSE;

        context->update->BeginPaint = begin_paint;
        context->update->EndPaint = end_paint;
        context->update->PlaySound = play_sound;
        context->update->DesktopResize = desktop_resize;
        context->update->SetKeyboardIndicators = keyboard_set_indicators;
        context->update->SetKeyboardImeStatus = keyboard_set_ime_status;
        return TRUE;
    }

    /* This function is called whether a session ends by failure or success.
     * Clean up everything allocated by pre_connect and post_connect.
     */
    static void post_disconnect(freerdp *instance)
    {

        if (!instance)
            return;

        if (!instance->context)
            return;

        PubSub_UnsubscribeChannelConnected(instance->context->pubSub, OnChannelConnectedEventHandler);
        PubSub_UnsubscribeChannelDisconnected(instance->context->pubSub, OnChannelDisconnectedEventHandler);
        gdi_free(instance);
        /* TODO : Clean up custom stuff */
    }

    /* Optional global initializer.
     * Here we just register a signal handler to print out stack traces
     * if available. */
    static BOOL client_global_init()
    {
        if (freerdp_handle_signals() != 0)
            return FALSE;

        return TRUE;
    }

    /* Optional global tear down */
    static void client_global_uninit()
    {
    }

    static int logon_error_info(freerdp *instance, UINT32 data, UINT32 type)
    {
        const char *str_data = freerdp_get_logon_error_info_data(data);
        const char *str_type = freerdp_get_logon_error_info_type(type);

        if (!instance || !instance->context)
            return -1;

        WLog_INFO(TAG, "Logon Error Info %s [%s]", str_data, str_type);

        return 1;
    }

    static BOOL client_new(freerdp *instance, rdpContext *context)
    {

        if (!instance || !context)
            return FALSE;

        instance->PreConnect = pre_connect;
        instance->PostConnect = post_connect;
        instance->PostDisconnect = post_disconnect;
        instance->LogonErrorInfo = logon_error_info;
        /* TODO: Client display set up */
        return TRUE;
    }

    static void client_free(freerdp *instance, rdpContext *context)
    {
        /* TODO: Client display tear down */
    }

    static int client_start(rdpContext *context)
    {
        /* TODO: Start client related stuff */
        WINPR_UNUSED(context);
        return 0;
    }

    static int client_stop(rdpContext *context)
    {
        /* TODO: Stop client related stuff */
        WINPR_UNUSED(context);
        return 0;
    }

    int RdpClientEntry(RDP_CLIENT_ENTRY_POINTS *pEntryPoints)
    {
        WINPR_ASSERT(pEntryPoints);

        ZeroMemory(pEntryPoints, sizeof(RDP_CLIENT_ENTRY_POINTS));
        pEntryPoints->Version = RDP_CLIENT_INTERFACE_VERSION;
        pEntryPoints->Size = sizeof(RDP_CLIENT_ENTRY_POINTS_V1);
        pEntryPoints->GlobalInit = client_global_init;
        pEntryPoints->GlobalUninit = client_global_uninit;
        pEntryPoints->ContextSize = sizeof(tfContext);
        pEntryPoints->ClientNew = client_new;
        pEntryPoints->ClientFree = client_free;
        pEntryPoints->ClientStart = client_start;
        pEntryPoints->ClientStop = client_stop;
        return 0;
    }
};

IRdpClient* createRdpClient() {
    return new RdpClient();
}