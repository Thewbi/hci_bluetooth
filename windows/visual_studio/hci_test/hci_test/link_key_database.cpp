#include "link_key_database.h"

bool LinkKeyDatabase::contains(bd_addr& bd_addr)
{
	return data_store.find(bd_addr) != data_store.end();
}

void LinkKeyDatabase::add(bd_addr& bd_addr_key, link_key& link_key_value)
{
	data_store.insert(std::pair<bd_addr, link_key>(bd_addr_key, link_key_value));
}

void LinkKeyDatabase::remove(bd_addr& bd_addr)
{
	data_store.erase(bd_addr);
}

link_key LinkKeyDatabase::retrieve(bd_addr& bd_addr)
{
	return data_store[bd_addr];
}

void LinkKeyDatabase::load_from_file(std::string filename)
{
	std::cout << "Trying to read from \"" << filename << "\"" << std::endl;

	data_store.clear();

	std::fstream file(filename, std::ios::in | std::ios::binary);
	if (!file.is_open())
	{
		std::cout << "Could not open \"" << filename << "\"" << std::endl;
	
		return;
	}

	size_t amount_of_pairs = 0;
	file.read(reinterpret_cast<char *>(&amount_of_pairs), sizeof(uint16_t));

	for (size_t i = 0; i < amount_of_pairs; i++)
	{
		bd_addr bd_addr;
		file.read(reinterpret_cast<char *>(&bd_addr), bd_addr.size());

		link_key link_key;
		file.read(reinterpret_cast<char *>(&link_key), link_key.size());

		add(bd_addr, link_key);
	}

	file.close();
}

void LinkKeyDatabase::store_to_file(std::string filename)
{
	std::cout << "Trying to store to \"" << filename << "\"" << std::endl;

	std::fstream file;
	file.open(filename, std::ios::out | std::ios::trunc | std::ios::binary);
	if (!file.is_open())
	{
		std::cout << "Could not open \"" << filename << "\"" << std::endl;

		return;
	}

	// write the amount of entries in the datastore (2 byte)
	const uint16_t amount_of_pairs = data_store.size();
	file.write(reinterpret_cast<const char*>(&amount_of_pairs), sizeof(uint16_t));

	for (const auto& bd_arr_link_key_pair : data_store)
	{
		file.write(reinterpret_cast<const char*>(&(bd_arr_link_key_pair.first)), sizeof(uint8_t) * bd_arr_link_key_pair.first.size());
		file.write(reinterpret_cast<const char*>(&(bd_arr_link_key_pair.second)), sizeof(uint8_t) * bd_arr_link_key_pair.second.size());
	}

	file.flush();
	file.close();
}