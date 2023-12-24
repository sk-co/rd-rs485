#include "cmd-processor.h"
#include <cstring>
#include "error-handler.h"
#include "board-delay.h"
#include "app_config.h"
#include "crc16.h"

namespace app {

void CmdProcessor::Init(IUart *uart) {
  uart_ = uart;
  state_ = State::OFF;
}
void CmdProcessor::Start() {
  if (uart_->Config(IUart::ConfigParam{
    19200,
    UART_8_BIT,
    UART_1_STOP_BIT,
    NO_PARITY})) {
    ErrorHandler("CmdProcessor start error");
  }
  uart_->On();
  if (!uart_->StartRead()) {
    ErrorHandler("CmdProcessor start read error");
  }
  state_ = State::NO;
}
void CmdProcessor::Work() {
  if (uart_->GetError() != IUart::Error::NO) {
    // Перезапускаем.
    uart_->Off();
    Start();
  }
  switch (state_) {
    case State::NO:
    case State::BAD_CRC:
    case State::MSG_SENDED: {
      if (!uart_->GetRxDataSize()) {
        // Принятых данных нет.
        return;
      }
      uint8_t rx_byte = 0;
      uart_->ReadRxData(&rx_byte, 1);
      if (rx_byte == proto::kMarker) {
        state_ = State::READING_HDR;
        end_read_time_ms_ = GetSysMs() + kReadCmdTimeMs;
      }
    }
      break;
    case State::READING_HDR:
      if (GetSysMs() > end_read_time_ms_) {
        state_ = State::NO;
        return;
      }
      if (uart_->GetRxDataSize() < sizeof(proto::MsgHdr))
        return;
      uart_->ReadRxData(buf_rx_, sizeof(proto::MsgHdr));
      last_rx_hdr_ = reinterpret_cast<proto::MsgHdr *>(buf_rx_);
      if (last_rx_hdr_->size) {
        state_ = State::READING_DATA;
      } else {
        state_ = State::CMD_RECEIVED;
      }
      break;
    case State::READING_DATA: {
      if (GetSysMs() > end_read_time_ms_) {
        state_ = State::NO;
        return;
      }
      if (uart_->GetRxDataSize() < last_rx_hdr_->size)
        return;
      uart_->ReadRxData(buf_rx_ + sizeof(proto::MsgHdr), last_rx_hdr_->size);
      state_ = State::CMD_RECEIVED;
    }
      break;
    case State::CMD_RECEIVED: {
      bool is_our_cmd = (last_rx_hdr_->id == 0 &&
        last_rx_hdr_->cmd.cmd_type == uint8_t(proto::CmdType::INFO)) ||
        (last_rx_hdr_->id == conf::kID);
      if (!is_our_cmd) {
        state_ = State::NO;
        break;
      }
      uint16_t crc_rx = last_rx_hdr_->crc;
      last_rx_hdr_->crc = 0;
      uint16_t crc = crypt::Crc16(buf_rx_, sizeof(proto::MsgHdr) + last_rx_hdr_->size);
      last_rx_hdr_->crc = crc_rx;
      if (crc != last_rx_hdr_->crc) {
        state_ = State::BAD_CRC;
        break;
      }
      if(!cmd_handler_fn_) {
        // Обработчика нет, игнорируем запрос.
        state_ = State::NO;
        break;
      }
      // Обрабатываем данные.
      buf_tx_[0] = proto::kMarker;
      auto tx_hdr = reinterpret_cast<proto::MsgHdr *>(buf_tx_ + 1);
      tx_hdr->id = conf::kID;
      tx_hdr->cmd.cmd_type = last_rx_hdr_->cmd.cmd_type;
      tx_hdr->cmd.answer_flag = 1;
      if(cmd_handler_fn_(last_rx_hdr_, buf_rx_ + sizeof(proto::MsgHdr),
                         tx_hdr, buf_tx_ + sizeof(proto::MsgHdrWithMarker))) {
        // Возвращаем ответ.
        tx_len_ = sizeof(proto::MsgHdrWithMarker) + tx_hdr->size;
        tx_hdr->crc = 0;  // Перед вычислением надо занулить.
        tx_hdr->crc = crypt::Crc16(buf_tx_ + 1, tx_len_ - 1);
        state_ = State::SEND_MSG;
      }
      else {
        // Ошибка обработки.
        state_ = State::NO;
      }
    }
      break;
    case State::SEND_MSG:
      if (!uart_->StartWrite(buf_tx_, tx_len_))
        return;
      state_ = State::SENDING_MSG;
      break;
    case State::SENDING_MSG:
      if (uart_->IsWriting())
        return;
      state_ = State::MSG_SENDED;
      break;
    case State::OFF:return;
  }
}
void CmdProcessor::SendMsg(const proto::MsgHdr &hdr, const uint8_t *data) {
  buf_tx_[0] = proto::kMarker;
  auto tx_hdr = reinterpret_cast<proto::MsgHdr *>(buf_tx_ + 1);
  tx_hdr->id = conf::kID;
  tx_hdr->size = hdr.size;
  tx_hdr->cmd.cmd_type = hdr.cmd.cmd_type;
//  tx_hdr->cmd.answer_flag = 1;
  memcpy(buf_tx_ + sizeof(proto::MsgHdrWithMarker), data, tx_hdr->size);
  tx_len_ = sizeof(proto::MsgHdrWithMarker) + tx_hdr->size;
  tx_hdr->crc = 0;  // Перед вычислением надо занулить.
  tx_hdr->crc = crypt::Crc16(buf_tx_ + 1, tx_len_ - 1);
  state_ = State::SEND_MSG;
}
}
