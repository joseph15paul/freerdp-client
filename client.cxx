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

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define TAG CLIENT_TAG("custom-client")

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

void tf_OnChannelConnectedEventHandler(void *context, const ChannelConnectedEventArgs *e)
{

	tfContext *tf = (tfContext *)context;

	WINPR_ASSERT(tf);
	WINPR_ASSERT(e);

	if (strcmp(e->name, RAIL_SVC_CHANNEL_NAME) == 0)
	{
	}
	else if (strcmp(e->name, CLIPRDR_SVC_CHANNEL_NAME) == 0)
	{
		CliprdrClientContext *clip = (CliprdrClientContext *)e->pInterface;
		WINPR_ASSERT(clip);
		clip->custom = context;
	}
	else
		freerdp_client_OnChannelConnectedEventHandler(&tf->common, e);
}

void tf_OnChannelDisconnectedEventHandler(void *context, const ChannelDisconnectedEventArgs *e)
{

	tfContext *tf = (tfContext *)context;

	WINPR_ASSERT(tf);
	WINPR_ASSERT(e);

	if (strcmp(e->name, RAIL_SVC_CHANNEL_NAME) == 0)
	{
	}
	else if (strcmp(e->name, CLIPRDR_SVC_CHANNEL_NAME) == 0)
	{
		CliprdrClientContext *clip = (CliprdrClientContext *)e->pInterface;
		WINPR_ASSERT(clip);
		clip->custom = NULL;
	}
	else
		freerdp_client_OnChannelDisconnectedEventHandler(&tf->common, e);
}

