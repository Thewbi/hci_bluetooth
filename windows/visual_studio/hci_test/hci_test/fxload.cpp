/*
 * Copyright � 2001 Stephen Williams (steve@icarus.com)
 * Copyright � 2001-2002 David Brownell (dbrownell@users.sourceforge.net)
 * Copyright � 2008 Roger Williams (rawqux@users.sourceforge.net)
 * Copyright � 2012 Pete Batard (pete@akeo.ie)
 * Copyright � 2013 Federico Manzan (f.manzan@gmail.com)
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

//#include <config.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <sys/types.h>
//#include <getopt.h>
#include <XGetopt.h>
#include "atlstr.h"



#include "libusb.h"
#include "ezusb.h"

#if !defined(_WIN32) || defined(__CYGWIN__)
#include <syslog.h>
static bool dosyslog = false;
#include <strings.h>
#define _stricmp strcasecmp
#endif

#ifndef FXLOAD_VERSION
#define FXLOAD_VERSION (__DATE__ " (libusb)")
#endif

#ifndef ARRAYSIZE
#define ARRAYSIZE(A) (sizeof(A)/sizeof((A)[0]))
#endif



static int print_usage(int error_code) {
	fprintf(stderr, "\nUsage: fxload [-v] [-V] [-t type] [-d vid:pid] [-p bus,addr] [-s loader] -i firmware\n");
	fprintf(stderr, "  -i <path>       -- Firmware to upload\n");
	fprintf(stderr, "  -s <path>       -- Second stage loader\n");
	fprintf(stderr, "  -t <type>       -- Target type: an21, fx, fx2, fx2lp, fx3\n");
	fprintf(stderr, "  -d <vid:pid>    -- Target device, as an USB VID:PID\n");
	fprintf(stderr, "  -p <bus,addr>   -- Target device, as a libusb bus number and device address path\n");
	fprintf(stderr, "  -v              -- Increase verbosity\n");
	fprintf(stderr, "  -q              -- Decrease verbosity (silent mode)\n");
	fprintf(stderr, "  -V              -- Print program version\n");
	return error_code;
}

#define MAX_LENGTH 256

#define FIRMWARE 0
#define LOADER 1
int main_deactivated(int argc, char* argv[])
{
	printf("Worst app ever!\n");


	fx_known_device known_device[] = FX_KNOWN_DEVICES;
	const char *path[] = { NULL, NULL };

	//const char *device_id = NULL;
	//const TCHAR *device_id = NULL;
	//char device_id[MAX_LENGTH];
	char device_id[MAX_LENGTH];

	//const char *device_path = getenv("DEVICE");
	char device_path[MAX_LENGTH];

	char firmware_path[MAX_LENGTH];

	const char *type = NULL;
	const char *fx_name[FX_TYPE_MAX] = FX_TYPE_NAMES;
	const char *ext, *img_name[] = IMG_TYPE_NAMES;
	int fx_type = FX_TYPE_UNDEFINED, img_type[ARRAYSIZE(path)];
	int opt, status;
	unsigned int i, j;
	unsigned vid = 0, pid = 0;
	unsigned busnum = 0, devaddr = 0, _busnum, _devaddr;
	libusb_device *dev, **devs;
	libusb_device_handle *device = NULL;
	struct libusb_device_descriptor desc;

	
	size_t nNumCharConverted;

	/*TCHAR** tchar_arg = new TCHAR*[argc];
	
	for (int i = 0; i < argc; i++)
	{
		tchar_arg[i] = (TCHAR*)argv[i];

		printf("Original Parameter: %s\n", argv[i]);
		printf("Converted Parameter: %s\n", tchar_arg[i]);
	}*/

	//argc = 2;
	//TCHAR** tchar_arg = new TCHAR*[argc];
	//tchar_arg[0] = (TCHAR*)("fxload.exe");
	//tchar_arg[1] = (TCHAR*)("-v");
	
	//tchar_arg[0] = TEXT("-v");

	USES_CONVERSION;

	/*
	const int argcc = 5;
	TCHAR *argvv[argcc];

	argvv[0] = new TCHAR[MAX_LENGTH];
	_tcscpy(argvv[0], A2T("fxload"));

	argvv[1] = new TCHAR[MAX_LENGTH];
	_tcscpy(argvv[1], A2T("-v"));

	argvv[2] = new TCHAR[MAX_LENGTH];
	_tcscpy(argvv[2], A2T("-V"));

	argvv[3] = new TCHAR[MAX_LENGTH];
	_tcscpy(argvv[3], A2T("-d 04B4:8613"));

	argvv[4] = new TCHAR[MAX_LENGTH];
	_tcscpy(argvv[4], A2T("-i C:\aaa_se\fx2lp\fx2lib\examples\bulkloop\build\bulkloop.hex"));
	*/

	/**/
	const int argcc = argc;
	TCHAR **argvv = nullptr;
	argvv = new TCHAR*[argc];
	for (int i = 0; i < argcc; i++)
	{
		argvv[i] = new TCHAR[256];
		_tcscpy(argvv[i], A2T(argv[i]));
	}

	// convert the format string to TCHAR
	TCHAR * optstring = new TCHAR[256];
	_tcscpy(optstring, A2T("qvV?hd:p:i:I:s:S:t:"));

	device_id[0] = NULL;
	device_path[0] = NULL;
	
	while ((opt = getopt(argcc, argvv, optstring)) != EOF)
	{
		printf("opt: %d\n");

		switch (opt) {

		// Target device, as an USB VID:PID
		case 'd':

			printf("d\n");
			
			//device_id = optarg;

			wcstombs_s(&nNumCharConverted, device_id, MAX_LENGTH,
				optarg, MAX_LENGTH);
			
			if (sscanf(device_id, "%x:%x", &vid, &pid) != 2) 
			{
				fputs("please specify VID & PID as \"vid:pid\" in hexadecimal format\n", stderr);
				return -1;
			}
			break;

		// Target device, as a libusb bus number and device address path
		case 'p':

			printf("p\n");
			//device_path = optarg;

			wcstombs_s(&nNumCharConverted, device_path, MAX_LENGTH,
				optarg, MAX_LENGTH);

			if (sscanf(device_path, "%u,%u", &busnum, &devaddr) != 2) {
				fputs("please specify bus number & device number as \"bus,dev\" in decimal format\n", stderr);
				return -1;
			}
			break;

		// Firmware to upload
		case 'i':
		case 'I':
			printf("i - Firmware to upload\n");
			//path[FIRMWARE] = (const char*)optarg;

			wcstombs_s(&nNumCharConverted, firmware_path, MAX_LENGTH,
				optarg, MAX_LENGTH);

			path[FIRMWARE] = firmware_path;

			printf("i - Firmware to upload - %s\n", path[FIRMWARE]);

			break;

		// Second stage loader
		case 's':
		case 'S':
			printf("s\n");
			path[LOADER] = (const char*)optarg;
			break;

		// Print program version
		case 'V':
			printf("V - Print program version\n");
			puts(FXLOAD_VERSION);
			//return 0;
			break;

		// Target type: an21, fx, fx2, fx2lp, fx3
		case 't':
			printf("t\n");
			type = (const char*)optarg;
			break;

		// Increase verbosity
		case _T('v - Increase verbosity'):
			printf("v\n");
			verbose++;
			break;

		// Decrease verbosity (silent mode)
		case 'q':
			printf("q\n");
			verbose--;
			break;

		// help
		case '?':
		case 'h':
		default:
			printf("help\n");
			return print_usage(-1);

		}
	}

	if (path[FIRMWARE] == NULL) {
		logerror("no firmware specified!\n");
		return print_usage(-1);
	}
	if ((device_id[0] != NULL) && (device_path[0] != NULL)) {
		logerror("only one of -d or -p can be specified\n");
		return print_usage(-1);
	}

	/* determine the target type */
	if (type != NULL) {
		for (i = 0; i < FX_TYPE_MAX; i++) {
			if (strcmp(type, fx_name[i]) == 0) {
				fx_type = i;
				break;
			}
		}
		if (i >= FX_TYPE_MAX) {
			logerror("illegal microcontroller type: %s\n", type);
			return print_usage(-1);
		}
	}

	/* open the device using libusb */
	status = libusb_init(NULL);
	if (status < 0) {
		logerror("libusb_init() failed: %s\n", libusb_error_name(status));
		return -1;
	}
	libusb_set_option(NULL, LIBUSB_OPTION_LOG_LEVEL, verbose);

	/* try to pick up missing parameters from known devices */
	if ((type == NULL) || (device_id == NULL) || (device_path != NULL)) {
		if (libusb_get_device_list(NULL, &devs) < 0) {
			logerror("libusb_get_device_list() failed: %s\n", libusb_error_name(status));
			goto err;
		}
		for (i = 0; (dev = devs[i]) != NULL; i++) {
			_busnum = libusb_get_bus_number(dev);
			_devaddr = libusb_get_device_address(dev);
			if ((type != NULL) && (device_path != NULL)) {
				// if both a type and bus,addr were specified, we just need to find our match
				if ((libusb_get_bus_number(dev) == busnum) && (libusb_get_device_address(dev) == devaddr))
					break;
			}
			else {
				status = libusb_get_device_descriptor(dev, &desc);
				if (status >= 0) {
					if (verbose >= 3) {
						logerror("examining %04x:%04x (%d,%d)\n",
							desc.idVendor, desc.idProduct, _busnum, _devaddr);
					}
					for (j = 0; j < ARRAYSIZE(known_device); j++) {
						if ((desc.idVendor == known_device[j].vid)
							&& (desc.idProduct == known_device[j].pid)) {
							if (// nothing was specified
								((type == NULL) && (device_id == NULL) && (device_path == NULL)) ||
								// vid:pid was specified and we have a match
								((type == NULL) && (device_id != NULL) && (vid == desc.idVendor) && (pid == desc.idProduct)) ||
								// bus,addr was specified and we have a match
								((type == NULL) && (device_path != NULL) && (busnum == _busnum) && (devaddr == _devaddr)) ||
								// type was specified and we have a match
								((type != NULL) && (device_id == NULL) && (device_path == NULL) && (fx_type == known_device[j].type))) {
								fx_type = known_device[j].type;
								vid = desc.idVendor;
								pid = desc.idProduct;
								busnum = _busnum;
								devaddr = _devaddr;
								break;
							}
						}
					}
					if (j < ARRAYSIZE(known_device)) {
						if (verbose)
							logerror("found device '%s' [%04x:%04x] (%d,%d)\n",
								known_device[j].designation, vid, pid, busnum, devaddr);
						break;
					}
				}
			}
		}
		if (dev == NULL) {
			libusb_free_device_list(devs, 1);
			libusb_exit(NULL);
			logerror("could not find a known device - please specify type and/or vid:pid and/or bus,dev\n");
			return print_usage(-1);
		}
		status = libusb_open(dev, &device);
		libusb_free_device_list(devs, 1);
		if (status < 0) {
			logerror("libusb_open() failed: %s\n", libusb_error_name(status));
			goto err;
		}
	}
	else if (device_id != NULL) {
		device = libusb_open_device_with_vid_pid(NULL, (uint16_t)vid, (uint16_t)pid);
		if (device == NULL) {
			logerror("libusb_open() failed\n");
			goto err;
		}
	}

	/* We need to claim the first interface */
	libusb_set_auto_detach_kernel_driver(device, 1);
	status = libusb_claim_interface(device, 0);
	if (status != LIBUSB_SUCCESS) {
		libusb_close(device);
		logerror("libusb_claim_interface failed: %s\n", libusb_error_name(status));
		goto err;
	}

	if (verbose)
		logerror("microcontroller type: %s\n", fx_name[fx_type]);

	for (i = 0; i < ARRAYSIZE(path); i++) {
		if (path[i] != NULL) {
			ext = path[i] + strlen(path[i]) - 4;
			if ((_stricmp(ext, ".hex") == 0) || (strcmp(ext, ".ihx") == 0))
				img_type[i] = IMG_TYPE_HEX;
			else if (_stricmp(ext, ".iic") == 0)
				img_type[i] = IMG_TYPE_IIC;
			else if (_stricmp(ext, ".bix") == 0)
				img_type[i] = IMG_TYPE_BIX;
			else if (_stricmp(ext, ".img") == 0)
				img_type[i] = IMG_TYPE_IMG;
			else {
				logerror("%s is not a recognized image type\n", path[i]);
				goto err;
			}
		}
		if (verbose && path[i] != NULL)
			logerror("%s: type %s\n", path[i], img_name[img_type[i]]);
	}

	if (path[LOADER] == NULL) {
		/* single stage, put into internal memory */
		if (verbose > 1)
			logerror("single stage: load on-chip memory\n");
		status = ezusb_load_ram(device, path[FIRMWARE], fx_type, img_type[FIRMWARE], 0);
	}
	else {
		/* two-stage, put loader into internal memory */
		if (verbose > 1)
			logerror("1st stage: load 2nd stage loader\n");
		status = ezusb_load_ram(device, path[LOADER], fx_type, img_type[LOADER], 0);
		if (status == 0) {
			/* two-stage, put firmware into internal memory */
			if (verbose > 1)
				logerror("2nd state: load on-chip memory\n");
			status = ezusb_load_ram(device, path[FIRMWARE], fx_type, img_type[FIRMWARE], 1);
		}
	}

	libusb_release_interface(device, 0);
	libusb_close(device);
	libusb_exit(NULL);
	return status;
err:
	libusb_exit(NULL);
	return -1;
}