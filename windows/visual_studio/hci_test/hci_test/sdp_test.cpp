#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>

#include <libusb.h>
#include <windows.h>

#include <map>
#include <vector>

#include "sdp.h"

void test_4byte_integer(const uint8_t id);
void test_2byte_integer(const uint8_t id);
void test_2byte_uuid(const uint8_t id);
void test_string(const uint8_t id);
void test_sequence_with_three_nested_2byte_integer(const uint8_t id);
void test_response(const uint8_t id);
void test_request(const uint8_t id);

void test_serialize_1(const uint8_t id);
void test_serialize_2(const uint8_t id);
void test_serialize_3(const uint8_t id);
void test_serialize_4(const uint8_t id);
void test_serialize_5(const uint8_t id);
void test_serialize_6(const uint8_t id);

//int main()
//{
//	std::cout << "Hello SDP test!" << std::endl;
//
//	test_4byte_integer(1);
//	test_2byte_integer(2);
//	test_2byte_uuid(3);
//	test_string(4);
//	test_sequence_with_three_nested_2byte_integer(5);
//	test_response(7);	
//	test_request(8);
//
//	test_serialize_1(9);
//	test_serialize_2(10);
//	test_serialize_3(11);
//	test_serialize_4(12);
//	test_serialize_5(13);
//	test_serialize_6(14);
//
//	std::cout << "Hello SDP test done!" << std::endl;
//
//	return 0;
//}

void test_serialize_1(const uint8_t id)
{
	std::cout << "test_serialize_1() ... " << std::endl;

	std::array<uint8_t, 256> target = { 0 };

	DataElement data_element;
	data_element.type = DataElementType::uint;
	data_element.dataElementVarSizeInBytes = 4;
	data_element.value_uint32 = 0x0000FFFF;	

	const uint8_t start_index = 0;
	auto bytes_created = serializeDataElement(target, data_element, start_index);

	// Data Element : Unsigned Integer 4 bytes
	//	0000 1... = Data Element Type : Unsigned Integer(1)
	//	.... .010 = Data Element Size : 4 bytes(2)
	//	Data Value
	//	Attribute Range : 0x0000ffff
	//	Attribute Range From : 0x0000
	//	Attribute Range To : 0xffff

	const int data_size = 5;
	uint8_t data[data_size] = { 0x0a, 0x00, 0x00, 0xff, 0xff };

	if (data_size != bytes_created)
	{
		throw id;
	}
	for (int i = 0; i < data_size; i++)
	{
		if (data[i] != target.at(i))
		{
			throw id;
		}
	}

	std::cout << "test_serialize_1() done. " << std::endl;
}

void test_serialize_2(const uint8_t id)
{
	std::cout << "test_serialize_2() ... " << std::endl;

	std::array<uint8_t, 256> target = { 0 };

	DataElement data_element;
	data_element.type = DataElementType::uint;
	data_element.dataElementVarSizeInBytes = 2;
	data_element.value_uint32 = 0x1234;

	const uint8_t start_index = 0;
	auto bytes_created = serializeDataElement(target, data_element, start_index);

	// Data Element : Unsigned Integer 2 bytes
	//	0000 1... = Data Element Type : Unsigned Integer(1)
	//	.... .001 = Data Element Size : 2 bytes(1)
	//	Data Value
	//	Attribute ID : Service Record Handle(0x1234)
	const uint8_t data_size = 3;
	uint8_t data[data_size] = { 0x09, 0x12, 0x34 };

	if (data_size != bytes_created)
	{
		throw id;
	}
	for (int i = 0; i < data_size; i++)
	{
		if (data[i] != target.at(i))
		{
			throw id;
		}
	}

	std::cout << "test_serialize_2() done. " << std::endl;
}

