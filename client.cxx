#include <cstring>
#include <iostream>
#include <ostream>
#define TAG CLIENT_TAG("josepg.common.cmdline")

#include <freerdp/freerdp.h>
#include <freerdp/utils/signal.h>
#include <winpr/wtypes.h>
#include <winpr/wlog.h>
#include <freerdp/settings.h>
#include <freerdp/client.h>
#include <freerdp/channels/urbdrc.h>
#include <freerdp/locale/locale.h>
#include <freerdp/channels/channels.h>

#include <winpr/assert.h>
#include <winpr/string.h>
#include <winpr/crt.h>
#include <winpr/wlog.h>
#include <winpr/path.h>
#include <winpr/ncrypt.h>
#include <winpr/environment.h>
#include <winpr/timezone.h>

#if defined(CHANNEL_AINPUT_CLIENT)
#include <freerdp/channels/ainput.h>
#endif

#include <freerdp/channels/audin.h>
#include <freerdp/channels/echo.h>

#include <freerdp/client/cmdline.h>
#include <freerdp/version.h>
#include <freerdp/client.h>

#include <freerdp/channels/drdynvc.h>
#include <freerdp/channels/cliprdr.h>
#include <freerdp/channels/encomsp.h>
#include <freerdp/channels/rdpear.h>
#include <freerdp/channels/rdp2tcp.h>
#include <freerdp/channels/remdesk.h>
#include <freerdp/channels/rdpsnd.h>
#include <freerdp/channels/disp.h>
#include <freerdp/client/channels.h>


