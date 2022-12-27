#include "usb_device.h"

#include <iostream>

USBDevice::USBDevice()
{
	std::cout << "[USBDevice] ctor" << std::endl;

	libusb_device = nullptr;
	libusb_device_handle = nullptr;
	can_be_opened = false;
}

USBDevice::~USBDevice()
{
	std::cout << "[USBDevice] dtor" << std::endl;

	libusb_device = nullptr;
	libusb_device_handle = nullptr;
	can_be_opened = false;
}

void USBDevice::retrieve_device_information()
{
	if (libusb_device == nullptr)
	{
		return;
	}

	auto result = libusb_get_device_descriptor(libusb_device, &libusb_device_descriptor);
	if (result < 0)
	{
		return;
	}

	result = libusb_open(libusb_device, &libusb_device_handle);
	if (result < 0)
	{
		can_be_opened = false;
		return;
	}

	can_be_opened = true;

	result = libusb_get_string_descriptor_ascii(libusb_device_handle,
		libusb_device_descriptor.iProduct, (unsigned char*)string_descriptor_ascii,
		sizeof(string_descriptor_ascii));
}

void USBDevice::display_device_information()
{
	std::cout << std::endl << "Device Information:" << std::endl;

	std::cout << "VendorId:ProductId [" 
		<< std::setfill('0') << std::hex << std::setw(4) << std::right
		<< libusb_device_descriptor.idVendor 
		<< ":" 
		<< std::setfill('0') << std::hex << std::setw(4) << std::right
		<< libusb_device_descriptor.idProduct 
		<< "]"
		<< std::endl;

	auto usbHumanNameDeviceDescriptor = usbHumanName->findByVendorIdAndProductId(libusb_device_descriptor.idVendor, libusb_device_descriptor.idProduct);
	if (usbHumanNameDeviceDescriptor != nullptr)
	{
		std::cout << "VendorName: \"" << usbHumanNameDeviceDescriptor->vendorName << "\" ProductName: \"" << usbHumanNameDeviceDescriptor->productName << "\"" << std::endl;
	}

	/*auto usbHumanNameDeviceDescriptor = usbHumanName->findByProductId(libusb_device_descriptor.idProduct);
	if (usbHumanNameDeviceDescriptor != nullptr)
	{
		std::cout << "VendorName: \"" << usbHumanNameDeviceDescriptor->vendorName << "\" ProductName: \"" << usbHumanNameDeviceDescriptor->productName << "\"" << std::endl;
	}

	usbHumanNameDeviceDescriptor = usbHumanName->findByVendorId(libusb_device_descriptor.idVendor);
	if (usbHumanNameDeviceDescriptor != nullptr)
	{
		std::cout << "VendorName: \"" << usbHumanNameDeviceDescriptor->vendorName << "\" ProductName: \"" << usbHumanNameDeviceDescriptor->productName << "\"" << std::endl;
	}*/

	std::cout << "iProduct: "	
		<< std::setfill('0') << std::hex << std::setw(4) << std::right
		<< static_cast<int>(libusb_device_descriptor.iProduct)
		<< std::endl;

	std::cout << "can_be_opened: " << can_be_opened << std::endl;

	if (can_be_opened) 
	{
		std::cout << "string_descriptor_ascii: \"" << string_descriptor_ascii << "\"" << std::endl;
	}
}

uint8_t USBDevice::scan_for_bt_endpoints()
{
	uint32_t result;

	// get endpoints from interface descriptor for currently active configuration
	struct libusb_config_descriptor* config_descriptor = nullptr;
	result = libusb_get_active_config_descriptor(libusb_device, &config_descriptor);
	if (result != 0)
	{
		return result;
	}

	int num_interfaces = config_descriptor->bNumInterfaces;
	std::cout << "Active configuration has " << num_interfaces << " interfaces" << std::endl;

	for (int i = 0; i < num_interfaces; i++)
	{
		const struct libusb_interface* interface = &config_descriptor->interface[i];
		const struct libusb_interface_descriptor* interface_descriptor = interface->altsetting;

		// DEBUG
		//		printf("interface %u: %u endpoints\n", i, interface_descriptor->bNumEndpoints);

		std::cout << "    * Interface " << i << " has " << static_cast<int>(interface_descriptor->bNumEndpoints) << " endpoints" << std::endl;

		const struct libusb_endpoint_descriptor *endpoint = interface_descriptor->endpoint;

		for (int j = 0; j < interface_descriptor->bNumEndpoints; j++, endpoint++)
		{
			//			printf("- endpoint %x, attributes %x\n", endpoint->bEndpointAddress, endpoint->bmAttributes);

			std::cout << "        endpoint address: " 
				<< std::setfill('0') << std::hex << std::setw(2) << std::right
				<< static_cast<int>(endpoint->bEndpointAddress) 
				<< " attributes: " 
				<< std::setfill('0') << std::hex << std::setw(2) << std::right
				<< static_cast<int>(endpoint->bmAttributes) << std::endl;

			switch (endpoint->bmAttributes & 0x03)
			{

			case LIBUSB_TRANSFER_TYPE_INTERRUPT:
				//printf("-> LIBUSB_TRANSFER_TYPE_INTERRUPT 0x%2.2X \n", endpoint->bEndpointAddress);
				std::cout << "          -> LIBUSB_TRANSFER_TYPE_INTERRUPT " << std::endl;
				break;

			case LIBUSB_TRANSFER_TYPE_BULK:
				//printf("-> LIBUSB_TRANSFER_TYPE_BULK 0x%2.2X \n", endpoint->bEndpointAddress);
				std::cout << "          -> LIBUSB_TRANSFER_TYPE_BULK " << std::endl;
				break;

			case LIBUSB_TRANSFER_TYPE_ISOCHRONOUS:
				//printf("-> LIBUSB_TRANSFER_TYPE_ISOCHRONOUS 0x%2.2X \n", endpoint->bEndpointAddress);
				std::cout << "          -> LIBUSB_TRANSFER_TYPE_ISOCHRONOUS " << std::endl;
				break;

			default:
				std::cout << "          -> Unknown interface" << std::endl;
				break;
			}
		}
	}

	if (config_descriptor != nullptr)
	{
		libusb_free_config_descriptor(config_descriptor);
		config_descriptor = nullptr;
	}

	return 0;
}

uint16_t USBDevice::get_vendor_id()
{
	return libusb_device_descriptor.idVendor;
}

uint16_t USBDevice::get_device_id()
{
	return libusb_device_descriptor.idProduct;
}

libusb_error USBDevice::open()
{
	if (nullptr != libusb_device_handle)
	{
		return libusb_error::LIBUSB_SUCCESS;
	}

	can_be_opened = true;
	libusb_error result = static_cast<libusb_error>(libusb_open(libusb_device, &libusb_device_handle));
	if (result < 0)
	{
		can_be_opened = false;
	}

	return result;
}

bool USBDevice::is_open() 
{
	return libusb_device_handle != nullptr;
}