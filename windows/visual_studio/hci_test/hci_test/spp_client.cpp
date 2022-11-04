//#include <string>
//#include <iostream>
//#include <iomanip>
//#include <fstream>
//
//#include <algorithm>
//#include <iterator>
//
//#include <libusb.h>
//#include <windows.h>
//
//#include <map>
//#include <vector>
//#include <array>
//
//#include "hci.h"
//
//#include "libpcap.h"
//#include "pklg.h"
//#include <intrin.h>
//
//#include "command_template.h"
//
//uint32_t wait = 100;
//
//int interface_number = 0;
//
//libusb_context* context = NULL;
//libusb_device_handle* handle = NULL;
//
//#define EVENT_IN_BUFFER_COUNT 3
//static struct libusb_transfer *event_in_transfer[EVENT_IN_BUFFER_COUNT];
//
//#define ACL_IN_BUFFER_COUNT 3
//static struct libusb_transfer *acl_in_transfer[ACL_IN_BUFFER_COUNT];
//
//static struct libusb_transfer *command_out_transfer;
//static struct libusb_transfer *acl_out_transfer;
//
//// endpoint addresses
//static int event_in_addr;
//static int acl_in_addr;
//static int acl_out_addr;
//static int sco_in_addr;
//static int sco_out_addr;
//
//// incoming buffer for HCI Events
//static uint8_t hci_event_in_buffer[EVENT_IN_BUFFER_COUNT][HCI_ACL_BUFFER_SIZE];
//
//// incoming buffer for ACL Packets
//static uint8_t hci_acl_in_buffer[ACL_IN_BUFFER_COUNT][HCI_INCOMING_PRE_BUFFER_SIZE + HCI_ACL_BUFFER_SIZE];
//
//
//
//static int usb_acl_out_active = 0;
//static int usb_command_active = 0;
//
//static void LIBUSB_CALL async_callback(struct libusb_transfer *transfer);
//static void LIBUSB_CALL async_acl_callback(struct libusb_transfer *transfer);
//
//template<size_t OPCODE_LEN, size_t PAYLOAD_LEN>
//static int usb_send_cmd_packet(command_template<OPCODE_LEN, PAYLOAD_LEN>& templ, std::function<void(uint8_t* data, uint8_t data_len, struct libusb_transfer* transfer)> callback);
//
//template<size_t OPCODE_LEN, size_t PAYLOAD_LEN>
//static int usb_send_acl_packet(command_template<OPCODE_LEN, PAYLOAD_LEN>& templ, std::function<void(uint8_t* data, uint8_t data_len, struct libusb_transfer* transfer)> callback);
//
//
///*
// * https://github.com/JohnOrlando/uhd_with_burx/blob/master/host/lib/transport/libusb1_zero_copy.cpp
// *
// * Print status errors of a completed transfer
// * \param lut pointer to an libusb_transfer
// */
//void print_transfer_status(struct libusb_transfer *lut)
//{
//	switch (lut->status)
//	{
//
//	case LIBUSB_TRANSFER_COMPLETED:
//		printf("LIBUSB_TRANSFER_COMPLETED\n");
//		// if (lut->actual_length < lut->length) {
//		//     std::cerr << "USB: transfer completed with short write,"
//		//               << " length = " << lut->length
//		//               << " actual = " << lut->actual_length << std::endl;
//		// }
//
//		// if ((lut->actual_length < 0) || (lut->length < 0)) {
//		//     std::cerr << "USB: transfer completed with invalid response"
//		//               << std::endl;
//		// }
//		break;
//
//	case LIBUSB_TRANSFER_CANCELLED:
//		printf("LIBUSB_TRANSFER_CANCELLED\n");
//		break;
//
//	case LIBUSB_TRANSFER_NO_DEVICE:
//		printf("USB: device was disconnected\n");
//		break;
//
//	case LIBUSB_TRANSFER_OVERFLOW:
//		printf("USB: device sent more data than requested\n");
//		break;
//
//	case LIBUSB_TRANSFER_TIMED_OUT:
//		printf("USB: transfer timed out\n");
//		break;
//
//	case LIBUSB_TRANSFER_STALL:
//		printf("USB: halt condition detected (stalled)\n");
//		break;
//
//	case LIBUSB_TRANSFER_ERROR:
//		printf("USB: transfer failed\n");
//		break;
//
//	default:
//		printf("USB: received unknown transfer status\n");
//		break;
//	}
//}
//
//static void LIBUSB_CALL async_acl_callback(struct libusb_transfer *transfer)
//{
//	//// without this, the incoming events are not received after all transfers have been used once!
//	//transfer->user_data = NULL;
//	//libusb_submit_transfer(transfer);
//
//	// always handling an event as we're called when data is ready
//	struct timeval tv;
//	memset(&tv, 0, sizeof(struct timeval));
//	libusb_handle_events_timeout(NULL, &tv);
//
//	printf("\n\n\n");
//
//	printf("**********************************************************\n");
//	printf("async_acl_callback()\n");
//	printf("**********************************************************\n");
//
//	printf("async_acl_callback() - transfer->endpoint: 0x%02X\n", transfer->endpoint);
//
//	// DEBUG dump raw data
//	printf("<<< ");
//	for (int i = 0; i < transfer->actual_length; i++) {
//		fprintf(stdout, "%02X%s", transfer->buffer[i], (i + 1) % 16 == 0 ? "\n" : " ");
//	}
//	printf("\n");
//
//	//// without this, the incoming events are not received after all transfers have been used once!
//	//transfer->user_data = NULL;
//	//libusb_submit_transfer(transfer);
//}
//
//static void LIBUSB_CALL async_callback(struct libusb_transfer *transfer)
////void async_callback(struct libusb_transfer* transfer)
//{
//	//// without this, the incoming events are not received after all transfers have been used once!
//	//transfer->user_data = NULL;
//	//libusb_submit_transfer(transfer);
//
//	printf("\n\n\n");
//
//	printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
//	printf("async_callback() - transfer->endpoint: 0x%02X\n", transfer->endpoint);
//	printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
//
//	// always handling an event as we're called when data is ready
//	struct timeval tv;
//	memset(&tv, 0, sizeof(struct timeval));
//	libusb_handle_events_timeout(NULL, &tv);
//
//	// DEBUG dump raw data
//	printf("<<< ");
//	for (int i = 0; i < transfer->actual_length; i++) {
//		fprintf(stdout, "%02X%s", transfer->buffer[i], (i + 1) % 16 == 0 ? "\n" : " ");
//	}
//	printf("\n");
//
//	// decode event
//	uint8_t event_type = transfer->buffer[0];
//
//	uint16_t opcode = 0x0000;
//
//	switch (event_type) {
//
//		// 0x03
//		case e_hci_event_code::connect_complete:
//		{
//			printf("hci_event_type: connect_complete (0x03)\n");
//
//			uint8_t parameter_total_length = transfer->buffer[1];
//			uint8_t status = transfer->buffer[2];
//			uint16_t connection_handle = (static_cast<uint8_t>(transfer->buffer[3]) << 8U) + static_cast<uint8_t>(transfer->buffer[4]);
//			// BD-ADDR
//			uint8_t link_type = transfer->buffer[11];
//			uint8_t encryption_type = transfer->buffer[12];
//
//			printf("parameter_total_length: %d\n", parameter_total_length);
//			switch (status)
//			{
//			case 0x00:
//				printf("status: success 0x%02x\n", status);
//				break;
//
//			default:
//				//printf("status: unknown????? 0x%02x\n", status);
//				printf("status error: \"%s\"\n", hci_error_name(status));
//				break;
//			}
//			printf("BD_ADDR of communication partner this client is about to connect to: ");
//			for (int i = (3 + 5); i >= 3; --i) {
//				if (i != 3 + 5)
//				{
//					printf(":");
//				}
//				fprintf(stdout, "%02X%s", transfer->buffer[i], (i + 1) % 16 == 0 ? "\n" : "");
//			}
//			printf("\n");
//			printf("connection_handle: 0x%04x\n", connection_handle);
//			printf("link_type: 0x%02x - ", link_type);
//			switch (link_type)
//			{
//			case 0x01:
//				printf("ACL connection (Data Channels), packet oriented, asynchronous");
//				break;
//
//			default:
//				printf("unknown");
//				break;
//			}
//			printf("\n");
//			printf("encryption_type: 0x%02x - ", encryption_type);
//			switch (encryption_type)
//			{
//			case 0x00:
//				printf("Encryption Disabled");
//				break;
//
//			default:
//				printf("unknown");
//				break;
//			}
//
//			// send next command
//			if (0x00 == status)
//			{
//				Sleep(1000);
//
//				//// TODO: this command has a hard-coded connection_handle of 0x0b00!
//				//// make the payload dynamic!!
//				//printf("\n\n\nsending read_remote_supported_features ...\n");
//				//usb_send_cmd_packet<2, 3>(read_remote_supported_features_template, [](uint8_t* data, uint8_t data_len, struct libusb_transfer* transfer) -> void {
//				//	printf("read_remote_supported_features.callback_function\n");
//				//	
//				//	libusb_submit_transfer(transfer);
//				//	Sleep(wait);
//				//});
//
//				printf("\n\n\nSending connection Request for SDP ...\n");
//
//				command_template<2, 0x0E> tt({ 0xB0, 0x20 }, {
//
//					// total parameter length ( 2 byte)
//					0x0C, 0x00,
//
//					// l2cap length (2 byte)
//					0x08, 0x00,
//
//					// l2cap signaling channel (2 byte)
//					0x01, 0x00,
//
//					// command code (0x02 == connection request) (1 byte)
//					0x02,
//
//					// command identifier (1 byte)
//					0x04,
//
//					// command length (2 byte)
//					0x04, 0x00,
//
//					// Protocol/Service Multiplexer (PSM) - 0x0100 == SDP
//					0x01, 0x00,
//
//					// Source CID - allocated channel 0x4400
//					0x44, 0x00
//					});
//
//				usb_send_acl_packet<2, 0x0E>(tt, [](uint8_t* data, uint8_t data_len, struct libusb_transfer* transfer) -> void {
//					printf("Sending connection Request for SDP .callback_function\n");
//					libusb_submit_transfer(transfer);
//					Sleep(wait);
//				});
//			}
//		}
//		break;
//
//		// 0x05
//		case e_hci_event_code::disconnection_complete:
//		{
//			printf("hci_event_type: disconnection_complete (0x05)\n");
//
//			uint8_t parameter_total_length = transfer->buffer[1];
//			uint8_t status = transfer->buffer[2];
//			uint16_t connection_handle = (static_cast<uint8_t>(transfer->buffer[3]) << 8U) + static_cast<uint8_t>(transfer->buffer[4]);
//			uint8_t reason = transfer->buffer[5];
//
//			printf("parameter_total_length: %d\n", parameter_total_length);
//			switch (status)
//			{
//			case 0x00:
//				printf("status: success 0x%02x\n", status);
//				break;
//
//			default:
//				printf("status: unknown????? 0x%02x\n", status);
//				break;
//			}
//			printf("connection_handle: 0x%04x\n", connection_handle);			
//			printf("reason for disconnect: \"%s\"\n", hci_error_name(reason));
//		}
//		break;
//
//		// 0x0b
//		case read_remote_supported_features_complete:
//		{
//			printf("hci_event_type: read_remote_supported_features_complete (0x0b)\n");
//
//			uint8_t parameter_total_length = transfer->buffer[1];
//			uint8_t status = transfer->buffer[2];
//			uint16_t connection_handle = (static_cast<uint8_t>(transfer->buffer[3]) << 8U) + static_cast<uint8_t>(transfer->buffer[4]);
//
//			printf("parameter_total_length: %d\n", parameter_total_length);
//			switch (status)
//			{
//			case 0x00:
//				printf("status: success 0x%02x\n", status);
//				break;
//
//			default:
//				printf("status: unknown????? 0x%02x\n", status);
//				break;
//			}
//			printf("connection_handle: 0x%04x\n", connection_handle);
//
//			// DEBUG dump raw data
//			printf("feature bitmask: ");
//			for (int i = 5; i < (5+8); i++) {
//				fprintf(stdout, "%02X%s", transfer->buffer[i], (i + 1) % 16 == 0 ? "\n" : " ");
//			}
//			printf("\n");
//
//			// send next command
//			if (0x00 == status)
//			{
//				/*printf("\n\n\nsending write_simple_pairing_mode ...\n");
//				usb_send_cmd_packet<2, 2>(write_simple_pairing_mode_template, [](uint8_t* data, uint8_t data_len, struct libusb_transfer* transfer) -> void {
//					printf("write_simple_pairing_mode_template.callback_function\n");
//					libusb_submit_transfer(transfer);
//					Sleep(wait);
//				});*/
//
//				//printf("\n\n\nSending connection Request for SDP ...\n");
//
//				//command_template<2, 0x0E> tt({0xB0, 0x20}, { 
//
//				//	// total parameter length ( 2 byte)
//				//	0x0C, 0x00,
//
//				//	// l2cap length (2 byte)
//				//	0x08, 0x00,
//
//				//	// l2cap signaling channel (2 byte)
//				//	0x01, 0x00,
//
//				//	// command code (0x02 == connection request) (1 byte)
//				//	0x02,
//
//				//	// command identifier (1 byte)
//				//	0x04,
//
//				//	// command length (2 byte)
//				//	0x04, 0x00,
//
//				//	// Protocol/Service Multiplexer (PSM) - 0x0100 == SDP
//				//	0x01, 0x00,
//
//				//	// Source CID - allocated channel 0x4400
//				//	0x44, 0x00
//				//	});
//
//				//usb_send_cmd_packet<2, 0x0E>(tt, [](uint8_t* data, uint8_t data_len, struct libusb_transfer* transfer) -> void {
//				//	printf("Sending connection Request for SDP .callback_function\n");
//				//	libusb_submit_transfer(transfer);
//				//	Sleep(wait);
//				//});
//			}
//		}
//		break;
//
//		// 0x0e
//		case e_hci_event_code::command_complete:
//		{
//			printf("hci_event_type: command_complete (0x0e)\n");
//
//			uint8_t parameter_total_length = transfer->buffer[1];
//			uint8_t number_of_allowed_packets = transfer->buffer[2];
//			opcode = (static_cast<uint8_t>(transfer->buffer[3]) << 8U) + static_cast<uint8_t>(transfer->buffer[4]);
//			uint8_t status = transfer->buffer[5];
//
//			printf("parameter_total_length: %d\n", parameter_total_length);
//			printf("number_of_allowed_packets: %d\n", number_of_allowed_packets);
//			switch (status)
//			{
//			case 0x00:
//				printf("status: success 0x%02x\n", status);
//				break;
//
//			default:
//				//printf("status: unknown????? 0x%02x\n", status);
//				printf("status error: \"%s\"\n", hci_error_name(status));
//				break;
//			}
//			printf("opcode that caused this event: 0x%04x\n", opcode);
//		}
//		break;
//
//		// command_status 0x0F
//		case e_hci_event_code::command_status:
//		{
//			printf("hci_event_type: command_status (0x0f)\n");
//
//			uint8_t parameter_total_length = transfer->buffer[1];
//			uint8_t status = transfer->buffer[2];
//			uint8_t number_of_allowed_packets = transfer->buffer[3];
//			opcode = (static_cast<uint8_t>(transfer->buffer[4]) << 8U) + static_cast<uint8_t>(transfer->buffer[5]);
//
//			printf("parameter_total_length: %d\n", parameter_total_length);
//			switch (status)
//			{
//			case 0x00:
//				printf("status: currently pending 0x%02x\n", status);
//				break;
//
//			default:
//				printf("status: command failed 0x%02x - %s\n", status, hci_error_name(status));
//				break;
//			}
//			printf("number_of_allowed_packets: %d\n", parameter_total_length);
//			printf("opcode that caused this event: 0x%04x\n", opcode);
//		}
//		break;
//
//		// 0x13
//		case e_hci_event_code::number_of_completed_packets:
//		{
//			printf("hci_event_type: number_of_completed_packets (0x13)\n");
//
//			uint8_t parameter_total_length = transfer->buffer[1];
//			uint8_t number_of_connection_handles = transfer->buffer[2];
//			uint16_t connection_handle = (static_cast<uint8_t>(transfer->buffer[3]) << 8U) + static_cast<uint8_t>(transfer->buffer[4]);
//			uint8_t number_of_completed_packets = (static_cast<uint8_t>(transfer->buffer[5]) << 8U) + static_cast<uint8_t>(transfer->buffer[6]);
//
//			printf("parameter_total_length: %d\n", parameter_total_length);
//			printf("number_of_connection_handles: %d\n", number_of_connection_handles);
//			printf("connection_handle: 0x%04x\n", connection_handle);
//			printf("number_of_completed_packets: 0x%04x\n", number_of_completed_packets);
//		}
//		break;
//
//		// 0x1b
//		case e_hci_event_code::max_slots_changed:
//		{
//			printf("hci_event_type: max_slots_changed (0x1b)\n");
//
//			uint8_t parameter_total_length = transfer->buffer[1];
//			uint16_t connection_handle = (static_cast<uint8_t>(transfer->buffer[2]) << 8U) + static_cast<uint8_t>(transfer->buffer[3]);
//			uint8_t maximum_number_of_slots = transfer->buffer[4];
//			
//			printf("parameter_total_length: %d\n", parameter_total_length);
//			printf("connection_handle: 0x%04x\n", connection_handle);
//			printf("maximum_number_of_slots: %d\n", maximum_number_of_slots);
//		}
//		break;
//
//		default:
//		{
//			printf("Unknown event: event code: 0x%02x\n", event_type);
//		}
//		break;
//	}
//	
//	switch (opcode)
//	{
//		case 0x00:
//		// the event 0x21 0x00 0x00 has no callback
//		break;
//
//		default:
//		{
//			std::map<uint16_t, std::function<void(uint8_t* data, uint8_t data_len, struct libusb_transfer* transfer)>>::iterator it = callback_map.find(opcode);
//			if (it != callback_map.end())
//			{
//				callback_map[opcode](transfer->buffer, transfer->actual_length, transfer);
//			}
//			else
//			{
//				printf("Received event for which now callback was registered!\n");
//				printf("The event is:\n");
//				// DEBUG dump raw data
//				printf("<<< ");
//				for (int i = 0; i < transfer->actual_length; i++) {
//					fprintf(stdout, "%02X%s", transfer->buffer[i], (i + 1) % 16 == 0 ? "\n" : " ");
//				}
//				printf("\n");
//			}
//		}
//		break;
//	}
//
//	//// without this, the incoming events are not received after all transfers have been used once!
//	//transfer->user_data = NULL;
//	//libusb_submit_transfer(transfer);
//}
//
//int init_transfers(libusb_device **devs)
//{
//	/*
//	ASYNC - https://vovkos.github.io/doxyrest/samples/libusb/group_libusb_asyncio.html
//	We can view asynchronous I/O as a 5 step process:
//
//	1. Allocation : allocate a libusb_transfer
//	2. Filling : populate the libusb_transfer instance with information about the transfer you wish to perform
//	3. Submission : ask libusb to submit the transfer
//	4. Completion handling : examine transfer results in the libusb_transfer structure
//	5. Deallocation : clean up resources
//	*/
//
//	// STEP 1 - allocate transfer handlers
//	int c;
//	for (c = 0; c < EVENT_IN_BUFFER_COUNT; c++)
//	{
//		// 0 isochronous transfers Events
//		event_in_transfer[c] = libusb_alloc_transfer(0);
//		if (!event_in_transfer[c])
//		{
//			printf("libusb_alloc_transfer() failed\n");
//
//			libusb_release_interface(handle, interface_number);
//			printf("libusb_close() - closing handle ...\n");
//			libusb_close(handle);
//			context = NULL;
//			printf("libusb_close() - closing handle done\n");
//			libusb_exit(NULL);
//
//			return 0;
//		}
//	}
//	for (c = 0; c < ACL_IN_BUFFER_COUNT; c++)
//	{
//		// 0 isochronous transfers ACL in
//		acl_in_transfer[c] = libusb_alloc_transfer(0);
//		if (!acl_in_transfer[c])
//		{
//			libusb_release_interface(handle, interface_number);
//			printf("libusb_close() - closing handle ...\n");
//			libusb_close(handle);
//			context = NULL;
//			printf("libusb_close() - closing handle done\n");
//			libusb_exit(NULL);
//
//			return 0;
//		}
//	}
//
//	command_out_transfer = libusb_alloc_transfer(0);
//	acl_out_transfer = libusb_alloc_transfer(0);
//
//	// STEP 2 - filling
//
//	for (int c = 0; c < EVENT_IN_BUFFER_COUNT; c++)
//	{
//		printf("<><><><> libusb_fill_interrupt_transfer() <><><><>\n");
//
//		printf("libusb_fill_interrupt_transfer() ...\n");
//		// configure event_in handlers
//		libusb_fill_interrupt_transfer(
//			event_in_transfer[c],
//			handle,
//			event_in_addr,
//			hci_event_in_buffer[c],
//			HCI_ACL_BUFFER_SIZE,
//			async_callback,
//			NULL,
//			0);
//		printf("libusb_fill_interrupt_transfer() done.\n");
//
//		// STEP 3 - submission
//
//		event_in_transfer[c]->user_data = NULL;
//
//		printf("libusb_submit_transfer() ...\n");
//		int result = libusb_submit_transfer(event_in_transfer[c]);
//		if (result)
//		{
//			printf("libusb_submit_transfer() - Error submitting interrupt transfer %d\n", result);
//
//			libusb_release_interface(handle, interface_number);
//			printf("libusb_close() - closing handle ...\n");
//			libusb_close(handle);
//			context = NULL;
//			printf("libusb_close() - closing handle done\n");
//			libusb_exit(NULL);
//
//			return 0;
//		}
//		printf("libusb_submit_transfer() done.\n");
//
//		//printf("print_transfer_status() ...\n");
//		//print_transfer_status(event_in_transfer[c]);
//		//printf("print_transfer_status() done.\n");
//	}
//
//	for (int c = 0; c < ACL_IN_BUFFER_COUNT; c++)
//	{
//		printf("<><><><> libusb_fill_bulk_transfer() <><><><>\n");
//
//		/*acl_in_transfer[c]->flags = LIBUSB_TRANSFER_SHORT_NOT_OK
//		| LIBUSB_TRANSFER_FREE_BUFFER | LIBUSB_TRANSFER_FREE_TRANSFER;*/
//
//		// unable to match endpoint to an open interface
//
//		// 130 = 0x82
//		//
//		//active configuration has 4 interfaces
//		//	interface 0 : 3 endpoints
//		//	- endpoint 81, attributes 3
//		//	-> using 0x81 for HCI Events
//		//	- endpoint 82, attributes 2
//		//	-> using 0x82 for ACL Data In
//		//	- endpoint 2, attributes 2
//		//	-> using 0x02 for ACL Data Out
//		//	interface 1 : 2 endpoints
//		//	- endpoint 83, attributes 1
//		//	-> using 0x83 for SCO Data In
//		//	- endpoint 3, attributes 1
//		//	-> using 0x03 for SCO Data Out
//		//	interface 2 : 2 endpoints
//		//	- endpoint 84, attributes 2
//		//	- endpoint 4, attributes 2
//		//	interface 3 : 0 endpoints
//
//		// configure acl_in handlers
//		void* user_data = NULL;
//		printf("libusb_fill_bulk_transfer() ...\n");
//		uint8_t timeout = 0;
//		libusb_fill_bulk_transfer(
//			acl_in_transfer[c],
//			handle,
//			acl_in_addr,
//			hci_acl_in_buffer[c] + HCI_INCOMING_PRE_BUFFER_SIZE, HCI_ACL_BUFFER_SIZE,
//			async_acl_callback,
//			user_data,
//			timeout);
//		printf("libusb_fill_bulk_transfer() done.\n");
//
//		acl_in_transfer[c]->user_data = NULL;
//
//		// https://stackoverflow.com/questions/56729045/how-to-fix-libusb-error-not-found-error-when-calling-libusb-bulk-transfer
//		printf("libusb_submit_transfer() ...\n");
//		int result = libusb_submit_transfer(acl_in_transfer[c]);
//		if (result) {
//			printf("Error submitting bulk in transfer [%d] %s\n", result, libusb_error_name(result));
//
//			switch (result) {
//
//			case LIBUSB_SUCCESS:
//				printf("LIBUSB_SUCCESS\n", result);
//				break;
//
//			case LIBUSB_ERROR_BUSY:
//				printf("LIBUSB_ERROR_BUSY\n", result);
//				break;
//
//			case LIBUSB_ERROR_NO_DEVICE:
//				printf("LIBUSB_ERROR_NO_DEVICE\n", result);
//				break;
//
//			case LIBUSB_ERROR_NOT_FOUND:
//				printf("LIBUSB_ERROR_NOT_FOUND\n", result);
//				break;
//
//			default:
//				break;
//			}
//
//			std::cout << "libusb_free_device_list() ..." << std::endl;
//			libusb_free_device_list(devs, 1);
//			*devs = NULL;
//			std::cout << "libusb_free_device_list() done." << std::endl;
//
//			libusb_release_interface(handle, 0);
//
//			printf("libusb_close() ...\n");
//			libusb_close(handle);
//			handle = NULL;
//			printf("libusb_close() done\n");
//
//			printf("libusb_exit() ...\n");
//			libusb_exit(context);
//			context = NULL;
//			printf("libusb_exit() done.\n");
//
//			return 0;
//		}
//		printf("libusb_submit_transfer() ...\n");
//
//		//print_transfer_status(acl_in_transfer[c]);
//	}
//}
//
//// copied from hci_transport_h2_libusb.c from bluekitchen's btstack
//static int scan_for_bt_endpoints(libusb_device *dev)
//{
//	int r;
//
//	event_in_addr = 0;
//	acl_in_addr = 0;
//	acl_out_addr = 0;
//	sco_out_addr = 0;
//	sco_in_addr = 0;
//
//	// get endpoints from interface descriptor
//	struct libusb_config_descriptor *config_descriptor;
//	r = libusb_get_active_config_descriptor(dev, &config_descriptor);
//	if (r < 0)
//	{
//		return r;
//	}
//
//	int num_interfaces = config_descriptor->bNumInterfaces;
//	printf("active configuration has %u interfaces\n", num_interfaces);
//
//	int i;
//	for (i = 0; i < num_interfaces; i++)
//	{
//		const struct libusb_interface *interface = &config_descriptor->interface[i];
//		const struct libusb_interface_descriptor * interface_descriptor = interface->altsetting;
//		printf("interface %u: %u endpoints\n", i, interface_descriptor->bNumEndpoints);
//
//		const struct libusb_endpoint_descriptor *endpoint = interface_descriptor->endpoint;
//
//		for (r = 0; r < interface_descriptor->bNumEndpoints; r++, endpoint++)
//		{
//
//			printf("- endpoint %x, attributes %x\n", endpoint->bEndpointAddress, endpoint->bmAttributes);
//
//			switch (endpoint->bmAttributes & 0x03)
//			{
//			case LIBUSB_TRANSFER_TYPE_INTERRUPT:
//				if (event_in_addr) {
//					continue;
//				}
//				event_in_addr = endpoint->bEndpointAddress;
//				printf("-> using 0x%2.2X for HCI Events\n", event_in_addr);
//				break;
//
//			case LIBUSB_TRANSFER_TYPE_BULK:
//				if (endpoint->bEndpointAddress & 0x80) {
//					if (acl_in_addr)
//					{
//						continue;
//					}
//					acl_in_addr = endpoint->bEndpointAddress;
//					printf("-> using 0x%2.2X for ACL Data In\n", acl_in_addr);
//				}
//				else {
//					if (acl_out_addr) continue;
//					acl_out_addr = endpoint->bEndpointAddress;
//					printf("-> using 0x%2.2X for ACL Data Out\n", acl_out_addr);
//				}
//				break;
//
//			case LIBUSB_TRANSFER_TYPE_ISOCHRONOUS:
//				if (endpoint->bEndpointAddress & 0x80) {
//					if (sco_in_addr) continue;
//					sco_in_addr = endpoint->bEndpointAddress;
//					printf("-> using 0x%2.2X for SCO Data In\n", sco_in_addr);
//				}
//				else {
//					if (sco_out_addr) continue;
//					sco_out_addr = endpoint->bEndpointAddress;
//					printf("-> using 0x%2.2X for SCO Data Out\n", sco_out_addr);
//				}
//				break;
//
//			default:
//				printf("Unknown interface");
//				break;
//			}
//		}
//	}
//	libusb_free_config_descriptor(config_descriptor);
//
//	return 0;
//}
//
//int init_asus_bt400(libusb_device *dev, struct libusb_device_descriptor& desc)
//{
//	int ret = libusb_open(dev, &handle);
//	switch (ret) {
//	case LIBUSB_SUCCESS:
//	{
//		printf("libusb_open() succeeded for device [0x%04X:0x%04X] with error [%d] %s\n", desc.idVendor, desc.idProduct, ret, libusb_error_name(ret));
//		
//		char string[256];
//		ret = libusb_get_string_descriptor_ascii(handle, desc.iProduct, (unsigned char*)string, sizeof(string));
//		if (ret > 0)
//		{
//			printf(string);
//			printf("\n");
//		}
//	}
//	break;
//
//	default:
//		printf("libusb_open() failed for device [0x%04X:0x%04X] with error [%d] %s\n", desc.idVendor, desc.idProduct, ret, libusb_error_name(ret));
//
//		//libusb_exit(NULL);
//		//continue;
//		return -1;
//	}
//
//	scan_for_bt_endpoints(dev);
//
//	// https://stackoverflow.com/questions/4813764/libusb-basic-example-wanted
//
//	// // Detach a kernel driver from an interface. This is needed to claim an interface
//	// // already claimed by a kernel driver. Returns 0 on success, LIBUSB_ERROR_NOT_FOUND
//	// // if no kernel driver was active, LIBUSB_ERROR_INVALID_PARAM if the interface does not exist,
//	// // LIBUSB_ERROR_NO_DEVICE if the device has been disconnected and a LIBUSB_ERROR code on failure.
//	// // This function is non-portable.
//	//uint32_t detach_result = libusb_detach_kernel_driver(handle, 0);
//	//printf("libusb_detach_kernel_driver(): %s\n", detach_result ? "failed" : "passed");
//
//	// this usb reset might be another reset compared to the HCI reset!
//	// I think it is necessary first to reset the usb device using this call to libusb_reset_device()
//	// and then later to perform a HCI reset
//	libusb_reset_device(handle);
//	Sleep(wait);
//
//	int configuration = 1;
//	libusb_set_configuration(handle, configuration);
//	Sleep(wait);
//
//	if (libusb_kernel_driver_active(handle, interface_number) == 1)
//	{
//		printf("Kernel Driver Active\n");
//		if (libusb_detach_kernel_driver(handle, interface_number) == 0)
//		{
//			printf("Kernel Driver Detached!\n");
//		}
//		else
//		{
//			printf("Couldn't detach kernel driver!\n");
//			//libusb_free_device_list(devs, 1);
//			libusb_close(handle);
//			return -1;
//		}
//	}
//
//	uint32_t claim_result = libusb_claim_interface(handle, interface_number);
//	printf("libusb_claim_interface(): %s\n", claim_result ? "failed" : "passed");
//	if (claim_result)
//	{
//		printf("libusb_close() - closing handle ...\n");
//		libusb_close(handle);
//		context = NULL;
//		printf("libusb_close() - closing handle done\n");
//		libusb_exit(NULL);
//
//		return 0;
//	}
//
//	Sleep(wait);	
//}
//
//static uint8_t find_dev(libusb_device **devs)
//{
//	libusb_device *dev;
//	int i = 0;
//	int j = 0;
//
//	while ((dev = devs[i++]) != NULL) {
//
//		struct libusb_device_descriptor desc;
//
//		int ret;		
//
//		int r = libusb_get_device_descriptor(dev, &desc);
//		if (r < 0) {
//			fprintf(stderr, "failed to get device descriptor");
//
//			libusb_exit(NULL);
//			return 0;
//		}
//
//		printf("get [%04X:%04X] device string descriptor \n", desc.idVendor, desc.idProduct);
//		printf("iProduct[%d]: ", desc.iProduct);
//
//		if (desc.idVendor == 0x0b05 && desc.idProduct == 0x17cb) {
//
//			printf("ASUS BT400 found!\n");
//
//			//
//			// init device
//			//
//
//			init_asus_bt400(dev, desc);
//
//			//
//			// init transfers
//			// 
//
//			if (!init_transfers(devs))
//			{
//				libusb_free_device_list(devs, 1);
//				return -1;
//			}
//
//			//
//			// HCI command - Perform a HCI reset
//			//
//
//			printf("\n\n\nsending reset_command_template ...\n");
//			usb_send_cmd_packet<2, 1>(reset_command_template, [](uint8_t* data, uint8_t data_len, struct libusb_transfer* transfer) -> void {
//				printf("reset_command_template.callback_function\n");
//				libusb_submit_transfer(transfer);
//
//				printf("\n\n\nsending read_local_version_information_template ...\n");
//				usb_send_cmd_packet<2, 1>(read_local_version_information_template, [](uint8_t* data, uint8_t data_len, struct libusb_transfer* transfer) -> void {
//					printf("read_local_version_information_template.callback_function\n");
//					libusb_submit_transfer(transfer);
//					Sleep(wait);
//
//					printf("\n\n\nsending read_local_name_template ...\n");
//					usb_send_cmd_packet<2, 1>(read_local_name_template, [](uint8_t* data, uint8_t data_len, struct libusb_transfer* transfer) -> void {
//						printf("read_local_name_template.callback_function\n");
//						libusb_submit_transfer(transfer);
//						Sleep(wait);
//
//						printf("\n\n\nsending read_local_supported_commands_template ...\n");
//						usb_send_cmd_packet<2, 1>(read_local_supported_commands_template, [](uint8_t* data, uint8_t data_len, struct libusb_transfer* transfer) -> void {
//							printf("read_local_supported_commands_template.callback_function\n");
//							libusb_submit_transfer(transfer);
//							Sleep(wait);
//
//							printf("\n\n\nsending read_bd_addr_template ...\n");
//							usb_send_cmd_packet<2, 1>(read_bd_addr_template, [](uint8_t* data, uint8_t data_len, struct libusb_transfer* transfer) -> void {
//								printf("read_bd_addr_template.callback_function\n");
//								libusb_submit_transfer(transfer);
//								Sleep(wait);
//
//								printf("\n\n\nsending read_buffer_size_template ...\n");
//								usb_send_cmd_packet<2, 1>(read_buffer_size_template, [](uint8_t* data, uint8_t data_len, struct libusb_transfer* transfer) -> void {
//									printf("read_buffer_size_template.callback_function\n");
//									libusb_submit_transfer(transfer);
//									Sleep(wait);
//
//									printf("\n\n\nsending read_local_supported_features_template ...\n");
//									usb_send_cmd_packet<2, 1>(read_local_supported_features_template, [](uint8_t* data, uint8_t data_len, struct libusb_transfer* transfer) -> void {
//										printf("read_local_supported_features_template.callback_function\n");
//										libusb_submit_transfer(transfer);
//										Sleep(wait);
//
//										printf("\n\n\nsending send_set_event_mask_template ...\n");
//										usb_send_cmd_packet<2, 9>(send_set_event_mask_template, [](uint8_t* data, uint8_t data_len, struct libusb_transfer* transfer) -> void {
//											printf("send_set_event_mask_template.callback_function\n");
//											libusb_submit_transfer(transfer);
//											Sleep(wait);
//
//											/*printf("\n\n\nsending inquiry_template ...\n");
//											usb_send_cmd_packet<2, 6>(inquiry_template, [](uint8_t* data, uint8_t data_len, struct libusb_transfer* transfer) -> void {
//												printf("inquiry_template.callback_function\n");
//												libusb_submit_transfer(transfer);
//												Sleep(wait);
//											});*/
//
//											printf("\n\n\nsending create connection ...\n");
//											usb_send_cmd_packet<2, 0x0E>(create_connection_template, [](uint8_t* data, uint8_t data_len, struct libusb_transfer* transfer) -> void {
//												printf("create_connection_template.callback_function\n");
//												libusb_submit_transfer(transfer);
//												Sleep(wait);
//											});
//										});
//									});
//								});
//							});
//						});
//					});
//				});
//			});
//			//libusb_handle_events(context);
//			Sleep(wait);
//
//			/*printf("sending read_local_version_information_template ...\n");
//			usb_send_cmd_packet<2, 1>(read_local_version_information_template, [](uint8_t* data, uint8_t data_len) -> void {
//				printf("read_local_version_information_template.callback_function\n");
//			});
//			Sleep(wait);*/
//			
//
//			
//
//			//bool is_sent = true;
//			//write_pcaprec_hdr_t(file, 1000, is_sent, hci_packet_type_t::hci_command, idx, buffer);
//
//			Sleep(wait);
//
//			printf("resetting the device done.\n");
//		}
//	}
//}
//
//template<size_t OPCODE_LEN, size_t PAYLOAD_LEN>
//static int usb_send_cmd_packet(command_template<OPCODE_LEN, PAYLOAD_LEN>& templ, std::function<void(uint8_t* data, uint8_t data_len, struct libusb_transfer* transfer)> callback)
//{
//	callback_map[ARRAY_TO_UINT16(templ.opcode)] = callback;
//
//	printf("usb_send_cmd_packet() in main.c - libusb_fill_control_setup(), libusb_submit_transfer()\n");
//
//	/*printf(">>> [size of message: %d] ", size);
//	for (int i = 0; i < size; i++)
//	{
//		fprintf(stdout, "%02X%s", packet[i], (i + 1) % 16 == 0 ? "\n" : " ");
//	}
//	printf("\n");*/
//
//	// outgoing buffer for HCI Command packets
//	static uint8_t hci_cmd_buffer[3 + 256 + LIBUSB_CONTROL_SETUP_SIZE];
//
//	// this will first add a USB specific part to the buffer and after the USB specific part
//	// it will add the HCI packet
//	libusb_fill_control_setup(hci_cmd_buffer, LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE, 0, 0, 0, (OPCODE_LEN + PAYLOAD_LEN));
//	std::copy(std::begin(templ.opcode), std::end(templ.opcode), std::begin(hci_cmd_buffer) + LIBUSB_CONTROL_SETUP_SIZE);
//	std::copy(std::begin(templ.payload), std::end(templ.payload), std::begin(hci_cmd_buffer) + LIBUSB_CONTROL_SETUP_SIZE + OPCODE_LEN);
//
//	// for (int i = 0; i < (3 + 256 + LIBUSB_CONTROL_SETUP_SIZE); ++i) {
//	//     fprintf(stdout, "%02X%s", hci_cmd_buffer[i], ( i + 1 ) % 16 == 0 ? "\r\n" : " " );
//	// }
//	// printf("\n");
//
//	//printf(">>> [size of message: %d] ", size);
//	printf(">>> ");
//	for (int i = LIBUSB_CONTROL_SETUP_SIZE; i < (LIBUSB_CONTROL_SETUP_SIZE + OPCODE_LEN + PAYLOAD_LEN); i++)
//	{
//		fprintf(stdout, "%02X%s", hci_cmd_buffer[i], (i + 1) % 16 == 0 ? "\n" : " ");
//	}
//	printf("\n");
//
//	// prepare transfer
//	int completed = 0;
//	libusb_fill_control_transfer(command_out_transfer, handle, hci_cmd_buffer, async_callback, &completed, 0);
//	command_out_transfer->flags = LIBUSB_TRANSFER_FREE_BUFFER;
//
//	// update state before submitting transfer
//	usb_command_active = 1;
//
//#if 1
//	printf("OUT: ");
//	print_transfer_status(command_out_transfer);
//#endif
//
//	// submit transfer
//	int libusb_submit_transfer_result = 1;
//	while (libusb_submit_transfer_result) 
//	{
//		printf("libusb_submit_transfer() ...\n");
//
//		Sleep(wait);
//
//		libusb_submit_transfer_result = libusb_submit_transfer(command_out_transfer);
//		if (libusb_submit_transfer_result < 0)
//		{
//			usb_command_active = 0;
//
//			printf("\n");
//			printf("!!!!!!!!!!!!!!! Error submitting cmd transfer %d\n", libusb_submit_transfer_result);
//			printf("!!!!!!!!!!!!!!! Error submitting cmd transfer %d\n", libusb_submit_transfer_result);
//			printf("!!!!!!!!!!!!!!! Error submitting cmd transfer %d\n", libusb_submit_transfer_result);
//			printf("!!!!!!!!!!!!!!! Error submitting cmd transfer %d\n", libusb_submit_transfer_result);
//			printf("!!!!!!!!!!!!!!! Error submitting cmd transfer %d\n", libusb_submit_transfer_result);
//
//			switch (libusb_submit_transfer_result)
//			{
//			case LIBUSB_SUCCESS:
//				printf("Success(no error)\n");
//				break;
//
//			case LIBUSB_ERROR_IO:
//				printf("Input / output error.\n");
//				break;
//
//			case LIBUSB_ERROR_INVALID_PARAM:
//				printf("Invalid parameter.\n");
//				break;
//
//			case LIBUSB_ERROR_ACCESS:
//				printf("Access denied(insufficient permissions)\n");
//				break;
//
//			case LIBUSB_ERROR_NO_DEVICE:
//				printf("No such device(it may have been disconnected)\n");
//				break;
//
//			case LIBUSB_ERROR_NOT_FOUND:
//				printf("Entity not found.\n");
//				break;
//
//			case LIBUSB_ERROR_BUSY:
//				printf("Resource busy.\n");
//				break;
//
//			case LIBUSB_ERROR_TIMEOUT:
//				printf("Operation timed out.\n");
//				break;
//
//			case LIBUSB_ERROR_OVERFLOW:
//				printf("Overflow.\n");
//				break;
//
//			case LIBUSB_ERROR_PIPE:
//				printf("Pipe error.\n");
//				break;
//
//			case LIBUSB_ERROR_INTERRUPTED:
//				printf("System call interrupted(perhaps due to signal)\n");
//				break;
//
//			case LIBUSB_ERROR_NO_MEM:
//				printf("Insufficient memory.\n");
//				break;
//
//			case LIBUSB_ERROR_NOT_SUPPORTED:
//				printf("Operation not supported or unimplemented on this platform.\n");
//				break;
//
//			case LIBUSB_ERROR_OTHER:
//				printf("Other error.\n");
//				break;
//			}
//
//			//print_transfer_status(command_out_transfer);
//
//			//libusb_submit_transfer(command_out_transfer);
//
//			//return -1;
//		}
//		else 
//		{
//			printf("libusb_submit_transfer() done.\n");
//		}
//	}
//
//#if 1
//	printf("OUT: ");
//	print_transfer_status(command_out_transfer);
//#endif
//
//	//libusb_submit_transfer(command_out_transfer);
//
//	return 0;
//}
//
//template<size_t OPCODE_LEN, size_t PAYLOAD_LEN>
//static int usb_send_acl_packet(command_template<OPCODE_LEN, PAYLOAD_LEN>& templ, std::function<void(uint8_t* data, uint8_t data_len, struct libusb_transfer* transfer)> callback)
//{
//	printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
//	printf("usb_send_acl_packet() in main.c - libusb_fill_control_setup(), libusb_submit_transfer()\n");
//	printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
//
//	/*printf(">>> [size of message: %d] ", size);
//	for (int i = 0; i < size; ++i)
//	{
//		fprintf(stdout, "%02X%s", packet[i], (i + 1) % 16 == 0 ? "\n" : " ");
//	}
//	printf("\n");*/
//
//	// async
//	//libusb_fill_control_setup(hci_cmd_buffer, LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE, 0, 0, 0, size);
//	//memcpy(hci_cmd_buffer + LIBUSB_CONTROL_SETUP_SIZE, packet, size);
//
//	// for (int i = 0; i < (3 + 256 + LIBUSB_CONTROL_SETUP_SIZE); ++i) {
//	//     fprintf(stdout, "%02X%s", hci_cmd_buffer[i], ( i + 1 ) % 16 == 0 ? "\r\n" : " " );
//	// }
//	// printf("\n");
//
//	// outgoing buffer for HCI Command packets
//	static uint8_t acl_packet_buffer[3 + 256 + LIBUSB_CONTROL_SETUP_SIZE];
//
//	// this will first add a USB specific part to the buffer and after the USB specific part
//	// it will add the HCI packet
//	libusb_fill_control_setup(acl_packet_buffer, LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE, 0, 0, 0, (OPCODE_LEN + PAYLOAD_LEN));
//	std::copy(std::begin(templ.opcode), std::end(templ.opcode), std::begin(acl_packet_buffer) + LIBUSB_CONTROL_SETUP_SIZE);
//	std::copy(std::begin(templ.payload), std::end(templ.payload), std::begin(acl_packet_buffer) + LIBUSB_CONTROL_SETUP_SIZE + OPCODE_LEN);
//
//	// prepare transfer
//	int completed = 0;
//	//libusb_fill_control_transfer(acl_out_transfer, handle, hci_cmd_buffer, async_acl_callback, &completed, 0);
//	//libusb_fill_control_transfer(acl_out_transfer, handle, hci_cmd_buffer, async_callback, &completed, 0);
//	libusb_fill_bulk_transfer(acl_out_transfer, handle, acl_out_addr, acl_packet_buffer, (OPCODE_LEN + PAYLOAD_LEN), async_acl_callback, &completed, 0);
//	acl_out_transfer->flags = LIBUSB_TRANSFER_FREE_BUFFER;
//
//	// update state before submitting transfer
//	usb_acl_out_active = 1;
//
//	// submit transfer
//	int libusb_submit_transfer_result = libusb_submit_transfer(acl_out_transfer);
//	if (libusb_submit_transfer_result < 0)
//	{
//		usb_acl_out_active = 0;
//
//		printf("\n");
//		printf("!!!!!!!!!!!!!!! Error submitting acl transfer %d\n", libusb_submit_transfer_result);
//		printf("!!!!!!!!!!!!!!! Error submitting acl transfer %d\n", libusb_submit_transfer_result);
//		printf("!!!!!!!!!!!!!!! Error submitting acl transfer %d\n", libusb_submit_transfer_result);
//		printf("!!!!!!!!!!!!!!! Error submitting acl transfer %d\n", libusb_submit_transfer_result);
//		printf("!!!!!!!!!!!!!!! Error submitting acl transfer %d\n", libusb_submit_transfer_result);
//
//		printf("%s\n", libusb_error_name(libusb_submit_transfer_result));
//
//		/*switch (libusb_submit_transfer_result)
//		{
//		case LIBUSB_SUCCESS:
//			printf("Success(no error)\n");
//			break;
//
//		case LIBUSB_ERROR_IO:
//			printf("Input / output error.\n");
//			break;
//
//		case LIBUSB_ERROR_INVALID_PARAM:
//			printf("Invalid parameter.\n");
//			break;
//
//		case LIBUSB_ERROR_ACCESS:
//			printf("Access denied(insufficient permissions)\n");
//			break;
//
//		case LIBUSB_ERROR_NO_DEVICE:
//			printf("No such device(it may have been disconnected)\n");
//			break;
//
//		case LIBUSB_ERROR_NOT_FOUND:
//			printf("Entity not found.\n");
//			break;
//
//		case LIBUSB_ERROR_BUSY:
//			printf("Resource busy.\n");
//			break;
//
//		case LIBUSB_ERROR_TIMEOUT:
//			printf("Operation timed out.\n");
//			break;
//
//		case LIBUSB_ERROR_OVERFLOW:
//			printf("Overflow.\n");
//			break;
//
//		case LIBUSB_ERROR_PIPE:
//			printf("Pipe error.\n");
//			break;
//
//		case LIBUSB_ERROR_INTERRUPTED:
//			printf("System call interrupted(perhaps due to signal)\n");
//			break;
//
//		case LIBUSB_ERROR_NO_MEM:
//			printf("Insufficient memory.\n");
//			break;
//
//		case LIBUSB_ERROR_NOT_SUPPORTED:
//			printf("Operation not supported or unimplemented on this platform.\n");
//			break;
//
//		case LIBUSB_ERROR_OTHER:
//			printf("Other error.\n");
//			break;
//		}*/
//
//		//print_transfer_status(acl_out_transfer);
//
//		/*int r = libusb_submit_transfer(acl_out_transfer);
//		if (r < 0) {
//			usb_acl_out_active = 0;
//			printf("Error submitting acl transfer, %d", r);
//			return -1;
//		}*/
//
//		return -1;
//	}
//
//	return 0;
//}
//
//int main()
//{
//	/*
//	std::string filename = "C:/aaa_se/hci_bluetooth/doc/hci_client.pklg";
//	std::fstream file;
//	file.open(filename, std::ios::out | std::ios::trunc | std::ios::binary);
//	if (!file.is_open())
//	{
//		std::cout << "Could not open " << filename << std::endl;
//	
//		return 0;
//	}
//
//	packetlogger_header_t packetlogger_header;
//	reset_packetlogger_header_t(packetlogger_header);
//
//	// the length field stores the length field of all FOLLOWING data.
//	// This means the length field's value is the 
//	// timestamp field (8 byte) + type field (1 byte) + payload (x byte).
//	//packetlogger_header.len = 8 + 1 + 17;
//	//packetlogger_header.len = 8 + 1 + 13;
//	//packetlogger_header.len = 8 + 1 + 20;
//	packetlogger_header.len = 8 + 1 + 22;
//	packetlogger_header.len = _byteswap_ulong(packetlogger_header.len);
//
//	// timestamp
//	packetlogger_header.ts = 0x00;
//	packetlogger_header.ts = _byteswap_uint64(packetlogger_header.ts);
//
//	//// header = length + timestamp
//	//file.write(reinterpret_cast<char *>(&packetlogger_header.len), sizeof(packetlogger_header.len));
//	//file.write(reinterpret_cast<char *>(&packetlogger_header.ts), sizeof(packetlogger_header.ts));
//
//	//// hci packet type
//	////e_hci_packet_type hci_packet_type = e_hci_packet_type::hci_event;
//	//e_hci_packet_type hci_packet_type = e_hci_packet_type::hci_asynchronous_data;
//	////e_hci_packet_type hci_packet_type = e_hci_packet_type::hci_synchronous_data;
//	////e_hci_packet_type hci_packet_type = e_hci_packet_type::hci_command;
//	//file.write(reinterpret_cast<char *>(&hci_packet_type), sizeof(char));
//
//	//// data
//	//char data[] = { 0x02, 0x0F, 0x01, 0xF7, 0x1C, 
//	//	0xF6, 0x0F, 0x6B, 0x88, 0x01, 
//	//	0x00, 0x00, 0x00, 0x1F, 0x00, 
//	//	0x79, 0x09 };
//	//for (int i = 0; i < 17; i++)
//	//{
//	//	uint8_t d = data[i];
//	//	file.write(reinterpret_cast<char *>(&d), sizeof(d));
//	//}
//
//	//// 03 0B 00 0B 00 13 D1 97 08 40 6C 01 00
//	//char data[] = { 0x03, 0x0B, 0x00, 0x0B, 0x00, 0x13, 0xD1, 0x97, 0x08, 0x40, 0x6C, 0x01, 0x00 };
//	//for (int i = 0; i < 13; i++)
//	//{
//	//	uint8_t d = data[i];
//	//	file.write(reinterpret_cast<char *>(&d), sizeof(d));
//	//}
//
//	//// 0B 20 10 00 0C 00 01 00 04 05 08 00 40 00 00 00 01 02 00 04
//	//char data[] = { 0x0B, 0x20, 0x10, 0x00, 0x0C, 0x00, 0x01, 0x00, 0x04, 0x05, 0x08, 0x00, 0x40, 0x00, 0x00, 0x00, 0x01, 0x02, 0x00, 0x04 };
//	//for (int i = 0; i < 20; i++)
//	//{
//	//	uint8_t d = data[i];
//	//	file.write(reinterpret_cast<char *>(&d), sizeof(d));
//	//}
//
//	//// 0B 00 12 00 0E   00 01 00 05 05   0A 00 40 00 00   00 00 00 01 02   00 04
//	//uint8_t data[] = { 0x0B, 0x00, 0x12, 0x00, 0x0E, 0x00, 0x01, 0x00, 0x05, 0x05, 0x0A, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x00, 0x04 };
//	//for (int i = 0; i < 22; i++)
//	//{
//	//	file.write(reinterpret_cast<char *>(&data[i]), sizeof(uint8_t));
//	//}
//
//	//// reset
//	//03 0C 00
//	//0E 04 01 03 0C 00
//
//	//// Read local version info
//	//	01 10 00
//	//	0E 0C 01 01 10 00 06 00 10 06 0F 00 0E 22
//
//	//// Read local name
//	//	14 0C 00
//
//	//	 0E FC 01 14 0C 00 42 43 4D 32 30 37 30 32 41 00
//	//	00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
//	//	00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
//	//	00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
//	//	00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
//	//	00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
//	//	00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
//	//	00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
//	//	00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
//	//	00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
//	//	00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
//	//	00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
//	//	00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
//	//	00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
//	//	00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
//	//	00 00 00 00 00 00 00 00 00 00 00 00 00 00
//
//	//// Read local supported commands
//	//	02 10 00
//
//	//	0E 44 01 02 10 00 FF FF FF 03 CC FF EF FF FF FF
//	//	EC 1F F2 0F E8 FE 3F F7 8F FF 1C 00 00 00 61 F7
//	//	FF FF 7F 00 00 00 00 00 00 00 00 00 00 00 00 00
//	//	00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
//	//	00 00 00 00 00 00
//
//	//// Read BD ADDR
//	//	09 10 00
//
//	//	0E 0A 01 09 10 00 96 0E 7D 70 F3 5C
//
//	//// Read Buffer Size
//	//	05 10 00
//
//	//	0E 0B 01 05 10 00 FD 03 40 08 00 01 00
//
//	//	// Read Local Supported features
//	//	03 10 00
//
//	//	0E 0C 01 03 10 00 BF FE CF FE DB FF 7B 87
//
//	//	// Set Event Mask
//	//	01 0C 08 FF FF FF FF FF FF FF 3F
//
//	//	0E 04 01 01 0C 00
//
//	//	// LE Read Buffer Size
//	//	02 20 00
//
//	//	0E 07 01 02 20 00 1B 00 0F
//
//	//	// HCI_Read_Scan_Enable
//	//	19 0C 00
//	//	0E 05 01 19 0C 00 00
//
//	//	// HCI_Write_Page_Timeout
//	//	18 0C 02 00 60
//	//	0E 04 01 18 0C 00
//
//	//	// HCI_Write_Class_of_Device
//	//	24 0C 03 0C 02 7A
//	//	0E 04 01 24 0C 00
//
//	//	// HCI_Send_Change_Local_Name
//	//	13 0C F8 57 46 42 20 43 6F 75 6E 74 65 72 20 35
//	//	43 3A 46 33 3A 37 30 3A 37 44 3A 30 45 3A 39 36
//	//	00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
//	//	00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
//	//	00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
//	//	00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
//	//	00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
//	//	00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
//	//	00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
//	//	00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
//	//	00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
//	//	00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
//	//	00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
//	//	00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
//	//	00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
//	//	00 00 00 00 00 00 00 00 00 00 00
//
//	//	0E 04 01 13 0C 00
//
//	//	// HCI_Write_Extended_Inquiry_Response
//	//	52 0C F1 00 1E 09 57 46 42 20 43 6F 75 6E 74 65
//	//	72 20 35 43 3A 46 33 3A 37 30 3A 37 44 3A 30 45
//	//	3A 39 36 00 00 00 00 00 00 00 00 00 00 00 00 00
//	//	00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
//	//	00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
//	//	00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
//	//	00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
//	//	00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
//	//	00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
//	//	00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
//	//	00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
//	//	00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
//	//	00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
//	//	00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
//	//	00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
//	//	00 00 00 00
//
//	//		0E 04 01 52 0C 00
//
//	//		 // HCI_Write_Inquiry_Mode
//
//	//		45 0C 01 00
//	//		0E 04 01 45 0C 00
//
//	//		// HCI_Write_Scan_Enable
//	//		1A 0C 01 03
//	//		0E 04 01 1A 0C 00
//
//	//		// Bluetooth HCI Command - Write Default Erroneous Data Reporting
//	//		5B 0C 01 01
//	//		0E 04 01 5B 0C 00
//
//	//		// Bluetooth HCI Command - Vendor Command 0xfc1c - Write SCO PCM INT Parameter
//	//		1C FC 05 01 00 00 00 00
//	//		0E 04 01 1C FC 00
//
//	//// HCI_LE_Set_Scan_Parameters
//	//write_packet(file, e_hci_packet_type::hci_command, { 0x0B, 0x20, 0x07, 0x01, 0xE0, 0x01, 0x30, 0x00, 0x00, 0x00 });
//	//write_packet(file, e_hci_packet_type::hci_event, { 0x0E, 0x04, 0x01, 0x0B, 0x20, 0x00 });
//
//	//// simple pairing mode
//	//uint8_t write_simple_pairing_mode[] = { 0x56, 0x0C, 0x01, 0x01 };
//	//write_packet(file, e_hci_packet_type::hci_command, write_simple_pairing_mode, 4);
//	//uint8_t resp_simple_pairing_mode[] = { 0x0E, 0x04, 0x01, 0x56, 0x0C, 0x00 };
//	//write_packet(file, e_hci_packet_type::hci_event, resp_simple_pairing_mode, 6);
//
//	
//	
//	//uint8_t data1[] = { 0x0B, 0x20, 0x0A, 0x00, 0x06,
//	//	0x00, 0x01, 0x00, 0x0A, 0x03, 
//	//	0x02, 0x00, 0x03, 0x00 };
//	//write_packet(file, e_hci_packet_type::hci_asynchronous_data, data1, 14);
//
//	//uint8_t data2[] = { 0x0B, 0x00, 0x14, 0x00, 0x10, 
//	//	0x00, 0x01, 0x00, 0x0B, 0x03, 
//	//	0x0C, 0x00, 0x03, 0x00, 0x00, 
//	//	0x00, 0x06, 0x00, 0x00, 0x00, 
//	//	0x00, 0x00, 0x00, 0x00 };
//	//write_packet(file, e_hci_packet_type::hci_asynchronous_data, data2, 24);
//
//	//uint8_t data3[] = { 0x0B, 0x20, 0x10, 0x00, 0x0C, 
//	//	0x00, 0x01, 0x00, 0x04, 0x05, 
//	//	0x08, 0x00, 0x40, 0x00, 0x00, 
//	//	0x00, 0x01, 0x02, 0x00, 0x04 };
//	//write_packet(file, e_hci_packet_type::hci_asynchronous_data, data3, 20);
//
//	//uint8_t data4[] = { 0x0B, 0x00, 0x12, 0x00, 0x0E, 
//	//	0x00, 0x01, 0x00, 0x05, 0x05, 
//	//	0x0A, 0x00, 0x40, 0x00, 0x00, 
//	//	0x00, 0x00, 0x00, 0x01, 0x02, 
//	//	0x00, 0x04 };
//	//write_packet(file, e_hci_packet_type::hci_asynchronous_data, data4, 22);
//
//	// l2cap connect request
//	uint8_t data5[] = { 0x0B, 0x20, 0x0C, 0x00, 0x08, 
//		0x00, 0x01, 0x00, 0x02, 0x04, 
//		0x04, 0x00, 0x01, 0x00, 0x42, 
//		0x00 };
//	write_packet(file, e_hci_packet_type::hci_asynchronous_data, data5, 16);
//	// l2cap connect response
//	uint8_t data6[] = { 0x0B, 0x00, 0x10, 0x00, 0x0C, 
//		0x00, 0x01, 0x00, 0x03, 0x04, 
//		0x08, 0x00, 0x40, 0x00, 0x42, 
//		0x00, 0x00, 0x00, 0x00, 0x00 };
//	write_packet(file, e_hci_packet_type::hci_asynchronous_data, data6, 20);
//
//	file.flush();
//	file.close();
//
//	std::cout << "spp_client!" << std::endl;
//	*/
//
//	std::cout << "libusb_init() ..." << std::endl;
//	int result = libusb_init(&context);
//	if (result < 0)
//	{
//		std::cout << "libusb_init() failed with error code."<< result << std::endl;
//
//		return result;
//	}
//	std::cout << "libusb_init() done." << std::endl;
//
//	libusb_device **devs = NULL;
//
//	std::cout << "libusb_get_device_list() ..." << std::endl;
//	ssize_t cnt = libusb_get_device_list(context, &devs);
//	if (cnt < 0) {
//		libusb_exit(context);
//		context = NULL;
//	
//		return (int)cnt;
//	}
//	std::cout << "libusb_get_device_list() done." << std::endl;
//
//	// if the ASUS BT 400 USB dongle is detected, it will start a Bluetooth client
//	find_dev(devs);
//
//	// https://libusb.sourceforge.io/api-1.0/group__libusb__poll.html
//	while (1) {
//		//printf("libusb_handle_events() ...\n");
//		libusb_handle_events(context);
//	}
//
//	if (handle != NULL)
//	{
//		printf("libusb_close() - closing handle ...\n");
//		libusb_close(handle);
//		printf("libusb_close() - closing handle done\n");
//	}
//	
//	if (*devs != NULL)
//	{
//		std::cout << "libusb_free_device_list() ..." << std::endl;
//		libusb_free_device_list(devs, 1);
//		*devs = NULL;
//		std::cout << "libusb_free_device_list() done." << std::endl;
//	}
//	
//	if (context != NULL)
//	{
//		std::cout << "libusb_exit() ..." << std::endl;
//		libusb_exit(context);
//		context = NULL;
//		std::cout << "libusb_exit() done." << std::endl;
//	}
//
//	return 0;
//}
