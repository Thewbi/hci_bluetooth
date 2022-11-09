#include "sdp.h"

DataElement::DataElement()
{
	reset();
}

void DataElement::reset() {
	start_index = 0;

	// index into the Type Descriptor table, the type stored in the body
	type = DataElementType::unknown;

	// how many bytes the body consist of, retrieved via the "size descriptor" and optionally the "Data Element Var Size"
	dataElementVarSizeInBytes = 0;

	// memory for possible payload stored in the body
	value_uint32 = 0;
	value_uuid = 0;
	value_text = "";
	value_bool = false;
	value_url = "";

	DataElement* parent = nullptr;
	children.clear();
}

uint8_t readDataElementHeader(const uint8_t* data, uint8_t index, DataElement* data_element)
{
	uint8_t bytes_consumed = 0;
	data_element->start_index = index;

	uint8_t data_element_type = (data[index + 0] & 0xF8) >> 3;
	switch (data_element_type) {

		case 1:
			data_element->type = DataElementType::uint;
			break;

		case 3:
			data_element->type = DataElementType::uuid;
			break;

		case 4:
			data_element->type = DataElementType::text_string;
			break;

		case 6:
			data_element->type = DataElementType::sequence;
			break;

		default:
			break;
	}

	uint8_t data_element_size = (data[index + 0] & 0x07) >> 0;

	bytes_consumed++;

	switch (data_element_size)
	{
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
		data_element->dataElementVarSizeInBytes = pow(2, data_element_size);
		break;
	case 5:
		data_element->dataElementVarSizeInBytes = data[index + 1];
		bytes_consumed += 1;
		break;
	case 6:
		data_element->dataElementVarSizeInBytes = (data[index + 1] << 8) + data[index + 2];
		bytes_consumed += 2;
		break;
	case 7:
		data_element->dataElementVarSizeInBytes = (data[index + 1] << 16) + (data[index + 2] << 8) + data[index + 3];
		bytes_consumed += 3;
		break;
	}
	
	return bytes_consumed;
}

uint8_t readDataElementBody(const uint8_t* data, uint8_t index, DataElement** data_element)
{
	uint8_t bytes_consumed = 0;
	switch ((*data_element)->type)
	{
		// usigned integer
		case DataElementType::uint:
			for (int i = 0; i < (*data_element)->dataElementVarSizeInBytes; i++)
			{
				(*data_element)->value_uint32 <<= 8;
				(*data_element)->value_uint32 += data[index + bytes_consumed];
				bytes_consumed++;
			}
			if ((*data_element)->parent != nullptr) {
				(*data_element) = (*data_element)->parent;
			}
			break;

		// UUID
		case DataElementType::uuid:
			for (int i = 0; i < (*data_element)->dataElementVarSizeInBytes; i++)
			{
				(*data_element)->value_uuid <<= 8;
				(*data_element)->value_uuid += data[index + bytes_consumed];
				bytes_consumed++;
			}
			if ((*data_element)->parent != nullptr) {
				(*data_element) = (*data_element)->parent;
			}
			break;

		// string
		case DataElementType::text_string:
			for (int i = 0; i < (*data_element)->dataElementVarSizeInBytes; i++)
			{
				(*data_element)->value_text += data[index + bytes_consumed];
				bytes_consumed++;
			}
			if ((*data_element)->parent != nullptr) {
				(*data_element) = (*data_element)->parent;
			}
			break;

		// sequence
		case DataElementType::sequence:
		{
			// nothing
		}
		break;

		default:
			throw 1;
	}

	return bytes_consumed;
}

uint8_t processSDPData(const uint8_t* data, const uint8_t start_index, DataElement* current_data_element)
{
	uint8_t bytes_consumed = 0;

	// before having read the DataElement's header, it is unknown how large the packet is going to be
	int data_size = -1;
	//while ((data_size == -1) || ((start_index + bytes_consumed) < data_size)) {
	while ((data_size == -1) || (bytes_consumed < data_size)) {

		// if a sequence type DataElement is processed, add a child for each DataElement contained
		// in the sequence
		if (current_data_element->type == DataElementType::sequence) {

			// add a child
			DataElement* child_data_element = new DataElement();
			child_data_element->reset();
			child_data_element->parent = current_data_element;

			current_data_element->children.push_back(child_data_element);
			current_data_element = child_data_element;
		}

		bytes_consumed += readDataElementHeader(data, start_index+bytes_consumed, current_data_element);

		// the loop is executed until the DataElement is consumed
		// The loop termination condition is therefore based on data_size which
		// is read from the current DataElement itself
		if (data_size == -1)
		{
			data_size = current_data_element->dataElementVarSizeInBytes;
			if (current_data_element->type == DataElementType::sequence)
			{
				// a sequence adds another three byte to the entire data size
				data_size += 2;
			}
		}

		bytes_consumed += readDataElementBody(data, start_index + bytes_consumed, &current_data_element);

		// is the sequence finished?
		if (current_data_element->type == DataElementType::sequence) {
			if (bytes_consumed >= (current_data_element->start_index + current_data_element->dataElementVarSizeInBytes))
			{
				// go to parent
				if (current_data_element->parent != nullptr) {
					current_data_element = current_data_element->parent;
				}
			}
		}

		// is the child finished?
	}

	return bytes_consumed;
}

void dumpDataElement(const DataElement* data_element, const uint8_t indent_count, const std::string indent_string)
{
	std::string temp_indent_string = "";
	for (int i = 0; i < indent_count; i++)
	{
		temp_indent_string.append(indent_string);
	}	
	
	std::cout << temp_indent_string << "start_index: " << data_element->start_index << std::endl;

	switch (data_element->type)
	{
	case DataElementType::uint:
		std::cout << temp_indent_string << "type: int" << std::endl;
		break;

	case DataElementType::uuid:
		std::cout << temp_indent_string << "type: UUID" << std::endl;
		break;

	case DataElementType::text_string:
		std::cout << temp_indent_string << "type: string" << std::endl;
		break;

	case DataElementType::sequence:
		std::cout << temp_indent_string << "type: sequence" << std::endl;
		break;

	default:
		throw 1;
	}

	std::cout << temp_indent_string << "size: " << data_element->dataElementVarSizeInBytes << std::endl;

	switch (data_element->type)
	{
	case DataElementType::uint:
		std::cout << temp_indent_string << "value: " << std::hex << std::setfill('0') << std::setw(8) << data_element->value_uint32 << std::dec << " (" << data_element->value_uint32 << "d)" << std::endl;
		break;

	case DataElementType::uuid:
		std::cout << temp_indent_string << "value: " << data_element->value_uuid << std::endl;
		break;

	case DataElementType::text_string:
		std::cout << temp_indent_string << "value: " << data_element->value_text << std::endl;
		break;

	case DataElementType::sequence:
		std::cout << temp_indent_string << "value: see children" << std::endl;
		break;

	default:
		throw 1;
	}	

	std::cout << temp_indent_string << "{" << std::endl;

	for (auto child : data_element->children)
	{
		dumpDataElement(child, indent_count + 1, indent_string);
	}

	std::cout << temp_indent_string << "}" << std::endl;
}