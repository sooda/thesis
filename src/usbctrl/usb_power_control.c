/*****************************************************************************/

/*
 *      usb_power_control.c  --  Power on/off and reset USB bus
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*****************************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdarg.h>

#include <usb.h>

#define _GNU_SOURCE
#include <getopt.h>

enum action_type {
	nothing,
	reset,
	power_on,
	power_off
};

#define USB_DIR_IN					0x80
#define USB_RT_HUB					(USB_TYPE_CLASS | USB_RECIP_DEVICE)
#define USB_RT_PORT				(USB_TYPE_CLASS | USB_RECIP_OTHER)
#define USB_PORT_FEAT_POWER		8

#define CTRL_TIMEOUT	(5*1000)	/* milliseconds */


struct usb_device *query_devices(int *busnum, int *devnum, int vendorid, int productid)
{
	struct usb_bus		*bus, *usb_busses;
	struct usb_device	*dev, *devs = NULL;

	usb_busses = usb_get_busses();
	for (bus = usb_busses; bus && devs == NULL; bus = bus->next) {
		for (dev = bus->devices; dev && devs == NULL; dev = dev->next) {
			if (vendorid != dev->descriptor.idVendor || productid != dev->descriptor.idProduct) continue;
			*busnum = strtol(bus->dirname, NULL, 10);
			*devnum = strtol(dev->filename, NULL, 10);
			devs = dev;
		}
	}

	return devs;
}

int main(int argc, char *argv[])
{
	int		c, err = EXIT_SUCCESS;
	int		bus = -1, devnum = -1, vendor = -1, product = -1, port = -1;
	char	*cp;
	enum action_type	action = nothing;

	while ((c = getopt(argc, argv, "dep:rv:")) != -1) {
		switch(c) {

		case 'v':
			cp = strchr(optarg, ':');
			if (!cp) {
				err = EXIT_FAILURE;
				break;
			}
			*cp++ = 0;
			if (*optarg)
				vendor = strtoul(optarg, NULL, 16);
			if (*cp)
				product = strtoul(cp, NULL, 16);
			break;

		case 'r':
			action = reset;
			break;

		case 'e':
			action = power_on;
			break;

		case 'd':
			action = power_off;
			break;

		case 'p':
			port = atoi(optarg);
			break;

		case '?':
		default:
			err = EXIT_FAILURE;
			break;
		}
	}
	if (err != EXIT_SUCCESS || argc > optind) {
		fprintf(stderr, "Usage: usb_power_control [options]...\n"
			"Control USB devices\n"
			"  -v vendor:[product]\n"
			"      Specify target vendor and product ID numbers (in hexadecimal)\n"
			"  -e\n"
			"      Power on\n"
			"  -d\n"
			"      Power off\n"
			"  -p port\n"
			"      Specify port number to power on/off\n"
			"  -r reset\n"
			"      Reset target device\n"
			);
		return err;
	}

	usb_init();

	usb_find_busses();
	usb_find_devices();

	struct usb_device *dev;
	dev = query_devices(&bus, &devnum, vendor, product);

	if (dev != NULL) {
		usb_dev_handle	*dh;
		char buf[1024];
		int len;
		if ((dh = usb_open(dev)) == NULL) {
			fprintf(stderr, "usb_open Error.(%s)\n", usb_strerror());
			return EXIT_FAILURE;
		}

		switch (action) {
			case reset:
				usb_reset(dh);
				/* Cannot close dh, because dh is no more avairable */
				break;

			case power_on:
			case power_off:
				if (dev->descriptor.bDeviceClass != USB_CLASS_HUB) {
					fprintf (stderr, "Device is not hub.\n");
					err = EXIT_FAILURE;
					break;
				}
				if ((len = usb_control_msg(dh, USB_DIR_IN | USB_RT_HUB, USB_REQ_GET_DESCRIPTOR, USB_DT_HUB << 8, 0, buf, 1024, CTRL_TIMEOUT)) > 0) {
					unsigned int wHubChar = (buf[4] << 8) | buf[3];

					if (((wHubChar & 0x03) != 0) && ((wHubChar & 0x03) != 1)) {
						fprintf(stderr, "Cannot power switching (usb 1.0).\n");
						break;
					}

					int		request, feature;
					if (action == power_on) {
						request = USB_REQ_SET_FEATURE;
						feature = USB_PORT_FEAT_POWER;
					} else {
						request = USB_REQ_CLEAR_FEATURE;
						feature = USB_PORT_FEAT_POWER;
					}
					if (port > 0) {
						if (usb_control_msg(dh, USB_RT_PORT, request, feature, port, NULL, 0, CTRL_TIMEOUT) < 0) {
							perror ("Failed to control power.\n");
							err = EXIT_FAILURE;
						}
					} else {
						int k;
						for (k = 1; k <= buf[2] && err == EXIT_SUCCESS; k++) {
							if (usb_control_msg(dh, USB_RT_PORT, request, feature, k, NULL, 0, CTRL_TIMEOUT) < 0) {
								perror ("Failed to control power.\n");
								err = EXIT_FAILURE;
							}
						}
					}
				} else {
					perror ("usb_get_descriptor");
					err = EXIT_FAILURE;
				}
				usb_release_interface(dh, 0);
				usb_close (dh);
				err = EXIT_SUCCESS;
				break;

			default:
				break;
		}

	} else {
		fprintf(stderr, "Target device not found\n");
		err = EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}


