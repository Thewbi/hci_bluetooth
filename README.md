# hci_bluetooth
USB hci bluetooth

Look at Bluekitchens Bluetooth stack. The Bluekitchen stack was used to learn how bluetooth stacks work!

# What does this repository contain

Warning: This is work in progress! This section talks about planned and already implemented features alike!

This repository contains a user space bluetooth stack for bluetooth classic aka. Basic Rate/Enhanced Data Rate (BR/EDR).

It leverages the HCI layer to talk to a bluetooth dongle using libusb.
This means you can turn off your operating systems bluetooth functionality (Mac and Windows allow you to turn on or turn off bluetooth)
and let the user space stack connect to the USB bluetooth dongle.

As a Dongle, this stack currently only was tested with the ASUS USB-BT400 USB bluetooth dongle.

The stack will then listen for bluetooth connections and act as a RFCOMM server.

# HCI

HCI is the Host Controller Interface. In this interface, there are two parties talking to each other.
The controller is the part that is closer to the hardware. The host is closer to the user space application.

When plugging in a USB bluetooth dongle, the dongle takes over the role of the controller.
The user space stack that talks to the USB dongle using libusb takes over the role of the host.

The controller manages the hardware. When it gets HCI commands from the host, it will execute them and for example
reset or configure the hardware. HCI also contains not only commands but also events that can arrive asynchronously.
Events are used to signal that a command has been executed or that a connection request arrived amongst other things.

The hardware takes care of the RF side (Sending and receiving data over Radio Frequency Signals). 
It contains the RF hardware and implements the Link Layer (LL) and the Physical Layer (PHY) with its Baseband Radio. 

The host implements the Host part of the HCI and from there talks to the SPP, L2CAP, SDP, RFCOMM and other layers of the 
bluetooth stack as specified in the bluetooth core specification. On top of that bluetooth stack, there is an application that
can leverage the bluetooth stack to send and receive application specific data. The application can use Bluetooth as a medium
to transfer data. The bluetooth stack then sends and receives the data.

The user space bluetooth stack acts as the host in the Host Controller Interface HCI.

It will send a set of HCI commands to initialize and prepare the USB Bluetooth dongle for receiving connections.
It will control advertising (as a server) or inquiry and scanning for servers (as a client).
It will set the local name of the bluetooth device 
It will define which process is used during pairing between devices (Does the device have a UI or not? Does it require a Pin or not?)
It will get HCI connection requests and either accept or deny them.

## L2CAP

Once a HCI connection is established, that HCI connection can be used by a client to send a L2CAP connection.
L2CAP is a true utilitary transport protocol. It acts as the base transmission layer to run other higher-level protocols over.
The L2CAP protocol is a protocol that performs packet segmentation and reassembly should the minimal transmission (MTU) unit be exceeded.
L2CAP also adds the concept of channels to the bluetooth stack. There is initially always the control channel with the channel id (CID) 0x0001.
The control channel can be used to establish connections on new channels.
L2CAP also provides a feature for running several different protocols over channels. Having several channels that can each run a different
protocol allows a client to multiplex several data flows over one bluetooth connection at the same time. This is referred to as multiplexing.

To run a specific protocol, the client asks for a connection on a new channel and in the connection request, it specifies the protocol
to run in a L2CAP field called Protocol Service Multiplexer (PSM). 

## Service Discovery Protocol (SDP)

Inside the L2CAP Protocol Service Multiplexer (PSM) field, the value 0x0001 for example selects the Service Discovery Protocol (SDP).
The Service Discovery Protocol defines messages that allow a client to ask a sever for the services that the server provides.
The client can then select one of those services and connect to the service.

It is also possible to skip the SDP queries and directly connect to a service. This is guesswork as the service might or might
not be available. The SDP query is a way to discover services for a more save way of connecting to a server.

Examples of SDP messages are:

```
async_callback() in hci_transport_h2_libusb.c
begin async_callback endpoint 82, status 0, actual length 106
-> Transfer Completed
<<< async_callback() in hci_transport_h2_libusb.c [0B 20 66 00 62 00 40 00 07 00 01 00 5D 00 5A 36
00 57 36 00 54 09 00 01 36 00 03 19 11 01 09 00
04 36 00 0E 36 00 03 19 01 00 36 00 05 19 00 03
08 01 09 00 05 36 00 03 19 10 02 09 00 06 36 00
09 09 65 6E 09 00 6A 09 01 00 09 00 09 36 00 09
36 00 06 19 11 01 09 11 02 09 01 00 25 0B 53 50
50 20 43 6F 75 6E 74 65 72 00 ]
end async_callback

handle_completed_transfer() in /platform/libusb/hci_transport_h2_libusb.c 
B 
packet_handler() in hci.c
<<< packet_handler() in hci.c:2332 [0B 20 66 00 62 00 40 00 07 00 01 00 5D 00 5A 36
00 57 36 00 54 09 00 01 36 00 03 19 11 01 09 00
04 36 00 0E 36 00 03 19 01 00 36 00 05 19 00 03
08 01 09 00 05 36 00 03 19 10 02 09 00 06 36 00
09 09 65 6E 09 00 6A 09 01 00 09 00 09 36 00 09
36 00 06 19 11 01 09 11 02 09 01 00 25 0B 53 50
50 20 43 6F 75 6E 74 65 72 00 ]
packet_handler() in hci.c HCI_ACL_DATA_PACKET --> calling acl_handler()

---
Record nr. 0
Attribute 0x0001: type   DES (6), element len  6 
    type  UUID (3), element len  3 , value: 0x00001101
Attribute 0x0004: type   DES (6), element len 17 
    type   DES (6), element len  6 
        type  UUID (3), element len  3 , value: 0x00000100
    type   DES (6), element len  8 
        type  UUID (3), element len  3 , value: 0x00000003
        type  UINT (1), element len  2 , value: 0x00000001
Attribute 0x0005: type   DES (6), element len  6 
    type  UUID (3), element len  3 , value: 0x00001002
Attribute 0x0006: type   DES (6), element len 12 
    type  UINT (1), element len  3 , value: 0x0000656e
    type  UINT (1), element len  3 , value: 0x0000006a
    type  UINT (1), element len  3 , value: 0x00000100
Attribute 0x0009: type   DES (6), element len 12 
    type   DES (6), element len  9 
        type  UUID (3), element len  3 , value: 0x00001101
        type  UINT (1), element len  3 , value: 0x00001102
Attribute 0x0100: type STRING (4), element len 13 len 11 (0x0b)
53 50 50 20 43 6F 75 6E 74 65 72
```

