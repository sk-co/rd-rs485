#ifndef RD_RS485_SRC_APPS_TENSOMETER_CMD_PROCESSOR_H_
#define RD_RS485_SRC_APPS_TENSOMETER_CMD_PROCESSOR_H_

#include "iuart.h"
#include "protocol.h"

namespace app {

class CmdProcessor {
 public:
  // Обработчик команды, в параметрах запрос и ответ.
  typedef bool (*CmdHandler)(const proto::MsgHdr *, const uint8_t *data,
                             proto::MsgHdr *, uint8_t *data_out);
  void Init(IUart *uart);
  void SetCmdHandler(CmdHandler cmd_handler_fn) { cmd_handler_fn_ = cmd_handler_fn; }
  [[nodiscard]] bool IsInit() { return uart_ != nullptr; }
  void Start();
  void Stop();
  void Pause() {uart_->ReadPause();}
  void Resume() {uart_->ReadResume();}
  void Work();
  bool IsCanSend() {
    return state_ == State::NO || state_ == State::BAD_CRC
      || state_ == State::MSG_SENDED;
  }
  void SendMsg(const proto::MsgHdr &hdr, const uint8_t *data);

 private:
  enum class State {
    NO,
    BAD_CRC,
    MSG_SENDED,
    READING_HDR,
    READING_DATA,
    CMD_RECEIVED,
    SEND_MSG,
    SENDING_MSG,
    OFF,
  };

  static constexpr auto kReadCmdTimeMs = 150;
  IUart *uart_ = nullptr;
  CmdHandler cmd_handler_fn_ = nullptr;
  State state_ = State::NO;
  uint32_t end_read_time_ms_ = 0;
  uint8_t buf_rx_[proto::kMaxMsgLen] = {};
  uint8_t buf_tx_[proto::kMaxMsgLen] = {};
  uint32_t tx_len_ = 0;
  proto::MsgHdr *last_rx_hdr_ = nullptr;
};

}

#endif //RD_RS485_SRC_APPS_TENSOMETER_CMD_PROCESSOR_H_
