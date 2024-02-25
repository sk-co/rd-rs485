#ifndef RD_RS485_SRC_APPS_TENSOMETER_PROTOCOL_H_
#define RD_RS485_SRC_APPS_TENSOMETER_PROTOCOL_H_

#include <cstdint>

namespace proto {

constexpr auto kMarker = 0xBC;
constexpr auto kMaxDataLen = 14;
constexpr auto kMaxMsgLen = 8 + 256;

enum class CmdType: uint8_t {
  NO = 0,
  INFO = 1,
  MEASUREMENT = 2,
  READ_DATA = 3,
  CLEAR_DATA = 4,
  SET_TIME = 5,
};
enum class MeasurementReason: uint8_t {
  REQUEST = 0,
  SIGNAL = 1,
};
enum class Errors: uint8_t {
  NO = 0,
  RAM_ERROR = 1,
  ADC_ERROR = 2,
  UNKNOWN,
};

#pragma pack(push, 1)
struct MsgHdr {
  uint32_t id;
  union {
    struct {
      uint8_t cmd_type: 7;
      uint8_t answer_flag: 1;
    };
    uint8_t value;
  } cmd;
  uint8_t size;
  uint16_t crc;
};
struct MsgHdrWithMarker {
  uint8_t marker;
  MsgHdr hdr;
};
struct InfoAns {
  uint32_t id;
  uint8_t channels_count;
  uint8_t storage_capacity;
  uint8_t storage_size;
  uint8_t error;
  uint64_t time_utc_ms;
};
struct MeasurementReq {
  uint8_t channel;
};
struct MeasurementAns {
  uint64_t time_utc_ms;
  uint8_t channel;
  float frequency;
  float resistance;
  uint8_t reason;
};
struct ReadDataReq {
  uint8_t first;    // начинается с 1
  uint8_t last;
};
struct ReadDataAns {
  uint8_t first;
  uint8_t last;
//  MeasurementAns data[kMaxDataLen];
};
struct SetTimeReq {
  uint64_t time_utc_ms;
};
struct SetTimeAns {
  uint64_t time_utc_ms;
};


#pragma pack(pop)

}

#endif //RD_RS485_SRC_APPS_TENSOMETER_PROTOCOL_H_
