#include "pklg.h"

void reset_packetlogger_header_t(packetlogger_header_t& packetlogger_header_t)
{
	packetlogger_header_t.len = 0;
	packetlogger_header_t.ts = 0;
}

int packetlogger_read_header(std::fstream& file, packetlogger_header_t& pl_hdr, uint8_t& hci_packet_type)
{
	reset_packetlogger_header_t(pl_hdr);

	// length (4 byte)
	file.read(reinterpret_cast<char *>(&pl_hdr.len), sizeof(pl_hdr.len));
	// https://stackoverflow.com/questions/105252/how-do-i-convert-between-big-endian-and-little-endian-values-in-c
	pl_hdr.len = _byteswap_ulong(pl_hdr.len);

	// timestamp (8 byte)
	file.read(reinterpret_cast<char *>(&pl_hdr.ts), sizeof(pl_hdr.ts));
	pl_hdr.ts = _byteswap_uint64(pl_hdr.ts);

	// type (1 byte)
	// 
	// see:
	// typedef enum e_hci_packet_type
	// {
	// 	hci_command = 0x01, // HCI command 0x01
	// 	hci_asynchronous_data = 0x02, // ACL - Asynchronous Data	0x02
	// 	hci_synchronous_data = 0x03, // SCO -	Synchronous Data	0x03
	// 	hci_event = 0x04, // HCI event 0x04
	// 	hci_extended_command = 0x09 // Extended Command	0x09
	// } hci_packet_type_t;
	// Codes in the 0xFx range are reserved for future use
	// In Apple's Packetlogger format, the 0xFx codes are used for human readable text
	//uint8_t hci_packet_type = 0;
	file.read(reinterpret_cast<char *>(&hci_packet_type), sizeof(hci_packet_type));

	// check magic
	if (!((8 <= pl_hdr.len) && (pl_hdr.len < 65536) &&
		(hci_packet_type < 0x04 || hci_packet_type == 0xFB || hci_packet_type == 0xFC || hci_packet_type == 0xFE || hci_packet_type == 0xFF)))
	{
		//std::cout << "Not a Apple (Bluetooth) packetlogger file! " << filename << std::endl;
		//file.close();

		return -1;
	}

	std::cout << "type: " << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(hci_packet_type & 0xFF) << std::endl;

	return 0;
}

// Read one packet
//
// A packet consists of a length field (4 byte), a timestamp field (8 byte), a type field (1 byte) and payload (length - 9) byte
// length - 4 byte
// timestamp - 8 byte 
// type field - 1 byte
// payload - (length - 9) byte
//
// the length field stores the length field of all FOLLOWING data.
// This means the length field's value is timestamp field + type field + payload.
//
// To read just the payload only, you have to read length - timestamp field - type field
// which is len-8-1 == len-9 bytes.
//
// After an entire packet has been read, you can read the next packet until the 
// file is consumed completely.
int packetlogger_read_packet(std::fstream& file, std::function<void(packetlogger_header_t& header, uint8_t hci_packet_type, char* data)> callback_function)
{
	// first, read the packet header
	packetlogger_header_t pl_hdr;
	reset_packetlogger_header_t(pl_hdr);
	uint8_t hci_packet_type = 0;

	int result = packetlogger_read_header(file, pl_hdr, hci_packet_type);
	if (result) 
	{
		return result;
	}

	// read the packet payload into a buffer
	char buffer[1024];
	
	file.read(buffer, pl_hdr.len-9);

	callback_function(pl_hdr, hci_packet_type, buffer);

	//// if string type, output a string
	//if (hci_packet_type >= 0xFB) {
	//	std::string s(buffer, (pl_hdr.len - 9));
	//	std::cout << s << std::endl;
	//}

	//// DEBUG dump packet
	//for (int i = 0; i < pl_hdr.len-9; i++)
	//{
	//	std::cout << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(buffer[i] & 0xFF) << " ";
	//}
	//std::cout << std::dec << std::endl << std::endl;

	return 0;
}


void write_packet(std::fstream& file, e_hci_packet_type hci_packet_type, const std::vector<uint8_t>& data)
{
	packetlogger_header_t packetlogger_header;
	reset_packetlogger_header_t(packetlogger_header);

	// the length field stores the length field of all FOLLOWING data.
	// This means the length field's value is the 
	// timestamp field (8 byte) + type field (1 byte) + payload (x byte)
	packetlogger_header.len = 8 + 1 + data.size();
	packetlogger_header.len = _byteswap_ulong(packetlogger_header.len);

	// timestamp
	packetlogger_header.ts = 0x00;
	packetlogger_header.ts = _byteswap_uint64(packetlogger_header.ts);

	// header = length + timestamp
	file.write(reinterpret_cast<char *>(&packetlogger_header.len), sizeof(packetlogger_header.len));
	file.write(reinterpret_cast<char *>(&packetlogger_header.ts), sizeof(packetlogger_header.ts));

	// hci packet type
	file.write(reinterpret_cast<char *>(&hci_packet_type), sizeof(char));

	for (auto d : data)
	{
		file.write(reinterpret_cast<char *>(&d), sizeof(uint8_t));
	}

	file.flush();
}

// This method dumps packets to a file
void write_packet(std::fstream& file, e_hci_packet_type hci_packet_type, uint8_t* data, uint8_t data_len)
{
	packetlogger_header_t packetlogger_header;
	reset_packetlogger_header_t(packetlogger_header);

	// the length field stores the length field of all FOLLOWING data.
	// This means the length field's value is the 
	// timestamp field (8 byte) + type field (1 byte) + payload (x byte)
	packetlogger_header.len = 8 + 1 + data_len;
	packetlogger_header.len = _byteswap_ulong(packetlogger_header.len);

	// timestamp
	packetlogger_header.ts = 0x00;
	packetlogger_header.ts = _byteswap_uint64(packetlogger_header.ts);

	// header = length + timestamp
	file.write(reinterpret_cast<char *>(&packetlogger_header.len), sizeof(packetlogger_header.len));
	file.write(reinterpret_cast<char *>(&packetlogger_header.ts), sizeof(packetlogger_header.ts));

	// hci packet type
	file.write(reinterpret_cast<char *>(&hci_packet_type), sizeof(char));

	for (int i = 0; i < data_len; i++)
	{
		file.write(reinterpret_cast<char *>(&data[i]), sizeof(uint8_t));
	}

	file.flush();
}