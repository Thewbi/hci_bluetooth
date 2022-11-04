#pragma once

#include <stdint.h>

#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>

#include <vector>

#include <functional>

// this enum is used when logging records for wireshark in various formats (.pcap, .pklg)
// It is not part of the HCI frame!
typedef enum e_hci_packet_type
{
	hci_command = 0x00,
	hci_event = 0x01, // HCI command 0x01
	hci_asynchronous_data = 0x02, // ACL - Asynchronous Data	0x02
	hci_synchronous_data = 0x03, // SCO -	Synchronous Data	0x03
	//hci_command = 0x04, // HCI event 0x04
	hci_extended_command = 0x09 // Extended Command	0x09
} hci_packet_type_t;

typedef struct packetlogger_header
{
	uint32_t len;
	uint64_t ts;
} packetlogger_header_t;

void reset_packetlogger_header_t(packetlogger_header_t& packetlogger_header_t);
int packetlogger_read_header(std::fstream& file, packetlogger_header_t& pl_hdr);
int packetlogger_read_packet(std::fstream& file, std::function<void(packetlogger_header_t& header, uint8_t hci_packet_type, char* data)> callback_function);

void write_packet(std::fstream& file, e_hci_packet_type hci_packet_type, const std::vector<uint8_t>& data);
void write_packet(std::fstream& file, e_hci_packet_type hci_packet_type, uint8_t* data, uint8_t data_len);