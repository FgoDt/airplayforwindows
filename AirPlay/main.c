#define _CRT_SECURE_NO_WARNINGS
#include "dnssd.h"
#include "raop.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include <Windows.h>
#include "airplay.h"
#include "audio.h"

#pragma comment( lib, "ws2_32.lib" )
#pragma comment(lib,"Winmm.lib")

typedef struct {
	char apname[56];
	char password[56];
	unsigned short port_raop;
	unsigned short port_airplay;
	char hwaddr[6];

	char ao_driver[56];
	char ao_devicename[56];
	char ao_deviceid[16];
	int	 enable_airplay;

} shairpaly_options_t;

static int running;

#ifndef WIN32

#include  <signal.h>

static void 
signal_handler(int sig)
{
	switch (sig)
	{
	case SIGINT:
	case SGTERM:
		running = 0;
		break;
	default:
		break;
	}
}
static void
init_signals(void)
{
	struct sigaction sigact;
	sigact.sa_handler = signal_handler;
	sigemptyset(&sigact.sa_mask);
	sigact.sa_flags = 0;
	sigaction(SIGINT, &sigact, NULL);
	sigaction(SIGTERM, &sigact, NULL);
}

#endif


static int
parse_hwaddr(const char *str, char *hwaddr, int hwaddrlen)
{
	int slen, i;
	slen = 3 * hwaddrlen - 1;
	if (strlen(str)!=slen)
	{
		return 1;
	}

	for (i = 0; i < slen; i++)
	{
		if (str[i] == ':' && (i % 3 == 2)) {
			continue;
		}
		if (str[i] >= '0' && str[i] <= '9') {
			continue;
		}
		if (str[i] >= 'a' && str[i] <= 'f') {
			continue;
		}
		return 1;
	}

	for (i = 0; i < hwaddrlen; i++)
	{
		hwaddr[i] = (char)strtol(str + (i * 3), NULL, 16);
	}
	return 0;
}

static int 
parse_options(shairpaly_options_t *opt,int argc,char *argv[])
{
	const char default_hwaddr[] = { 0x00, 0x24, 0xd7, 0xb2, 0x2e, 0x60 };

	strncpy(opt->apname, "fuckPlay", sizeof(opt->apname) - 1);
	opt->port_raop = 5000;
	opt->port_airplay = 7000;
	memcpy(opt->hwaddr, default_hwaddr, sizeof(opt->hwaddr));
	opt->enable_airplay = 1;
}

static void *
audio_init(void *cls, int bits, int channels, int samplerate)
{

}

static void
audio_process(void *cls, void *opaque, const void *buffer, int buflen)
{
}

static void
audio_destroy(void *cls, void *opaque)
{

}

static void
audio_set_volume(void *cls, void *opaque, float volume)
{

}

int main()
{
	shairpaly_options_t options;

	dnssd_t *dnssd;
	raop_t *raop;
	raop_callbacks_t raop_cbs;
	airplay_t *airplay;
	airplay_callbacks_t airplay_cbs;

	const char default_hwaddr[] = { 0x00, 0x24, 0xd7, 0xb2, 0x2e, 0x60 };

	char *password = NULL;
	int error;

	memset(&options, 0, sizeof(options));

	strncpy(options.apname, "xplay", sizeof(options.apname) - 1);
	options.port_raop = 5000;
	options.port_airplay = 7000;

	memcpy(options.hwaddr, default_hwaddr, sizeof(options.hwaddr));
	options.enable_airplay = 1;
	
	//raop

	audio_prepare(&raop_cbs);
	raop = raop_init_from_keyfile(10, &raop_cbs, "airport.key", NULL);
	raop_set_log_level(raop, RAOP_LOG_DEBUG);
	raop_start(raop, &options.port_raop, options.hwaddr, sizeof(options.hwaddr), password);


	//airplay
	memset(&airplay_cbs, 0, sizeof(airplay_cbs));
	airplay_cbs.cls = NULL;
	airplay_cbs.audio_init = audio_init;
	airplay_cbs.audio_process = audio_process;
	airplay_cbs.audio_destroy = audio_destroy;

	airplay = airplay_init_from_keyfile(10, &airplay_cbs, "airport.key", NULL);
	airplay_set_log_level(airplay, AIRPLAY_LOG_DEBUG);
	airplay_start(airplay, &options.port_airplay, options.hwaddr, sizeof(options.hwaddr), password);


	//dnssd
	error = 0;
	dnssd = dnssd_init(&error);
	if (error)
	{
		raop_destroy(raop);
//		airplay_destroy(airplay);
		return -1;
	}
	dnssd_register_raop(dnssd, options.apname, options.port_raop, options.hwaddr, sizeof(options.hwaddr), password);
	dnssd_register_airplay(dnssd, options.apname, options.port_airplay, options.hwaddr, sizeof(options.hwaddr));
	running = 1;
	while (running)
	{
		Sleep(1000);
	}

	return 0;
}