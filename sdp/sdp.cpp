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

uint8_t deserializeDataElement(const uint8_t* data, const uint8_t start_index, DataElement* current_data_element)
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
		std::cout << temp_indent_string << "value: " << std::hex << std::setfill('0') << std::setw(8) << data_element->value_uuid << std::dec << " (" << data_element->value_uuid << "d)" << std::endl;
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

uint8_t type_to_code(DataElementType type) 
{
	switch (type)
	{
	case DataElementType::uint:
		return 1;

	case DataElementType::twos_complement_int:
		return 2;

	case DataElementType::uuid:
		return 3;

	case DataElementType::text_string:
		return 4;

	case DataElementType::boolean:
		return 5;

	case DataElementType::sequence:
		return 6;

	case DataElementType::alternative:
		return 7;

	case DataElementType::url:
		return 8;

	case DataElementType::dummy_uint:
		return 0xFF;

	default:
		throw 1;
	}
}

uint8_t serializeSimpleDataElement(std::array<uint8_t, 256>& target, const DataElement& data_element, const uint8_t start_index)
{
	uint8_t bytes_created = 0;

	// type descriptor
	uint8_t type_descriptor = type_to_code(data_element.type);

	// size descriptor
	uint8_t size_descriptor = 0;
	uint8_t max = 0;
	switch (data_element.type)
	{
	case DataElementType::dummy_uint:
	{
		max = data_element.dataElementVarSizeInBytes;
	}
	break;

	case DataElementType::uint:
	{
		switch (data_element.dataElementVarSizeInBytes)
		{
		case 1:
			size_descriptor = 0;
			max = 1;
			break;

		case 2:
			size_descriptor = 1;
			max = 2;
			break;

		case 4:
			size_descriptor = 2;
			max = 4;
			break;

		case 8:
			throw 8; // how to convert a uint32 into a 8 byte?

		case 16:
			throw 16; // how to convert a uint32 into a 16 byte?

		default:
			throw 2;
		}		
	}
	break;

	case DataElementType::uuid:
	{
		switch (data_element.dataElementVarSizeInBytes)
		{
		case 1:
			size_descriptor = 0;
			max = 1;
			break;

		case 2:
			size_descriptor = 1;
			max = 2;
			break;

		case 4:
			size_descriptor = 2;
			max = 4;
			break;

		default:
			throw 3;
		}
	}
	break;

	case DataElementType::text_string:
	{
		size_descriptor = 5;
		max = data_element.value_text.size();
	}
	break;

	case DataElementType::sequence:
	{
		// for whatever reason, some stacks use 2 byte even if one byte would suffice
		/*if (data_element.dataElementVarSizeInBytes <= 0xFF)
		{
			size_descriptor = 5;
		} 
		else*/ if (data_element.dataElementVarSizeInBytes <= 0xFFFF)
		{
			size_descriptor = 6;

			// type and size descriptor (1 Byte) + 2 byte length variable
			max = 2;
		}
		else if (data_element.dataElementVarSizeInBytes <= 0xFFFFFF)
		{
			size_descriptor = 7;

			// type and size descriptor (1 Byte) + 3 byte length variable
			max = 3;
		}
		else
		{
			throw 1;
		}		
	}
	break;

	default:
		throw 1;
	}

	// for a dummy, we add plain old bytes without any encoding!
	if (data_element.type != DataElementType::dummy_uint)
	{
		target.at(start_index) = ((type_descriptor & 0x1F) << 3) + (size_descriptor & 0x07);
		bytes_created++;
	}

	switch (data_element.type)
	{
	case DataElementType::dummy_uint:
		// dummy_uint will just add its bytes without any special encoding!
		for (int i = max; i > 0; i--)
		{
			target.at(start_index + bytes_created + i - 1) = data_element.value_uint32 >> (8 * (max - i));
		}
		break;

	case DataElementType::uint:
		// value
		for (int i = max; i > 0; i--)
		{
			target.at(start_index + bytes_created + i - 1) = data_element.value_uint32 >> (8 * (max - i));
		}
		break;

	case DataElementType::uuid:
		// value
		for (int i = max; i > 0; i--)
		{
			target.at(start_index + bytes_created + i - 1) = data_element.value_uuid >> (8 * (max - i));
		}
		break;

	case DataElementType::text_string:
		target.at(start_index + 1) = data_element.value_text.size();
		bytes_created++;

		std::copy(data_element.value_text.begin(), data_element.value_text.end(), target.data() + start_index  + 2);
		break;

	case DataElementType::sequence:
		// write the size variable after the type descriptor and the size descriptor
		for (int i = (size_descriptor-4); i > 0; i--)
		{
			target.at(start_index + bytes_created + i - 1) = data_element.dataElementVarSizeInBytes >> (8 * (max - i));
			//bytes_created++;
		}

		//std::copy(data_element.value_text.begin(), data_element.value_text.end(), target.data() + start_index + 2);
		break;

	default: 
		throw 1;
	}
	
	bytes_created += max;

	return bytes_created;
}

