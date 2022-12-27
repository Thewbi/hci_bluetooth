#pragma once

#include <iostream>
#include <iomanip>

#include <libusb.h>

#include "usb-human-name.h"

class USBDevice
{
public:
	USBDevice();
	~USBDevice();

	/**
	 * Reads the VendorId and the DeviceId as well as the iProduct information.
	 * Then tries to open the device to retrieve the descriptor string.
	 */
	void retrieve_device_information();

	/**
	 * Writes all information about the device to std::cout.
	 */
	void display_device_information();

	uint8_t scan_for_bt_endpoints();

	uint16_t get_vendor_id();

	uint16_t get_device_id();

	libusb_error open();

	bool is_open();

public:
	libusb_device *libusb_device;
	libusb_device_handle *libusb_device_handle;
	struct libusb_device_descriptor libusb_device_descriptor;
	char string_descriptor_ascii[256] = {};
	bool can_be_opened;
	USBHumanName *usbHumanName;
};