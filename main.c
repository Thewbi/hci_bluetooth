#include <libusb-1.0/libusb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/**
 * packet types - used in BTstack and over the H4 UART interface
 */
#define HCI_COMMAND_DATA_PACKET 0x01
#define HCI_ACL_DATA_PACKET     0x02
#define HCI_SCO_DATA_PACKET     0x03
#define HCI_EVENT_PACKET        0x04

// packet header sizes
#define HCI_CMD_HEADER_SIZE          3
#define HCI_ACL_HEADER_SIZE          4
#define HCI_SCO_HEADER_SIZE          3
#define HCI_EVENT_HEADER_SIZE        2

// BTstack configuration. buffers, sizes, ...
#define HCI_ACL_PAYLOAD_SIZE (1691 + 4)
#define HCI_INCOMING_PRE_BUFFER_SIZE 14 // sizeof BNEP header, avoid memcpy

// from hci.h
#define HCI_ACL_BUFFER_SIZE        (HCI_ACL_HEADER_SIZE   + HCI_ACL_PAYLOAD_SIZE)

// from btstack/platform/libusb/hci_transport_h2_libusb.c

#define EVENT_IN_BUFFER_COUNT 3
static struct libusb_transfer *event_in_transfer[EVENT_IN_BUFFER_COUNT];

#define ACL_IN_BUFFER_COUNT 3
static struct libusb_transfer *acl_in_transfer[ACL_IN_BUFFER_COUNT];

// incoming buffer for HCI Events and ACL Packets
static uint8_t hci_event_in_buffer[EVENT_IN_BUFFER_COUNT][HCI_ACL_BUFFER_SIZE];

static uint8_t hci_acl_in_buffer[ACL_IN_BUFFER_COUNT][HCI_INCOMING_PRE_BUFFER_SIZE + HCI_ACL_BUFFER_SIZE];

// outgoing buffer for HCI Command packets
static uint8_t hci_cmd_buffer[3 + 256 + LIBUSB_CONTROL_SETUP_SIZE];

static struct libusb_transfer *command_out_transfer;
static struct libusb_transfer *acl_out_transfer;

static int usb_acl_out_active = 0;
static int usb_command_active = 0;

libusb_device_handle* handle = NULL;

libusb_context* context = NULL;

uint32_t wait = 20;

// libusb_control_transfer() request types
#define GET_STATUS 0x00
#define CLEAR_FEATURE 0x01
#define SET_FEATURE 0x03
#define SET_ADDRESS 0x05
#define GET_DESCRIPTOR 0x06
#define SET_DESCRIPTOR 0x07
#define GET_CONFIGURATION 0x08
#define SET_CONFIGURATION 0x09

// descriptor types
#define DEVICE_DESCRIPTOR 0x01
#define CONFIGURATION_DESCRIPTOR 0x02
#define INTERFACE_DESCRIPTOR 0x04
#define ENDPOINT_DESCRIPTOR 0x05
#define DEVICE_QUALIFIER_DESCRIPTOR 0x06

// https://vovkos.github.io/doxyrest/samples/libusb/group_libusb_dev.html
//
// tell the OS to not use external usb bluetooth adapters
// sudo nvram bluetoothHostControllerSwitchBehavior=never
//
// https://github.com/bluekitchen/btstack/blob/master/platform/libusb/hci_transport_h2_libusb.c
//
// le-counter: https://github.com/bluekitchen/btstack/blob/master/port/libusb-intel/README.md
//
// https://www.amd.e-technik.uni-rostock.de/ma/gol/lectures/wirlec/bluetooth_info/hci.html#Testing%20Commands
//
// HCI: https://code.google.com/archive/p/btstack/wikis/HCI_USB_Transport.wiki
//
// NetzMafia: http://www.netzmafia.de/skripten/hardware/PC-Schnittstellen/usb.html
//
// PS3: https://android.googlesource.com/platform/external/libusb/+/refs/heads/master/examples/xusb.c
//
// https://eleccelerator.com/usbdescreqparser/
// Input: 12 01 00 02 FF 01 01 40 05 0B CB 17 12 01 01 02 03 01
// Hit the button "USB Standard Descriptor" or "I do not know, make a guess for me"
// 0x12,        // bLength
// 0x01,        // bDescriptorType (Device)
// 0x00, 0x02,  // bcdUSB 2.00
// 0xFF,        // bDeviceClass
// 0x01,        // bDeviceSubClass
// 0x01,        // bDeviceProtocol
// 0x40,        // bMaxPacketSize0 64
// 0x05, 0x0B,  // idVendor 0x0B05
// 0xCB, 0x17,  // idProduct 0x17CB
// 0x12, 0x01,  // bcdDevice 2.12
// 0x01,        // iManufacturer (String Index)
// 0x02,        // iProduct (String Index)
// 0x03,        // iSerialNumber (String Index)
// 0x01,        // bNumConfigurations 1
//
// 18 bytes
//
// make && clear
// clear && ./main
//
// Next steps get wireshark to load on a small log file so it does not crash and
// send the same commands. Add architecture.

// /Users/bischowg/dev/bluetooth/bt_stack_experiments/isr-btstack/btstack/platform/libusb/hci_transport_h2_libusb.c
// contains the configuration for libusb
//
// HCI commands, HCI events and HCI error codes
// http://affon.narod.ru/BT/bluetooth_app_c10.pdf - 4.4.2 HCI Event Packet
// https://www.lisha.ufsc.br/teaching/shi/ine5346-2003-1/work/bluetooth/hci_commands.html
// https://www.st.com/resource/en/user_manual/um1865-the-bluenrgms-bluetooth-le-stack-application-command-interface-aci-stmicroelectronics.pdf
//
// HCI events:
// Event code (8 bits): identifies the event.
// Parameter length (8-bit): total length of all parameters in bytes.
// Event parameters: the number of parameters and their length is event specific.
//
// The format of all HCI commands and HCI events is contained in Core_V4.0.pdf: https://www.bluetooth.org/docman/handlers/downloaddoc.ashx?doc_id=229737

// endpoint addresses
static int event_in_addr;
static int acl_in_addr;
static int acl_out_addr;
static int sco_in_addr;
static int sco_out_addr;

static int usb_send_cmd_packet(uint8_t *packet, int size);

// copied from hci_transport_h2_libusb.c from bluekitchen's btstack
static int scan_for_bt_endpoints(libusb_device *dev)
{
    int r;

    event_in_addr = 0;
    acl_in_addr = 0;
    acl_out_addr = 0;
    sco_out_addr = 0;
    sco_in_addr = 0;

    // get endpoints from interface descriptor
    struct libusb_config_descriptor *config_descriptor;
    r = libusb_get_active_config_descriptor(dev, &config_descriptor);
    if (r < 0)
    {
      return r;
    }

    int num_interfaces = config_descriptor->bNumInterfaces;
    printf("active configuration has %u interfaces\n", num_interfaces);

    int i;
    for (i = 0; i < num_interfaces ; i++)
    {
        const struct libusb_interface *interface = &config_descriptor->interface[i];
        const struct libusb_interface_descriptor * interface_descriptor = interface->altsetting;
        printf("interface %u: %u endpoints\n", i, interface_descriptor->bNumEndpoints);

        const struct libusb_endpoint_descriptor *endpoint = interface_descriptor->endpoint;

        for (r=0;r<interface_descriptor->bNumEndpoints;r++,endpoint++)
        {

            printf("- endpoint %x, attributes %x\n", endpoint->bEndpointAddress, endpoint->bmAttributes);

            switch (endpoint->bmAttributes & 0x03)
            {
                case LIBUSB_TRANSFER_TYPE_INTERRUPT:
                    if (event_in_addr) {
                      continue;
                    }
                    event_in_addr = endpoint->bEndpointAddress;
                    printf("-> using 0x%2.2X for HCI Events\n", event_in_addr);
                    break;

                case LIBUSB_TRANSFER_TYPE_BULK:
                    if (endpoint->bEndpointAddress & 0x80) {
                        if (acl_in_addr) continue;
                        acl_in_addr = endpoint->bEndpointAddress;
                        printf("-> using 0x%2.2X for ACL Data In\n", acl_in_addr);
                    } else {
                        if (acl_out_addr) continue;
                        acl_out_addr = endpoint->bEndpointAddress;
                        printf("-> using 0x%2.2X for ACL Data Out\n", acl_out_addr);
                    }
                    break;

                case LIBUSB_TRANSFER_TYPE_ISOCHRONOUS:
                    if (endpoint->bEndpointAddress & 0x80) {
                        if (sco_in_addr) continue;
                        sco_in_addr = endpoint->bEndpointAddress;
                        printf("-> using 0x%2.2X for SCO Data In\n", sco_in_addr);
                    } else {
                        if (sco_out_addr) continue;
                        sco_out_addr = endpoint->bEndpointAddress;
                        printf("-> using 0x%2.2X for SCO Data Out\n", sco_out_addr);
                    }
                    break;

                default:
                    printf("Unknown interface");
                    break;
            }
        }
    }
    libusb_free_config_descriptor(config_descriptor);

    return 0;
}

