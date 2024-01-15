#include "app_core.h"
#include "app_objects.h"
#include "app_config.h"
#include "board-delay.h"
#include "gtime.h"
#include "board.h"
#include "board-peripherals.h"
#include "log.h"
#include "board-lpm.h"
#include "board-rtc.h"

namespace {

app::State state_ = app::State::NO;
proto::Errors error_ = proto::Errors::NO;
uint_fast8_t channel_ = 0;
proto::MeasurementAns measurement_ = {};
uint32_t end_wait_time_ms = 0;

proto::MeasurementAns storage_[conf::kStorageSize];
uint32_t storage_size_ = 0;

}


namespace app {

proto::Errors Measurement(uint32_t channel, proto::MeasurementAns *result) {
  board::InitAdc();
  volatile auto start = GetSysMs();
  // Генерируем возбуждающий сигнал.
  Obj::ImpulseGenerator(channel)->Generate(conf::kFStart, conf::kFEnd, conf::kImpCount);
  // Ждем переходные процессы.
  DelayMs(conf::kWaitAfterGenerationMs);
  // Считываем сигнал, определяем частоту.
  start = GetSysMs();
  auto error = Obj::FreqReader(channel)->ReadData();
  if(error != proto::Errors::NO) {
    board::DeInitAdc();
    return error;
  }
  volatile auto elapsed_time = GetSysMs() - start;
  elapsed_time += 1;
  Obj::FreqReader(channel)->CalculateFreq(&result->frequency);
  // Определяем температуру.
  if(Obj::TemperatureSensor(channel)->GetValue(&result->temperature)) {
    board::DeInitAdc();
    return proto::Errors::ADC_ERROR;
  }
  result->channel = channel + 1;
  result->reason = uint8_t(proto::MeasurementReason::REQUEST);
  result->time_utc_ms = app::time::GetUTCTime();
  elapsed_time = GetSysMs() - start;

//  // Вывод сырых данных от тензометра в uart.
//  Obj::FreqReader(channel)->OutData();

  board::DeInitAdc();
  return proto::Errors::NO;
}

void Init() {
  error_ = proto::Errors::NO;
  storage_size_ = 0;
  // Индикация успешного запуска платы.
  for(auto i=0; i < 3; ++i) {
    board::led1.On();
    DelayMs(100);
    board::led1.Off();
    DelayMs(200);
  }
  Obj::CommandProcessor().SetCmdHandler(CmdHandler);
  Obj::CommandProcessor().Start();

//  // Для проверки заполняем хранилище.
//  storage_size_ = 3;
//  for(uint32_t i=0; i < storage_size_; ++i) {
//    storage_[i].channel = channel_;
//    storage_[i].frequency = 812.4f + i*12;
//    storage_[i].temperature = 25 + i;
//    storage_[i].reason = uint8_t(proto::MeasurementReason::SIGNAL);
//    storage_[i].time_utc_ms = app::time::GetUTCTime() + i*1000;
//  }

  Obj::ExtSignal().StartWait();
  state_ = State::IDLE;

//  // Проверка генерации импульса.
//  while(true){
//    Obj::ImpulseGenerator(0)->Generate(conf::kFStart, conf::kFEnd, conf::kImpCount);
//  }
}
void Work() {
  Obj::CommandProcessor().Work();
  if(state_ != State::ERROR && Obj::ExtSignal().IsWait()) {
    if(Obj::ExtSignal().IsSignal()) {
      state_ = State::MEASUREMENT_BY_SIGNAL;
      Obj::ExtSignal().Stop();
    }
  }
  switch (state_) {
    case State::IDLE: {
      board::led1.On();
      end_wait_time_ms = board::GetSysTick() + 50;
      state_ = State::LED_ON;
    }
      break;
    case State::LED_ON: {
      if(board::GetSysTick() > end_wait_time_ms) {
        board::led1.Off();
        state_ = State::SLEEP;
      }
    }
      break;
    case State::SLEEP: {
      RtcStartAlarm(RtcMs2Tick(conf::kLedPeriodMs));
      board::LpmEnterSleepMode();
      state_ = State::IDLE;
    }
      break;
    case State::START_MEASUREMENT: {
      Obj::CommandProcessor().Pause();
      board::led1.On();
      error_ = Measurement(channel_, &measurement_);
      Obj::CommandProcessor().Resume();
      if(error_ != proto::Errors::NO) {
        state_ = State::ERROR;
        break;
      }
      LOG_TRC("Channel: %d, Temp: %d, Freq: %f\n", measurement_.channel,
              measurement_.temperature, measurement_.frequency);
      while(!LOG_IS_TRANSMITED());
      state_ = State::SENDING_MEASUREMENT;
      board::led1.Off();
    }
      break;
    case State::SENDING_MEASUREMENT: {
      if(Obj::CommandProcessor().IsCanSend()) {
        proto::MsgHdr hdr{};
        hdr.size = sizeof(proto::MeasurementAns);
        hdr.cmd.cmd_type = uint8_t(proto::CmdType::MEASUREMENT);
        Obj::CommandProcessor().SendMsg(hdr, reinterpret_cast<uint8_t*>(&measurement_));
        state_ = State::IDLE;
      }
    }
      break;
    case State::ERROR: {
      volatile auto now = GetSysMs();
      if(now > end_wait_time_ms) {
        board::BoardResetMcu();
      }
      if(now % 500 < 150)
        board::led1.On();
      else
        board::led1.Off();
    }
      break;
    case State::MEASUREMENT_BY_SIGNAL: {
      board::led1.Off();
      DelayMs(50);
      board::led1.On();
      for(uint32_t channel = 0; channel < conf::kNChannels; ++channel) {
        if (storage_size_ < conf::kStorageSize) {
          error_ = Measurement(channel, &storage_[storage_size_]);
          storage_[storage_size_].reason = uint8_t(proto::MeasurementReason::SIGNAL);
          if(error_ != proto::Errors::NO) {
            state_ = State::ERROR;
            return;
          }
          storage_size_++;
        }
        board::led1.Off();
        DelayMs(50);
        board::led1.On();
      }
      Obj::ExtSignal().StartWait();
      state_ = State::IDLE;
    }
      break;
    default:
      break;
  }
}
bool CmdHandler(const proto::MsgHdr *in_hdr, const uint8_t *in,
                proto::MsgHdr *out_hdr, uint8_t *out) {
  switch (in_hdr->cmd.cmd_type) {
    case uint8_t(proto::CmdType::INFO): {
      auto info = reinterpret_cast<proto::InfoAns*>(out);
      info->id = conf::kID;
      info->channels_count = conf::kNChannels;
      info->storage_capacity = conf::kStorageSize;
      info->storage_size = storage_size_;
      info->error = uint8_t(error_);
      info->time_utc_ms = app::time::GetUTCTime();
      out_hdr->size = sizeof(proto::InfoAns);
      return true;
    }
    case uint8_t(proto::CmdType::MEASUREMENT): {
      if(state_ == State::ERROR)
        return false;
      if(in_hdr->size != sizeof(proto::MeasurementReq))
        return false;
      auto req = reinterpret_cast<const proto::MeasurementReq*>(in);
      channel_ = req->channel - 1;
      if(channel_ > conf::kNChannels)
        return false;
      state_ = State::START_MEASUREMENT;
      // Ответ не передаем, выдаем после измерений.
      return false;
    }
    case uint8_t(proto::CmdType::READ_DATA): {
      if(state_ == State::ERROR)
        return false;
      if(in_hdr->size != sizeof(proto::ReadDataReq))
        return false;
      auto req = reinterpret_cast<const proto::ReadDataReq*>(in);
      auto ans = reinterpret_cast<proto::ReadDataAns*>(out);
      if(req->first > req->last || req->first == 0)
        return false;
      if(req->first > storage_size_) {
        ans->first = req->first;
        ans->last = req->last;
        out_hdr->size = sizeof(proto::ReadDataAns);
        return true;
      }
      ans->first = req->first;
      ans->last = (req->last > storage_size_)? storage_size_: req->last;
      auto count = ans->last - ans->first + 1;
      if(count > proto::kMaxDataLen) {
        count = proto::kMaxDataLen;
        ans->last = ans->last - (count - proto::kMaxDataLen);
      }
      memcpy(out + sizeof(proto::ReadDataAns), storage_, count*sizeof(proto::MeasurementAns));
      out_hdr->size = sizeof(proto::ReadDataAns) + count*sizeof(proto::MeasurementAns);
      return true;
    }
      break;
    case uint8_t(proto::CmdType::CLEAR_DATA): {
      if(state_ == State::ERROR)
        return false;
      if(in_hdr->size != 0)
        return false;
      storage_size_ = 0;
      out_hdr->size = 0;
      return true;
    }
      break;
    case uint8_t(proto::CmdType::SET_TIME): {
      if(state_ == State::ERROR)
        return false;
      if(in_hdr->size != sizeof(proto::SetTimeReq))
        return false;
      auto req = reinterpret_cast<const proto::SetTimeReq*>(in);
      app::time::SetUTCTime(req->time_utc_ms);
      auto ans = reinterpret_cast<proto::SetTimeAns*>(out);
      ans->time_utc_ms = app::time::GetUTCTime();
      out_hdr->size = sizeof(proto::SetTimeAns);
      return true;
    }
      break;
    default:
      return false;
  }
  return false;
}

}
