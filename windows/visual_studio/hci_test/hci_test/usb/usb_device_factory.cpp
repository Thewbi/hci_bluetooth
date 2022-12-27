#include "usb_device_factory.h"

USBDeviceFactory::USBDeviceFactory(const string &dbFilePath) : usbHumanName(dbFilePath)
{

}

void USBDeviceFactory::start(libusb_context* libusb_context)
{
	// call libusb::libusb_get_device_list() to retrieve all devices
	// and store them into the member variable libusb_devices
	amount_of_libusb_devices = libusb_get_device_list(libusb_context, &libusb_devices);
}

void USBDeviceFactory::create(std::vector<std::shared_ptr<USBDevice>>& devices)
{
	libusb_device *libusb_device = nullptr;

	int device_index = 0;
	while ((libusb_device = libusb_devices[device_index++]) != NULL) 
	{
		auto usbDevicePointer = std::make_shared<USBDevice>();
		usbDevicePointer->libusb_device = libusb_device;
		usbDevicePointer->usbHumanName = &usbHumanName;
		devices.emplace_back(usbDevicePointer);
	}
}

void USBDeviceFactory::stop()
{
	if (*libusb_devices != NULL)
	{
		// If unref_device is set to 1 all devices in the list have their 
		// reference counter decremented once.
		uint8_t unref_device = 1U;
		libusb_free_device_list(libusb_devices, unref_device);
	}

	*libusb_devices = NULL;
	amount_of_libusb_devices = -1;
}