void test_serialize_3(const uint8_t id)
{
	std::cout << "test_serialize_3() ... " << std::endl;

	std::array<uint8_t, 256> target = { 0 };

	DataElement data_element;
	data_element.type = DataElementType::uuid;
	data_element.dataElementVarSizeInBytes = 2;
	data_element.value_uuid = 0x1101;

	const uint8_t start_index = 0;
	auto bytes_created = serializeDataElement(target, data_element, start_index);

	// Data Element : UUID 2 bytes
	//	0001 1... = Data Element Type : UUID(3)
	//	.... .001 = Data Element Size : 2 bytes(1)
	//	Data Value
	//	Value : UUID: Serial Port(0x1101)
	const uint8_t data_size = 3;
	uint8_t data[data_size] = { 0x19, 0x11, 0x01 };

	if (data_size != bytes_created)
	{
		throw id;
	}
	for (int i = 0; i < data_size; i++)
	{
		if (data[i] != target.at(i))
		{
			throw id;
		}
	}

	std::cout << "test_serialize_3() done. " << std::endl;
}

void test_serialize_4(const uint8_t id)
{
	std::cout << "test_serialize_4() ... " << std::endl;

	std::array<uint8_t, 256> target = { 0 };

	DataElement data_element;
	data_element.type = DataElementType::text_string;
	data_element.dataElementVarSizeInBytes = 13;
	data_element.value_text = "SPP Counter";

	const uint8_t start_index = 0;
	auto bytes_created = serializeDataElement(target, data_element, start_index);

	// Data Element : Text string uint8 11 bytes
	// 0010 0... = Data Element Type : Text string(4)
	// .... .101 = Data Element Size : uint8(5)
	// Data Element Var Size : 11
	// Data Value
	// Service Name : SPP Counter
	const uint8_t data_size = 13;
	uint8_t data[data_size] = { 0x25, 0x0b, 0x53, 0x50, 0x50, 0x20, 0x43, 0x6f, 0x75, 0x6e, 0x74, 0x65, 0x72 };

	if (data_size != bytes_created)
	{
		throw id;
	}
	for (int i = 0; i < data_size; i++)
	{
		if (data[i] != target.at(i))
		{
			throw id;
		}
	}

	std::cout << "test_serialize_4() done. " << std::endl;
}

void test_serialize_5(const uint8_t id)
{
	std::cout << "test_serialize_5() ... " << std::endl;

	std::array<uint8_t, 256> target = { 0 };

	DataElement root_data_element;
	root_data_element.type = DataElementType::sequence;
	root_data_element.dataElementVarSizeInBytes = 9;

	// add children
	DataElement child_data_element_1;
	root_data_element.children.push_back(&child_data_element_1);
	child_data_element_1.type = DataElementType::uint;
	child_data_element_1.dataElementVarSizeInBytes = 2;
	child_data_element_1.value_uint32 = 0x656e;

	DataElement child_data_element_2;
	root_data_element.children.push_back(&child_data_element_2);
	child_data_element_2.type = DataElementType::uint;
	child_data_element_2.dataElementVarSizeInBytes = 2;
	child_data_element_2.value_uint32 = 0x006a;

	DataElement child_data_element_3;
	root_data_element.children.push_back(&child_data_element_3);
	child_data_element_3.type = DataElementType::uint;
	child_data_element_3.dataElementVarSizeInBytes = 2;
	child_data_element_3.value_uint32 = 0x0100;

	const uint8_t start_index = 0;
	auto bytes_created = serializeDataElement(target, root_data_element, start_index);

	// Data Element : Sequence uint16 9 bytes
	// 0011 0... = Data Element Type : Sequence(6)
	// .... .110 = Data Element Size : uint16(6)
	// Data Element Var Size : 9
	// Data Value
	//	Language #1: Lang: en, Encoding : UTF - 8, Attribute Base : 0x0100
	//
	//		Data Element : Unsigned Integer 2 bytes
	// 		0000 1... = Data Element Type : Unsigned Integer(1)
	//		.... .001 = Data Element Size : 2 bytes(1)
	// 		Data Value
	// 		Language Code : en
	//
	// 		Data Element : Unsigned Integer 2 bytes
	// 		0000 1... = Data Element Type : Unsigned Integer(1)
	// 		.... .001 = Data Element Size : 2 bytes(1)
	// 		Data Value
	// 		Language Encoding : UTF - 8 (106)
	//
	// 		Data Element : Unsigned Integer 2 bytes
	// 		0000 1... = Data Element Type : Unsigned Integer(1)
	// 		.... .001 = Data Element Size : 2 bytes(1)
	// 		Data Value
	// 		Attribute Base : 0x0100

	const int data_size = 12;
	uint8_t data[data_size] = { 0x36, 0x00, 0x09, 0x09, 0x65, 0x6e, 0x09, 0x00, 0x6a, 0x09, 0x01, 0x00 };

	if (data_size != bytes_created)
	{
		throw id;
	}
	for (int i = 0; i < data_size; i++)
	{
		if (data[i] != target.at(i))
		{
			throw id;
		}
	}

	std::cout << "test_serialize_5() done. " << std::endl;
}

