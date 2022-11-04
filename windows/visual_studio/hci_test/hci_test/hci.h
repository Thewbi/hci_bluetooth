#pragma once

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

// OGFs
#define OGF_LINK_CONTROL 0x01
#define OGF_LINK_POLICY 0x02
#define OGF_CONTROLLER_BASEBAND 0x03
#define OGF_INFORMATIONAL_PARAMETERS 0x04
#define OGF_STATUS_PARAMETERS 0x05
#define OGF_TESTING 0x06
#define OGF_LE_CONTROLLER 0x08
#define OGF_BTSTACK 0x3d
#define OGF_VENDOR 0x3f

// OGF - Opcode Group Field (6 upper bits)
// OCF - Opcode Command Field (10 lower bits)
//
// (OGF << 10) + OCF = Command Opcode
//
// e.g. RESET:
// OGF - 0000 11 (= 0x03)
// OCF - 00 0000 0011 (= 0x03)
// (OGF << 10) + OCF = 0000 1100 0000 0011 = 0x0C03
//
// Hint: the enum hci_opcode_t in this file has all opcodes preconstructed!
//
// calculate hci opcode for ogf/ocf value
#define HCI_OPCODE(ogf, ocf) ((ocf) | ((ogf) << 10))
#define HCI_OPCODE_HIGH(data) static_cast<uint8_t>(data >> 8)
#define HCI_OPCODE_LOW(data) static_cast<uint8_t>(data & 0xFF)
#define HCI_OPCODE_TO_ARRAY(data) { HCI_OPCODE_LOW(data), HCI_OPCODE_HIGH(data) }
#define ARRAY_TO_UINT16(data) { static_cast<uint16_t>((data[0] << 8) + data[1]) }

/**
 * compact HCI Command packet description
 */
typedef struct {
	uint16_t    opcode;
	const char *format;
} hci_cmd_t;

