#ifndef VMPL_DEFS_H
#define VMPL_DEFS_H

#include <cstdint>

#define SUBTYPE 0x00
#define VERSION 0x02
#define PACKET_TYPE 0xcc
#define SUB_ID 0x00
#define PADDING 0x00

// Сетевые константы
#define DEFAULT_BUFLEN 1024
#define DEFAULT_RTP_BUFLEN 4108
#define RTP_HEADER_LEN 12
#define AMOUNT_OF_COMPLEX_NUMBERS 512


#pragma pack (push, 1)

struct rtcp_header
{
    uint8_t subtype : 5;
    uint8_t padding : 1;
    uint8_t version : 2;

    uint8_t packet_type;
    uint16_t data_length;
    uint32_t sub_id;
    uint8_t rtcp_req_ans[4];
    uint32_t cmd_id;

    rtcp_header() : subtype(SUBTYPE), padding(PADDING), version(VERSION), packet_type(PACKET_TYPE), sub_id(SUB_ID), rtcp_req_ans{'R','P','R','M'}  {

    }
};

struct rtp_header
{
    uint8_t CC : 4;
    uint8_t X  : 1;
    uint8_t P  : 1;
    uint8_t V  : 2;

    uint8_t PT : 7;
    uint8_t M  : 1;

    uint16_t SN;
    uint32_t TimeStamp;
    uint32_t SSRC;
};

struct set_frequency_req {
    rtcp_header header;
    uint32_t frequency_Hz;
};

struct IQ_req {
    rtcp_header header;
    uint32_t state;
};


enum cmd_id_t
{
    GetDiag                 = 0x01, AnsDiag                 = 0x81,
    GetCurrentState         = 0x02, AnsCurrentState         = 0x82,
    SetFilterMode           = 0x03, AckFilterMode           = 0x83,
    SetFrequency            = 0x04, AckFrequency            = 0x84,
    GetStat                 = 0x05, AnsStat                 = 0x85,
    ResetStat               = 0x06, AckResentStat           = 0x86,
    ResetRtpSeqNum          = 0x07, AckResetRtpSeqNum       = 0x87,
    GetAddr                 = 0x09, AnsAddr                 = 0x89,
    SetAddr                 = 0x0a, AckAddr                 = 0x8a,
    SetAtt20dB              = 0x0c, AckAtt20dB              = 0x8c,
    SetAtt45dB              = 0x0d, AckAtt45dB              = 0x8d,
    SetCustomFilter         = 0x0e, AckCustomFilter         = 0x8e,
    SetAgcDelay             = 0x0f, AckAgcDelay             = 0x8f,
    SetAnt                  = 0x10, AckAnt                  = 0x90,
    SetGen                  = 0x11, AckGen                  = 0x91,
    AddMemChannel           = 0x12, AckAddMemChannel        = 0x92,
    SetMemChannel           = 0x13, AckMemChannel           = 0x93,
    SetRefFreqTrimmer       = 0x15, AckRefFreqTrimmer       = 0x95,
    GetSoftVers             = 0x16, AnsSoftVers             = 0x96,
    SetPhase                = 0x17, AckPhase                = 0x97,
    SetRtpCtrl              = 0x18, AckRtpCtrl              = 0x98,
    SetAdcProtect           = 0x19, AckAdcProtect           = 0x99,
    SetRtpSyncMode          = 0x1a, AckRtpSyncMode          = 0x9a,
    SetAntCtrl              = 0x1b, AckAntCtrl              = 0x9b,
    GetRefFreqTrimmer       = 0x20, AnsRefFreqTrimmer       = 0xa0,
    Error                   = 0xff
};


#pragma pack (pop)

#endif // VMPL_DEFS_H