void test_serialize_6(const uint8_t id)
{
	std::cout << "test_serialize_6() ... " << std::endl;

	std::array<uint8_t, 256> target = { 0 };

	DataElement data_element_1;
	data_element_1.type = DataElementType::sequence;
	data_element_1.dataElementVarSizeInBytes = 95;

	DataElement data_element_2;
	data_element_1.children.push_back(&data_element_2);
	data_element_2.type = DataElementType::sequence;
	data_element_2.dataElementVarSizeInBytes = 92;

	// Service Attribute: Service Record Handle > Attribute ID
	DataElement data_element_3;
	data_element_2.children.push_back(&data_element_3);
	data_element_3.type = DataElementType::uint;
	data_element_3.dataElementVarSizeInBytes = 2;
	data_element_3.value_uint32 = 0x0000;

	// Service Attribute: Service Record Handle > Value
	DataElement data_element_4;
	data_element_2.children.push_back(&data_element_4);
	data_element_4.type = DataElementType::uint;
	data_element_4.dataElementVarSizeInBytes = 4;
	data_element_4.value_uint32 = 0x00010001;

	// Service Attribute: Service Class ID List > Attribute ID
	DataElement data_element_5;
	data_element_2.children.push_back(&data_element_5);
	data_element_5.type = DataElementType::uint;
	data_element_5.dataElementVarSizeInBytes = 2;
	data_element_5.value_uint32 = 0x0001;

	// Service Attribute: Service Class ID List > Value
	DataElement data_element_6;
	data_element_2.children.push_back(&data_element_6);
	data_element_6.type = DataElementType::sequence;
	data_element_6.dataElementVarSizeInBytes = 3;
	data_element_6.value_uint32 = 0x0001;

	DataElement data_element_7;
	data_element_6.children.push_back(&data_element_7);
	data_element_7.type = DataElementType::uuid;
	data_element_7.dataElementVarSizeInBytes = 2;
	data_element_7.value_uuid = 0x1101;

	// Service Attribute: Protocol Descriptor List > Attribute ID
	DataElement data_element_8;
	data_element_2.children.push_back(&data_element_8);
	data_element_8.type = DataElementType::uint;
	data_element_8.dataElementVarSizeInBytes = 2;
	data_element_8.value_uint32 = 0x0004;

	// Service Attribute: Protocol Descriptor List > Value
	DataElement data_element_9;
	data_element_2.children.push_back(&data_element_9);
	data_element_9.type = DataElementType::sequence;
	data_element_9.dataElementVarSizeInBytes = 14;
	data_element_9.value_uint32 = 0x0001;

	// protocol #1: L2CAP
	DataElement data_element_10;
	data_element_9.children.push_back(&data_element_10);
	data_element_10.type = DataElementType::sequence;
	data_element_10.dataElementVarSizeInBytes = 3;

	DataElement data_element_11;
	data_element_10.children.push_back(&data_element_11);
	data_element_11.type = DataElementType::uuid;
	data_element_11.dataElementVarSizeInBytes = 2;
	data_element_11.value_uuid = 0x0100;

	// protocol #2: RFCOMM
	DataElement data_element_12;
	data_element_9.children.push_back(&data_element_12);
	data_element_12.type = DataElementType::sequence;
	data_element_12.dataElementVarSizeInBytes = 5;

	DataElement data_element_13;
	data_element_12.children.push_back(&data_element_13);
	data_element_13.type = DataElementType::uuid;
	data_element_13.dataElementVarSizeInBytes = 2;
	data_element_13.value_uuid = 0x0003;

	//// protocol #3: unknown
	//DataElement data_element_14;
	//data_element_9.children.push_back(&data_element_14);
	//data_element_14.type = DataElementType::sequence;
	//data_element_14.dataElementVarSizeInBytes = 5;

	//DataElement data_element_15;
	//data_element_14.children.push_back(&data_element_15);
	//data_element_15.type = DataElementType::uuid;
	//data_element_15.dataElementVarSizeInBytes = 2;
	//data_element_15.value_uuid = 0x0003;

	// Service Attribute: Browse Group List > Attribute ID
	DataElement data_element_16;
	data_element_2.children.push_back(&data_element_16);
	data_element_16.type = DataElementType::uint;
	data_element_16.dataElementVarSizeInBytes = 2;
	data_element_16.value_uint32 = 0x0005;

	// Service Attribute: Browse Group List > Value
	DataElement data_element_17;
	data_element_2.children.push_back(&data_element_17);
	data_element_17.type = DataElementType::sequence;
	data_element_17.dataElementVarSizeInBytes = 3;

	DataElement data_element_18;
	data_element_17.children.push_back(&data_element_18);
	data_element_18.type = DataElementType::uuid;
	data_element_18.dataElementVarSizeInBytes = 2;
	data_element_18.value_uuid = 0x1002;

	// Service Attribute: Language Base Attribute ID List > Attribute ID
	DataElement data_element_19;
	data_element_2.children.push_back(&data_element_19);
	data_element_19.type = DataElementType::uint;
	data_element_19.dataElementVarSizeInBytes = 2;
	data_element_19.value_uint32 = 0x0006;

	// Service Attribute: Language Base Attribute ID List > Value
	DataElement data_element_20;
	data_element_2.children.push_back(&data_element_20);
	data_element_20.type = DataElementType::sequence;
	data_element_20.dataElementVarSizeInBytes = 9;

	DataElement data_element_21;
	data_element_20.children.push_back(&data_element_21);
	data_element_21.type = DataElementType::uint;
	data_element_21.dataElementVarSizeInBytes = 2;
	data_element_21.value_uint32 = 0x656e;

	DataElement data_element_22;
	data_element_20.children.push_back(&data_element_22);
	data_element_22.type = DataElementType::uint;
	data_element_22.dataElementVarSizeInBytes = 2;
	data_element_22.value_uint32 = 0x006a;

	DataElement data_element_23;
	data_element_20.children.push_back(&data_element_23);
	data_element_23.type = DataElementType::uint;
	data_element_23.dataElementVarSizeInBytes = 2;
	data_element_23.value_uint32 = 0x0100;

	// Service Attribute: Bluetooth Profile Descriptor List > Attribute ID
	DataElement data_element_24;
	data_element_2.children.push_back(&data_element_24);
	data_element_24.type = DataElementType::uint;
	data_element_24.dataElementVarSizeInBytes = 2;
	data_element_24.value_uint32 = 0x0009;

	// Service Attribute: Bluetooth Profile Descriptor List > Value
	DataElement data_element_25;
	data_element_2.children.push_back(&data_element_25);
	data_element_25.type = DataElementType::sequence;
	data_element_25.dataElementVarSizeInBytes = 9;

	DataElement data_element_26;
	data_element_25.children.push_back(&data_element_26);
	data_element_26.type = DataElementType::sequence;
	data_element_26.dataElementVarSizeInBytes = 6;

	// Profile Descriptor List #1
	DataElement data_element_27;
	data_element_26.children.push_back(&data_element_27);
	data_element_27.type = DataElementType::uuid;
	data_element_27.dataElementVarSizeInBytes = 2;
	data_element_27.value_uuid = 0x1101;

	DataElement data_element_28;
	data_element_26.children.push_back(&data_element_28);
	data_element_28.type = DataElementType::uint;
	data_element_28.dataElementVarSizeInBytes = 2;
	data_element_28.value_uint32 = 0x1102;

	// Service Attribute: Service Name > Attribute ID
	DataElement data_element_29;
	data_element_2.children.push_back(&data_element_29);
	data_element_29.type = DataElementType::uint;
	data_element_29.dataElementVarSizeInBytes = 2;
	data_element_29.value_uint32 = 0x0100;

	// Service Attribute: Service Name > Value
	DataElement data_element_30;
	data_element_2.children.push_back(&data_element_30);
	data_element_30.type = DataElementType::text_string;
	data_element_30.dataElementVarSizeInBytes = 11;
	data_element_30.value_text = "SPP Counter";

	const uint8_t start_index = 0;
	auto bytes_created = serializeDataElement(target, data_element_1, start_index);

	// DEBUG output hex stream
	for (int i = 0; i < bytes_created; i++)
	{
		std::cout << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(target.at(i)) << " ";
		if ((i > 0) && ((i+1) % 16 == 0))
		{
			std::cout << std::endl;
		}
	}
	std::cout << std::endl;

	/*36 00 5f 36 00 5c 09 00 00 0a 00 01 00 01 09 00
	01 36 00 03 19 11 01 09 00 04 36 00 0e 36 00 03
	19 01 00 36 00 05 19 00 03 [08 01] 09 00 05 36 00
	03 19 10 02 09 00 06 36 00 09 09 65 6e 09 00 6a
	09 01 00 09 00 09 36 00 09 36 00 06 19 11 01 09
	11 02 09 01 00 25 0b 53 50 50 20 43 6f 75 6e 74
	65 72*/

	const int data_size = 98;
	uint8_t data[data_size - 2] = { 
		0x36, 0x00, 0x5f, 0x36, 0x00, 0x5c, 0x09, 0x00, 0x00, 0x0a, 0x00, 0x01, 0x00, 0x01, 0x09, 0x00, 
		0x01, 0x36, 0x00, 0x03, 0x19, 0x11, 0x01, 0x09, 0x00, 0x04, 0x36, 0x00, 0x0e, 0x36, 0x00, 0x03, 
		0x19, 0x01, 0x00, 0x36, 0x00, 0x05, 0x19, 0x00, 0x03, /*0x08, 0x01,*/ 0x09, 0x00, 0x05, 0x36, 0x00, 
		0x03, 0x19, 0x10, 0x02, 0x09, 0x00, 0x06, 0x36, 0x00, 0x09, 0x09, 0x65, 0x6e, 0x09, 0x00, 0x6a, 
		0x09, 0x01, 0x00, 0x09, 0x00, 0x09, 0x36, 0x00, 0x09, 0x36, 0x00, 0x06, 0x19, 0x11, 0x01, 0x09, 
		0x11, 0x02, 0x09, 0x01, 0x00, 0x25, 0x0b, 0x53, 0x50, 0x50, 0x20, 0x43, 0x6f, 0x75, 0x6e, 0x74, 
		0x65, 0x72 };

	/*if (data_size != bytes_created)
	{
		throw id;
	}*/
	for (int i = 0; i < data_size - 2; i++)
	{
		if (i > bytes_created)
		{
			break;
		}
		if (data[i] != target.at(i))
		{
			throw id;
		}
	}

	std::cout << "test_serialize_6() done. " << std::endl;
}