typedef enum {

	// Link Control (0x01)
	HCI_OPCODE_HCI_INQUIRY = HCI_OPCODE(OGF_LINK_CONTROL, 0x01),
	HCI_OPCODE_HCI_INQUIRY_CANCEL = HCI_OPCODE(OGF_LINK_CONTROL, 0x02),
	HCI_OPCODE_HCI_PERIODIC_INQUIRY_MODE = HCI_OPCODE(OGF_LINK_CONTROL, 0x03),
	HCI_OPCODE_HCI_EXIT_PERIODIC_INQUIRY_MODE = HCI_OPCODE(OGF_LINK_CONTROL, 0x04),
	HCI_OPCODE_HCI_CREATE_CONNECTION = HCI_OPCODE(OGF_LINK_CONTROL, 0x05),
	HCI_OPCODE_HCI_DISCONNECT = HCI_OPCODE(OGF_LINK_CONTROL, 0x06),
	HCI_OPCODE_HCI_CREATE_CONNECTION_CANCEL = HCI_OPCODE(OGF_LINK_CONTROL, 0x08),
	HCI_OPCODE_HCI_ACCEPT_CONNECTION_REQUEST = HCI_OPCODE(OGF_LINK_CONTROL, 0x09),
	HCI_OPCODE_HCI_REJECT_CONNECTION_REQUEST = HCI_OPCODE(OGF_LINK_CONTROL, 0x0a),
	HCI_OPCODE_HCI_LINK_KEY_REQUEST_REPLY = HCI_OPCODE(OGF_LINK_CONTROL, 0x0b),
	HCI_OPCODE_HCI_LINK_KEY_REQUEST_NEGATIVE_REPLY = HCI_OPCODE(OGF_LINK_CONTROL, 0x0c),
	HCI_OPCODE_HCI_PIN_CODE_REQUEST_REPLY = HCI_OPCODE(OGF_LINK_CONTROL, 0x0d),
	HCI_OPCODE_HCI_PIN_CODE_REQUEST_NEGATIVE_REPLY = HCI_OPCODE(OGF_LINK_CONTROL, 0x0e),
	HCI_OPCODE_HCI_CHANGE_CONNECTION_PACKET_TYPE = HCI_OPCODE(OGF_LINK_CONTROL, 0x0f),
	HCI_OPCODE_HCI_AUTHENTICATION_REQUESTED = HCI_OPCODE(OGF_LINK_CONTROL, 0x11),
	HCI_OPCODE_HCI_SET_CONNECTION_ENCRYPTION = HCI_OPCODE(OGF_LINK_CONTROL, 0x13),
	HCI_OPCODE_HCI_CHANGE_CONNECTION_LINK_KEY = HCI_OPCODE(OGF_LINK_CONTROL, 0x15),
	HCI_OPCODE_HCI_REMOTE_NAME_REQUEST = HCI_OPCODE(OGF_LINK_CONTROL, 0x19),
	HCI_OPCODE_HCI_REMOTE_NAME_REQUEST_CANCEL = HCI_OPCODE(OGF_LINK_CONTROL, 0x1A),
	HCI_OPCODE_HCI_READ_REMOTE_SUPPORTED_FEATURES_COMMAND = HCI_OPCODE(OGF_LINK_CONTROL, 0x1B),
	HCI_OPCODE_HCI_READ_REMOTE_EXTENDED_FEATURES_COMMAND = HCI_OPCODE(OGF_LINK_CONTROL, 0x1C),
	HCI_OPCODE_HCI_READ_REMOTE_VERSION_INFORMATION = HCI_OPCODE(OGF_LINK_CONTROL, 0x1D),
	HCI_OPCODE_HCI_SETUP_SYNCHRONOUS_CONNECTION = HCI_OPCODE(OGF_LINK_CONTROL, 0x0028),
	HCI_OPCODE_HCI_ACCEPT_SYNCHRONOUS_CONNECTION = HCI_OPCODE(OGF_LINK_CONTROL, 0x0029),
	HCI_OPCODE_HCI_IO_CAPABILITY_REQUEST_REPLY = HCI_OPCODE(OGF_LINK_CONTROL, 0x2b),
	HCI_OPCODE_HCI_USER_CONFIRMATION_REQUEST_REPLY = HCI_OPCODE(OGF_LINK_CONTROL, 0x2c),
	HCI_OPCODE_HCI_USER_CONFIRMATION_REQUEST_NEGATIVE_REPLY = HCI_OPCODE(OGF_LINK_CONTROL, 0x2d),
	HCI_OPCODE_HCI_USER_PASSKEY_REQUEST_REPLY = HCI_OPCODE(OGF_LINK_CONTROL, 0x2e),
	HCI_OPCODE_HCI_USER_PASSKEY_REQUEST_NEGATIVE_REPLY = HCI_OPCODE(OGF_LINK_CONTROL, 0x2f),
	HCI_OPCODE_HCI_REMOTE_OOB_DATA_REQUEST_REPLY = HCI_OPCODE(OGF_LINK_CONTROL, 0x30),
	HCI_OPCODE_HCI_REMOTE_OOB_DATA_REQUEST_NEGATIVE_REPLY = HCI_OPCODE(OGF_LINK_CONTROL, 0x33),
	HCI_OPCODE_HCI_IO_CAPABILITY_REQUEST_NEGATIVE_REPLY = HCI_OPCODE(OGF_LINK_CONTROL, 0x34),
	HCI_OPCODE_HCI_ENHANCED_SETUP_SYNCHRONOUS_CONNECTION = HCI_OPCODE(OGF_LINK_CONTROL, 0x3d),
	HCI_OPCODE_HCI_ENHANCED_ACCEPT_SYNCHRONOUS_CONNECTION = HCI_OPCODE(OGF_LINK_CONTROL, 0x3e),
	HCI_OPCODE_HCI_REMOTE_OOB_EXTENDED_DATA_REQUEST_REPLY = HCI_OPCODE(OGF_LINK_CONTROL, 0x45),

	// Link Policy (0x02)
	HCI_OPCODE_HCI_HOLD_MODE = HCI_OPCODE(OGF_LINK_POLICY, 0x01),
	HCI_OPCODE_HCI_SNIFF_MODE = HCI_OPCODE(OGF_LINK_POLICY, 0x03),
	HCI_OPCODE_HCI_EXIT_SNIFF_MODE = HCI_OPCODE(OGF_LINK_POLICY, 0x04),
	HCI_OPCODE_HCI_PARK_STATE = HCI_OPCODE(OGF_LINK_POLICY, 0x05),
	HCI_OPCODE_HCI_EXIT_PARK_STATE = HCI_OPCODE(OGF_LINK_POLICY, 0x06),
	HCI_OPCODE_HCI_QOS_SETUP = HCI_OPCODE(OGF_LINK_POLICY, 0x07),
	HCI_OPCODE_HCI_ROLE_DISCOVERY = HCI_OPCODE(OGF_LINK_POLICY, 0x09),
	HCI_OPCODE_HCI_SWITCH_ROLE_COMMAND = HCI_OPCODE(OGF_LINK_POLICY, 0x0b),
	HCI_OPCODE_HCI_READ_LINK_POLICY_SETTINGS = HCI_OPCODE(OGF_LINK_POLICY, 0x0c),
	HCI_OPCODE_HCI_WRITE_LINK_POLICY_SETTINGS = HCI_OPCODE(OGF_LINK_POLICY, 0x0d),
	HCI_OPCODE_HCI_WRITE_DEFAULT_LINK_POLICY_SETTING = HCI_OPCODE(OGF_LINK_POLICY, 0x0F),
	HCI_OPCODE_HCI_FLOW_SPECIFICATION = HCI_OPCODE(OGF_LINK_POLICY, 0x10),
	HCI_OPCODE_HCI_SNIFF_SUBRATING = HCI_OPCODE(OGF_LINK_POLICY, 0x11),

	// Controller Baseband (0x03)
	HCI_OPCODE_HCI_SET_EVENT_MASK = HCI_OPCODE(OGF_CONTROLLER_BASEBAND, 0x01),
	HCI_OPCODE_HCI_RESET = HCI_OPCODE(OGF_CONTROLLER_BASEBAND, 0x03),
	HCI_OPCODE_HCI_FLUSH = HCI_OPCODE(OGF_CONTROLLER_BASEBAND, 0x08),
	HCI_OPCODE_HCI_READ_PIN_TYPE = HCI_OPCODE(OGF_CONTROLLER_BASEBAND, 0x09),
	HCI_OPCODE_HCI_WRITE_PIN_TYPE = HCI_OPCODE(OGF_CONTROLLER_BASEBAND, 0x0A),
	HCI_OPCODE_HCI_DELETE_STORED_LINK_KEY = HCI_OPCODE(OGF_CONTROLLER_BASEBAND, 0x12),
	HCI_OPCODE_HCI_WRITE_LOCAL_NAME = HCI_OPCODE(OGF_CONTROLLER_BASEBAND, 0x13),
	HCI_OPCODE_HCI_READ_LOCAL_NAME = HCI_OPCODE(OGF_CONTROLLER_BASEBAND, 0x14),
	HCI_OPCODE_HCI_READ_PAGE_TIMEOUT = HCI_OPCODE(OGF_CONTROLLER_BASEBAND, 0x17),
	HCI_OPCODE_HCI_WRITE_PAGE_TIMEOUT = HCI_OPCODE(OGF_CONTROLLER_BASEBAND, 0x18),
	HCI_OPCODE_HCI_WRITE_SCAN_ENABLE = HCI_OPCODE(OGF_CONTROLLER_BASEBAND, 0x1A),
	HCI_OPCODE_HCI_READ_PAGE_SCAN_ACTIVITY = HCI_OPCODE(OGF_CONTROLLER_BASEBAND, 0x1B),
	HCI_OPCODE_HCI_WRITE_PAGE_SCAN_ACTIVITY = HCI_OPCODE(OGF_CONTROLLER_BASEBAND, 0x1C),
	HCI_OPCODE_HCI_READ_INQUIRY_SCAN_ACTIVITY = HCI_OPCODE(OGF_CONTROLLER_BASEBAND, 0x1D),
	HCI_OPCODE_HCI_WRITE_INQUIRY_SCAN_ACTIVITY = HCI_OPCODE(OGF_CONTROLLER_BASEBAND, 0x1E),
	HCI_OPCODE_HCI_WRITE_AUTHENTICATION_ENABLE = HCI_OPCODE(OGF_CONTROLLER_BASEBAND, 0x20),
	HCI_OPCODE_HCI_WRITE_CLASS_OF_DEVICE = HCI_OPCODE(OGF_CONTROLLER_BASEBAND, 0x24),
	HCI_OPCODE_HCI_WRITE_AUTOMATIC_FLUSH_TIMEOUT = HCI_OPCODE(OGF_CONTROLLER_BASEBAND, 0x28),
	HCI_OPCODE_HCI_READ_NUM_BROADCAST_RETRANSMISSIONS = HCI_OPCODE(OGF_CONTROLLER_BASEBAND, 0x29),
	HCI_OPCODE_HCI_WRITE_NUM_BROADCAST_RETRANSMISSIONS = HCI_OPCODE(OGF_CONTROLLER_BASEBAND, 0x2a),
	HCI_OPCODE_HCI_READ_TRANSMIT_POWER_LEVEL = HCI_OPCODE(OGF_CONTROLLER_BASEBAND, 0x2D),
	HCI_OPCODE_HCI_WRITE_SYNCHRONOUS_FLOW_CONTROL_ENABLE = HCI_OPCODE(OGF_CONTROLLER_BASEBAND, 0x2f),
	HCI_OPCODE_HCI_SET_CONTROLLER_TO_HOST_FLOW_CONTROL = HCI_OPCODE(OGF_CONTROLLER_BASEBAND, 0x31),
	HCI_OPCODE_HCI_HOST_BUFFER_SIZE = HCI_OPCODE(OGF_CONTROLLER_BASEBAND, 0x33),
	HCI_OPCODE_HCI_HOST_NUMBER_OF_COMPLETED_PACKETS = HCI_OPCODE(OGF_CONTROLLER_BASEBAND, 0x35),
	HCI_OPCODE_HCI_READ_LINK_SUPERVISION_TIMEOUT = HCI_OPCODE(OGF_CONTROLLER_BASEBAND, 0x36),
	HCI_OPCODE_HCI_WRITE_LINK_SUPERVISION_TIMEOUT = HCI_OPCODE(OGF_CONTROLLER_BASEBAND, 0x37),
	HCI_OPCODE_HCI_WRITE_CURRENT_IAC_LAP_TWO_IACS = HCI_OPCODE(OGF_CONTROLLER_BASEBAND, 0x3A),
	HCI_OPCODE_HCI_READ_INQUIRY_SCAN_TYPE = HCI_OPCODE(OGF_CONTROLLER_BASEBAND, 0x42),
	HCI_OPCODE_HCI_WRITE_INQUIRY_SCAN_TYPE = HCI_OPCODE(OGF_CONTROLLER_BASEBAND, 0x43),
	HCI_OPCODE_HCI_READ_INQUIRY_MODE = HCI_OPCODE(OGF_CONTROLLER_BASEBAND, 0x44),
	HCI_OPCODE_HCI_WRITE_INQUIRY_MODE = HCI_OPCODE(OGF_CONTROLLER_BASEBAND, 0x45),
	HCI_OPCODE_HCI_READ_PAGE_SCAN_TYPE = HCI_OPCODE(OGF_CONTROLLER_BASEBAND, 0x46),
	HCI_OPCODE_HCI_WRITE_PAGE_SCAN_TYPE = HCI_OPCODE(OGF_CONTROLLER_BASEBAND, 0x47),
	HCI_OPCODE_HCI_WRITE_EXTENDED_INQUIRY_RESPONSE = HCI_OPCODE(OGF_CONTROLLER_BASEBAND, 0x52),
	HCI_OPCODE_HCI_WRITE_SIMPLE_PAIRING_MODE = HCI_OPCODE(OGF_CONTROLLER_BASEBAND, 0x56),
	HCI_OPCODE_HCI_READ_LOCAL_OOB_DATA = HCI_OPCODE(OGF_CONTROLLER_BASEBAND, 0x57),
	HCI_OPCODE_HCI_WRITE_DEFAULT_ERRONEOUS_DATA_REPORTING = HCI_OPCODE(OGF_CONTROLLER_BASEBAND, 0x5B),
	HCI_OPCODE_HCI_SET_EVENT_MASK_2 = HCI_OPCODE(OGF_CONTROLLER_BASEBAND, 0x63),
	HCI_OPCODE_HCI_READ_LE_HOST_SUPPORTED = HCI_OPCODE(OGF_CONTROLLER_BASEBAND, 0x6c),
	HCI_OPCODE_HCI_WRITE_LE_HOST_SUPPORTED = HCI_OPCODE(OGF_CONTROLLER_BASEBAND, 0x6d),
	HCI_OPCODE_HCI_WRITE_SECURE_CONNECTIONS_HOST_SUPPORT = HCI_OPCODE(OGF_CONTROLLER_BASEBAND, 0x7a),
	HCI_OPCODE_HCI_READ_LOCAL_EXTENDED_OOB_DATA = HCI_OPCODE(OGF_CONTROLLER_BASEBAND, 0x7d),
	HCI_OPCODE_HCI_READ_EXTENDED_PAGE_TIMEOUT = HCI_OPCODE(OGF_CONTROLLER_BASEBAND, 0x7e),
	HCI_OPCODE_HCI_WRITE_EXTENDED_PAGE_TIMEOUT = HCI_OPCODE(OGF_CONTROLLER_BASEBAND, 0x7f),
	HCI_OPCODE_HCI_READ_EXTENDED_INQUIRY_LENGTH = HCI_OPCODE(OGF_CONTROLLER_BASEBAND, 0x80),
	HCI_OPCODE_HCI_WRITE_EXTENDED_INQUIRY_LENGTH = HCI_OPCODE(OGF_CONTROLLER_BASEBAND, 0x81),
	HCI_OPCODE_HCI_SET_ECOSYSTEM_BASE_INTERVAL = HCI_OPCODE(OGF_CONTROLLER_BASEBAND, 0x82),
	HCI_OPCODE_HCI_CONFIGURE_DATA_PATH = HCI_OPCODE(OGF_CONTROLLER_BASEBAND, 0x83),
	HCI_OPCODE_HCI_SET_MIN_ENCRYPTION_KEY_SIZE = HCI_OPCODE(OGF_CONTROLLER_BASEBAND, 0x84),	

	// Information Parameters (0x04)
	HCI_OPCODE_HCI_READ_LOCAL_VERSION_INFORMATION = HCI_OPCODE(OGF_INFORMATIONAL_PARAMETERS, 0x01),
	HCI_OPCODE_HCI_READ_LOCAL_SUPPORTED_COMMANDS = HCI_OPCODE(OGF_INFORMATIONAL_PARAMETERS, 0x02),
	HCI_OPCODE_HCI_READ_LOCAL_SUPPORTED_FEATURES = HCI_OPCODE(OGF_INFORMATIONAL_PARAMETERS, 0x03),
	HCI_OPCODE_HCI_READ_BUFFER_SIZE = HCI_OPCODE(OGF_INFORMATIONAL_PARAMETERS, 0x05),
	HCI_OPCODE_HCI_READ_BD_ADDR = HCI_OPCODE(OGF_INFORMATIONAL_PARAMETERS, 0x09),

	// Status Parameters (0x05)
	HCI_OPCODE_HCI_READ_RSSI = HCI_OPCODE(OGF_STATUS_PARAMETERS, 0x05),
	HCI_OPCODE_HCI_READ_ENCRYPTION_KEY_SIZE = HCI_OPCODE(OGF_STATUS_PARAMETERS, 0x08),

	// Testing (0x06)
	HCI_OPCODE_HCI_READ_LOOPBACK_MODE = HCI_OPCODE(OGF_TESTING, 0x01),
	HCI_OPCODE_HCI_WRITE_LOOPBACK_MODE = HCI_OPCODE(OGF_TESTING, 0x02),
	HCI_OPCODE_HCI_ENABLE_DEVICE_UNDER_TEST_MODE = HCI_OPCODE(OGF_TESTING, 0x03),
	HCI_OPCODE_HCI_WRITE_SIMPLE_PAIRING_DEBUG_MODE = HCI_OPCODE(OGF_TESTING, 0x04),
	HCI_OPCODE_HCI_WRITE_SECURE_CONNECTIONS_TEST_MODE = HCI_OPCODE(OGF_TESTING, 0x0a),

	// LE Controller (0x08)
	HCI_OPCODE_HCI_LE_SET_EVENT_MASK = HCI_OPCODE(OGF_LE_CONTROLLER, 0x01),
	HCI_OPCODE_HCI_LE_READ_BUFFER_SIZE = HCI_OPCODE(OGF_LE_CONTROLLER, 0x02),
	HCI_OPCODE_HCI_LE_READ_LOCAL_SUPPORTED_FEATURES = HCI_OPCODE(OGF_LE_CONTROLLER, 0x03),
	HCI_OPCODE_HCI_LE_SET_RANDOM_ADDRESS = HCI_OPCODE(OGF_LE_CONTROLLER, 0x05),
	HCI_OPCODE_HCI_LE_SET_ADVERTISING_PARAMETERS = HCI_OPCODE(OGF_LE_CONTROLLER, 0x06),
	HCI_OPCODE_HCI_LE_READ_ADVERTISING_CHANNEL_TX_POWER = HCI_OPCODE(OGF_LE_CONTROLLER, 0x07),
	HCI_OPCODE_HCI_LE_SET_ADVERTISING_DATA = HCI_OPCODE(OGF_LE_CONTROLLER, 0x08),
	HCI_OPCODE_HCI_LE_SET_SCAN_RESPONSE_DATA = HCI_OPCODE(OGF_LE_CONTROLLER, 0x09),
	HCI_OPCODE_HCI_LE_SET_ADVERTISE_ENABLE = HCI_OPCODE(OGF_LE_CONTROLLER, 0x0a),
	HCI_OPCODE_HCI_LE_SET_SCAN_PARAMETERS = HCI_OPCODE(OGF_LE_CONTROLLER, 0x0b),
	HCI_OPCODE_HCI_LE_SET_SCAN_ENABLE = HCI_OPCODE(OGF_LE_CONTROLLER, 0x0c),
	HCI_OPCODE_HCI_LE_CREATE_CONNECTION = HCI_OPCODE(OGF_LE_CONTROLLER, 0x0d),
	HCI_OPCODE_HCI_LE_CREATE_CONNECTION_CANCEL = HCI_OPCODE(OGF_LE_CONTROLLER, 0x0e),
	HCI_OPCODE_HCI_LE_READ_WHITE_LIST_SIZE = HCI_OPCODE(OGF_LE_CONTROLLER, 0x0f),
	HCI_OPCODE_HCI_LE_CLEAR_WHITE_LIST = HCI_OPCODE(OGF_LE_CONTROLLER, 0x10),
	HCI_OPCODE_HCI_LE_ADD_DEVICE_TO_WHITE_LIST = HCI_OPCODE(OGF_LE_CONTROLLER, 0x11),
	HCI_OPCODE_HCI_LE_REMOVE_DEVICE_FROM_WHITE_LIST = HCI_OPCODE(OGF_LE_CONTROLLER, 0x12),
	HCI_OPCODE_HCI_LE_CONNECTION_UPDATE = HCI_OPCODE(OGF_LE_CONTROLLER, 0x13),
	HCI_OPCODE_HCI_LE_SET_HOST_CHANNEL_CLASSIFICATION = HCI_OPCODE(OGF_LE_CONTROLLER, 0x14),
	HCI_OPCODE_HCI_LE_READ_CHANNEL_MAP = HCI_OPCODE(OGF_LE_CONTROLLER, 0x15),
	HCI_OPCODE_HCI_LE_READ_REMOTE_USED_FEATURES = HCI_OPCODE(OGF_LE_CONTROLLER, 0x16),
	HCI_OPCODE_HCI_LE_ENCRYPT = HCI_OPCODE(OGF_LE_CONTROLLER, 0x17),
	HCI_OPCODE_HCI_LE_RAND = HCI_OPCODE(OGF_LE_CONTROLLER, 0x18),
	HCI_OPCODE_HCI_LE_START_ENCRYPTION = HCI_OPCODE(OGF_LE_CONTROLLER, 0x19),
	HCI_OPCODE_HCI_LE_LONG_TERM_KEY_REQUEST_REPLY = HCI_OPCODE(OGF_LE_CONTROLLER, 0x1a),
	HCI_OPCODE_HCI_LE_LONG_TERM_KEY_NEGATIVE_REPLY = HCI_OPCODE(OGF_LE_CONTROLLER, 0x1b),
	HCI_OPCODE_HCI_LE_READ_SUPPORTED_STATES = HCI_OPCODE(OGF_LE_CONTROLLER, 0x1c),
	HCI_OPCODE_HCI_LE_RECEIVER_TEST = HCI_OPCODE(OGF_LE_CONTROLLER, 0x1d),
	HCI_OPCODE_HCI_LE_TRANSMITTER_TEST = HCI_OPCODE(OGF_LE_CONTROLLER, 0x1e),
	HCI_OPCODE_HCI_LE_TEST_END = HCI_OPCODE(OGF_LE_CONTROLLER, 0x1f),
	HCI_OPCODE_HCI_LE_REMOTE_CONNECTION_PARAMETER_REQUEST_REPLY = HCI_OPCODE(OGF_LE_CONTROLLER, 0x20),
	HCI_OPCODE_HCI_LE_REMOTE_CONNECTION_PARAMETER_REQUEST_NEGATIVE_REPLY = HCI_OPCODE(OGF_LE_CONTROLLER, 0x21),
	HCI_OPCODE_HCI_LE_SET_DATA_LENGTH = HCI_OPCODE(OGF_LE_CONTROLLER, 0x22),
	HCI_OPCODE_HCI_LE_READ_SUGGESTED_DEFAULT_DATA_LENGTH = HCI_OPCODE(OGF_LE_CONTROLLER, 0x23),
	HCI_OPCODE_HCI_LE_WRITE_SUGGESTED_DEFAULT_DATA_LENGTH = HCI_OPCODE(OGF_LE_CONTROLLER, 0x24),
	HCI_OPCODE_HCI_LE_READ_LOCAL_P256_PUBLIC_KEY = HCI_OPCODE(OGF_LE_CONTROLLER, 0x25),
	HCI_OPCODE_HCI_LE_GENERATE_DHKEY = HCI_OPCODE(OGF_LE_CONTROLLER, 0x26),
	HCI_OPCODE_HCI_LE_ADD_DEVICE_TO_RESOLVING_LIST = HCI_OPCODE(OGF_LE_CONTROLLER, 0x27),
	HCI_OPCODE_HCI_LE_REMOVE_DEVICE_FROM_RESOLVING_LIST = HCI_OPCODE(OGF_LE_CONTROLLER, 0x28),
	HCI_OPCODE_HCI_LE_CLEAR_RESOLVING_LIST = HCI_OPCODE(OGF_LE_CONTROLLER, 0x29),
	HCI_OPCODE_HCI_LE_READ_RESOLVING_LIST_SIZE = HCI_OPCODE(OGF_LE_CONTROLLER, 0x2A),
	HCI_OPCODE_HCI_LE_READ_PEER_RESOLVABLE_ADDRESS = HCI_OPCODE(OGF_LE_CONTROLLER, 0x2B),
	HCI_OPCODE_HCI_LE_READ_LOCAL_RESOLVABLE_ADDRESS = HCI_OPCODE(OGF_LE_CONTROLLER, 0x2C),
	HCI_OPCODE_HCI_LE_SET_ADDRESS_RESOLUTION_ENABLED = HCI_OPCODE(OGF_LE_CONTROLLER, 0x2D),
	HCI_OPCODE_HCI_LE_SET_RESOLVABLE_PRIVATE_ADDRESS_TIMEOUT = HCI_OPCODE(OGF_LE_CONTROLLER, 0x2E),
	HCI_OPCODE_HCI_LE_READ_MAXIMUM_DATA_LENGTH = HCI_OPCODE(OGF_LE_CONTROLLER, 0x2F),
	HCI_OPCODE_HCI_LE_READ_PHY = HCI_OPCODE(OGF_LE_CONTROLLER, 0x30),
	HCI_OPCODE_HCI_LE_SET_DEFAULT_PHY = HCI_OPCODE(OGF_LE_CONTROLLER, 0x31),
	HCI_OPCODE_HCI_LE_SET_PHY = HCI_OPCODE(OGF_LE_CONTROLLER, 0x32),
	HCI_OPCODE_HCI_LE_RECEIVER_TEST_V2 = HCI_OPCODE(OGF_LE_CONTROLLER, 0x33),
	HCI_OPCODE_HCI_LE_TRANSMITTER_TEST_V2 = HCI_OPCODE(OGF_LE_CONTROLLER, 0x34),
	HCI_OPCODE_HCI_LE_SET_ADVERTISING_SET_RANDOM_ADDRESS = HCI_OPCODE(OGF_LE_CONTROLLER, 0x35),
	HCI_OPCODE_HCI_LE_SET_EXTENDED_ADVERTISING_PARAMETERS = HCI_OPCODE(OGF_LE_CONTROLLER, 0x36),
	HCI_OPCODE_HCI_LE_SET_EXTENDED_ADVERTISING_DATA = HCI_OPCODE(OGF_LE_CONTROLLER, 0x37),
	HCI_OPCODE_HCI_LE_SET_EXTENDED_SCAN_RESPONSE_DATA = HCI_OPCODE(OGF_LE_CONTROLLER, 0x38),
	HCI_OPCODE_HCI_LE_SET_EXTENDED_ADVERTISING_ENABLE = HCI_OPCODE(OGF_LE_CONTROLLER, 0x39),
	HCI_OPCODE_HCI_LE_READ_MAXIMUM_ADVERTISING_DATA_LENGTH = HCI_OPCODE(OGF_LE_CONTROLLER, 0x3a),
	HCI_OPCODE_HCI_LE_READ_NUMBER_OF_SUPPORTED_ADVERTISING_SETS = HCI_OPCODE(OGF_LE_CONTROLLER, 0x3b),
	HCI_OPCODE_HCI_LE_REMOVE_ADVERTISING_SET = HCI_OPCODE(OGF_LE_CONTROLLER, 0x3c),
	HCI_OPCODE_HCI_LE_CLEAR_ADVERTISING_SETS = HCI_OPCODE(OGF_LE_CONTROLLER, 0x3d),
	HCI_OPCODE_HCI_LE_SET_PERIODIC_ADVERTISING_PARAMETERS = HCI_OPCODE(OGF_LE_CONTROLLER, 0x3e),
	HCI_OPCODE_HCI_LE_SET_PERIODIC_ADVERTISING_DATA = HCI_OPCODE(OGF_LE_CONTROLLER, 0x3f),
	HCI_OPCODE_HCI_LE_SET_PERIODIC_ADVERTISING_ENABLE = HCI_OPCODE(OGF_LE_CONTROLLER, 0x40),
	HCI_OPCODE_HCI_LE_SET_EXTENDED_SCAN_PARAMETERS = HCI_OPCODE(OGF_LE_CONTROLLER, 0x41),
	HCI_OPCODE_HCI_LE_SET_EXTENDED_SCAN_ENABLE = HCI_OPCODE(OGF_LE_CONTROLLER, 0x42),
	HCI_OPCODE_HCI_LE_EXTENDED_CREATE_CONNECTION = HCI_OPCODE(OGF_LE_CONTROLLER, 0x43),
	HCI_OPCODE_HCI_LE_PERIODIC_ADVERTISING_CREATE_SYNC = HCI_OPCODE(OGF_LE_CONTROLLER, 0x44),
	HCI_OPCODE_HCI_LE_PERIODIC_ADVERTISING_CREATE_SYNC_CANCEL = HCI_OPCODE(OGF_LE_CONTROLLER, 0x45),
	HCI_OPCODE_HCI_LE_PERIODIC_ADVERTISING_TERMINATE_SYNC = HCI_OPCODE(OGF_LE_CONTROLLER, 0x46),
	HCI_OPCODE_HCI_LE_ADD_DEVICE_TO_PERIODIC_ADVERTISER_LIST = HCI_OPCODE(OGF_LE_CONTROLLER, 0x47),
	HCI_OPCODE_HCI_LE_REMOVE_DEVICE_FROM_PERIODIC_ADVERTISER_LIST = HCI_OPCODE(OGF_LE_CONTROLLER, 0x48),
	HCI_OPCODE_HCI_LE_CLEAR_PERIODIC_ADVERTISER_LIST = HCI_OPCODE(OGF_LE_CONTROLLER, 0x49),
	HCI_OPCODE_HCI_LE_READ_PERIODIC_ADVERTISER_LIST_SIZE = HCI_OPCODE(OGF_LE_CONTROLLER, 0x4a),
	HCI_OPCODE_HCI_LE_READ_TRANSMIT_POWER = HCI_OPCODE(OGF_LE_CONTROLLER, 0x4b),
	HCI_OPCODE_HCI_LE_READ_RF_PATH_COMPENSATION = HCI_OPCODE(OGF_LE_CONTROLLER, 0x4c),
	HCI_OPCODE_HCI_LE_WRITE_RF_PATH_COMPENSATION = HCI_OPCODE(OGF_LE_CONTROLLER, 0x4d),
	HCI_OPCODE_HCI_LE_SET_PRIVACY_MODE = HCI_OPCODE(OGF_LE_CONTROLLER, 0x4e),
	HCI_OPCODE_HCI_LE_RECEIVER_TEST_V3 = HCI_OPCODE(OGF_LE_CONTROLLER, 0x4f),
	HCI_OPCODE_HCI_LE_TRANSMITTER_TEST_V3 = HCI_OPCODE(OGF_LE_CONTROLLER, 0x50),
	HCI_OPCODE_HCI_LE_SET_CONNECTIONLESS_CTE_TRANSMIT_PARAMETERS = HCI_OPCODE(OGF_LE_CONTROLLER, 0x51),
	HCI_OPCODE_HCI_LE_SET_CONNECTIONLESS_CTE_TRANSMIT_ENABLE = HCI_OPCODE(OGF_LE_CONTROLLER, 0x52),
	HCI_OPCODE_HCI_LE_SET_CONNECTIONLESS_IQ_SAMPLING_ENABLE = HCI_OPCODE(OGF_LE_CONTROLLER, 0x53),
	HCI_OPCODE_HCI_LE_SET_CONNECTION_CTE_RECEIVE_PARAMETERS = HCI_OPCODE(OGF_LE_CONTROLLER, 0x54),
	HCI_OPCODE_HCI_LE_SET_CONNECTION_CTE_TRANSMIT_PARAMETERS = HCI_OPCODE(OGF_LE_CONTROLLER, 0x55),
	HCI_OPCODE_HCI_LE_CONNECTION_CTE_REQUEST_ENABLE = HCI_OPCODE(OGF_LE_CONTROLLER, 0x56),
	HCI_OPCODE_HCI_LE_CONNECTION_CTE_RESPONSE_ENABLE = HCI_OPCODE(OGF_LE_CONTROLLER, 0x57),
	HCI_OPCODE_HCI_LE_READ_ANTENNA_INFORMATION = HCI_OPCODE(OGF_LE_CONTROLLER, 0x58),
	HCI_OPCODE_HCI_LE_SET_PERIODIC_ADVERTISING_RECEIVE_ENABLE = HCI_OPCODE(OGF_LE_CONTROLLER, 0x59),
	HCI_OPCODE_HCI_LE_PERIODIC_ADVERTISING_SYNC_TRANSFER = HCI_OPCODE(OGF_LE_CONTROLLER, 0x5a),
	HCI_OPCODE_HCI_LE_PERIODIC_ADVERTISING_SET_INFO_TRANSFER = HCI_OPCODE(OGF_LE_CONTROLLER, 0x5b),
	HCI_OPCODE_HCI_LE_SET_PERIODIC_ADVERTISING_SYNC_TRANSFER_PARAMETERS = HCI_OPCODE(OGF_LE_CONTROLLER, 0x5c),
	HCI_OPCODE_HCI_LE_SET_DEFAULT_PERIODIC_ADVERTISING_SYNC_TRANSFER_PARAMETERS = HCI_OPCODE(OGF_LE_CONTROLLER, 0x5d),
	HCI_OPCODE_HCI_LE_GENERATE_DHKEY_V2 = HCI_OPCODE(OGF_LE_CONTROLLER, 0x5e),
	HCI_OPCODE_HCI_LE_MODIFY_SLEEP_CLOCK_ACCURACY = HCI_OPCODE(OGF_LE_CONTROLLER, 0x5f),
	HCI_OPCODE_HCI_LE_READ_BUFFER_SIZE_V2 = HCI_OPCODE(OGF_LE_CONTROLLER, 0x60),
	HCI_OPCODE_HCI_LE_READ_ISO_TX_SYNC = HCI_OPCODE(OGF_LE_CONTROLLER, 0x61),
	HCI_OPCODE_HCI_LE_SET_CIG_PARAMETERS = HCI_OPCODE(OGF_LE_CONTROLLER, 0x62),
	HCI_OPCODE_HCI_LE_SET_CIG_PARAMETERS_TEST = HCI_OPCODE(OGF_LE_CONTROLLER, 0x63),
	HCI_OPCODE_HCI_LE_CREATE_CIS = HCI_OPCODE(OGF_LE_CONTROLLER, 0x64),
	HCI_OPCODE_HCI_LE_REMOVE_CIG = HCI_OPCODE(OGF_LE_CONTROLLER, 0x65),
	HCI_OPCODE_HCI_LE_ACCEPT_CIS_REQUEST = HCI_OPCODE(OGF_LE_CONTROLLER, 0x66),
	HCI_OPCODE_HCI_LE_REJECT_CIS_REQUEST = HCI_OPCODE(OGF_LE_CONTROLLER, 0x67),
	HCI_OPCODE_HCI_LE_CREATE_BIG = HCI_OPCODE(OGF_LE_CONTROLLER, 0x68),
	HCI_OPCODE_HCI_LE_CREATE_BIG_TEST = HCI_OPCODE(OGF_LE_CONTROLLER, 0x69),
	HCI_OPCODE_HCI_LE_TERMINATE_BIG = HCI_OPCODE(OGF_LE_CONTROLLER, 0x6a),
	HCI_OPCODE_HCI_LE_BIG_CREATE_SYNC = HCI_OPCODE(OGF_LE_CONTROLLER, 0x6b),
	HCI_OPCODE_HCI_LE_BIG_TERMINATE_SYNC = HCI_OPCODE(OGF_LE_CONTROLLER, 0x6c),
	HCI_OPCODE_HCI_LE_REQUEST_PEER_SCA = HCI_OPCODE(OGF_LE_CONTROLLER, 0x6d),
	HCI_OPCODE_HCI_LE_SETUP_ISO_DATA_PATH = HCI_OPCODE(OGF_LE_CONTROLLER, 0x6e),
	HCI_OPCODE_HCI_LE_REMOVE_ISO_DATA_PATH = HCI_OPCODE(OGF_LE_CONTROLLER, 0x6f),
	HCI_OPCODE_HCI_LE_ISO_TRANSMIT_TEST = HCI_OPCODE(OGF_LE_CONTROLLER, 0x70),
	HCI_OPCODE_HCI_LE_ISO_RECEIVE_TEST = HCI_OPCODE(OGF_LE_CONTROLLER, 0x71),
	HCI_OPCODE_HCI_LE_ISO_READ_TEST_COUNTERS = HCI_OPCODE(OGF_LE_CONTROLLER, 0x72),
	HCI_OPCODE_HCI_LE_ISO_TEST_END = HCI_OPCODE(OGF_LE_CONTROLLER, 0x73),
	HCI_OPCODE_HCI_LE_SET_HOST_FEATURE = HCI_OPCODE(OGF_LE_CONTROLLER, 0x74),
	HCI_OPCODE_HCI_LE_READ_ISO_LINK_QUALITY = HCI_OPCODE(OGF_LE_CONTROLLER, 0x75),
	HCI_OPCODE_HCI_LE_ENHANCED_READ_TRANSMIT_POWER_LEVEL = HCI_OPCODE(OGF_LE_CONTROLLER, 0x76),
	HCI_OPCODE_HCI_LE_READ_REMOTE_TRANSMIT_POWER_LEVEL = HCI_OPCODE(OGF_LE_CONTROLLER, 0x77),
	HCI_OPCODE_HCI_LE_SET_PATH_LOSS_REPORTING_PARAMETERS = HCI_OPCODE(OGF_LE_CONTROLLER, 0x78),
	HCI_OPCODE_HCI_LE_SET_PATH_LOSS_REPORTING_ENABLE = HCI_OPCODE(OGF_LE_CONTROLLER, 0x79),
	HCI_OPCODE_HCI_LE_SET_TRANSMIT_POWER_REPORTING_ENABLE = HCI_OPCODE(OGF_LE_CONTROLLER, 0x7a),
	HCI_OPCODE_HCI_LE_TRANSMITTER_TEST_V4 = HCI_OPCODE(OGF_LE_CONTROLLER, 0x7B),

	HCI_OPCODE_HCI_BCM_WRITE_SCO_PCM_INT = HCI_OPCODE(0x3f, 0x1c),
	HCI_OPCODE_HCI_BCM_SET_SLEEP_MODE = HCI_OPCODE(0x3f, 0x27),
	HCI_OPCODE_HCI_BCM_WRITE_I2SPCM_INTERFACE_PARAM = HCI_OPCODE(0x3f, 0x6d),
	HCI_OPCODE_HCI_BCM_ENABLE_WBS = HCI_OPCODE(0x3f, 0x7e),
	HCI_OPCODE_HCI_BCM_WRITE_TX_POWER_TABLE = HCI_OPCODE(0x3f, 0x1C9),
	HCI_OPCODE_HCI_BCM_SET_TX_PWR = HCI_OPCODE(0x3f, 0x1A5),
	HCI_OPCODE_HCI_TI_VS_CONFIGURE_DDIP = 0xFD55,
} hci_opcode_t;



