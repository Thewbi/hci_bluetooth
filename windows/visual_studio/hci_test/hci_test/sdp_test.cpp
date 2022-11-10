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

int main()
{
	std::cout << "Hello SDP test!" << std::endl;

	test_4byte_integer(1);
	test_2byte_integer(2);
	test_2byte_uuid(3);
	test_string(4);
	test_sequence_with_three_nested_2byte_integer(5);
	test_response(7);	
	test_request(8);

	test_serialize_1(9);
	test_serialize_2(10);
	test_serialize_3(11);
	test_serialize_4(12);
	test_serialize_5(13);

	std::cout << "Hello SDP test done!" << std::endl;

	return 0;
}

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
	auto bytes_created = serializeSequenceDataElement(target, root_data_element, start_index);

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

	uint8_t bytes_consumed = processSDPData(data, 21, current_data_element);

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

	uint8_t bytes_consumed = processSDPData(data, 0, current_data_element);

	dumpDataElement(current_data_element, 0, "    ");

	// indexes and bytes
	if (data_size != bytes_consumed) {
		throw id;
	}
}

void test_sequence_with_three_nested_2byte_integer(const uint8_t id)
{
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

	uint8_t bytes_consumed = processSDPData(data, 0, current_data_element);

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
}

void test_string(const uint8_t id)
{
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

	uint8_t bytes_consumed = processSDPData(data, 0/*, data_size*/, current_data_element);

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
}

void test_2byte_uuid(const uint8_t id)
{
	// Data Element : UUID 2 bytes
	//	0001 1... = Data Element Type : UUID(3)
	//	.... .001 = Data Element Size : 2 bytes(1)
	//	Data Value
	//	Value : UUID: Serial Port(0x1101)
	const uint8_t data_size = 3;
	uint8_t data[data_size] = { 0x19, 0x11, 0x01 };

	DataElement data_element;
	DataElement* current_data_element = &data_element;

	uint8_t bytes_consumed = processSDPData(data, 0/*, data_size*/, current_data_element);

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
}

void test_2byte_integer(const uint8_t id)
{
	// Data Element : Unsigned Integer 2 bytes
	//	0000 1... = Data Element Type : Unsigned Integer(1)
	//	.... .001 = Data Element Size : 2 bytes(1)
	//	Data Value
	//	Attribute ID : Service Record Handle(0x1234)
	const uint8_t data_size  = 3;
	uint8_t data[data_size] = { 0x09, 0x12, 0x34 };

	DataElement data_element;
	DataElement* current_data_element = &data_element;

	uint8_t bytes_consumed = processSDPData(data, 0/*, data_size*/, current_data_element);

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
}

void test_4byte_integer(const uint8_t id)
{
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

	uint8_t bytes_consumed = processSDPData(data, 0/*, data_size*/, current_data_element);

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
}