void test_request(const uint8_t id)
{
	std::cout << "test_request() ..." << std::endl;

	const uint8_t data_size = 29;
	uint8_t data[data_size] = {
		0x03, 0x0b, 0x20, 0x18, 0x00, 0x14, 0x00, 0x41, 0x00, 0x06, 0x00, 0x00, 0x00, 0x0f, 0x35, 0x03,
		0x19, 0x11, 0x01, 0x03, 0xf0, 0x35, 0x05, 0x0a, 0x00, 0x00, 0xff, 0xff, 0x00
	};

	DataElement data_element;
	DataElement* current_data_element = &data_element;

	uint8_t bytes_consumed = deserializeDataElement(data, 21, current_data_element);

	dumpDataElement(current_data_element, 0, "    ");

	// The DataElement at location 21 in the test data has a length of 7
	if (7 != bytes_consumed) {
		throw id;
	}

	std::cout << "test_request() done!" << std::endl;
}

void test_response(const uint8_t id)
{
	std::cout << "test_response() ..." << std::endl;

	const uint8_t data_size = 98;
	uint8_t data[data_size] = { 
		//0x07, 0x00, 0x00, 0x00, 0x65, 0x00, 0x62, 
		0x36, 0x00, 0x5f, 0x36, 0x00, 0x5c, 0x09, 0x00, 0x00, 0x0a, 0x00, 0x01, 0x00, 0x01, 0x09, 0x00, 
		0x01, 0x36, 0x00, 0x03, 0x19, 0x11, 0x01, 0x09, 0x00, 0x04, 0x36, 0x00, 0x0e, 0x36, 0x00, 0x03, 
		0x19, 0x01, 0x00, 0x36, 0x00, 0x05, 0x19, 0x00, 0x03, 0x08, 0x01, 0x09, 0x00, 0x05, 0x36, 0x00, 
		0x03, 0x19, 0x10, 0x02, 0x09, 0x00, 0x06, 0x36, 0x00, 0x09, 0x09, 0x65, 0x6e, 0x09, 0x00, 0x6a, 
		0x09, 0x01, 0x00, 0x09, 0x00, 0x09, 0x36, 0x00, 0x09, 0x36, 0x00, 0x06, 0x19, 0x11, 0x01, 0x09, 
		0x11, 0x02, 0x09, 0x01, 0x00, 0x25, 0x0b, 0x53, 0x50, 0x50, 0x20, 0x43, 0x6f, 0x75, 0x6e, 0x74, 
		0x65, 0x72
		//, 0x00 
	};

	DataElement data_element;
	DataElement* current_data_element = &data_element;

	uint8_t bytes_consumed = deserializeDataElement(data, 0, current_data_element);

	dumpDataElement(current_data_element, 0, "    ");

	// indexes and bytes
	if (data_size != bytes_consumed) {
		throw id;
	}

	std::cout << "test_response() done." << std::endl;
}

