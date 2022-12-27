#include "link_key_database.h"
#include "link_key_database_test.h"

//int main()
//{
//
//	bd_addr bd_addr_0;
//	clear_bd_addr(bd_addr_0);
//
//	uint8_t data_array[10] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09 };
//	uint8_t* data_array_ptr = data_array;
//
//	uint8_t idx = 2;
//	//std::copy(data_array + idx, data_array + idx + 6, std::back_inserter(bd_addr_0));
//	//std::copy(std::begin(data_array) + idx, std::begin(data_array) + idx + 6, std::begin(bd_addr_0));
//	//std::copy(std::begin(data_array_ptr) + idx, std::begin(data_array_ptr) + idx + 6, std::begin(bd_addr_0));
//	read_bd_addr(bd_addr_0, data_array, idx);
//	print_bd_addr(bd_addr_0);
//	printf("\n");
//
//
//	bd_addr bd_addr_1;
//
//	// clear the bd_addr
//	clear_bd_addr(bd_addr_1);
//	if ((bd_addr_1[0] != 0x00) || (bd_addr_1[1] != 0x00) || (bd_addr_1[2] != 0x00) ||
//		(bd_addr_1[3] != 0x00) || (bd_addr_1[4] != 0x00) || (bd_addr_1[5] != 0x00))
//	{
//		throw 1;
//	}
//
//	// print bd_addr_1
//	print_bd_addr(bd_addr_1);
//	printf("\n");
//
//	bd_addr_1[0] = 0x66;
//	bd_addr_1[1] = 0x55;
//	bd_addr_1[2] = 0x44;
//	bd_addr_1[3] = 0x33;
//	bd_addr_1[4] = 0x22;
//	bd_addr_1[5] = 0x11;
//
//	// print bd_addr_1
//	print_bd_addr(bd_addr_1);
//	printf("\n");
//
//	LinkKeyDatabase link_key_database;
//
//	link_key_database.load_from_file("file_does_not_exist.xyz");
//
//	if (link_key_database.contains(bd_addr_1))
//	{
//		throw 1;
//	}
//
//	link_key link_key_1;
//	for (uint8_t i = 0; i < 16; i++)
//	{
//		link_key_1[i] = i;
//	}
//
//	link_key_database.add(bd_addr_1, link_key_1);
//
//	if (!link_key_database.contains(bd_addr_1))
//	{
//		throw 1;
//	}
//
//	bd_addr bd_addr_2;
//	bd_addr_2[0] = 0x21;
//	bd_addr_2[1] = 0x22;
//	bd_addr_2[2] = 0x23;
//	bd_addr_2[3] = 0x24;
//	bd_addr_2[4] = 0x25;
//	bd_addr_2[5] = 0x26;
//	link_key link_key_2;
//	for (uint8_t i = 0; i < 16; i++)
//	{
//		link_key_2[i] = i*10;
//	}
//	link_key_database.add(bd_addr_2, link_key_2);
//
//	link_key_database.store_to_file("link_key_database.lkd");
//
//
//	LinkKeyDatabase link_key_database_2;
//	link_key_database_2.load_from_file("link_key_database.lkd");
//
//	if (!link_key_database_2.contains(bd_addr_1))
//	{
//		throw 1;
//	}
//	if (!link_key_database_2.contains(bd_addr_2))
//	{
//		throw 1;
//	}
//
//	return 0;
//}