#define HCI_EVENT_CODE_COMMAND_COMPLETE 0x0E
#define HCI_EVENT_TRANSFER_COMPLETE 0x21
#define HCI_Set_Event_Mask 0x0c01
//#define HCI_ 0x0c02
#define HCI_Reset 0x0c03
#define HCI_Set_Event_Filter 0x0c05
#define HCI_Flush 0x0c08
#define HCI_Read_Pin_Type 0x0c09
#define HCI_Write_Pin_Type 0x0c0A
#define HCI_Create_New_Unit_Key 0x0c0B
#define HCI_Read_Stored_Link_Key 0x0c0D
#define HCI_Write_Stored_Link_Key 0x0c11
#define HCI_Delete_Stored_Link_Key 0x0c12
#define HCI_Write_Local_Name 0x0c13
#define HCI_Read_Local_Name 0x0c14
#define HCI_Read_Connection_Accept_Timeout 0x0c15
#define HCI_Write_Connection_Accept_Timeout 0x0c16
#define HCI_Read_Page_Timeout 0x0c17
#define HCI_Write_Page_Timeout 0x0c18
#define HCI_Read_Scan_Enable 0x0c19
#define HCI_Write_Scan_Enable 0x0c1A
#define HCI_Read_Page_Scan_Activity 0x0c1B
#define HCI_Write_Page_Scan_Activity 0x0c1C
#define HCI_Read_Inquiry_Scan_Activity 0x0c1D
#define HCI_Write_Inquiry_Scan_Activity 0x0c1E
#define HCI_Read_Authentication_Enable 0x0c1F
#define HCI_Write_Authentication_Enable 0x0c20
#define HCI_Read_Class_of_Device 0x0c23
#define HCI_Write_Class_of_Device 0x0c24
#define HCI_Read_Voice_Setting 0x0c25
#define HCI_Write_Voice_Setting 0x0c26
#define HCI_Read_Automatic_Flush_Timeout 0x0c27
#define HCI_Write_Automatic_Flush_Timeout 0x0c28
#define HCI_Read_Num_Broadcast_Retransmissions 0x0c29
#define HCI_Write_Num_Broadcast_Retransmissions 0x0c2A
#define HCI_Read_Hold_Mode_Activity 0x0c2B
#define HCI_Write_Hold_Mode_Activity 0x0c2C
#define HCI_Read_Transmit_Power_Level 0x0c2D
#define HCI_Read_Synchronous_Flow_Control_Enable 0x0c2E
#define HCI_Write_Synchronous_Flow_Control_Enable 0x0c2F
#define HCI_Set_Controller_To_Host_Flow_Control 0x0c31
#define HCI_Host_Buffer_Size 0x0c33
#define HCI_Host_Number_Of_Completed_Packets 0x0c35
#define HCI_Read_Link_Supervision_Timeout 0x0c36
#define HCI_Write_Link_Supervision_Timeout 0x0c37
#define HCI_Read_Number_Of_Supported_IAC 0x0c38
#define HCI_Read_Current_IAC_LAP 0x0c39
#define HCI_Write_Current_IAC_LAP 0x0c3A
#define Set_AFH_Host_Channel_Classification 0x0c3F
#define HCI_Read_Inquiry_Scan_Type 0x0c42
#define HCI_Write_Inquiry_Scan_Type 0x0c43
#define HCI_Read_Inquiry_Mode 0x0c44
#define HCI_Write_Inquiry_Mode 0x0c45
#define HCI_Read_Page_Scan_Type 0x0c46
#define HCI_Write_Page_Scan_Type 0x0c47
#define Read_AFH_Channel_Assessment_Mode 0x0c48
#define Write_AFH_Channel_Assessment_Mode 0x0c49
#define HCI_Read_Extended_Inquiry_Response 0x0c51
#define HCI_Write_Extended_Inquiry_Response 0x0c52
#define HCI_Refresh_Encryption_Key 0x0c53
#define HCI_Read_Simple_Pairing_Mode 0x0c55
#define HCI_Write_Simple_Pairing_Mode 0x0c56
#define HCI_Read_Local_OOB_Data 0x0c57
#define HCI_Read_Inquiry_Response_Transmit_Power_Level 0x0c58
#define HCI_Write_Inquiry_Transmit_Power_Level 0x0c59
#define HCI_Send_Keypress_Notification 0x0c60
#define HCI_Read_Default_Erroneous_Data_Reporting 0x0c5A
#define HCI_Write_Default_Erroneous_Data_Reporting 0x0c5B
#define HCI_Enhanced_Flush 0x0c5F
#define HCI_Read_Logical_Link_Accept_Timeout0x0c61
#define HCI_Write_Logical_Link_Accept_Timeout 0x0c62
#define HCI_Set_Event_Mask_Page_2 0x0c63
#define HCI_Read_Location_Data 0x0c64
#define HCI_Write_Location_Data 0x0c65
#define HCI_Read_Flow_Control_Mode 0x0c66
#define HCI_Write_Flow_Control_Mode 0x0c67
#define HCI_Read_Enhance_Transmit_Power_Level 0x0c68
#define HCI_Read_Best_Effort_Flush_Timeout 0x0c69
#define HCI_Write_Best_Effort_Flush_Timeout 0x0c6A
#define HCI_Short_Range_Mode 0x0c6B
#define HCI_Read_LE_Host_Support 0x0c6C
#define HCI_Write_LE_Host_Support 0x0c6D

#define HCI_Read_Local_Version_Information 0x1001
#define HCI_Read_Local_Supported_Commands 0x1002
#define HCI_Read_Local_Supported_Features 0x1003
#define HCI_Read_Buffer_Size 0x1005
#define HCI_Read_BD_ADDR 0x1009


#define LE_Read_Buffer_Size 0x2002