void test_sequence_with_three_nested_2byte_integer(const uint8_t id)
{
	std::cout << "test_sequence_with_three_nested_2byte_integer() ... " << std::endl;

	// Data Element : Sequence uint16 9 bytes
	// 0011 0... = Data Element Type : Sequence(6)
	// .... .110 = Data Element Size : uint16(6)
	// Data Element Var Size : 9
	// Data Value
	//	Language #1: Lang: en, Encoding : UTF - 8, Attribute Base : 0x0100
	//
	//		Data Element : Unsigned Integer 2 bytes
	// 		0000 1... = Data Element Type : Unsigned Integer(1)
	//		.... .001 = Data Element Size : 2 bytes(1)
	// 		Data Value
	// 		Language Code : en
	//
	// 		Data Element : Unsigned Integer 2 bytes
	// 		0000 1... = Data Element Type : Unsigned Integer(1)
	// 		.... .001 = Data Element Size : 2 bytes(1)
	// 		Data Value
	// 		Language Encoding : UTF - 8 (106)
	//
	// 		Data Element : Unsigned Integer 2 bytes
	// 		0000 1... = Data Element Type : Unsigned Integer(1)
	// 		.... .001 = Data Element Size : 2 bytes(1)
	// 		Data Value
	// 		Attribute Base : 0x0100

	const int data_size = 12;
	uint8_t data[data_size] = { 0x36, 0x00, 0x09, 0x09, 0x65, 0x6e, 0x09, 0x00, 0x6a, 0x09, 0x01, 0x00 };

	DataElement data_element;
	DataElement* current_data_element = &data_element;

	uint8_t bytes_consumed = deserializeDataElement(data, 0, current_data_element);

	// indexes and bytes
	if (data_size != bytes_consumed) {
		throw id;
	}
	if (0 != data_element.start_index) {
		throw id;
	}
	if (9 != data_element.dataElementVarSizeInBytes) {
		throw id;
	}

	// values
	if (0x00 != data_element.value_uint32) {
		throw id;
	}
	if (0x00 != data_element.value_uuid) {
		throw id;
	}
	if (data_element.value_text.compare("") != 0) {
		throw id;
	}
	if (false != data_element.value_bool) {
		throw id;
	}
	if (data_element.value_url.compare("") != 0) {
		throw id;
	}

	// parent and children
	if (nullptr != data_element.parent) {
		throw id;
	}
	if (3 != data_element.children.size())
	{
		throw id;
	}

	// child 1
	auto child_1 = data_element.children.at(0);

	// indexes and bytes
	if (3 != child_1->start_index) {
		throw id;
	}
	if (2 != child_1->dataElementVarSizeInBytes) {
		throw id;
	}

	// type
	if (DataElementType::uint != child_1->type) {
		throw id;
	}

	// values
	if (0x0000656e != child_1->value_uint32) {
		throw id;
	}
	if (0x00 != child_1->value_uuid) {
		throw id;
	}
	if (child_1->value_text.compare("") != 0) {
		throw id;
	}
	if (false != child_1->value_bool) {
		throw id;
	}
	if (child_1->value_url.compare("") != 0) {
		throw id;
	}

	// parent and children
	if (nullptr == child_1->parent) {
		throw id;
	}
	if (0 != child_1->children.size())
	{
		throw id;
	}

	// child 2
	auto child_2 = data_element.children.at(1);

	// indexes and bytes
	if (6 != child_2->start_index) {
		throw id;
	}
	if (2 != child_2->dataElementVarSizeInBytes) {
		throw id;
	}

	// type
	if (DataElementType::uint != child_2->type) {
		throw id;
	}

	// values
	if (0x0000006a != child_2->value_uint32) {
		throw id;
	}
	if (0x00 != child_2->value_uuid) {
		throw id;
	}
	if (child_2->value_text.compare("") != 0) {
		throw id;
	}
	if (false != child_2->value_bool) {
		throw id;
	}
	if (child_2->value_url.compare("") != 0) {
		throw id;
	}

	// parent and children
	if (nullptr == child_2->parent) {
		throw id;
	}
	if (0 != child_2->children.size())
	{
		throw id;
	}

	// child 3
	auto child_3 = data_element.children.at(2);

	// indexes and bytes
	if (9 != child_3->start_index) {
		throw id;
	}
	if (2 != child_3->dataElementVarSizeInBytes) {
		throw id;
	}

	// type
	if (DataElementType::uint != child_3->type) {
		throw id;
	}

	// values
	if (0x00000100 != child_3->value_uint32) {
		throw id;
	}
	if (0x00 != child_3->value_uuid) {
		throw id;
	}
	if (child_3->value_text.compare("") != 0) {
		throw id;
	}
	if (false != child_3->value_bool) {
		throw id;
	}
	if (child_3->value_url.compare("") != 0) {
		throw id;
	}

	// parent and children
	if (nullptr == child_3->parent) {
		throw id;
	}
	if (0 != child_3->children.size())
	{
		throw id;
	}

	std::cout << "test_sequence_with_three_nested_2byte_integer() done. " << std::endl;
}

