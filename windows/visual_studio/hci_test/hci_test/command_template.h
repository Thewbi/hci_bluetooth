#pragma once

#include "hci.h"

template<size_t OPCODE_LEN, size_t PAYLOAD_LEN>
class command_template
{
public:
	command_template(std::array<uint8_t, OPCODE_LEN> opcode_in,
		std::array<uint8_t, PAYLOAD_LEN > payload_in) :
		opcode(opcode_in), payload(payload_in) {}

	std::array<uint8_t, OPCODE_LEN> opcode;
	std::array<uint8_t, PAYLOAD_LEN> payload;
};

command_template<2, 1> reset_command_template(HCI_OPCODE_TO_ARRAY(hci_opcode_t::HCI_OPCODE_HCI_RESET), { 0x00 });
command_template<2, 1> read_local_version_information_template(HCI_OPCODE_TO_ARRAY(hci_opcode_t::HCI_OPCODE_HCI_READ_LOCAL_VERSION_INFORMATION), { 0x00 });
command_template<2, 1> read_local_name_template(HCI_OPCODE_TO_ARRAY(hci_opcode_t::HCI_OPCODE_HCI_READ_LOCAL_NAME), { 0x00 });
command_template<2, 1> read_local_supported_commands_template(HCI_OPCODE_TO_ARRAY(hci_opcode_t::HCI_OPCODE_HCI_READ_LOCAL_SUPPORTED_COMMANDS), { 0x00 });
command_template<2, 1> read_bd_addr_template(HCI_OPCODE_TO_ARRAY(hci_opcode_t::HCI_OPCODE_HCI_READ_BD_ADDR), { 0x00 });
command_template<2, 1> read_buffer_size_template(HCI_OPCODE_TO_ARRAY(hci_opcode_t::HCI_OPCODE_HCI_READ_BUFFER_SIZE), { 0x00 });
command_template<2, 1> read_local_supported_features_template(HCI_OPCODE_TO_ARRAY(hci_opcode_t::HCI_OPCODE_HCI_READ_LOCAL_SUPPORTED_FEATURES), { 0x00 });
command_template<2, 9> send_set_event_mask_template(HCI_OPCODE_TO_ARRAY(hci_opcode_t::HCI_OPCODE_HCI_SET_EVENT_MASK), { 0x08, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x3F });

// 0x05, 0x33, 0x8b, 0x9E, 0x10, 0x00
//
// 0x05 - length of parameters
// 0x33, 0x8b, 0x9E - LAP
// 0xFF - Inquiry length (run inquiry for N * 1.28 seconds)
// 0x00 - Num Responses (Terminate after N responses. 0x00 is unlimited)
command_template<2, 0x06> inquiry_template(HCI_OPCODE_TO_ARRAY(hci_opcode_t::HCI_OPCODE_HCI_INQUIRY), { 0x05, 0x33, 0x8b, 0x9E, 0x20, 0x00 });

// 0x0d - total length of parameters
// 0x13, 0xD1, 0x97, 0x08, 0x40, 0x6C - target MAC BD-Address in reverse order
// 18 CC - Packet_Type
// 01 - Page_Scan_Repetition_Mode
// 00 - Page_Scan_Mode
// 00 00 - Clock_Offset
// 00 - Allow_Role_Switch
command_template<2, 0x0E> create_connection_template(HCI_OPCODE_TO_ARRAY(hci_opcode_t::HCI_OPCODE_HCI_CREATE_CONNECTION), { 0x0D, 0x13, 0xD1, 0x97, 0x08, 0x40, 0x6C, 0x18, 0xCC, 0x01, 0x00, 0x00, 0x00, 0x00 });

std::map<uint16_t, std::function<void(uint8_t* data, uint8_t data_len, struct libusb_transfer* transfer)>> callback_map;