void dump_hci_event(struct libusb_transfer *transfer)
{
  printf("dump_hci_event()\n");

  uint8_t idx = 0;

  uint8_t event_code = transfer->buffer[idx++];
  //printf("event code: 0x%02X\n", event_code);

  uint8_t parameter_total_length;

  // the Number of HCI command packets which are allowed to be sent to the
  // Controller from the Host. Range for N: 0 – 255
  uint8_t num_HCI_command_packets;

  // opcode of the command which caused this event
  uint16_t command_opcode;

  uint8_t code_lower;
  uint8_t code_upper;

  uint8_t status;

  uint16_t temp;

  uint8_t acl_connection_data_channels;

  unsigned char buffer[1024];

  printf("A\n");

  switch (event_code)
  {
    case 0x3E:
      printf("LE META\n");

      // parameter total length
      parameter_total_length = transfer->buffer[idx++];
      printf("parameter_total_length: 0x%02x\n", parameter_total_length);

      uint8_t sub_event_code = transfer->buffer[idx++];
      uint8_t num_reports = transfer->buffer[idx++];
      uint8_t event_type = transfer->buffer[idx++];
      uint8_t address_type = transfer->buffer[idx++];

      // address
      printf("BD_ADDR\n");
      for (int i = (idx+5); i >= idx; --i) {
        if (i != idx+5)
        {
          printf(":");
        }
        fprintf(stdout, "%02X%s", transfer->buffer[i], ( i + 1 ) % 16 == 0 ? "\n" : "" );
      }
      printf("\n");
      break;

    case HCI_EVENT_TRANSFER_COMPLETE:
      printf("HCI_EVENT_TRANSFER_COMPLETE\n");
      break;

    case 0x03:
      printf("CONNECTION COMPLETE event received!\n");

      // 7.7.3 Connection Complete Event - Core_V4.0.pdf - page 718 of 1114
      // 03 0B 10 0B 00 AB 8A 0F A3 5F 70 01 00

      // parameter total length
      parameter_total_length = transfer->buffer[idx++];

      // result code
      uint8_t result_code = transfer->buffer[idx++];
      printf("result_code: %02X\n", result_code);

      switch (result_code) {

        case 0x00:
          printf("success (0x00)\n");
          break;

          case 0x10:
            printf("2.16 CONNECTION ACCEPT TIMEOUT EXCEEDED (0X10) The Connection Accept Timeout Exceeded error code indicates that the Connection Accept Timeout has been exceeded for this connection attempt.\n");
            break;

          default:
              printf("Unknown error code: %02X\n", result_code);
            break;
      }

      // connection handle
      uint8_t connection_handle_0 = transfer->buffer[idx++];
      uint8_t connection_handle_1 = transfer->buffer[idx++];
      printf("connection_handle: %02X:%02X\n", connection_handle_1, connection_handle_0);

      // address
      printf("BD_ADDR\n");
      for (int i = (idx+5); i >= idx; --i) {
        if (i != idx+5)
        {
          printf(":");
        }
        fprintf(stdout, "%02X%s", transfer->buffer[i], ( i + 1 ) % 16 == 0 ? "\n" : "" );
      }
      printf("\n");

      idx += 6;

      acl_connection_data_channels = transfer->buffer[idx++];
      printf("Link Type: acl_connection_data_channels: %02X\n", acl_connection_data_channels);

      uint8_t encryption_mode_encryption_disabled = transfer->buffer[idx++];
      printf("Encryption Mode: Encryption Disabled: %02X\n", encryption_mode_encryption_disabled);

      break;

    // 0B 20 0A 00 06 00 01 00 0A 02 02 00 02 00
    // 0B 20 0A 00   aclLength(06 00)   aclChannel(01 00)       0A 02 02 00 02 00
    case 0x0B:
      printf("L2CAP - ACL received!\n");

      uint8_t header_byte_2 = transfer->buffer[idx++];

      // data total length
      uint8_t data_total_length_lower = transfer->buffer[idx++];
      uint8_t data_total_length_upper = transfer->buffer[idx++];
      uint8_t data_total_length = (data_total_length_upper << 8) + data_total_length_lower;
      printf("data_total_length: 0x%04x\n", data_total_length);

      // wrapped l2cap packet length
      uint8_t l2cap_length_lower = transfer->buffer[idx++];
      uint8_t l2cap_length_upper = transfer->buffer[idx++];
      uint8_t l2cap_length = (l2cap_length_upper << 8) + l2cap_length_lower;
      printf("l2cap_length: 0x%04x\n", l2cap_length);

      uint8_t l2cap_signaling_channel_lower = transfer->buffer[idx++];
      uint8_t l2cap_signaling_channel_upper = transfer->buffer[idx++];
      uint8_t l2cap_signaling_channel = (l2cap_signaling_channel_upper << 8) + l2cap_signaling_channel_lower;
      printf("l2cap_signaling_channel: 0x%04x\n", l2cap_signaling_channel);

      uint8_t l2cap_command_code = transfer->buffer[idx++];
      printf("l2cap_command_code: 0x%02x\n", l2cap_command_code);

      uint8_t l2cap_command_identifier = transfer->buffer[idx++];
      printf("l2cap_command_identifier: 0x%02x\n", l2cap_command_identifier);

      uint8_t l2cap_command_length_lower = transfer->buffer[idx++];
      uint8_t l2cap_command_length_upper = transfer->buffer[idx++];
      uint8_t l2cap_command_length = (l2cap_command_length_upper << 8) + l2cap_command_length_lower;
      printf("l2cap_command_length: 0x%04x\n", l2cap_command_length);

      uint8_t l2cap_information_type_lower = transfer->buffer[idx++];
      uint8_t l2cap_information_type_upper = transfer->buffer[idx++];
      uint8_t l2cap_information_type = (l2cap_information_type_upper << 8) + l2cap_information_type_lower;
      printf("l2cap_information_type: 0x%04x\n", l2cap_information_type);

      switch (l2cap_command_code) {

        case 0x02:
          printf("l2cap_command_code: connection request\n");

          uint8_t l2cap_source_cid_lower = transfer->buffer[idx++];
          uint8_t l2cap_source_cid_upper = transfer->buffer[idx++];
          uint8_t l2cap_source_cid = (l2cap_source_cid_upper << 8) + l2cap_source_cid_lower;
          printf("l2cap_source_cid: 0x%04x\n", l2cap_source_cid);

          // response

          for (int i = 0; i < 1024; ++i) {
            buffer[i] = 0x00;
          }

          int idx = 0;
          buffer[idx++] = 0x0b;
          buffer[idx++] = 0x00;

          // data total length
          buffer[idx++] = 0x10;
          buffer[idx++] = 0x00;

          // length of lcap packet
          buffer[idx++] = 0x0c;
          buffer[idx++] = 0x00;

          // signaling channel 0x0001
          buffer[idx++] = 0x01;
          buffer[idx++] = 0x00;

          // command code - connection response
          buffer[idx++] = 0x03;

          // command identifier
          buffer[idx++] = 0x04;

          // command length
          buffer[idx++] = 0x08;
          buffer[idx++] = 0x00;

          // destination CID - our own randomly choosen CID
          buffer[idx++] = 0x40;
          buffer[idx++] = 0x00;

          // source CID
          // buffer[idx++] = 0x44;
          // buffer[idx++] = 0x00;
          buffer[idx++] = l2cap_source_cid_lower;
          buffer[idx++] = l2cap_source_cid_upper;

          // result success
          buffer[idx++] = 0x00;
          buffer[idx++] = 0x00;

          // status (0x0000 = no further information)
          buffer[idx++] = 0x00;
          buffer[idx++] = 0x00;

          usleep(wait * 1000);
          usb_send_cmd_packet(buffer, idx);

          transfer->user_data = NULL;
          libusb_submit_transfer(transfer);

          break;

        case 0x04:
          printf("l2cap_command_code: configure request\n");

          // response is
          // 0b 00 12 00 0e 00 01 00 05 05 0a 00 4b 00 00 00 00 00 01 02 9b 06

          if (l2cap_information_type == 0x02) {
              printf("l2cap_command_code: configure request - extended features mask\n");
          } else if (l2cap_information_type == 0x03) {
              printf("l2cap_command_code: configure request - ???\n");
          }

/*
          for (int i = 0; i < 1024; ++i) {
            buffer[i] = 0x00;
          }

          int idx = 0;
          buffer[idx++] = 0x0b;
          buffer[idx++] = 0x00;
          buffer[idx++] = 0x12;
          buffer[idx++] = 0x00;
          buffer[idx++] = 0x0e;
          buffer[idx++] = 0x00;
          buffer[idx++] = 0x01;
          buffer[idx++] = 0x00;
          buffer[idx++] = 0x05;
          buffer[idx++] = 0x05;
          buffer[idx++] = 0x0a;
          buffer[idx++] = 0x00;
          buffer[idx++] = 0x4b;
          buffer[idx++] = 0x00;
          buffer[idx++] = 0x00;
          buffer[idx++] = 0x00;
          buffer[idx++] = 0x00;
          buffer[idx++] = 0x00;
          buffer[idx++] = 0x01;
          buffer[idx++] = 0x02;
          buffer[idx++] = 0x9b;
          buffer[idx++] = 0x06;

          usleep(wait * 1000);
          usb_send_cmd_packet(buffer, idx);

          transfer->user_data = NULL;
          libusb_submit_transfer(transfer);
*/

          break;

        case 0x0A:
          // L2CAP - information request
          printf("l2cap_command_code: information request\n");

          // response
          // 0b 00 14 00 10 00 01 00 0b 03 0c 00 03 00 00 00 06 00 00 00 00 00 00 00
          // 0b 00 10 00 0c 00 01 00 0b 02 08 00 02 00 00 00 80 02 00 00

          if (l2cap_information_type == 0x02) {
              printf("l2cap_command_code: configure request - extended features mask\n");

              for (int i = 0; i < 1024; ++i) {
                buffer[i] = 0x00;
              }
              buffer[idx++] = 0x0b;
              buffer[idx++] = 0x00;
              buffer[idx++] = 0x10;
              buffer[idx++] = 0x00;
              buffer[idx++] = 0x0c;
              buffer[idx++] = 0x00;
              buffer[idx++] = 0x01;
              buffer[idx++] = 0x00;
              buffer[idx++] = 0x0b;
              buffer[idx++] = 0x02;
              buffer[idx++] = 0x08;
              buffer[idx++] = 0x00;
              buffer[idx++] = 0x02;
              buffer[idx++] = 0x00;
              buffer[idx++] = 0x00;
              buffer[idx++] = 0x00;
              buffer[idx++] = 0x80;
              buffer[idx++] = 0x02;
              buffer[idx++] = 0x00;
              buffer[idx++] = 0x00;

              usleep(wait * 1000);
              usb_send_cmd_packet(buffer, idx);

              transfer->user_data = NULL;
              libusb_submit_transfer(transfer);

          } else if (l2cap_information_type == 0x03) {
              printf("l2cap_command_code: configure request - ???\n");
          }

/*
          for (int i = 0; i < 1024; ++i) {
            buffer[i] = 0x00;
          }

          buffer[idx++] = 0x0b;
          buffer[idx++] = 0x00;
          buffer[idx++] = 0x14;
          buffer[idx++] = 0x00;
          buffer[idx++] = 0x10;
          buffer[idx++] = 0x00;
          buffer[idx++] = 0x01;
          buffer[idx++] = 0x00;
          buffer[idx++] = 0x0b;
          buffer[idx++] = 0x03;
          buffer[idx++] = 0x0c;
          buffer[idx++] = 0x00;
          buffer[idx++] = 0x03;
          buffer[idx++] = 0x00;
          buffer[idx++] = 0x00;
          buffer[idx++] = 0x00;
          buffer[idx++] = 0x06;
          buffer[idx++] = 0x00;
          buffer[idx++] = 0x00;
          buffer[idx++] = 0x00;
          buffer[idx++] = 0x00;
          buffer[idx++] = 0x00;
          buffer[idx++] = 0x00;
          buffer[idx++] = 0x00;

          usleep(wait * 1000);
          usb_send_cmd_packet(buffer, idx);

          transfer->user_data = NULL;
          libusb_submit_transfer(transfer);
*/
          break;

        default:
          printf("l2cap_command_code: UNKNOWN (0x%02x)\n", l2cap_command_code);
          break;
      }

      break;

    // case 0x0C:
    //   printf("ACL received!\n");
    //   break;

    case 0x04:
      printf("CONNECTION REQUEST received!\n");

      // 04 0A AB 8A 0F A3 5F 70 0C 02 5A 01

      // parameter total length
      parameter_total_length = transfer->buffer[idx++];

      // address
      printf("BD_ADDR\n");
      for (int i = (idx+5); i >= idx; --i) {
        if (i != idx+5)
        {
          printf(":");
        }
        fprintf(stdout, "%02X%s", transfer->buffer[i], ( i + 1 ) % 16 == 0 ? "\n" : "" );
      }
      printf("\n");

      idx += 6;

      uint8_t class_of_device_0 = transfer->buffer[idx++];
      uint8_t class_of_device_1 = transfer->buffer[idx++];
      uint8_t class_of_device_2 = transfer->buffer[idx++];
      printf("class_of_device: %02X:%02X:%02X\n", class_of_device_2, class_of_device_1, class_of_device_0);

      acl_connection_data_channels = transfer->buffer[idx++];
      printf("Link Type: acl_connection_data_channels: %02X\n", acl_connection_data_channels);

      // TODO: send Sent Accept Connection Request
      // 113	67.214792	host	controller	HCI_CMD	11	Sent Accept Connection Request

      // 00 09 04 07 ab 8a 0f a3 5f 70 01

      printf("Send Sent Accept Connection Request ...\n");

      //unsigned char buffer[1024];
      for (int i = 0; i < 1024; ++i) {
        buffer[i] = 0x00;
      }

      int idx = 0;
      buffer[idx++] = 0x09;
      buffer[idx++] = 0x04;
      buffer[idx++] = 0x07;
      buffer[idx++] = 0xab;
      buffer[idx++] = 0x8a;
      buffer[idx++] = 0x0f;
      buffer[idx++] = 0xa3;
      buffer[idx++] = 0x5f;
      buffer[idx++] = 0x70;
      buffer[idx++] = 0x01;

      usleep(wait * 1000);
      usb_send_cmd_packet(buffer, idx);

      transfer->user_data = NULL;
      libusb_submit_transfer(transfer);

      // TODO Decode this:
      // 128	67.246620	70:5f:a3:0f:8a:ab ()	Cc&CTech_7d:0e:96 (SPP Counter 5C:F3:70:7D:0E:96)	L2CAP	15	Rcvd Information Request (Extended Features Mask)
      // 0B 20 0A 00 06 00 01 00 0A 02 02 00 02 00

      // It is answered with:
      // 129	67.246677	Cc&CTech_7d:0e:96 (SPP Counter 5C:F3:70:7D:0E:96)	70:5f:a3:0f:8a:ab ()	L2CAP	21	Sent Information Response (Extended Features Mask, Success)
      // 02 0b 00 10 00 0c 00 01 00 0b 02 08 00 02 00 00 00 80 02 00 00

      break;

    // Core_V4.0.pdf - 7.7.14 Command Complete Event - page 732 of 1114
    case HCI_EVENT_CODE_COMMAND_COMPLETE:
      printf("HCI_EVENT_CODE_COMMAND_COMPLETE\n");

      // This is the return parameter(s) for the command specified in the
      // Command_Opcode event parameter. See each command’s definition for
      // the list of return parameters associated with that command.

      printf("A \n");

      if (transfer == NULL) {
        printf("transfer is NULL \n");
      } else {
        printf("transfer->length %d \n", transfer->length);
        printf("transfer->actual_length %d \n", transfer->actual_length);
        printf("transfer->buffer %d \n", transfer->buffer);
      }

      // parameter total length
      printf("accessing buffer ...\n");
      parameter_total_length = transfer->buffer[idx++];
      printf("accessing buffer done.\n");
      //printf("parameter_total_length: 0x%02x\n", parameter_total_length);

      printf("B\n");

      // number of allowed command packets
      num_HCI_command_packets = transfer->buffer[idx++];
      //printf("num_HCI_command_packets: 0x%02x\n", num_HCI_command_packets);

      printf("C\n");

      // command opcode
      code_lower = transfer->buffer[idx++];
      code_upper = transfer->buffer[idx++];
      command_opcode = (code_upper << 8) + code_lower;
      //printf("command_opcode: 0x%04x\n", command_opcode);

      printf("D\n");

      printf("switch command_opcode\n");
      switch (command_opcode)
      {
        case HCI_Write_Page_Timeout:
          printf("Response to HCI_Write_Page_Timeout\n");
          break;

        case HCI_Write_Class_of_Device:
          printf("Response to HCI_Write_Class_of_Device\n");
          break;

        case HCI_Write_Local_Name:
          printf("Response to HCI_Write_Local_Name\n");
          break;

        case HCI_Write_Extended_Inquiry_Response:
          printf("Response to HCI_Write_Extended_Inquiry_Response\n");
          break;

        case HCI_Write_Inquiry_Mode:
          printf("Response to HCI_Write_Inquiry_Mode\n");
          break;

        case HCI_Write_Scan_Enable:
          printf("Response to HCI_Write_Scan_Enable\n");
          break;

        case HCI_Write_Simple_Pairing_Mode:
          printf("Response to HCI_Write_Simple_Pairing_Mode\n");
          break;

        case HCI_Set_Event_Mask:
          printf("Response to HCI_Set_Event_Mask\n");
          break;

        case HCI_Reset:
          printf("Response to HCI_Reset\n");
          break;

        case HCI_Read_Scan_Enable:
          printf("Response to HCI_Read_Scan_Enable\n");

          // status
          status = transfer->buffer[idx++];
          printf("status: 0x%02x\n", status);

          temp = transfer->buffer[idx++];

          switch (temp)
          {
            case 0x00:
              printf("(0x00) No Scans enabled.\n");
              break;

            case 0x01:
              printf("(0x01) Inquiry Scan enabled + Page Scan disabled.\n");
              break;

            case 0x02:
              printf("(0x02) Inquiry Scan disabled. + Page Scan enabled.\n");
              break;

            case 0x03:
              printf("(0x03) Inquiry Scan enabled. + Page Scan enabled.\n");
              break;

            default:
              printf("(0x04 - 0xFF) Reserved.\n");
              break;
          }
          break;

        case HCI_Read_Local_Version_Information:
          printf("Response to HCI_Read_Local_Version_Information\n");

          // status
          status = transfer->buffer[idx++];
          printf("status: 0x%02x\n", status);

          // HCI Version (1 Byte)
          printf("HCI Version: 0x%02x\n", transfer->buffer[idx++]);

          // HCI Revision (2 Byte)
          temp = transfer->buffer[idx++];
          printf("HCI Revision: 0x%04x\n", (transfer->buffer[idx++] << 8) + temp);

          // LMP Version (1 Byte)
          printf("LMP Version: 0x%02x\n", transfer->buffer[idx++]);

          // Manufacturer_Name (2 Byte)
          temp = transfer->buffer[idx++];
          printf("Manufacturer_Name: 0x%04x\n", (transfer->buffer[idx++] << 8) + temp);

          // LMP Subversion (2 Byte)
          temp = transfer->buffer[idx++];
          printf("LMP Subversion: 0x%02x\n", (transfer->buffer[idx++] << 8) + temp);

          break;

        case HCI_Read_Local_Name:
          printf("Response to HCI_Read_Local_Name\n");

          // status
          status = transfer->buffer[idx++];
          printf("status: 0x%02x\n", status);

          printf("DeviceLocalName: \"");
          printf((transfer->buffer + idx));
          printf("\"\n");
          break;

        case HCI_Read_Local_Supported_Commands:
          printf("Response to HCI_Read_Local_Supported_Commands\n");

          // status
          status = transfer->buffer[idx++];
          printf("status: 0x%02x\n", status);

          printf("Local supported commands\n");
          for (int i = idx; i < transfer->actual_length; ++i) {
            fprintf(stdout, "%02X%s", transfer->buffer[i], ( i + 1 ) % 16 == 0 ? "\r\n" : " " );
          }
          printf("\n");
          break;

        case HCI_Read_Local_Supported_Features:
          printf("Response to HCI_Read_Local_Supported_Commands\n");

          // 0E 0C 01 03 10 00 BF FE CF FE DB FF 7B 87

          // status
          status = transfer->buffer[idx++];
          printf("status: 0x%02x\n", status);

          // example response:
          //
          // LMP Features
          //   0xBF - 1011 1111
          //   .... ...1 = 3-slot packets: True
          //   .... ..1. = 5-slot packets: True
          //   .... .1.. = Encryption: True
          //   .... 1... = Slot Offset: True
          //   ...1 .... = Timing Accuracy: True
          //   ..1. .... = Role Switch: True
          //   .0.. .... = Hold Mode: False
          //   1... .... = Sniff Mode: True
          temp = transfer->buffer[idx++];

          uint8_t threeSlotPackets = temp & 0x01; temp >>= 1;
          uint8_t fiveSlotPackets = temp & 0x01; temp >>= 1;
          uint8_t encryption = temp & 0x01; temp >>= 1;
          uint8_t slotOffset = temp & 0x01; temp >>= 1;
          uint8_t timingAccuracy = temp & 0x01; temp >>= 1;
          uint8_t roleSwitch = temp & 0x01; temp >>= 1;
          uint8_t holdMode = temp & 0x01; temp >>= 1;
          uint8_t sniffMode = temp & 0x01; temp >>= 1;

          printf("3-slot packets: %s\n", threeSlotPackets ? "True" : "False");
          printf("5-slot packets: %s\n", fiveSlotPackets ? "True" : "False");
          printf("Encryption: %s\n", encryption ? "True" : "False");
          printf("Slot Offset: %s\n", slotOffset ? "True" : "False");
          printf("Timing Accuracy: %s\n", timingAccuracy ? "True" : "False");
          printf("Role Switch: %s\n", roleSwitch ? "True" : "False");
          printf("Hold Mode: %s\n", holdMode ? "True" : "False");
          printf("Sniff Mode: %s\n", sniffMode ? "True" : "False");

          //   0xfe
          //   .... ...0 = Park Mode: False
          //   .... ..1. = Power Control Requests: True
          //   .... .1.. = Channel Quality Driven Data Rate: True
          //   .... 1... = SCO Link: True
          //   ...1 .... = HV2 packets: True
          //   ..1. .... = HV3 packets: True
          //   .1.. .... = u-law Log Synchronous Data: True
          //   1... .... = A-law Log Synchronous Data: True
          temp = transfer->buffer[idx++];

          uint8_t parkMode = temp & 0x01; temp >>= 1;
          uint8_t powerControlRequests = temp & 0x01; temp >>= 1;
          uint8_t channelQualityDrivenDataRate = temp & 0x01; temp >>= 1;
          uint8_t scoLink = temp & 0x01; temp >>= 1;
          uint8_t hv2Packets = temp & 0x01; temp >>= 1;
          uint8_t hv3Packets = temp & 0x01; temp >>= 1;
          uint8_t uLawLogSynchronousData = temp & 0x01; temp >>= 1;
          uint8_t aLawLogSynchronousData = temp & 0x01; temp >>= 1;

          printf("Park Mode: %s\n", parkMode ? "True" : "False");
          printf("Power Control Requests: %s\n", powerControlRequests ? "True" : "False");
          printf("Channel Quality Driven Data Rate: %s\n", channelQualityDrivenDataRate ? "True" : "False");
          printf("SCO Link: %s\n", scoLink ? "True" : "False");
          printf("HV2 packets: %s\n", hv2Packets ? "True" : "False");
          printf("HV3 packets: %s\n", hv3Packets ? "True" : "False");
          printf("u-law Log Synchronous Data: %s\n", uLawLogSynchronousData ? "True" : "False");
          printf("A-law Log Synchronous Data: %s\n", aLawLogSynchronousData ? "True" : "False");

          //   .... ...1 = CVSD Synchronous Data: True
          //   .... ..1. = Paging Parameter Negotiation: True
          //   .... .1.. = Power Control: True
          //   .... 1... = Transparent Synchronous Data: True
          //   .100 .... = Flow Control Lag: 4 (1024 bytes)
          //   1... .... = Broadband Encryption: True
          //
          //   .... ...0 = Reserved: False
          //   .... ..1. = EDR ACL 2 Mbps Mode: True
          //   .... .1.. = EDR ACL 3 Mbps Mode: True
          //   .... 1... = Enhanced Inquiry Scan: True
          //   ...1 .... = Interlaced Inquiry Scan: True
          //   ..1. .... = Interlaced Page Scan: True
          //   .1.. .... = RSSI with Inquiry Results: True
          //   1... .... = EV3 Packets: True
          //
          //   .... ...1 = EV4 Packets: True
          //   .... ..1. = EV5 Packets: True
          //   .... .0.. = Reserved: False
          //   .... 1... = AFH Capable Slave: True
          //   ...1 .... = AFH Classification Slave: True
          //   ..0. .... = BR/EDR Not Supported: False
          //   .1.. .... = LE Supported Controller: True
          //   1... .... = 3-slot EDR ACL packets: True
          //
          //   .... ...1 = 5-slot EDR ACL packets: True
          //   .... ..1. = Sniff Subrating: True
          //   .... .1.. = Pause Encryption: True
          //   .... 1... = AFH Capable Master: True
          //   ...1 .... = AFH Classification Master: True
          //   ..1. .... = EDR eSCO 2 Mbps Mode: True
          //   .1.. .... = EDR eSCO 3 Mbps Mode: True
          //   1... .... = 3-slot EDR eSCO Packets: True
          //
          //   .... ...1 = Extended Inquiry Response: True
          //   .... ..1. = Simultaneous LE and BR/EDR to Same Device Capable Controller: True
          //   .... .0.. = Reserved: False
          //   .... 1... = Secure Simple Pairing: True
          //   ...1 .... = Encapsulated PDU: True
          //   ..1. .... = Erroneous Data Reporting: True
          //   .1.. .... = Non-flushable Packet Boundary Flag: True
          //   0... .... = Reserved: False
          //
          //   .... ...1 = Link Supervision Timeout Changed Event: True
          //   .... ..1. = Inquiry TX Power Level: True
          //   .... .1.. = Enhanced Power Control: True
          //   .000 0... = Reserved: False
          //   1... .... = Extended Features: True


          printf("Local supported featuress\n");
          break;

        case HCI_Read_Buffer_Size:
          printf("Response to HCI_Read_Buffer_Size\n");

          // 0E 0B 01 05 10 00 FD 03 40 08 00 01 00

          // status
          status = transfer->buffer[idx++];
          printf("status: 0x%02x\n", status);

          // (2 Byte) Host ACL Data Packet Length in bytes
          temp = transfer->buffer[idx++];
          temp = (transfer->buffer[idx++] << 8) + temp;
          printf("Host ACL Data Packet Length in bytes: %d (0x%04x)\n", temp, temp);

          // (1 Byte) Host SCO Data Packet Length in bytes
          temp = transfer->buffer[idx++];
          printf("Host SCO Data Packet Length in bytes: %d (0x%02x)\n", temp, temp);

          // (2 Byte) Host Total Num ACL Data Packets
          temp = transfer->buffer[idx++];
          temp = (transfer->buffer[idx++] << 8) + temp;
          printf("Host Total Num ACL Data Packets: %d (0x%04x)\n", temp, temp);

          // (2 Byte) Host Total Num SCO Data Packets
          temp = transfer->buffer[idx++];
          temp = (transfer->buffer[idx++] << 8) + temp;
          printf("Host Total Num SCO Data Packets: %d (0x%04x)\n", temp, temp);

          printf("Buffer Size: \n");
          break;

        case HCI_Read_BD_ADDR:
          printf("Response to HCI_Read_BD_ADDR\n");

          // status
          status = transfer->buffer[idx++];
          printf("status: 0x%02x\n", status);

          printf("BD_ADDR\n");
          for (int i = (transfer->actual_length-1); i >= idx; --i) {
            if (i != (transfer->actual_length-1))
            {
              printf(":");
            }
            fprintf(stdout, "%02X%s", transfer->buffer[i], ( i + 1 ) % 16 == 0 ? "\n" : "" );
          }
          printf("\n");
          break;

        case LE_Read_Buffer_Size:
          printf("Response to LE_Read_Buffer_Size\n");

          // status
          status = transfer->buffer[idx++];
          printf("status: 0x%02x\n", status);

          // le_acl_data_pkt_len
          temp = transfer->buffer[idx++];
          temp = (transfer->buffer[idx++] << 8) + temp;
          printf("LE ACL Data Packet Length: %d (0x%04x)\n", temp, temp);

          // le_total_num_acl_data_pkts
          temp = transfer->buffer[idx++];
          printf("Total Number LE ACL Data Packets: %d (0x%02x)\n", temp, temp);

          break;

        default:
          printf("Response to UNKNOWN command\n");
          break;
      }
      break;

    default:
      printf("Unknown event code: 0x%02x\n", event_code);
      break;

  }

}