void test_string(const uint8_t id)
{
	std::cout << "test_string() ... " << std::endl;

	// Data Element : Text string uint8 11 bytes
	// 0010 0... = Data Element Type : Text string(4)
	// .... .101 = Data Element Size : uint8(5)
	// Data Element Var Size : 11
	// Data Value
	// Service Name : SPP Counter
	const uint8_t data_size = 13;
	uint8_t data[data_size] = { 0x25, 0x0b, 0x53, 0x50, 0x50, 0x20, 0x43, 0x6f, 0x75, 0x6e, 0x74, 0x65, 0x72 };

	DataElement data_element;
	DataElement* current_data_element = &data_element;

	uint8_t bytes_consumed = deserializeDataElement(data, 0, current_data_element);

	// indexes and bytes
	if (data_size != bytes_consumed) {
		throw id;
	}
	if (0 != data_element.start_index) {
		throw id;
	}
	if (11 != data_element.dataElementVarSizeInBytes) {
		throw id;
	}

	// type
	if (DataElementType::text_string != data_element.type) {
		throw id;
	}

	// values
	if (0x00 != data_element.value_uint32) {
		throw id;
	}
	if (0x00 != data_element.value_uuid) {
		throw id;
	}
	if (data_element.value_text.compare("SPP Counter") != 0) {
		throw id;
	}
	if (false != data_element.value_bool) {
		throw id;
	}
	if ("" != data_element.value_url) {
		throw id;
	}

	// parent and children
	if (nullptr != data_element.parent) {
		throw id;
	}
	if (0 != data_element.children.size()) {
		throw id;
	}

	std::cout << "test_string() done. " << std::endl;
}

