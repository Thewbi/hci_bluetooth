#pragma once

#include <vector>
#include <memory>

#include <libusb.h>
#include "usb_device.h"
#include "usb-human-name.h"

class USBDeviceFactory
{
public:

	// @param dbFilePath the path the USB vendorId::deviceId names.
	// This parameter is forwarded to the initializer list that creates
	// the USBHumanName instance.
	USBDeviceFactory(const string &dbFilePath);

	// Loads connected devices
	//
	// @param libusb_context - the lib usb context 
	void start(libusb_context* libusb_context);

	// Creates a list of USBDevice from the connected usb devices
	// and places them into the devices parameter
	//
	// @devices a list where the factory will place the created detected devices
	void create(std::vector<std::shared_ptr<USBDevice>>& devices);

	// Performs cleanup by calling libusb::libusb_free_device_list
	void stop();

private:
	// all connected devices. Filled by calling start()
	libusb_device **libusb_devices;

	// amount of connected devices. Filled by calling start()
	ssize_t amount_of_libusb_devices;

	// resolves USB vendor and device id to human readable names
	// read from a text file
	USBHumanName usbHumanName;
};