static int transfer_completed = 0;


// FROM https://github.com/JohnOrlando/uhd_with_burx/blob/master/host/lib/transport/libusb1_zero_copy.cpp
/*
 * https://github.com/JohnOrlando/uhd_with_burx/blob/master/host/lib/transport/libusb1_zero_copy.cpp
 *
 * Print status errors of a completed transfer
 * \param lut pointer to an libusb_transfer
 */
void print_transfer_status(struct libusb_transfer *lut)
{
    switch (lut->status)
    {

      case LIBUSB_TRANSFER_COMPLETED:
          printf("LIBUSB_TRANSFER_COMPLETED\n");
          // if (lut->actual_length < lut->length) {
          //     std::cerr << "USB: transfer completed with short write,"
          //               << " length = " << lut->length
          //               << " actual = " << lut->actual_length << std::endl;
          // }

          // if ((lut->actual_length < 0) || (lut->length < 0)) {
          //     std::cerr << "USB: transfer completed with invalid response"
          //               << std::endl;
          // }
          break;

      case LIBUSB_TRANSFER_CANCELLED:
          printf("LIBUSB_TRANSFER_CANCELLED\n");
          break;

      case LIBUSB_TRANSFER_NO_DEVICE:
          printf("USB: device was disconnected\n");
          break;

      case LIBUSB_TRANSFER_OVERFLOW:
          printf("USB: device sent more data than requested\n");
          break;

      case LIBUSB_TRANSFER_TIMED_OUT:
          printf("USB: transfer timed out\n");
          break;

      case LIBUSB_TRANSFER_STALL:
          printf("USB: halt condition detected (stalled)\n");
          break;

      case LIBUSB_TRANSFER_ERROR:
          printf("USB: transfer failed\n");
          break;

      default:
          printf("USB: received unknown transfer status\n");
          break;
    }
}