BOOL option_equals(const char *what, const char *val)
{
	WINPR_ASSERT(what);
	WINPR_ASSERT(val);
	return _stricmp(what, val) == 0;
}
static BOOL freerdp_path_valid(const char *path, BOOL *special)
{
	const char DynamicDrives[] = "DynamicDrives";
	BOOL isPath = FALSE;
	BOOL isSpecial = 0;
	if (!path)
		return FALSE;

	isSpecial =
		(option_equals("*", path) || option_equals(DynamicDrives, path) || option_equals("%", path))
			? TRUE
			: FALSE;
	if (!isSpecial)
		isPath = winpr_PathFileExists(path);

	if (special)
		*special = isSpecial;

	return isSpecial || isPath;
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
extern BOOL VCAPITYPE drdynvc_VirtualChannelEntryEx(PCHANNEL_ENTRY_POINTS,PVOID);

static BOOL freerdp_client_load_static_channel_addin(rdpChannels *channels, rdpSettings *settings,
													 const char *name, void *data)
{
	PVIRTUALCHANNELENTRY entry = NULL;
	PVIRTUALCHANNELENTRY pvce = freerdp_load_channel_addin_entry(
		name, NULL, NULL, FREERDP_ADDIN_CHANNEL_STATIC | FREERDP_ADDIN_CHANNEL_ENTRYEX);
	PVIRTUALCHANNELENTRYEX pvceex = WINPR_FUNC_PTR_CAST(pvce, PVIRTUALCHANNELENTRYEX);

	if (!pvceex)
		entry = freerdp_load_channel_addin_entry(name, NULL, NULL, FREERDP_ADDIN_CHANNEL_STATIC);

		printf("Loading 111 static channel: %s\n", name);
	if (pvceex)
	{
				printf("Loading 222 static channel: %s\n", name);

		if (freerdp_channels_client_load_ex(channels, settings, pvceex, data) == 0)
		{
			WLog_DBG(TAG, "loading channelEx %s", name);
			return TRUE;
		}
	}
	else if (entry)
	{
				printf("Loading 333 static channel: %s\n", name);

		if (freerdp_channels_client_load(channels, settings, entry, data) == 0)
		{
			WLog_DBG(TAG, "loading channel %s", name);
			return TRUE;
		}
	}

	return FALSE;
}

BOOL freerdp_client_add_static_channel(rdpSettings *settings, size_t count,
									   const char *const *params)
{
	ADDIN_ARGV *_args = NULL;

	if (!settings || !params || !params[0] || (count > INT_MAX))
		return FALSE;

	if (freerdp_static_channel_collection_find(settings, params[0]))
		return TRUE;

	_args = freerdp_addin_argv_new(count, params);

	if (!_args)
		return FALSE;

	if (!freerdp_static_channel_collection_add(settings, _args))
		goto fail;

	return TRUE;
fail:
	freerdp_addin_argv_free(_args);
	return FALSE;
}

typedef struct
{
	FreeRDP_Settings_Keys_Bool settingId;
	const char *channelName;
	void *args;
} ChannelToLoad;
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

// Callback function to verify the certificate
BOOL verify_x509_certificate(freerdp *instance, const unsigned char *common_name,
							 const unsigned long subject, const char *issuer,
							 const UINT16 fingerprint, DWORD host_mismatch)
{
	printf("Verifying certificate:\n");
	printf("Common Name: %s\n", common_name);
	printf("Issuer: %s\n", issuer);
	printf("Host Mismatch: %s\n", host_mismatch ? "Yes" : "No");
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
	std::cout << "client 1\n";

	if (freerdp_settings_get_bool(context->settings, FreeRDP_UseCommonStdioCallbacks))
		set_default_callbacks(context->instance);
	std::cout << "client 2\n";

#ifdef WITH_SSO_MIB
	rdpClientContext *client_context = (rdpClientContext *)context;
	client_context->mibClientWrapper = sso_mib_new(context);
	if (!client_context->mibClientWrapper)
		return ERROR_INTERNAL_ERROR;
#endif
	std::cout << "client 3\n";

	pEntryPoints = context->instance->pClientEntryPoints;
	int res = IFCALLRESULT(CHANNEL_RC_OK, pEntryPoints->ClientStart, context);
	std::cout << "client 4\n";

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

freerdp *freerdp_client_get_instance(rdpContext *context)
{
	if (!context || !context->instance)
		return NULL;

	return context->instance;
}

HANDLE freerdp_client_get_thread(rdpContext *context)
{
	if (!context)
		return NULL;

	return ((rdpClientContext *)context)->thread;
}
static BOOL wlf_client_global_init(void)
{
	// NOLINTNEXTLINE(concurrency-mt-unsafe)
	(void)setlocale(LC_ALL, "");

	if (freerdp_handle_signals() != 0)
		return FALSE;

	return TRUE;
}

static void wlf_client_global_uninit(void)
{
}

static int wlf_logon_error_info(freerdp *instance, UINT32 data, UINT32 type)
{
	const char *str_data = freerdp_get_logon_error_info_data(data);
	const char *str_type = freerdp_get_logon_error_info_type(type);

	if (!instance || !instance->context)
		return -1;
	std::cout << "Logon Error Info %s [%s] " << str_data << " " << str_type << "\n";
	return 1;
}

static BOOL wlf_client_new(freerdp *instance, rdpContext *context)
{
	wObject *obj = NULL;
	if (!instance || !context)
		return FALSE;

	return TRUE;
}

static int wfl_client_start(rdpContext *context)
{
	WINPR_UNUSED(context);
	return 0;
}

static int wlfreerdp_run(freerdp* instance)
{
	HANDLE handles[MAXIMUM_WAIT_OBJECTS] = { 0 };
	DWORD status = WAIT_ABANDONED;

	if (!instance)
		return -1;

	if (!freerdp_connect(instance))
	{
		return -1;
	}

	freerdp_disconnect(instance);
	return WINPR_ASSERTING_INT_CAST(int, status);
}

static int RdpClientEntry(RDP_CLIENT_ENTRY_POINTS *pEntryPoints)
{
	WINPR_ASSERT(pEntryPoints);
	ZeroMemory(pEntryPoints, sizeof(RDP_CLIENT_ENTRY_POINTS));
	pEntryPoints->Version = RDP_CLIENT_INTERFACE_VERSION;
	pEntryPoints->Size = sizeof(RDP_CLIENT_ENTRY_POINTS_V1);
	pEntryPoints->GlobalInit = wlf_client_global_init;
	pEntryPoints->GlobalUninit = wlf_client_global_uninit;
	pEntryPoints->ContextSize = 1000;
	pEntryPoints->ClientNew = wlf_client_new;
	// pEntryPoints->ClientFree = wlf_client_free;
	pEntryPoints->ClientStart = wfl_client_start;
	// pEntryPoints->ClientStop = freerdp_client_common_stop;
	return 0;
}

int main(int argc, char *argv[])
{
	int rc = -1;
	int status = 0;
	RDP_CLIENT_ENTRY_POINTS clientEntryPoints;
	rdpContext *context = NULL;
	rdpSettings *settings = NULL;
	// wlfContext* wlc = NULL;
	wLog *root = WLog_GetRoot();
	WLog_SetLogLevel(root, WLOG_DEBUG); // or WLOG_DEBUG, WLOG_INFO, etc.

	// freerdp_client_warn_deprecated(argc, argv);

	RdpClientEntry(&clientEntryPoints);
	context = freerdp_client_context_new(&clientEntryPoints);

	if (!context)
		goto fail;
	// wlc = (wlfContext*)context;
	settings = context->settings;

	freerdp_settings_set_string(settings, FreeRDP_ServerHostname, "127.0.0.1");
	freerdp_settings_set_string(settings, FreeRDP_Username, "user");
	freerdp_settings_set_string(settings, FreeRDP_Password, "root");

	freerdp_settings_set_bool(settings, FreeRDP_NetworkAutoDetect, TRUE);
	freerdp_settings_set_bool(settings, FreeRDP_IgnoreCertificate, TRUE); // Don't do this in production
	freerdp_settings_set_uint32(settings, FreeRDP_ConnectionType, CONNECTION_TYPE_AUTODETECT);
	(!freerdp_settings_set_bool(settings, FreeRDP_RemoteFxCodec, TRUE) ||
	 !freerdp_settings_set_bool(settings, FreeRDP_SupportGraphicsPipeline, TRUE));
	freerdp_settings_set_bool(settings, FreeRDP_SupportDynamicChannels, TRUE);

	if (freerdp_client_start(context) != 0)
		goto fail;
	// print_rdp_settings(settings);
	rc = wlfreerdp_run(context->instance);

	if (freerdp_client_stop(context) != 0)
		rc = -1;

fail:
	freerdp_client_context_free(context);
	return rc;
}