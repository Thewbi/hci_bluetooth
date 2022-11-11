#pragma once

#include <stdint.h>
#include <string>
#include <vector>
#include <array>
#include <deque>

#include <math.h>

#include <iostream>
#include <iomanip>

// Type Descriptor Value        	Valid Size Descriptor              			Values Type Description
// (indexes into the Size Descriptor Table)
// 0 								0 											Nil, the null type
// 1 								0, 1, 2, 3, 4 								Unsigned Integer
// 2 								0, 1, 2, 3, 4 								Signed twos - complement integer
// 3 								1, 2, 4 									UUID, a universally unique identifier
// 4 								5, 6, 7 									Text string
// 5 								0 											Boolean
// 6 								5, 6, 7 									Data element sequence, a data element whose data field  is a sequence of data elements
// 7 								5, 6, 7 									Data element alternative, data element whose data field is a sequence of data elements from which one data element is to be selected.
// 8 								5, 6, 7 									URL, a uniform resource locator
// 9 - 31 																		Reserved
enum class DataElementType {
	nil = 0,
	uint,
	twos_complement_int,
	uuid,
	text_string,
	boolean,
	sequence,
	alternative,
	url,
	unknown,

	dummy_uint
};

class DataElement
{
public:
	DataElement();
	void reset();

public:
	uint16_t start_index;

	// index into the Type Descriptor table, the type stored in the body
	DataElementType type;

	// how many bytes the body consist of, retrieved via the "size descriptor" and optionally the "Data Element Var Size"
	uint16_t dataElementVarSizeInBytes;

	// memory for possible payload stored in the body
	uint32_t value_uint32;
	uint16_t value_uuid;
	std::string value_text;
	bool value_bool;
	std::string value_url;

	DataElement* parent;
	std::vector<DataElement*> children;
};

/**
 * data - 
 * index -
 * data_element -
 *
 * return - amount of bytes that have been read and converted into the DataElement
 */
uint8_t readDataElementHeader(const uint8_t* data, uint8_t index, DataElement* data_element);

/**
 * data -
 * index -
 * data_element -
 *
 * return - amount of bytes that have been read and converted into the DataElement
 */
uint8_t readDataElementBody(const uint8_t* data, uint8_t index, DataElement** data_element);

/**
 * data -
 * start_index -
 * current_data_element -
 */
uint8_t deserializeDataElement(const uint8_t* data, const uint8_t start_index, DataElement* current_data_element);

/**
 * data_element - the DataElement that is output including all it's child DataElements.
 * indent_count - the level of recursion of the current function call. Start with 0 to dump a root node.
 * indent_string - the type of indentation pattern.
 */
void dumpDataElement(const DataElement* data_element, const uint8_t indent_count, const std::string indent_string);

uint8_t serializeDataElement(std::array<uint8_t, 256>& target, const DataElement& data_element, const uint8_t start_index);

uint8_t serializeSimpleDataElement(std::array<uint8_t, 256>& target, const DataElement& data_element, const uint8_t start_index);

uint8_t serializeSequenceDataElement(std::array<uint8_t, 256>& target, const DataElement& data_element, const uint8_t start_index);

uint8_t type_to_code(DataElementType type);

uint8_t sdpServiceSearchAttributeRequest(const unsigned char* data, const uint8_t current_index, uint16_t& sdp_transaction_id);

uint8_t sdpServiceSearchAttributeResponse(std::array<uint8_t, 256>& target);