// determines what type of event is exchanged between the host and the controller
typedef enum e_hci_event_code
{
	connect_complete = 0x03, // event that is not caused by a prior command
	connect_request = 0x04, // event that is not caused by a prior command
	command_complete = 0x0e, // is received as a response to a prior command
	command_status = 0x0f, // event that is not caused by a prior command
	number_of_completed_packets = 0x13, // event that is not caused by a prior command
	max_slots_changed = 0x1b, // event that is not caused by a prior command
	link_supervision_timeout_changed = 0x38, // event that is not caused by a prior command
} e_hci_event_code_t;


/*
https://github.com/nccgroup/BLE-Replay/blob/master/btsnoop/btsnoop/bt/l2cap.py
"""
Codes and names for L2CAP Signaling Protocol
"""
L2CAP_SCH_PDUS = {
		0x01 : "SCH Command reject",
		0x02 : "SCH Connection request",
		0x03 : "SCH Connection response",
		0x04 : "SCH Configure request",
		0x05 : "SCH Configure response",
		0x06 : "SCH Disconnection request",
		0x07 : "SCH Disconnection response",
		0x08 : "SCH Echo request",
		0x09 : "SCH Echo response",
		0x0a : "SCH Information request",
		0x0b : "SCH Information response",
		0x0c : "SCH Create Channel request",
		0x0d : "SCH Create Channel response",
		0x0e : "SCH Move Channel request",
		0x0f : "SCH Move Channel response",
		0x10 : "SCH Move Channel Confirmation",
		0x11 : "SCH Move Channel Confirmation response",
		0x12 : "LE SCH Connection_Parameter_Update_Request",
		0x13 : "LE SCH Connection_Parameter_Update_Response",
		0x14 : "LE SCH LE_Credit_Based_Connection Request",
		0x15 : "LE SCH LE_Credit_Based_Connection Response",
		0x16 : "LE SCH LE_Flow_Control_Credit",
}
*/