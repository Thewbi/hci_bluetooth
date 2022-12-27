#pragma once

#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <stdint.h>
#include <map>

#include "hci.h"

class LinkKeyDatabase
{
public:

	bool contains(bd_addr& bd_addr);
	void add(bd_addr& bd_addr, link_key& link_key);
	void remove(bd_addr& bd_addr);
	link_key retrieve(bd_addr& bd_addr);

	void load_from_file(std::string filename);
	void store_to_file(std::string filename);

private:
	std::map<bd_addr, link_key> data_store;

};