BOOL freerdp_client_add_dynamic_channel(rdpSettings *settings, size_t count,
										const char *const *params)
{
	ADDIN_ARGV *_args = NULL;

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
	PVIRTUALCHANNELENTRY entry = NULL;
	PVIRTUALCHANNELENTRY pvce = freerdp_load_channel_addin_entry(
		name, NULL, NULL, FREERDP_ADDIN_CHANNEL_STATIC | FREERDP_ADDIN_CHANNEL_ENTRYEX);
	PVIRTUALCHANNELENTRYEX pvceex = WINPR_FUNC_PTR_CAST(pvce, PVIRTUALCHANNELENTRYEX);

	if (!pvceex)
		entry = freerdp_load_channel_addin_entry(name, NULL, NULL, FREERDP_ADDIN_CHANNEL_STATIC);

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

BOOL freerdp_client_load_addins(rdpChannels *channels, rdpSettings *settings)
{
	ChannelToLoad dynChannels[] = {
#if defined(CHANNEL_AINPUT_CLIENT)
		{FreeRDP_BOOL_UNUSED, AINPUT_CHANNEL_NAME, NULL}, /* always loaded */
#endif
		{FreeRDP_AudioCapture, AUDIN_CHANNEL_NAME, NULL},
		{FreeRDP_AudioPlayback, RDPSND_CHANNEL_NAME, NULL},
#ifdef CHANNEL_RDPEI_CLIENT
		{FreeRDP_MultiTouchInput, RDPEI_CHANNEL_NAME, NULL},
#endif
		{FreeRDP_SupportGraphicsPipeline, RDPGFX_CHANNEL_NAME, NULL},
		{FreeRDP_SupportEchoChannel, ECHO_CHANNEL_NAME, NULL},
		{FreeRDP_SupportSSHAgentChannel, "sshagent", NULL},
		{FreeRDP_SupportDisplayControl, DISP_CHANNEL_NAME, NULL},
		{FreeRDP_SupportGeometryTracking, GEOMETRY_CHANNEL_NAME, NULL},
		{FreeRDP_SupportVideoOptimized, VIDEO_CHANNEL_NAME, NULL},
		{FreeRDP_RemoteCredentialGuard, RDPEAR_CHANNEL_NAME, NULL},
	};

	ChannelToLoad staticChannels[] = {
		{FreeRDP_AudioPlayback, RDPSND_CHANNEL_NAME, NULL},
		{FreeRDP_RedirectClipboard, CLIPRDR_SVC_CHANNEL_NAME, NULL},
#if defined(CHANNEL_ENCOMSP_CLIENT)
		{FreeRDP_EncomspVirtualChannel, ENCOMSP_SVC_CHANNEL_NAME, settings},
#endif
		{FreeRDP_RemdeskVirtualChannel, REMDESK_SVC_CHANNEL_NAME, settings},
		{FreeRDP_RemoteApplicationMode, RAIL_SVC_CHANNEL_NAME, settings}};

	/**
	 * Step 1: first load dynamic channels according to the settings
	 */
	for (size_t i = 0; i < ARRAYSIZE(dynChannels); i++)
	{
		if ((dynChannels[i].settingId == FreeRDP_BOOL_UNUSED) ||
			freerdp_settings_get_bool(settings, dynChannels[i].settingId))
		{
			const char *const p[] = {dynChannels[i].channelName};

			printf("Loading dynamic channel: %s\n", dynChannels[i].channelName);
			if (!freerdp_client_add_dynamic_channel(settings, ARRAYSIZE(p), p))
				return FALSE;
		}
	}

	/* step 4: do the static channels loading and init */
	for (UINT32 i = 0; i < freerdp_settings_get_uint32(settings, FreeRDP_StaticChannelCount); i++)
	{
		ADDIN_ARGV *_args = static_cast<ADDIN_ARGV *>(freerdp_settings_get_pointer_array_writable(
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

BOOL freerdp_client_load_channels(freerdp *instance)
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

SSIZE_T client_common_retry_dialog(freerdp *instance, const char *what, size_t current,
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
	RDP_CLIENT_ENTRY_POINTS *pEntryPoints = NULL;

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
	RDP_CLIENT_ENTRY_POINTS *pEntryPoints = NULL;

	WINPR_ASSERT(instance);
	WINPR_ASSERT(context);

	pEntryPoints = instance->pClientEntryPoints;
	WINPR_ASSERT(pEntryPoints);
	IFCALL(pEntryPoints->ClientFree, instance, context);
}

rdpContext *freerdp_client_context_new(const RDP_CLIENT_ENTRY_POINTS *pEntryPoints)
{
	freerdp *instance = NULL;
	rdpContext *context = NULL;

	if (!pEntryPoints)
		return NULL;

	IFCALL(pEntryPoints->GlobalInit);

	instance = freerdp_new();

	if (!instance)
		return NULL;

	instance->ContextSize = pEntryPoints->ContextSize;
	instance->ContextNew = freerdp_client_common_new;
	instance->ContextFree = freerdp_client_common_free;
	instance->pClientEntryPoints = (RDP_CLIENT_ENTRY_POINTS *)malloc(pEntryPoints->Size);

	if (!instance->pClientEntryPoints)
		goto out_fail;

	CopyMemory(instance->pClientEntryPoints, pEntryPoints, pEntryPoints->Size);

	if (!freerdp_context_new_ex(instance, pEntryPoints->settings))
		goto out_fail2;

	context = instance->context;
	context->instance = instance;

	if (freerdp_register_addin_provider(freerdp_channels_load_static_addin_entry, 0) !=
		CHANNEL_RC_OK)
		goto out_fail2;
	std::cout << "context successfully created\n";
	return context;
out_fail2:
	free(instance->pClientEntryPoints);
out_fail:
	freerdp_free(instance);
	return NULL;
}

void freerdp_client_context_free(rdpContext *context)
{
	freerdp *instance = NULL;

	if (!context)
		return;

	instance = context->instance;

	if (instance)
	{
		RDP_CLIENT_ENTRY_POINTS *pEntryPoints = instance->pClientEntryPoints;
		freerdp_context_free(instance);

		if (pEntryPoints)
			IFCALL(pEntryPoints->GlobalUninit);

		free(instance->pClientEntryPoints);
		freerdp_free(instance);
	}
}

int freerdp_client_start(rdpContext *context)
{
	RDP_CLIENT_ENTRY_POINTS *pEntryPoints = NULL;

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
	RDP_CLIENT_ENTRY_POINTS *pEntryPoints = NULL;

	if (!context || !context->instance || !context->instance->pClientEntryPoints)
		return ERROR_BAD_ARGUMENTS;

	pEntryPoints = context->instance->pClientEntryPoints;
	const int rc = IFCALLRESULT(CHANNEL_RC_OK, pEntryPoints->ClientStop, context);

#ifdef WITH_SSO_MIB
	rdpClientContext *client_context = (rdpClientContext *)context;
	sso_mib_free(client_context->mibClientWrapper);
	client_context->mibClientWrapper = NULL;
#endif // WITH_SSO_MIB
	return rc;
}

// Converts BGRA to RGBA and saves as PNG
int save_bgra_to_png(const char *filename, const uint8_t *bgra_data, int width, int height)
{
	if (!filename || !bgra_data || width <= 0 || height <= 0)
		return 0;

	// Allocate temporary RGBA buffer
	uint8_t *rgba_data = (uint8_t *)malloc(width * height * 4);
	if (!rgba_data)
		return 0;

	// Convert BGRA -> RGBA
	for (int i = 0; i < width * height; ++i)
	{
		rgba_data[i * 4 + 0] = bgra_data[i * 4 + 2]; // R
		rgba_data[i * 4 + 1] = bgra_data[i * 4 + 1]; // G
		rgba_data[i * 4 + 2] = bgra_data[i * 4 + 0]; // B
		rgba_data[i * 4 + 3] = bgra_data[i * 4 + 3]; // A
	}

	// Write to PNG
	int result = stbi_write_png(filename, width, height, 4, rgba_data, width * 4);

	free(rgba_data);
	return result;
}

static BOOL tf_begin_paint(rdpContext *context)
{
	rdpGdi *gdi = NULL;

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

static BOOL tf_end_paint(rdpContext *context)
{
	rdpGdi *gdi = NULL;

	WINPR_ASSERT(context);

	gdi = context->gdi;
	WINPR_ASSERT(gdi);
	WINPR_ASSERT(gdi->primary);
	WINPR_ASSERT(gdi->primary->hdc);
	WINPR_ASSERT(gdi->primary->hdc->hwnd);
	WINPR_ASSERT(gdi->primary->hdc->hwnd->invalid);
	int width = context->settings->DesktopWidth;
	int height = context->settings->DesktopHeight;

	// Save gdi->primary->data (BGRA32) as an image
	save_bgra_to_png("login.png", gdi->primary->bitmap->data, width, height);
	printf("image is saveddd.................................\n");

	if (gdi->primary->hdc->hwnd->invalid->null)
		return TRUE;

	return TRUE;
}

static BOOL tf_desktop_resize(rdpContext *context)
{
	rdpGdi *gdi = NULL;
	rdpSettings *settings = NULL;

	WINPR_ASSERT(context);

	settings = context->settings;
	WINPR_ASSERT(settings);

	gdi = context->gdi;
	return gdi_resize(gdi, freerdp_settings_get_uint32(settings, FreeRDP_DesktopWidth),
					  freerdp_settings_get_uint32(settings, FreeRDP_DesktopHeight));
}

/* This function is called to output a System BEEP */
static BOOL tf_play_sound(rdpContext *context, const PLAY_SOUND_UPDATE *play_sound)
{
	/* TODO: Implement */
	WINPR_UNUSED(context);
	WINPR_UNUSED(play_sound);
	return TRUE;
}

/* This function is called to update the keyboard indocator LED */
static BOOL tf_keyboard_set_indicators(rdpContext *context, UINT16 led_flags)
{
	/* TODO: Set local keyboard indicator LED status */
	WINPR_UNUSED(context);
	WINPR_UNUSED(led_flags);
	return TRUE;
}

/* This function is called to set the IME state */
static BOOL tf_keyboard_set_ime_status(rdpContext *context, UINT16 imeId, UINT32 imeState,
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
static BOOL tf_pre_connect(freerdp *instance)
{
	rdpSettings *settings = NULL;

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
	PubSub_SubscribeChannelConnected(instance->context->pubSub, tf_OnChannelConnectedEventHandler);
	PubSub_SubscribeChannelDisconnected(instance->context->pubSub,
										tf_OnChannelDisconnectedEventHandler);

	/* TODO: Any code your client requires */
	return TRUE;
}

/* Called after a RDP connection was successfully established.
 * Settings might have changed during negotiation of client / server feature
 * support.
 *
 * Set up local framebuffers and paing callbacks.
 * If required, register pointer callbacks to change the local mouse cursor
 * when hovering over the RDP window
 */
static BOOL tf_post_connect(freerdp *instance)
{
	rdpContext *context = NULL;

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

	context->update->BeginPaint = tf_begin_paint;
	context->update->EndPaint = tf_end_paint;
	context->update->PlaySound = tf_play_sound;
	context->update->DesktopResize = tf_desktop_resize;
	context->update->SetKeyboardIndicators = tf_keyboard_set_indicators;
	context->update->SetKeyboardImeStatus = tf_keyboard_set_ime_status;
	return TRUE;
}

/* This function is called whether a session ends by failure or success.
 * Clean up everything allocated by pre_connect and post_connect.
 */
static void tf_post_disconnect(freerdp *instance)
{

	if (!instance)
		return;

	if (!instance->context)
		return;

	PubSub_UnsubscribeChannelConnected(instance->context->pubSub,
									   tf_OnChannelConnectedEventHandler);
	PubSub_UnsubscribeChannelDisconnected(instance->context->pubSub,
										  tf_OnChannelDisconnectedEventHandler);
	gdi_free(instance);
	/* TODO : Clean up custom stuff */
}

/* RDP main loop.
 * Connects RDP, loops while running and handles event and dispatch, cleans up
 * after the connection ends. */
static DWORD WINAPI tf_client_thread_proc(LPVOID arg)
{
	freerdp *instance = (freerdp *)arg;
	DWORD nCount = 0;
	DWORD status = 0;
	DWORD result = 0;
	HANDLE handles[MAXIMUM_WAIT_OBJECTS] = {0};
	BOOL rc = freerdp_connect(instance);

	WINPR_ASSERT(instance->context);
	WINPR_ASSERT(instance->context->settings);
	if (freerdp_settings_get_bool(instance->context->settings, FreeRDP_AuthenticationOnly))
	{
		result = freerdp_get_last_error(instance->context);
		freerdp_abort_connect_context(instance->context);
		WLog_ERR(TAG, "Authentication only, exit status 0x%08" PRIx32 "", result);
		goto disconnect;
	}

	if (!rc)
	{
		result = freerdp_get_last_error(instance->context);
		WLog_ERR(TAG, "connection failure 0x%08" PRIx32, result);
		return result;
	}

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
	}

disconnect:
	freerdp_disconnect(instance);
	return result;
}

/* Optional global initializer.
 * Here we just register a signal handler to print out stack traces
 * if available. */
static BOOL tf_client_global_init(void)
{
	if (freerdp_handle_signals() != 0)
		return FALSE;

	return TRUE;
}

/* Optional global tear down */
static void tf_client_global_uninit(void)
{
}

static int tf_logon_error_info(freerdp *instance, UINT32 data, UINT32 type)
{
	const char *str_data = freerdp_get_logon_error_info_data(data);
	const char *str_type = freerdp_get_logon_error_info_type(type);

	if (!instance || !instance->context)
		return -1;

	WLog_INFO(TAG, "Logon Error Info %s [%s]", str_data, str_type);

	return 1;
}

static BOOL tf_client_new(freerdp *instance, rdpContext *context)
{

	if (!instance || !context)
		return FALSE;

	instance->PreConnect = tf_pre_connect;
	instance->PostConnect = tf_post_connect;
	instance->PostDisconnect = tf_post_disconnect;
	instance->LogonErrorInfo = tf_logon_error_info;
	/* TODO: Client display set up */
	return TRUE;
}

static void tf_client_free(freerdp *instance, rdpContext *context)
{
	/* TODO: Client display tear down */
}

static int tf_client_start(rdpContext *context)
{
	/* TODO: Start client related stuff */
	WINPR_UNUSED(context);
	return 0;
}

static int tf_client_stop(rdpContext *context)
{
	/* TODO: Stop client related stuff */
	WINPR_UNUSED(context);
	return 0;
}

static int RdpClientEntry(RDP_CLIENT_ENTRY_POINTS *pEntryPoints)
{
	WINPR_ASSERT(pEntryPoints);

	ZeroMemory(pEntryPoints, sizeof(RDP_CLIENT_ENTRY_POINTS));
	pEntryPoints->Version = RDP_CLIENT_INTERFACE_VERSION;
	pEntryPoints->Size = sizeof(RDP_CLIENT_ENTRY_POINTS_V1);
	pEntryPoints->GlobalInit = tf_client_global_init;
	pEntryPoints->GlobalUninit = tf_client_global_uninit;
	pEntryPoints->ContextSize = sizeof(tfContext);
	pEntryPoints->ClientNew = tf_client_new;
	pEntryPoints->ClientFree = tf_client_free;
	pEntryPoints->ClientStart = tf_client_start;
	pEntryPoints->ClientStop = tf_client_stop;
	return 0;
}

int main(int argc, char *argv[])
{
	int rc = -1;
	int status = 0;
	RDP_CLIENT_ENTRY_POINTS clientEntryPoints;
	rdpContext *context = NULL;
	rdpSettings *settings = NULL;
	wLog *root = WLog_GetRoot();
	WLog_SetLogLevel(root, WLOG_DEBUG); // or WLOG_DEBUG, WLOG_INFO, etc.

	RdpClientEntry(&clientEntryPoints);
	context = freerdp_client_context_new(&clientEntryPoints);

	if (!context)
		goto fail;

	if (!stream_dump_register_handlers(context, CONNECTION_STATE_MCS_CREATE_REQUEST, FALSE))
		goto fail;

	settings = context->settings;

	freerdp_settings_set_string(settings, FreeRDP_ServerHostname, "127.0.0.1");
	freerdp_settings_set_string(settings, FreeRDP_Username, "user");
	freerdp_settings_set_string(settings, FreeRDP_Password, "root");

	freerdp_settings_set_bool(settings, FreeRDP_IgnoreCertificate, TRUE); // TODO Don't do this in production
	freerdp_settings_set_uint32(settings, FreeRDP_ConnectionType, CONNECTION_TYPE_AUTODETECT);
	freerdp_settings_set_bool(settings, FreeRDP_SupportGraphicsPipeline, TRUE);

	if (freerdp_client_start(context) != 0)
		goto fail;

	rc = (int)tf_client_thread_proc(context->instance);

	if (freerdp_client_stop(context) != 0)
		rc = -1;

fail:
	freerdp_client_context_free(context);
	return rc;
}