LIBUSB_CALL static void async_callback(struct libusb_transfer *transfer)
{
  printf("\n\n\n");



#if 0
    printf("async_callback() main.c\n");
    print_transfer_status(transfer);
#endif

    // always handling an event as we're called when data is ready
    struct timeval tv;
    memset(&tv, 0, sizeof(struct timeval));
    libusb_handle_events_timeout(NULL, &tv);

#if 0
    int result = libusb_event_handler_active(context);
    if (result == 1)
    {
        printf("1 if a thread is handling events\n");
    }
    else
    {
        printf("0 if there are no threads currently handling events Multi-threaded applications and asynchronous I/O\n");
    }
#endif

#if 0
    // check if all done
    int completed = 1;
    for (int c = 0; c < EVENT_IN_BUFFER_COUNT; c++)
    {
        if (event_in_transfer[c])
        {
            printf("event_in_transfer[%u] still active (%p)\n", c, event_in_transfer[c]);
            completed = 0;

            print_transfer_status(event_in_transfer[c]);
            //break;
        }
    }
#endif

#if 0
    printf("begin async_callback endpoint %x, status %x, actual length %u \n",
      transfer->endpoint, transfer->status, transfer->actual_length );
#endif

    if (transfer->status == LIBUSB_TRANSFER_COMPLETED)
    {
        //printf("LIBUSB_TRANSFER_COMPLETED\n");

        //printf("<<< transfer->type: ", transfer->type);

        // dump raw data
        printf("<<< ");
        for (int i = 0; i < transfer->actual_length; i++) {
          fprintf(stdout, "%02X%s", transfer->buffer[i], (i + 1) % 16 == 0 ? "\n" : " ");
        }
        printf("\n");


        // switch () {
        //   case 0x03:
        //     break;

        //   case 0x0C:
        //     break;
        // }

      //   case 0x03:
      // printf("ACL Request received!\n");
      // break;

        dump_hci_event(transfer);

        transfer_completed++;

        printf("transfer_completed: %d\n", transfer_completed);

        if (transfer_completed == 2)
        {
          printf("\n\n\n");

          //
          // HCI command - Send "Read local version info" command
          //

          printf(">>> HCI command - Send \"Read local version info\" command\n");

          unsigned char buffer[1024];
          for (int i = 0; i < 1024; ++i) {
            buffer[i] = 0x00;
          }

          int idx = 0;
          buffer[idx++] = 0x01;
          buffer[idx++] = 0x10;
          buffer[idx++] = 0x00;

          usleep(wait * 1000);
          usb_send_cmd_packet(buffer, idx);

          transfer->user_data = NULL;
          libusb_submit_transfer(transfer);
        }

        if (transfer_completed == 4)
        {
          printf("\n\n\n");

          //
          // HCI command - Send "Read local name" command
          //

          printf(">>> HCI command - Send \"Read local name\" command\n");

          unsigned char buffer[1024];
          for (int i = 0; i < 1024; ++i) {
            buffer[i] = 0x00;
          }

          int idx = 0;
          buffer[idx++] = 0x14;
          buffer[idx++] = 0x0c;
          buffer[idx++] = 0x00;

          usleep(wait * 1000);
          usb_send_cmd_packet(buffer, idx);

          // without this, the incoming events are not received after all transfers have been used once!
          transfer->user_data = NULL;
          libusb_submit_transfer(transfer);
        }

        if (transfer_completed == 6)
        {
          printf("\n\n\n");

          //
          // HCI command - Send "Read local supported commands" command
          //

          printf(">>> HCI command - Send \"Read local supported commands\" command\n");

          unsigned char buffer[1024];
          for (int i = 0; i < 1024; ++i) {
            buffer[i] = 0x00;
          }

          int idx = 0;
          buffer[idx++] = 0x02;
          buffer[idx++] = 0x10;
          buffer[idx++] = 0x00;

          usleep(wait * 1000);
          usb_send_cmd_packet(buffer, idx);

          // without this, the incoming events are not received after all transfers have been used once!
          transfer->user_data = NULL;
          libusb_submit_transfer(transfer);
        }

        if (transfer_completed == 8)
        {
          printf("\n\n\n");

          //
          // HCI command - Send "Read BD ADDR" command
          //

          printf(">>> HCI command - Send \"Read BD ADDR\" command\n");

          unsigned char buffer[1024];
          for (int i = 0; i < 1024; ++i) {
            buffer[i] = 0x00;
          }

          int idx = 0;
          buffer[idx++] = 0x09;
          buffer[idx++] = 0x10;
          buffer[idx++] = 0x00;

          usleep(wait * 1000);
          usb_send_cmd_packet(buffer, idx);

          // without this, the incoming events are not received after all transfers have been used once!
          transfer->user_data = NULL;
          libusb_submit_transfer(transfer);
        }

        if (transfer_completed == 10)
        {
          printf("\n\n\n");

          //
          // HCI command - Send "Read Buffer Size" command
          //

          printf(">>> HCI command - Send \"Read Buffer Size\" command\n");

          unsigned char buffer[1024];
          for (int i = 0; i < 1024; ++i) {
            buffer[i] = 0x00;
          }

          int idx = 0;
          buffer[idx++] = 0x05;
          buffer[idx++] = 0x10;
          buffer[idx++] = 0x00;

          usleep(wait * 1000);
          usb_send_cmd_packet(buffer, idx);

          // without this, the incoming events are not received after all transfers have been used once!
          transfer->user_data = NULL;
          libusb_submit_transfer(transfer);
        }

        if (transfer_completed == 12)
        {
          printf("\n\n\n");

          //
          // HCI command - Send "Read Local Supported features" command
          //

          printf(">>> HCI command - Send \"Read Local Supported features\" command\n");

          unsigned char buffer[1024];
          for (int i = 0; i < 1024; ++i) {
            buffer[i] = 0x00;
          }

          int idx = 0;
          buffer[idx++] = 0x03;
          buffer[idx++] = 0x10;
          buffer[idx++] = 0x00;

          usleep(wait * 1000);
          usb_send_cmd_packet(buffer, idx);

          // without this, the incoming events are not received after all transfers have been used once!
          transfer->user_data = NULL;
          libusb_submit_transfer(transfer);
        }

        if (transfer_completed == 14)
        {
          printf("\n\n\n");

          //
          // HCI command - Send "Set Event Mask" command
          //

          printf(">>> HCI command - Send \"Set Event Mask\" command\n");

          unsigned char buffer[1024];
          for (int i = 0; i < 1024; ++i) {
            buffer[i] = 0x00;
          }

          int idx = 0;

          // 00 01 0c 08 ff ff ff ff ff ff ff 3f

          // Command Opcode: Set Event Mask (0x0C01)
          buffer[idx++] = 0x01;
          buffer[idx++] = 0x0C;

          // Parameter Total Length: 08
          buffer[idx++] = 0x08;

          // Paramter
          buffer[idx++] = 0xFF;
          buffer[idx++] = 0xFF;
          buffer[idx++] = 0xFF;
          buffer[idx++] = 0xFF;

          buffer[idx++] = 0xFF;
          buffer[idx++] = 0xFF;
          buffer[idx++] = 0xFF;
          buffer[idx++] = 0x3F;

          usleep(wait * 1000);
          usb_send_cmd_packet(buffer, idx);

          // without this, the incoming events are not received after all transfers have been used once!
          transfer->user_data = NULL;
          libusb_submit_transfer(transfer);
        }

/**/
        if (transfer_completed == 16)
        {
          printf("\n\n\n");

          //
          // HCI command - Send "LE Read Buffer Size" command
          //

          printf(">>> HCI command - Send \"LE Read Buffer Size\" command\n");

          unsigned char buffer[1024];
          for (int i = 0; i < 1024; ++i) {
            buffer[i] = 0x00;
          }

          // 00 02 20 00

          int idx = 0;
          buffer[idx++] = 0x02;
          buffer[idx++] = 0x20;
          buffer[idx++] = 0x00;

          usleep(wait * 1000);
          usb_send_cmd_packet(buffer, idx);

          // without this, the incoming events are not received after all transfers have been used once!
          transfer->user_data = NULL;
          libusb_submit_transfer(transfer);
        }

        if (transfer_completed == 18)
        {
          printf("\n\n\n");

          //
          // HCI command - Send "HCI_Read_Scan_Enable" command
          //

          printf(">>> HCI command - Send \"HCI_Read_Scan_Enable\" command\n");

          unsigned char buffer[1024];
          for (int i = 0; i < 1024; ++i) {
            buffer[i] = 0x00;
          }

          int idx = 0;
          buffer[idx++] = 0x19;
          buffer[idx++] = 0x0C;
          buffer[idx++] = 0x00;

          usleep(wait * 1000);
          usb_send_cmd_packet(buffer, idx);

          // without this, the incoming events are not received after all transfers have been used once!
          transfer->user_data = NULL;
          libusb_submit_transfer(transfer);
        }

        if (transfer_completed == 20)
        {
          printf("\n\n\n");

          //
          // HCI command - Send "HCI_LE_Set_Scan_Parameters" command
          //

          printf(">>> HCI command - Send \"HCI_LE_Set_Scan_Parameters\" command\n");

          unsigned char buffer[1024];
          for (int i = 0; i < 1024; ++i) {
            buffer[i] = 0x00;
          }

          // 00 0b 20 07 01 e0 01 30 00 00 00

          int idx = 0;
          buffer[idx++] = 0x0b;
          buffer[idx++] = 0x20;

          // parameter total length
          buffer[idx++] = 0x07;

          // scan type (0x01 - active)
          buffer[idx++] = 0x01;

          // scan interval
          buffer[idx++] = 0xe0;
          buffer[idx++] = 0x01;

          // scan window
          buffer[idx++] = 0x30;
          buffer[idx++] = 0x00;

          // own address type
          buffer[idx++] = 0x00;

          // scan filter policy -
          buffer[idx++] = 0x00;

          usleep(wait * 1000);
          usb_send_cmd_packet(buffer, idx);

          // without this, the incoming events are not received after all transfers have been used once!
          transfer->user_data = NULL;
          libusb_submit_transfer(transfer);
        }

/*
        if (transfer_completed == 22)
        {
          printf("\n\n\n");

          //
          // HCI command - Send "HCI_LE_Set_Scan_Enable" command
          //

          printf(">>> HCI command - Send \"HCI_LE_Set_Scan_Enable\" command\n");

          unsigned char buffer[1024];
          for (int i = 0; i < 1024; ++i) {
            buffer[i] = 0x00;
          }

          // int idx = 0;
          // buffer[idx++] = 0x20;
          // buffer[idx++] = 0x0C;
          // buffer[idx++] = 0x01;
          // buffer[idx++] = 0x02;
          // buffer[idx++] = 0x00;

          int idx = 0;
          buffer[idx++] = 0x0C;
          buffer[idx++] = 0x20;
          buffer[idx++] = 0x02;
          buffer[idx++] = 0x01;
          buffer[idx++] = 0x00;

          usleep(wait * 1000);
          usb_send_cmd_packet(buffer, idx);

          // without this, the incoming events are not received after all transfers have been used once!
          transfer->user_data = NULL;
          libusb_submit_transfer(transfer);
        }
*/

    if (transfer_completed == 22)
    {
      printf("\n\n\n");

      //
      // HCI command - Send "Write Simple Pairing mode" command
      //

        printf(">>> HCI command - Send \"Write Simple Pairing mode\" command\n");

        unsigned char buffer[1024];
          for (int i = 0; i < 1024; ++i) {
            buffer[i] = 0x00;
          }

          int idx = 0;
          buffer[idx++] = 0x56;
          buffer[idx++] = 0x0c;
          buffer[idx++] = 0x01;
          buffer[idx++] = 0x01;

          usleep(wait * 1000);
          usb_send_cmd_packet(buffer, idx);

          // without this, the incoming events are not received after all transfers have been used once!
          transfer->user_data = NULL;
          libusb_submit_transfer(transfer);
    }

    if (transfer_completed == 24)
    {
      printf("\n\n\n");

      //
      // HCI command - Send "HCI_Write_Page_Timeout" command
      //

        printf(">>> HCI command - Send \"HCI_Write_Page_Timeout\" command\n");

        unsigned char buffer[1024];
          for (int i = 0; i < 1024; ++i) {
            buffer[i] = 0x00;
          }

          int idx = 0;
          buffer[idx++] = 0x18;
          buffer[idx++] = 0x0c;
          buffer[idx++] = 0x02;
          buffer[idx++] = 0x00;
          buffer[idx++] = 0x60;

          usleep(wait * 1000);
          usb_send_cmd_packet(buffer, idx);

          // without this, the incoming events are not received after all transfers have been used once!
          transfer->user_data = NULL;
          libusb_submit_transfer(transfer);
    }

    if (transfer_completed == 26)
    {
      printf("\n\n\n");

      //
      // HCI command - Send "HCI_Write_Class_of_Device" command
      //

        printf(">>> HCI command - Send \"HCI_Write_Class_of_Device\" command\n");

        unsigned char buffer[1024];
          for (int i = 0; i < 1024; ++i) {
            buffer[i] = 0x00;
          }

          // 00 24 0c 03 0c 02 7a

          int idx = 0;
          buffer[idx++] = 0x24;
          buffer[idx++] = 0x0c;
          buffer[idx++] = 0x03;
          buffer[idx++] = 0x0c;
          buffer[idx++] = 0x02;
          buffer[idx++] = 0x7a;

          usleep(wait * 1000);
          usb_send_cmd_packet(buffer, idx);

          // without this, the incoming events are not received after all transfers have been used once!
          transfer->user_data = NULL;
          libusb_submit_transfer(transfer);
    }

    if (transfer_completed == 28)
    {
      printf("\n\n\n");

      //
      // HCI command - Send "HCI_Send_Change_Local_Name" command
      //

        printf(">>> HCI command - Send \"HCI_Send_Change_Local_Name\" command\n");

        unsigned char buffer[1024];
          for (int i = 0; i < 1024; ++i) {
            buffer[i] = 0x00;
          }

          // 00 13 0c f8 53 50 50 20 43 6f 75 6e 74 65 72 20 35 43 3a 46 33 3a 37 30 3a 37 44 3a 30 45 3a 39 36 000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000

          int idx = 0;
          buffer[idx++] = 0x13;
          buffer[idx++] = 0x0c;
          buffer[idx++] = 0xf8;

          // Device Name - 248 byte
          // buffer[idx++] = 0x53; // S
          // buffer[idx++] = 0x50; // P
          // buffer[idx++] = 0x50; // P
          buffer[idx++] = 0x57; // W
          buffer[idx++] = 0x46; // F
          buffer[idx++] = 0x42; // B
          buffer[idx++] = 0x20; //
          buffer[idx++] = 0x43; // C
          buffer[idx++] = 0x6f; // o
          buffer[idx++] = 0x75; // u
          buffer[idx++] = 0x6e; // n
          buffer[idx++] = 0x74; // t
          buffer[idx++] = 0x65; // e
          buffer[idx++] = 0x72; // r
          buffer[idx++] = 0x20;
          buffer[idx++] = 0x35;
          buffer[idx++] = 0x43;
          buffer[idx++] = 0x3a;
          buffer[idx++] = 0x46;
          buffer[idx++] = 0x33;
          buffer[idx++] = 0x3a;
          buffer[idx++] = 0x37;
          buffer[idx++] = 0x30;
          buffer[idx++] = 0x3a;
          buffer[idx++] = 0x37;
          buffer[idx++] = 0x44;
          buffer[idx++] = 0x3a;
          buffer[idx++] = 0x30;
          buffer[idx++] = 0x45;
          buffer[idx++] = 0x3a;
          buffer[idx++] = 0x39;
          buffer[idx++] = 0x36;

          usleep(wait * 1000);
          usb_send_cmd_packet(buffer, 251); // 3 byte header - 248 byte data

          // without this, the incoming events are not received after all transfers have been used once!
          transfer->user_data = NULL;
          libusb_submit_transfer(transfer);
    }


    if (transfer_completed == 30)
    {
      printf("\n\n\n");

      //
      // HCI command - Send "HCI_Write_Extended_Inquiry_Response" command
      //

        printf(">>> HCI command - Send \"HCI_Write_Extended_Inquiry_Response\" command\n");

        unsigned char buffer[1024];
          for (int i = 0; i < 1024; ++i) {
            buffer[i] = 0x00;
          }

          // 00 52 0c f1 00 1e 09 53 50 50 20 43 6f 75 6e 74 65 72 20 35 43 3a 46 33 3a 37 30 3a 37 44 3a 30 45 3a 39 36 0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000

          int idx = 0;
          buffer[idx++] = 0x52;
          buffer[idx++] = 0x0c;
          buffer[idx++] = 0xf1;
          buffer[idx++] = 0x00;

          // Extended Inquiry Response Data - 240 bytes
          buffer[idx++] = 0x1e;
          buffer[idx++] = 0x09;

          // buffer[idx++] = 0x53; // S
          // buffer[idx++] = 0x50; // P
          // buffer[idx++] = 0x50; // P
          buffer[idx++] = 0x57; // W
          buffer[idx++] = 0x46; // F
          buffer[idx++] = 0x42; // B
          buffer[idx++] = 0x20;
          buffer[idx++] = 0x43;
          buffer[idx++] = 0x6f;
          buffer[idx++] = 0x75;
          buffer[idx++] = 0x6e;
          buffer[idx++] = 0x74;
          buffer[idx++] = 0x65;
          buffer[idx++] = 0x72;
          buffer[idx++] = 0x20;
          buffer[idx++] = 0x35;
          buffer[idx++] = 0x43;
          buffer[idx++] = 0x3a;
          buffer[idx++] = 0x46;
          buffer[idx++] = 0x33;
          buffer[idx++] = 0x3a;
          buffer[idx++] = 0x37;
          buffer[idx++] = 0x30;
          buffer[idx++] = 0x3a;
          buffer[idx++] = 0x37;
          buffer[idx++] = 0x44;
          buffer[idx++] = 0x3a;
          buffer[idx++] = 0x30;
          buffer[idx++] = 0x45;
          buffer[idx++] = 0x3a;
          buffer[idx++] = 0x39;
          buffer[idx++] = 0x36;

          usleep(wait * 1000);
          usb_send_cmd_packet(buffer, 244); // 4 byte header - 240 byte data

          // without this, the incoming events are not received after all transfers have been used once!
          transfer->user_data = NULL;
          libusb_submit_transfer(transfer);
    }

    if (transfer_completed == 32)
    {
      printf("\n\n\n");

      //
      // HCI command - Send "HCI_Write_Inquiry_Mode" command
      //

        printf(">>> HCI command - Send \"HCI_Write_Inquiry_Mode\" command\n");

        unsigned char buffer[1024];
          for (int i = 0; i < 1024; ++i) {
            buffer[i] = 0x00;
          }

          // 00 45 0c 01 00

          int idx = 0;
          buffer[idx++] = 0x45;
          buffer[idx++] = 0x0c;
          buffer[idx++] = 0x01;
          buffer[idx++] = 0x00;

          usleep(wait * 1000);
          usb_send_cmd_packet(buffer, idx);

          // without this, the incoming events are not received after all transfers have been used once!
          transfer->user_data = NULL;
          libusb_submit_transfer(transfer);
    }

    if (transfer_completed == 34)
    {
      printf("\n\n\n");

      //
      // HCI command - Send "HCI_Write_Scan_Enable" command
      //

        printf(">>> HCI command - Send \"HCI_Write_Scan_Enable\" command\n");

        unsigned char buffer[1024];
          for (int i = 0; i < 1024; ++i) {
            buffer[i] = 0x00;
          }

          // 00 1a 0c 01 03

          int idx = 0;
          buffer[idx++] = 0x1a;
          buffer[idx++] = 0x0c;
          buffer[idx++] = 0x01;
          buffer[idx++] = 0x03;

          usleep(wait * 1000);
          usb_send_cmd_packet(buffer, idx);

          // without this, the incoming events are not received after all transfers have been used once!
          transfer->user_data = NULL;
          libusb_submit_transfer(transfer);
    }

    if (transfer_completed == 36)
    {
      printf("\n\n\n");

      //
      // HCI command - Send "Bluetooth HCI Command - Write Default Erroneous Data Reporting" command
      //

        printf(">>> HCI command - Send \"Bluetooth HCI Command - Write Default Erroneous Data Reporting\" command\n");

        unsigned char buffer[1024];
          for (int i = 0; i < 1024; ++i) {
            buffer[i] = 0x00;
          }

          // 00 5b 0c 01 01

          int idx = 0;
          buffer[idx++] = 0x5b;
          buffer[idx++] = 0x0c;
          buffer[idx++] = 0x01;
          buffer[idx++] = 0x01;

          usleep(wait * 1000);
          usb_send_cmd_packet(buffer, idx);

          // without this, the incoming events are not received after all transfers have been used once!
          transfer->user_data = NULL;
          libusb_submit_transfer(transfer);
    }

    if (transfer_completed == 38)
    {
      printf("\n\n\n");

      //
      // HCI command - Send "Bluetooth HCI Command - Vendor Command 0xfc1c - Write SCO PCM INT Parameter" command
      //

        printf(">>> HCI command - Send \"Bluetooth HCI Command - Vendor Command 0xfc1c - Write SCO PCM INT Parameter\" command\n");

        unsigned char buffer[1024];
          for (int i = 0; i < 1024; ++i) {
            buffer[i] = 0x00;
          }

          // 00 1c fc 05 01 00 00 00 00

          int idx = 0;
          buffer[idx++] = 0x1c;
          buffer[idx++] = 0xfc;
          buffer[idx++] = 0x05;
          buffer[idx++] = 0x01;
          buffer[idx++] = 0x00;
          buffer[idx++] = 0x00;
          buffer[idx++] = 0x00;
          buffer[idx++] = 0x00;

          usleep(wait * 1000);
          usb_send_cmd_packet(buffer, idx);

          // without this, the incoming events are not received after all transfers have been used once!
          transfer->user_data = NULL;
          libusb_submit_transfer(transfer);
    }







    }
    else if (transfer->status == LIBUSB_TRANSFER_STALL)
    {
        printf("LIBUSB_TRANSFER_STALL -> Transfer stalled, trying again\n");
        // r = libusb_clear_halt(handle, transfer->endpoint);
        // if (r) {
        //     log_error("Error rclearing halt %d", r);
        // }
        // r = libusb_submit_transfer(transfer);
        // if (r) {
        //     log_error("Error re-submitting transfer %d", r);
        // }
    }
    else
    {
        printf("async_callback. not data -> resubmit transfer, endpoint %x, status %x, length %u\n",
        transfer->endpoint, transfer->status, transfer->actual_length);

        // No usable data, just resubmit packet
        int r = libusb_submit_transfer(transfer);
        if (r) {
            printf("Error re-submitting transfer %d\n", r);
        }
    }
}