void test_2byte_uuid(const uint8_t id)
{
	std::cout << "test_2byte_uuid() ... " << std::endl;

	// Data Element : UUID 2 bytes
	//	0001 1... = Data Element Type : UUID(3)
	//	.... .001 = Data Element Size : 2 bytes(1)
	//	Data Value
	//	Value : UUID: Serial Port(0x1101)
	const uint8_t data_size = 3;
	uint8_t data[data_size] = { 0x19, 0x11, 0x01 };

	DataElement data_element;
	DataElement* current_data_element = &data_element;

	uint8_t bytes_consumed = deserializeDataElement(data, 0, current_data_element);

	// indexes and bytes
	if (3 != bytes_consumed) {
		throw id;
	}
	if (0 != data_element.start_index) {
		throw id;
	}
	if (2 != data_element.dataElementVarSizeInBytes) {
		throw id;
	}

	// type
	if (DataElementType::uuid != data_element.type) {
		throw id;
	}

	// values
	if (0x00 != data_element.value_uint32) {
		throw id;
	}
	if (0x1101 != data_element.value_uuid) {
		throw id;
	}
	if ("" != data_element.value_text) {
		throw id;
	}
	if (false != data_element.value_bool) {
		throw id;
	}
	if ("" != data_element.value_url) {
		throw id;
	}

	// parent and children
	if (nullptr != data_element.parent) {
		throw id;
	}
	if (0 != data_element.children.size()) {
		throw id;
	}

	std::cout << "test_2byte_uuid() done. " << std::endl;
}

