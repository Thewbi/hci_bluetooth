#pragma once

// https://wiki.wireshark.org/Development/LibpcapFileFormat

#include <stdint.h>

typedef struct pcap_hdr_s {
	uint32_t magic_number;   /* magic number */
	uint16_t version_major;  /* major version number */
	uint16_t version_minor;  /* minor version number */
	int32_t  thiszone;       /* GMT to local correction */
	uint32_t sigfigs;        /* accuracy of timestamps */
	uint32_t snaplen;        /* max length of captured packets, in octets */
	uint32_t network;        /* data link type */
} pcap_hdr_t;

typedef struct pcaprec_hdr_s {
	uint32_t ts_sec;         /* timestamp seconds */
	uint32_t ts_usec;        /* timestamp microseconds */
	uint32_t incl_len;       /* number of octets of packet saved in file */
	uint32_t orig_len;       /* actual length of packet */
} pcaprec_hdr_t;

struct pcaprec_modified_hdr {
	pcap_hdr_t hdr;	/* the regular header */
	uint32_t ifindex;	/* index, in *capturing* machine's list of
				   interfaces, of the interface on which this
				   packet came in. */
	uint16_t protocol;	/* Ethernet packet type */
	uint8_t pkt_type;	/* broadcast/multicast/etc. indication */
	uint8_t pad;		/* pad to a 4-byte boundary */
};

/* "libpcap" record header for Alexey's patched version in its ss990915
   incarnation; this version shows up in SuSE Linux 6.3. */
struct pcaprec_ss990915_hdr {
	pcap_hdr_t hdr;	/* the regular header */
	uint32_t ifindex;	/* index, in *capturing* machine's list of
				   interfaces, of the interface on which this
				   packet came in. */
	uint16_t protocol;	/* Ethernet packet type */
	uint8_t pkt_type;	/* broadcast/multicast/etc. indication */
	uint8_t cpu1, cpu2;	/* SMP debugging gunk? */
	uint8_t pad[3];		/* pad to a 4-byte boundary */
};

void reset_pcap_hdr_t(pcap_hdr_t& pcap_hdr)
{
	pcap_hdr.magic_number = 0;
	pcap_hdr.version_major = 0;
	pcap_hdr.version_minor = 0;
	pcap_hdr.thiszone = 0;
	pcap_hdr.sigfigs = 0;
	pcap_hdr.snaplen = 0;
	pcap_hdr.network = 0;
}

void reset_pcaprec_hdr_t(pcaprec_hdr_t& pcaprec_hdr)
{
	pcaprec_hdr.ts_sec = 0;
	pcaprec_hdr.ts_usec = 0;
	pcaprec_hdr.incl_len = 0;
	pcaprec_hdr.orig_len = 0;
}