uint8_t serializeSequenceDataElement(std::array<uint8_t, 256>& target, const DataElement& data_element, const uint8_t start_index)
{
	auto bytes_created = 0;
	std::deque<DataElement> deque;

	uint8_t temp_start_index = start_index;

	deque.push_back(data_element);

	while (!deque.empty())
	{ 
		DataElement current_data_element = deque.front();
		temp_start_index += serializeSimpleDataElement(target, current_data_element, temp_start_index);
		deque.pop_front();

		if (current_data_element.type == DataElementType::sequence)
		{
			// insert the children into the queue at the front if there are any
			std::vector<DataElement*>::reverse_iterator it = current_data_element.children.rbegin();
			while (it != current_data_element.children.rend())
			{
				deque.push_front(**it);
				it++;
			}
		}
	}

	return temp_start_index;
}

uint8_t serializeDataElement(std::array<uint8_t, 256>& target, const DataElement& data_element, const uint8_t start_index)
{
	if (data_element.type == DataElementType::sequence)
	{
		return serializeSequenceDataElement(target, data_element, start_index);
	}
	else 
	{
		return serializeSimpleDataElement(target, data_element, start_index);
	}
}

uint8_t sdpServiceSearchAttributeRequest(const unsigned char* data, const uint8_t current_index, uint16_t& sdp_transaction_id)
{
	uint8_t idx = current_index;

	// SDP transaction id
	uint8_t sdp_transaction_id_upper = data[idx++];
	uint8_t sdp_transaction_id_lower = data[idx++];
	sdp_transaction_id = (sdp_transaction_id_upper << 8) + sdp_transaction_id_lower;
	printf("sdp_transaction_id: 0x%04x\n", sdp_transaction_id);

	// SDP parameter length
	uint8_t sdp_parameter_length_upper = data[idx++];
	uint8_t sdp_parameter_length_lower = data[idx++];
	uint16_t sdp_parameter_length = (sdp_parameter_length_upper << 8) + sdp_parameter_length_lower;
	printf("sdp_parameter_length: 0x%04x\n", sdp_parameter_length);

	// Data Element - Service Search Pattern
	printf("Data Element - Service Search Pattern\n");
	DataElement service_search_pattern_data_element;
	idx += deserializeDataElement(data, idx, &service_search_pattern_data_element);

	// this data element contains a uuid which encodes the type of service
	// that the client is looking for (such as 0x1101 for Serial Port)
	DataElement* service_uuid = service_search_pattern_data_element.children.at(0);
	uint16_t requested_uuid = service_uuid->value_uuid;

	// DEBUG
	dumpDataElement(&service_search_pattern_data_element, 0, "    ");

	// SDP maximum attribute count
	uint8_t sdp_maximum_attribute_count_upper = data[idx++];
	uint8_t sdp_maximum_attribute_count_lower = data[idx++];
	uint16_t sdp_maximum_attribute_count = (sdp_maximum_attribute_count_upper << 8) + sdp_maximum_attribute_count_lower;
	printf("sdp_maximum_attribute_count: 0x%04x\n", sdp_maximum_attribute_count);

	// Data Element - Attribute ID List
	// this element contains boundaries for attribute ids and limits
	// the amount of attributes the client wants to know about
	printf("Data Element - Attribute ID List\n");
	DataElement attribute_id_list_data_element;
	idx += deserializeDataElement(data, idx, &attribute_id_list_data_element);

	// DEBUG
	dumpDataElement(&attribute_id_list_data_element, 0, "    ");

	switch (requested_uuid)
	{
	// 4353
	case 0x1101:
		// 0x1101 == Serial Port
		break;

	default:
		printf("SDP - Service Search Attribute Request - Unkown service uuid: 0x%04x\n", requested_uuid);
		break;
	}

	return idx;
}

uint8_t sdpServiceSearchAttributeResponse(std::array<uint8_t, 256>& target)
{
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

	// here we add two bytes of unknown origin! Even the wireshark dissector cannot parse them!
	// I do not know if I should drink more or less beer to understand this but it makes no
	// sense right now! This is not conforming to the spec!
	DataElement data_element_unknown_dummy;
	data_element_12.children.push_back(&data_element_unknown_dummy);
	data_element_unknown_dummy.type = DataElementType::dummy_uint;
	data_element_unknown_dummy.dataElementVarSizeInBytes = 2;
	data_element_unknown_dummy.value_uint32 = 0x0801;

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

	return bytes_created;
}