void test_2byte_integer(const uint8_t id)
{
	std::cout << "test_2byte_integer() ... " << std::endl;

	// Data Element : Unsigned Integer 2 bytes
	//	0000 1... = Data Element Type : Unsigned Integer(1)
	//	.... .001 = Data Element Size : 2 bytes(1)
	//	Data Value
	//	Attribute ID : Service Record Handle(0x1234)
	const uint8_t data_size  = 3;
	uint8_t data[data_size] = { 0x09, 0x12, 0x34 };

	DataElement data_element;
	DataElement* current_data_element = &data_element;

	uint8_t bytes_consumed = deserializeDataElement(data, 0, current_data_element);

	// indexes and bytes
	if (data_size != bytes_consumed) {
		throw id;
	}
	if (0 != data_element.start_index) {
		throw id;
	}
	if (2 != data_element.dataElementVarSizeInBytes) {
		throw id;
	}

	// type
	if (DataElementType::uint != data_element.type) {
		throw id;
	}

	// values
	if (0x1234 != data_element.value_uint32) {
		throw id;
	}
	if (0 != data_element.value_uuid) {
		throw id;
	}
	if ("" != data_element.value_text) {
		throw id;
	}
	if (false != data_element.value_bool) {
		throw id;
	}
	if ("" != data_element.value_url) {
		throw id;
	}

	// parent and children
	if (nullptr != data_element.parent) {
		throw id;
	}
	if (0 != data_element.children.size()) {
		throw id;
	}

	std::cout << "test_2byte_integer() done. " << std::endl;
}

void test_4byte_integer(const uint8_t id)
{
	std::cout << "test_4byte_integer() ... " << std::endl;

	// Data Element : Unsigned Integer 4 bytes
	//	0000 1... = Data Element Type : Unsigned Integer(1)
	//	.... .010 = Data Element Size : 4 bytes(2)
	//	Data Value
	//	Attribute Range : 0x0000ffff
	//	Attribute Range From : 0x0000
	//	Attribute Range To : 0xffff

	const int data_size = 5;
	uint8_t data[data_size] = { 0x0a, 0x00, 0x00, 0xff, 0xff };

	DataElement data_element;
	DataElement* current_data_element = &data_element;

	uint8_t bytes_consumed = deserializeDataElement(data, 0, current_data_element);

	if (0 != current_data_element->children.size())
	{
		throw id;
	}

	// indexes and bytes
	if (data_size != bytes_consumed) {
		throw id;
	}
	if (0 != data_element.start_index) {
		throw id;
	}
	if (4 != data_element.dataElementVarSizeInBytes) {
		throw id;
	}

	// type
	if (DataElementType::uint != data_element.type) {
		throw id;
	}

	// values
	if (0x0000FFFF != data_element.value_uint32) {
		throw id;
	}
	if (0 != data_element.value_uuid) {
		throw id;
	}
	if ("" != data_element.value_text) {
		throw id;
	}
	if (false != data_element.value_bool) {
		throw id;
	}
	if ("" != data_element.value_url) {
		throw id;
	}

	// parent and children
	if (nullptr != data_element.parent) {
		throw id;
	}
	if (0 != data_element.children.size()) {
		throw id;
	}

	std::cout << "test_4byte_integer() done. " << std::endl;
}