static int usb_send_cmd_packet(uint8_t *packet, int size)
{
    //printf("usb_send_cmd_packet() in main.c - libusb_fill_control_setup(), libusb_submit_transfer()\n");

    printf(">>> [size of message: %d] ", size);
    for (int i = 0; i < size; ++i)
    {
        fprintf(stdout, "%02X%s", packet[i], ( i + 1 ) % 16 == 0 ? "\n" : " " );
    }
    printf("\n");

    int r;

    // async
    libusb_fill_control_setup(hci_cmd_buffer, LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE, 0, 0, 0, size);
    memcpy(hci_cmd_buffer + LIBUSB_CONTROL_SETUP_SIZE, packet, size);

    // for (int i = 0; i < (3 + 256 + LIBUSB_CONTROL_SETUP_SIZE); ++i) {
    //     fprintf(stdout, "%02X%s", hci_cmd_buffer[i], ( i + 1 ) % 16 == 0 ? "\r\n" : " " );
    // }
    // printf("\n");

    // prepare transfer
    int completed = 0;
    libusb_fill_control_transfer(command_out_transfer, handle, hci_cmd_buffer, async_callback, &completed, 0);
    command_out_transfer->flags = LIBUSB_TRANSFER_FREE_BUFFER;

    // update state before submitting transfer
    usb_command_active = 1;

    // submit transfer
    int libusb_submit_transfer_result = libusb_submit_transfer(command_out_transfer);
    if (libusb_submit_transfer_result < 0)
    {
        usb_command_active = 0;

        printf("\n");
        printf("!!!!!!!!!!!!!!! Error submitting cmd transfer %d\n", libusb_submit_transfer_result);
        printf("!!!!!!!!!!!!!!! Error submitting cmd transfer %d\n", libusb_submit_transfer_result);
        printf("!!!!!!!!!!!!!!! Error submitting cmd transfer %d\n", libusb_submit_transfer_result);
        printf("!!!!!!!!!!!!!!! Error submitting cmd transfer %d\n", libusb_submit_transfer_result);
        printf("!!!!!!!!!!!!!!! Error submitting cmd transfer %d\n", libusb_submit_transfer_result);

        print_transfer_status(command_out_transfer);

        libusb_submit_transfer(command_out_transfer);

        return -1;
    }

#if 0
    printf("OUT: ");
    print_transfer_status(command_out_transfer);
#endif

    return 0;
}