## RFCOMM

RFCOMM is one of the services that a bluetooth classic stack can provide.
The stack first has to enter a SDP record into the SDP system so that the record can
be returned when a client queries the existing services.

If the SDP returns the existence of an RFCOMM server or the client just decides to connect to a RFCOMM server,
then a bluetooth based equivalent of socket connection is possible to exchange data (byte arrays).

RFCOMM runs on to of L2CAP.




# Logging with WireShark

The purpose of logging is to output the most low level byte array send and received and also 
decode the messages so that it is possible to discover what is going on and find errors in the
protocol implementations and the sequences of requests and responses.

The hard part of logging in a bluetooth stack at the HCI level from the host persepective
is that we can never see the data that the controller truly receives from the communication partner
since what really happens is this:

Imagine you want to debug a flow from a bluetooth stack on one user space stack on machine A to a 
user space bluetooth stack on machine B. The flow starts at machine A in the host. You can log that
packet on machine A and lookt at it in wireshark. That packet is not the packet that arrives at
the host on machine B since it first goes into the controller on machine A then from the controller
over the air to the controller on machine B! From the controller on machine B it is again transformed
into a HCI event that finally is received by the host on machine B. Again you can log that packet
on machine B but this is not the packet that machine A has sent!

Logging is a true mess in bluetooth development. It is hard to debug what is going on!
The way to log is to decode the outgoing and incoming HCI commands and events and write them to 
a regulary binary file on your PC. If you write the data in .cap or .pklg format, you can then
open the traces in Wireshark (The best tool ever!).
Wireshark will decode the byte arrays and you can then start to hunt for errors.
This is also a good way to learn how other bluetooth stacks work and which messages they send.

Explain the formats of the .cap format and the MacOS .pklg.



# libusb

Talking to a HCI device is possible over UART and other transmission links.
The type if transmission used by this bluetooth user space stack is USB.
To get a plattform neutral implementation of USB, libusb is used. 
It works on MacOS, Linux and Windows.

libusb is used in an asynchronous fashion since HCI events can occur at any point in time.
In asynchronous libusb, USB transfers are prepared once and added into a pool of transfers from
which they are leased, used, reaped and returned.

For me, this never worked! I was not able to return transfers so that they could be reaped and
reused!

This is something that I still have to learn. For now, transfers are allocated and never returned!
This is a serious implementation flaw and is one of the major points to fix!

There currently are two constants:

```
#define EVENT_IN_BUFFER_COUNT 100
static struct libusb_transfer *event_in_transfer[EVENT_IN_BUFFER_COUNT];

#define ACL_IN_BUFFER_COUNT 100
static struct libusb_transfer *acl_in_transfer[ACL_IN_BUFFER_COUNT];
```

Should your stack stop sending or receiving data without any clear indication of why it stops to work,
increase the numbers to get more transfers!

I know this is terrible but beginners usually write terrible software.
You have to start somewhere. The main point is to strive to get a better programmer and learn 
from your mistakes along the way. So lets make mistakes and then correct them!

Also another libusb error is "Resource busy". This happened because I was not able to reuse the one
outgoing transfer! For outgoing traffic, there is a single transfer defined in the original implementation.
Once that transfer has been used for the first time, since it is not returned properly, it is still 
busy and when using it again, the "Resource busy" error occurs.

To work around this issue, a new outgoing transfer is allocated each time a message is sent.
This allocated transfer is not returned currently. Again, I know this is terrible. I have to fix this issue.




# Establishing L2CAP connections

L2CAP connections when a client sends a L2CAP connection request over a HCI connection which has succesfully 
been established earlier.

0b200c00080001000201040001004000

The server will answer the L2CAP connection reques with a L2CAP connection response.

0b2010000c000100030108004100400000000000

The connection is not established yet! The connection goes into phase 2 - configuration.
In the configuration phase, the client has to send at least one configuration message to the server and
the server also has to send at least one configuration reques to the client.
It the server does not send at least one configuration reques to the client or the client does not 
send at least one configuration request to the server, then the connection will time out and it will
be dropped by the client!

The simplest way to get out of the configuration phase seems to be to send a configuration request for
the Maximum Transmission Unit (MTU). If both the server and the client send the configuration request
for the MTU and they both accept the other side's request, then the connection will be established.

The MTU is used by the L2CAP layer for messages fragemntation and reassembly. If you have limited resource
say only 100 byte of transmission memory, then setting the MTU to a value of at most 100 will take care
of the resource limit. L2CAP's responsibility is it to partition larger messages into parts so that each
part is smaller than the MTU. L2CAP will transmit the parts until the entire messages was transmitted.
On the receiver side, L2CAP will reassemble the message and hand over the entire message to the upper 
layers of the stack. To the upper layers it will look like as if a huge packet has been transferred
over the lower layers although the lowers are only capable of dealing with fractions of messages
one at a time.