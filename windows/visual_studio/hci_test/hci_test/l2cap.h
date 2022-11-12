#pragma once

#include <stdint.h>

typedef enum {

	SDP = 0x0001,
	RFCOMM = 0x0003

} protocol_service;

class channel_pair
{
public:

	// the channel id (CID) on the communication partner's side
	// this is were the communication partner wants the messages
	// to be sent to
	uint16_t source_cid;

	// the channel id (CID) on the own side
	uint16_t destination_cid;

	// Protocol Service Multiplexer (PSM) denotes which type of protocol
	// is run over this channel;
	uint16_t psm;

private:

};