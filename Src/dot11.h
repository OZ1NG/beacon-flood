#ifndef DOT11_H
#define DOT11_H
#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <list>

using namespace std;

typedef enum frame_control_field_type {
    MANAGEMENT_FRAME, // 00
    CONTROL_FRAME,    // 01
    DATA_FRAME        // 11
} FCF_TYPE;

typedef enum fcf_management_frame_subtype {
    AID_REQUEST_FRAME       = 0,
    AID_RESPONSE_FRAME      = 1,
    AID_RE_REQUEST_FRAME    = 2,
    AID_RE_RESPONSE_FRAME   = 3,
    PROBE_REQUEST_FRAME     = 4,
    PROBE_RESPONSE_FRAME    = 5,
    BECONE_FRAME            = 8, // [A]
    ATIM_FRAME              = 9,
    DISASSOCIATION_FRAME    = 10,
    AUTHENTICATION_FRAME    = 11,
    DEAUTHENTICATION_FRAME  = 12
} FCF_MGM_SUBTYPE;

typedef enum tag_number{
    SSID                     = 0,
    DSSS_PARAMETER_SET       = 2,
    DS                       = 3,
    CF                       = 4,
    TIM                      = 5,
    IBSS                     = 6,
    COUNTRY_CODE             = 7,
    POWER_CONSTRAINT         = 32,
    IBSS_DFS                 = 41,
    EXTENDED_SUPPORTED_RATES = 50,
    RSN_IE                   = 48,
    QOS_CAPABILITY           = 46,
    HT_CAPABILITIES          = 45,
    HT_OPERATION             = 61,
    EXTENDED_CAPABILITIES    = 127,
    VENDOR_SPECIFIC_ELEMENT  = 221  // 항상 태그의 마지막
}TAG_NUMBER;

typedef struct ieee80211_radiotap_header {
    uint8_t  it_version;     /* set to 0 */
    uint8_t  it_pad;
    unsigned short it_len;   /* entire length */
    uint32_t it_present0;     /* fields present */ // 여러개 있다..
    uint32_t it_present1;     /* fields present */ // 여러개 있다..
    uint32_t it_present2;     /* fields present */ // 여러개 있다..
} RADIOTAP_HEADER;

typedef struct common_dot11_field{ // 2byte
    uint8_t version; // static : 0, 2bit
    uint8_t type;    // frame type, 2bit
    uint8_t subtype; // sub type  , 4bit
    uint8_t flag;    // FCF_FLAG  , 8bit
} CDF;

typedef struct fixed_param{
    uint64_t timestamp = 1;
    uint16_t beacon_interval = 0x64;
    uint16_t capabilities_info = 0x1411;
} FIXED_PARAM;

typedef struct management_frame{
    uint16_t common_dot11 = 0x0080;
    uint16_t duration = 0;
    uint8_t  dest_addr[6] = {0xff,0xff,0xff,0xff,0xff,0xff};
    uint8_t  src_addr[6]  = {0x00,0x11,0x22,0x33,0x44,0x55};
    uint8_t  BSSID[6]     = {0x00,0x11,0x22,0x33,0x44,0x55};;
    uint16_t  number = 0x4070;       // sequence number(12bit) + fragment number(4bit)
} MGM_FRAME_STRUCT;

typedef struct essid_tag_struct {
    uint8_t tag_number = 0;
    uint8_t tag_length;
    u_char *data;
} ESSID_TAG_STRUCT;

typedef struct channel_tag_struct {
    uint8_t tag_number = 3;
    uint8_t tag_length = 1;
    uint8_t data = 1; // channel
} DS_TAG_STRUCT;

typedef struct vender_tag_struct {
    uint8_t tag_number = 221;
    uint8_t tag_length = 6;
    u_char data[6] = {0x00,0xe0,0x4c, 0x02, 0x01, 0x60};
} VENDER_TAG_STRUCT;

typedef struct management_frame_beacon_frame { // size : 0x18
    ESSID_TAG_STRUCT       ESSID;
    DS_TAG_STRUCT          channel;
    VENDER_TAG_STRUCT      vender; // VENDOR_SPECIFIC_ELEMENT // end
} MGM_BEACON_FRAME;

typedef struct beacon_packet_data{ // template
    RADIOTAP_HEADER   rh = {0,0,0x10,0,0,0}; // 16
    MGM_FRAME_STRUCT  mfs; // 24
    FIXED_PARAM       fixed_param; // 12
} BEACON_PACKET;

class Dot11
{
public:
    unsigned long packet_size;
    // packet
    BEACON_PACKET beacon_packet; // template

    // beacon_frame
    string ESSID;
    MGM_BEACON_FRAME mbf;

    uint8_t FCS[4] = {0x11,0x12,0x13,0x14};

    // final packet
    char * PACKET;

    Dot11();
    void set_random_bssid();
    void create_packet();
    void set_ESSID(string essid);
    void reset();
    void change_channel(double channel);
    void print_BSSID();

};

#endif // DOT11_H
