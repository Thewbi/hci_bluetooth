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
static struct libusb_transfer *event_in_transfer[EVENT_IN_BUFFER_COUNT];
//static struct libusb_transfer *acl_in_transfer[ACL_IN_BUFFER_COUNT];

static int usb_acl_out_active = 0;
static int usb_command_active = 0;

libusb_device_handle* handle = NULL;

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

// /Users/bischowg/dev/bluetooth/bt_stack_experiments/isr-btstack/btstack/platform/libusb/hci_transport_h2_libusb.c
// contains the configuration for libusb

// endpoint addresses
static int event_in_addr;
static int acl_in_addr;
static int acl_out_addr;
static int sco_in_addr;
static int sco_out_addr;

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

LIBUSB_CALL static void async_callback(struct libusb_transfer *transfer)
{
    printf("async_callback() in hci_transport_h2_libusb.c\n");

    int r;
    printf("begin async_callback endpoint %x, status %x, actual length %u \n",
      transfer->endpoint, transfer->status, transfer->actual_length );

    if (transfer->status == LIBUSB_TRANSFER_COMPLETED)
    {
        printf("LIBUSB_TRANSFER_COMPLETED\n");
        //queue_transfer(transfer);

        printf("<<< ");
        for (int i = 0; i < transfer->actual_length; ++i) {
          fprintf(stdout, "%02X%s", transfer->buffer[i], ( i + 1 ) % 16 == 0 ? "\r\n" : " " );
        }
        printf("\n");
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
        r = libusb_submit_transfer(transfer);
        if (r) {
            printf("Error re-submitting transfer %d\n", r);
        }
    }
}

static int usb_send_cmd_packet(uint8_t *packet, int size){

    printf("usb_send_cmd_packet() in hci_transport_h2_libusb.c - - libusb_fill_control_setup(), libusb_submit_transfer()\n");

    printf(">>> ");
    for (int i = 0; i < size; ++i)
    {
        fprintf(stdout, "%02X%s", packet[i], ( i + 1 ) % 16 == 0 ? "\r\n" : " " );
    }
    printf("\n");

    int r;

    //if (libusb_state != LIB_USB_TRANSFERS_ALLOCATED) return -1;

    printf("A\n");

    // async
    libusb_fill_control_setup(hci_cmd_buffer, LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE, 0, 0, 0, size);
    memcpy(hci_cmd_buffer + LIBUSB_CONTROL_SETUP_SIZE, packet, size);

    printf("B\n");

    // for (int i = 0; i < (3 + 256 + LIBUSB_CONTROL_SETUP_SIZE); ++i) {
    //     fprintf(stdout, "%02X%s", hci_cmd_buffer[i], ( i + 1 ) % 16 == 0 ? "\r\n" : " " );
    // }
    // printf("\n");

    // prepare transfer
    int completed = 0;
    libusb_fill_control_transfer(command_out_transfer, handle, hci_cmd_buffer, async_callback, &completed, 0);
    command_out_transfer->flags = LIBUSB_TRANSFER_FREE_BUFFER;

    printf("C\n");

    // update stata before submitting transfer
    usb_command_active = 1;

    // submit transfer
    r = libusb_submit_transfer(command_out_transfer);
    if (r < 0)
    {
        printf("D\n");

        usb_command_active = 0;
        printf("Error submitting cmd transfer %d", r);

        return -1;
    }

    printf("E\n");

    return 0;
}

libusb_context* context = NULL;

static void find_dev(libusb_device **devs)
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
            return;
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

          // Detach a kernel driver from an interface. This is needed to claim an interface
          // already claimed by a kernel driver. Returns 0 on success, LIBUSB_ERROR_NOT_FOUND
          // if no kernel driver was active, LIBUSB_ERROR_INVALID_PARAM if the interface does not exist,
          // LIBUSB_ERROR_NO_DEVICE if the device has been disconnected and a LIBUSB_ERROR code on failure.
          // This function is non-portable.
          // uint32_t detach_result = libusb_detach_kernel_driver(handle, 0);
          // printf ("libusb_detach_kernel_driver(): %s\n", detach_result ? "failed" : "passed");

          libusb_reset_device(handle);

          int configuration = 1;
          libusb_set_configuration(handle, configuration);



          //libusb_reset_device();
          //usleep(2000 * 1000);

          // Set the active configuration to config for the device contained by devh.
          // This function returns 0 on success, LIBUSB_ERROR_NOT_FOUND if the requested
          // configuration does not exist, LIBUSB_ERROR_BUSY if the interfaces are currently
          // claimed, LIBUSB_ERROR_NO_DEVICE if the device has been disconnected and a
          // LIBUSB_ERROR code on failure.
          uint32_t status = libusb_set_configuration(handle, 0);
          printf ("libusb_set_configuration(): %s\n", status ? "failed" : "passed");

          uint32_t claim_result = libusb_claim_interface(handle, 0);
          printf ("ibusb_claim_interface(): %s\n", claim_result ? "failed" : "passed");


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
          for (c = 0 ; c < EVENT_IN_BUFFER_COUNT ; c++) {
              event_in_transfer[c] = libusb_alloc_transfer(0); // 0 isochronous transfers Events
              if (!event_in_transfer[c]) {
                  libusb_exit(NULL);
                  return;
              }
          }
          for (c = 0 ; c < ACL_IN_BUFFER_COUNT ; c++) {
              acl_in_transfer[c]  =  libusb_alloc_transfer(0); // 0 isochronous transfers ACL in
              if (!acl_in_transfer[c]) {
                  libusb_exit(NULL);
                  return;
              }
          }

          command_out_transfer = libusb_alloc_transfer(0);
          acl_out_transfer     = libusb_alloc_transfer(0);

          // STEP 2 - filling

          for (int c = 0; c < EVENT_IN_BUFFER_COUNT; c++)
          {
              printf("A\n");

              printf("<><><><> libusb_fill_interrupt_transfer() <><><><>\n");

              // configure event_in handlers
              libusb_fill_interrupt_transfer(event_in_transfer[c], handle, event_in_addr,
                      hci_event_in_buffer[c], HCI_ACL_BUFFER_SIZE, async_callback, NULL, 0);

              printf("B\n");

              r = libusb_submit_transfer(event_in_transfer[c]);

              printf("C\n");

              if (r) {

                  printf("D\n");

                  printf("Error submitting interrupt transfer %d", r);
                  libusb_exit(NULL);
                  return;
              }
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
                  libusb_exit(NULL);
                  return;
              }

          }


          unsigned char buffer[1024];
          for (int i = 0; i < 1024; ++i) {
            buffer[i] = 0x00;
          }

          int idx = 0;
          //buffer[idx++] = 0x00;
          buffer[idx++] = 0x03;
          buffer[idx++] = 0x0C;
          buffer[idx++] = 0x00;



          usb_send_cmd_packet(buffer, idx);

          // // wait for aync operation to return an event
          // for (int j = 0; j < 10; j++) {
          //     usleep(1000 * 1000);
          // }




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

    libusb_set_option(context, LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_DEBUG);

    if (r < 0)
        return r;

    cnt = libusb_get_device_list(NULL, &devs);
    if (cnt < 0){
        libusb_exit(NULL);
        return (int) cnt;
    }

    find_dev(devs);

    printf("AA\n");

    libusb_free_device_list(devs, 1);

    printf("BB\n");

    libusb_exit(NULL);

    printf("CC\n");

    return 0;
}