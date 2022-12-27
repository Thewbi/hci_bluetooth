#include "hci.h"

const char* hci_error_name(uint8_t error_id)
{
	return error_code_to_msg.at(error_id).c_str();
}

void *revmemcpy(uint8_t *dest, const uint8_t *src, size_t len)
{
	uint8_t *d = dest + len - 1;
	const uint8_t *s = src;
	while (len--)
		*d-- = *s++;
	return dest;
}

void clear_bd_addr(bd_addr& bd_addr)
{
	bd_addr.fill({});
}

void read_bd_addr(bd_addr& target, uint8_t* source, uint8_t& index)
{
	const uint8_t len = 6;

	for (uint8_t i = 0; i < len; i++)
	{
		target[i] = source[index + i];
	}
	index += len;
}

void read_bd_addr_from_array(bd_addr& target, uint8_t(&source)[6], uint8_t& index)
{
	const uint8_t len = 6;

	// https://stackoverflow.com/questions/39376813/is-the-stdarray-bit-compatible-with-the-old-c-array
	// https://stackoverflow.com/questions/20059602/can-stdbegin-work-with-array-parameters-and-if-so-how
	std::copy(std::begin(source), std::begin(source) + len, std::begin(target));

	index += len;
}

void write_bd_addr(uint8_t* target, bd_addr& source, uint8_t& index)
{
	const uint8_t len = 6;

	for (uint8_t i = 0; i < len; i++)
	{
		target[index + i] = source[i];
	}

	index += len;
}

void print_bd_addr(bd_addr& bd_addr)
{
	uint8_t idx = 5;
	for (int i = idx; i >= 0; --i) {
		if (i != 5)
		{
			printf(":");
		}
		fprintf(stdout, "%02X%s", bd_addr[i], (i + 1) % 16 == 0 ? "\n" : "");
	}
}

void clear_link_key(link_key& link_key)
{
	link_key.fill({});
}

void read_link_key(link_key& target, uint8_t* source, uint8_t& index)
{
	const uint8_t len = 16;

	for (uint8_t i = 0; i < len; i++)
	{
		target[i] = source[index + i];
	}

	index += len;
}

void read_link_key_from_array(link_key& target, uint8_t(&source)[16], uint8_t& index)
{
	const uint8_t len = 16;
	std::copy(std::begin(source), std::begin(source) + len, std::begin(target));

	index += len;
}

void write_link_key(uint8_t* target, link_key& source, uint8_t& index)
{
	const uint8_t len = 16;

	for (uint8_t i = 0; i < len; i++)
	{
		target[index + i] = source[i];
	}

	index += len;
}

void print_link_key(link_key& link_key)
{
	uint8_t idx = 16;
	for (int i = 0; i < idx; i++) {
		if (i != 0)
		{
			printf(":");
		}
		fprintf(stdout, "%02X", link_key[i]);
	}
}