static uint8_t find_dev(libusb_device **devs)
{
    libusb_device *dev;
    int i = 0, j = 0;

    while ((dev = devs[i++]) != NULL) {

        struct libusb_device_descriptor desc;

        int ret;
        char string[256];

        int r = libusb_get_device_descriptor(dev, &desc);
        if (r < 0) {
            fprintf(stderr, "failed to get device descriptor");

            libusb_exit(NULL);
            return 0;
        }

        ret = libusb_open(dev, &handle);

        if (LIBUSB_SUCCESS == ret)
        {
            printf("get %04x:%04x device string descriptor \n", desc.idVendor, desc.idProduct);

            printf("iProduct[%d]: ", desc.iProduct);

            ret = libusb_get_string_descriptor_ascii(handle, desc.iProduct, string, sizeof(string));
            if (ret > 0)
            {
                printf(string);
                printf("\n");
            }
        }

        scan_for_bt_endpoints(dev);

        if (desc.idVendor == 0x0b05 && desc.idProduct == 0x17cb) {

          printf("ASUS BT400 found!\n");

          // https://stackoverflow.com/questions/4813764/libusb-basic-example-wanted

          // // Detach a kernel driver from an interface. This is needed to claim an interface
          // // already claimed by a kernel driver. Returns 0 on success, LIBUSB_ERROR_NOT_FOUND
          // // if no kernel driver was active, LIBUSB_ERROR_INVALID_PARAM if the interface does not exist,
          // // LIBUSB_ERROR_NO_DEVICE if the device has been disconnected and a LIBUSB_ERROR code on failure.
          // // This function is non-portable.
          // uint32_t detach_result = libusb_detach_kernel_driver(handle, 0);
          // printf ("libusb_detach_kernel_driver(): %s\n", detach_result ? "failed" : "passed");

          // this usb reset might be another reset compared to the HCI reset!
          // I think it is necessary first to reset the usb device using this call to libusb_reset_device()
          // and then later to perform a HCI reset
          libusb_reset_device(handle);
          usleep(1000 * 1000);

          int configuration = 1;
          libusb_set_configuration(handle, configuration);
          usleep(1000 * 1000);

          //libusb_reset_device();
          //usleep(2000 * 1000);

          // // Set the active configuration to config for the device contained by devh.
          // // This function returns 0 on success, LIBUSB_ERROR_NOT_FOUND if the requested
          // // configuration does not exist, LIBUSB_ERROR_BUSY if the interfaces are currently
          // // claimed, LIBUSB_ERROR_NO_DEVICE if the device has been disconnected and a
          // // LIBUSB_ERROR code on failure.
          // uint32_t status = libusb_set_configuration(handle, 0);
          // printf ("libusb_set_configuration(): %s\n", status ? "failed" : "passed");
          // usleep(1000 * 1000);


          uint32_t claim_result = libusb_claim_interface(handle, 0);
          printf ("ibusb_claim_interface(): %s\n", claim_result ? "failed" : "passed");
          if (claim_result)
          {
            printf("libusb_close() - closing handle ...\n");
            libusb_close(handle);
            context = NULL;
            printf("libusb_close() - closing handle done\n");
            libusb_exit(NULL);

            return 0;
          }
          usleep(1000 * 1000);


/*
ASYNC - https://vovkos.github.io/doxyrest/samples/libusb/group_libusb_asyncio.html
We can view asynchronous I/O as a 5 step process:

1. Allocation : allocate a libusb_transfer
2. Filling : populate the libusb_transfer instance with information about the transfer you wish to perform
3. Submission : ask libusb to submit the transfer
4. Completion handling : examine transfer results in the libusb_transfer structure
5. Deallocation : clean up resources
*/


          // STEP 1 - allocate transfer handlers
          int c;
          for (c = 0; c < EVENT_IN_BUFFER_COUNT; c++)
          {
              // 0 isochronous transfers Events
              event_in_transfer[c] = libusb_alloc_transfer(0);
              if (!event_in_transfer[c])
              {
                libusb_release_interface(handle, 0);
                  printf("libusb_close() - closing handle ...\n");
                  libusb_close(handle);
                  context = NULL;
                  printf("libusb_close() - closing handle done\n");
                  libusb_exit(NULL);

                  return 0;
              }
          }
          for (c = 0; c < ACL_IN_BUFFER_COUNT; c++)
          {
              // 0 isochronous transfers ACL in
              acl_in_transfer[c] = libusb_alloc_transfer(0);
              if (!acl_in_transfer[c])
              {
                  libusb_release_interface(handle, 0);
                  printf("libusb_close() - closing handle ...\n");
                  libusb_close(handle);
                  context = NULL;
                  printf("libusb_close() - closing handle done\n");
                  libusb_exit(NULL);

                  return 0;
              }
          }

          command_out_transfer = libusb_alloc_transfer(0);
          acl_out_transfer     = libusb_alloc_transfer(0);

          // STEP 2 - filling

          for (int c = 0; c < EVENT_IN_BUFFER_COUNT; c++)
          {
              printf("<><><><> libusb_fill_interrupt_transfer() <><><><>\n");

              printf("libusb_fill_interrupt_transfer() ...\n");
              // configure event_in handlers
              libusb_fill_interrupt_transfer(event_in_transfer[c], handle, event_in_addr,
                      hci_event_in_buffer[c], HCI_ACL_BUFFER_SIZE, async_callback, NULL, 0);
              printf("libusb_fill_interrupt_transfer() done.\n");

              // STEP 3 - submission

              printf("libusb_submit_transfer() ...\n");
              r = libusb_submit_transfer(event_in_transfer[c]);
              if (r)
              {
                  printf("libusb_submit_transfer() - Error submitting interrupt transfer %d\n", r);

                  libusb_release_interface(handle, 0);
                  printf("libusb_close() - closing handle ...\n");
                  libusb_close(handle);
                  context = NULL;
                  printf("libusb_close() - closing handle done\n");
                  libusb_exit(NULL);

                  return 0;
              }
              printf("libusb_submit_transfer() done.\n");

              printf("print_transfer_status() ...\n");
              print_transfer_status(event_in_transfer[c]);
              printf("print_transfer_status() done.\n");
          }

          for (int c = 0; c < ACL_IN_BUFFER_COUNT; c++)
          {
              printf("<><><><> libusb_fill_bulk_transfer() <><><><>\n");

              // configure acl_in handlers
              libusb_fill_bulk_transfer(acl_in_transfer[c], handle, acl_in_addr,
                      hci_acl_in_buffer[c] + HCI_INCOMING_PRE_BUFFER_SIZE, HCI_ACL_BUFFER_SIZE, async_callback, NULL, 0);
              r = libusb_submit_transfer(acl_in_transfer[c]);
              if (r) {
                  printf("Error submitting bulk in transfer %d\n", r);

                  libusb_release_interface(handle, 0);
                  printf("libusb_close() - closing handle ...\n");
                  libusb_close(handle);
                  context = NULL;
                  printf("libusb_close() - closing handle done\n");
                  libusb_exit(NULL);

                  return 0;
              }

              print_transfer_status(acl_in_transfer[c]);
          }

          //
          // HCI command - Perform a HCI reset
          //

          unsigned char buffer[1024];
          for (int i = 0; i < 1024; ++i) {
            buffer[i] = 0x00;
          }

          int idx = 0;
          buffer[idx++] = 0x03;
          buffer[idx++] = 0x0C;
          buffer[idx++] = 0x00;

          usb_send_cmd_packet(buffer, idx);

          usleep(30 * 1000);

          // // wait for aync operation to return an event
          // for (int j = 0; j < 10; j++) {
          //     usleep(1000 * 1000);
          // }





          // //
          // // Send local version info
          // //

          // //unsigned char buffer[1024];
          // for (int i = 0; i < 1024; ++i) {
          //   buffer[i] = 0x00;
          // }

          // idx = 0;
          // //buffer[idx++] = 0x00;
          // buffer[idx++] = 0x01;
          // buffer[idx++] = 0x10;
          // buffer[idx++] = 0x00;

          // usb_send_cmd_packet(buffer, idx);




          // https://libusb.sourceforge.io/api-1.0/group__libusb__syncio.html#gadb11f7a761bd12fc77a07f4568d56f38

          // https://www.beyondlogic.org/usbnutshell/usb6.shtml
          // https://www.usbmadesimple.co.uk/ums_4.htm
          // set fields for the setup packet as needed
          //uint8_t       bmReqType = 0x80; //0x21;   // the request type (direction of transfer)

          // https://www.usb.org/sites/default/files/hid1_11.pdf

          // 0x21 = 00100 00 1 - Device to Host && Standard &&
                                          // https://www.beyondlogic.org/usbnutshell/usb6.shtml
          // Offset	Field	Size	Value	Description
          // 0	bmRequestType	1	Bit-Map	D7 Data Phase Transfer Direction
          // 0 = Host to Device
          // 1 = Device to Host
          // D6..5 Type
          // 0 = Standard
          // 1 = Class
          // 2 = Vendor
          // 3 = Reserved
          // D4..0 Recipient
          // 0 = Device
          // 1 = Interface
          // 2 = Endpoint
          // 3 = Other
          // 4..31 = Reserved

/*
          // transfer the setup packet to the USB device
          printf ("libusb_control_transfer() ...\n");

          unsigned char buffer[1024];
          for (i = 0; i < 1024; ++i)
          {
            buffer[i] = 0;
          }
          // for(i = 0; i < 50; ++i) {
          //     fprintf(stdout, "%02X%s", buffer[i], ( i + 1 ) % 16 == 0 ? "\r\n" : " " );
          //     }
          // printf("\n");

          // // GET_DESCRIPTOR - works
          // printf(">>> send GET_DESCRIPTOR ...\n");
          // uint8_t       bmReqType = 0x80; //0x21;   // the request type (direction of transfer)
          // uint8_t            bReq = GET_DESCRIPTOR;   // the request field for this packet
          // uint16_t           wVal = 0x0100;   // the value field for this packet
          // uint16_t         wIndex = 0x0000;   // the index field for this packet
          // unsigned char*   data = buffer;   // the data buffer for the in/output data
          // uint16_t           wLen = 0x0012;   // length of this setup packet
          // unsigned int     timeout = 5000;       // timeout duration (if transfer fails)

          // // GET_STATUS
          // printf(">>> send GET_STATUS ...\n");
          // uint8_t            bReq = GET_STATUS;   // the request field for this packet
          // uint16_t           wVal = 0x0C03;   // the value field for this packet
          // uint16_t         wIndex = 0x0000;   // the index field for this packet
          // unsigned char*   data = buffer;   // the data buffer for the in/output data
          // uint16_t           wLen = 0x0004;   // length of this setup packet
          // unsigned int     timeout = 5000;       // timeout duration (if transfer fails)

          // // RESET - Does not work
          // printf(">>> send RESET ...\n");
          // int idx = 0;
          // //buffer[0] = 0x00;
          // // buffer[idx++] = 0x03;
          // // buffer[idx++] = 0x00;
          // // buffer[idx++] = 0x03;
          // // buffer[idx++] = 0x0C;
          // // buffer[idx++] = 0x00;
          // uint8_t       bmReqType = 0x00;   // the request type (direction of transfer)
          // uint8_t            bReq = 0x01;   // the request field for this packet
          // uint16_t           wVal = 0x0003;   // the value field for this packet
          // uint16_t         wIndex = 0x0C00;   // the index field for this packet
          // unsigned char*   data = buffer;   // the data buffer for the in/output data
          // //uint16_t           wLen = 0x000B;   // length of this setup packet
          // uint16_t           wLen = 0x0004;
          // unsigned int     timeout = 20000;       // timeout duration (if transfer fails)

          // Returns
          // on success, the number of bytes actually transferred
          // LIBUSB_ERROR_TIMEOUT if the transfer timed out
          // LIBUSB_ERROR_PIPE if the control request was not supported by the device
          // LIBUSB_ERROR_NO_DEVICE if the device has been disconnected
          // LIBUSB_ERROR_BUSY if called from event handling context
          // LIBUSB_ERROR_INVALID_PARAM if the transfer size is larger than the operating
          // system and/or hardware can support (see Transfer length limitations)
          // another LIBUSB_ERROR code on other failures

          //usleep(20000 * 1000);

          // https://learn.microsoft.com/en-us/windows-hardware/drivers/usbcon/usb-control-transfer

          uint32_t control_transfer_result = libusb_control_transfer(handle, bmReqType, bReq, wVal, wIndex, data, wLen, timeout);

          printf("libusb_control_transfer() result %d \n", control_transfer_result);

          if (control_transfer_result == LIBUSB_ERROR_TIMEOUT)
          {
            printf("LIBUSB_ERROR_TIMEOUT");
          }
          else if (control_transfer_result == LIBUSB_ERROR_PIPE)
          {
            printf("LIBUSB_ERROR_PIPE");
          }
          else if (control_transfer_result == LIBUSB_ERROR_NO_DEVICE)
          {
            printf("LIBUSB_ERROR_NO_DEVICE");
          }
          else if (control_transfer_result == LIBUSB_ERROR_BUSY)
          {
            printf("LIBUSB_ERROR_BUSY");
          }
          else if (control_transfer_result == LIBUSB_ERROR_INVALID_PARAM)
          {
            printf("LIBUSB_ERROR_INVALID_PARAM");
          }
          // if (control_transfer_result == LIBUSB_ERROR) {
          //   printf("LIBUSB_ERROR");
          // }
          else {

              printf("libusb_control_transfer() return %d bytes\n", control_transfer_result);

              printf("<<< ");
              for (i = 0; i < control_transfer_result; ++i)
              {
                fprintf(stdout, "%02X%s", buffer[i], ( i + 1 ) % 16 == 0 ? "\r\n" : " " );
              }
              printf("\n");

              // length of total data including the length field itself
              uint8_t length = buffer[0];

              // type of descriptor (one of device descriptor (0x01), configuration descriptor (0x02),
              // interface descriptor (0x04), endpoint (0x05), device qualifier (0x06))

              uint8_t descriptor_type = buffer[1];
              switch (descriptor_type) {

                case DEVICE_DESCRIPTOR:
                  printf("DEVICE_DESCRIPTOR\n");
                  break;

                case CONFIGURATION_DESCRIPTOR:
                  printf("CONFIGURATION_DESCRIPTOR\n");
                  break;

                case INTERFACE_DESCRIPTOR:
                  printf("INTERFACE_DESCRIPTOR\n");
                  break;

                case ENDPOINT_DESCRIPTOR:
                  printf("ENDPOINT_DESCRIPTOR\n");
                  break;

                case DEVICE_QUALIFIER_DESCRIPTOR:
                  printf("DEVICE_QUALIFIER_DESCRIPTOR\n");
                  break;

                default:
                  printf("UNKNOWN DESCRIPTOR DETECTED!\n");
                  break;
              }
          }

        //   if (libusb_control_transfer(handle, 0x21, 0x09, 0x3000, 0x0000, OutPutReport, 0x0040, 3000) < 0) {
        //     return -1;
        // }

        // if(0 > libusb_control_transfer(handle, 0xa1, 0x01, 0x3000, 0x0000, InPutReport, 0x0040, 5000)) {
        //     return -1;
        // }

*/
          // https://libusb.sourceforge.io/api-1.0/group__libusb__poll.html
          while (1) {
            libusb_handle_events(context);
          }

          printf("libusb_close() - closing handle ...\n");
          libusb_close(handle);
          printf("libusb_close() - closing handle done\n");
        }

        printf("\n");
    }

    printf("end find_dev()\n");
}

int main(void)
{
    libusb_device **devs;
    int r;
    ssize_t cnt;

    r = libusb_init(&context);

#if 0
    libusb_set_option(context, LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_DEBUG);
#endif

    if (r < 0)
        return r;

    cnt = libusb_get_device_list(NULL, &devs);
    if (cnt < 0){
        libusb_exit(NULL);
        return (int) cnt;
    }

    find_dev(devs);

    libusb_free_device_list(devs, 1);

    libusb_exit(NULL);

    return 0;
}