#include "f-answer-reader.h"
#include "board-adc.h"
#include "board-delay.h"
#include "app_config.h"
#include "log.h"

namespace app {

uint16_t FAnswerReader::buffer_[FAnswerReader::kBufLen_] = {};

FAnswerReader::FAnswerReader(PinNames pin_name_out_en,
                             PinNames pin_name_in_en,
                             Adc_t *adc,
                             AdcChannelId_t channel_id, ext::Ram23k256 *ram) {
  board::GpioInit(&pin_out_en_, pin_name_out_en, PIN_OUTPUT, PIN_PUSH_PULL,
                  PIN_PULL_DOWN, 0);
  board::GpioInit(&pin_in_en_, pin_name_in_en, PIN_OUTPUT, PIN_PUSH_PULL,
                  PIN_PULL_DOWN, 0);
  adc_ = adc;
  channel_id_ = channel_id;
  ram_ = ram;
}
proto::Errors FAnswerReader::ReadData() {
  board::AdcConfigChannel(adc_, channel_id_);
  board::GpioWrite(&pin_out_en_, 0);
  board::GpioWrite(&pin_in_en_, 1);
  DelayMs(1);
//  board::AdcStart(adc_);

  board::AdcStartDma(adc_, buffer_, kBufLen_);
  while(true){
    if(board::AdcWaitComplete(adc_)) {
      board::GpioWrite(&pin_in_en_, 0);
      break;
    }
  }
  board::AdcStop(adc_);
  return proto::Errors::NO;

//  // Первый блок.
//  if(ReadBlockData()) {
//    board::AdcStop(adc_);
//    board::GpioWrite(&pin_in_en_, 0);
//    return proto::Errors::ADC_ERROR;
//  }
//  if(!ram_->Write(0, reinterpret_cast<uint8_t*>(buffer_), 2*kBufLen_)) {
//    board::AdcStop(adc_);
//    board::GpioWrite(&pin_in_en_, 0);
//    return proto::Errors::RAM_ERROR;
//  }
//  // Второй блок.
//  if(ReadBlockData()) {
//    board::AdcStop(adc_);
//    board::GpioWrite(&pin_in_en_, 0);
//    return proto::Errors::ADC_ERROR;
//  }
//  if(!ram_->Write(2*kBufLen_, reinterpret_cast<uint8_t*>(buffer_), 2*kBufLen_)) {
//    board::AdcStop(adc_);
//    board::GpioWrite(&pin_in_en_, 0);
//    return proto::Errors::RAM_ERROR;
//  }
  // Третий блок.
  if(ReadBlockData()) {
    board::AdcStop(adc_);
    board::GpioWrite(&pin_in_en_, 0);
    return proto::Errors::ADC_ERROR;
  }
  board::AdcStop(adc_);
  board::GpioWrite(&pin_in_en_, 0);
  return proto::Errors::NO;
}
proto::Errors FAnswerReader::CalculateFreq(float *freq) {
  *freq = 0;
  constexpr auto kFilterLevel = 5;
//  BlockResult blocks[3] = {};
//  // Обрабатываем сначала блок в буффере.
  FilterData(kFilterLevel, buffer_, kBufLen_);
//  blocks[0] = ProcessBlock(buffer_, kBufLen_);
  BlockResult br = {};
  for(uint32_t i=0; i < 10; ++i){
    BlockResult br_i = ProcessBlock(buffer_ + i*50, kBufLen_ - i*100);
//    br.samples += br_i.last_i - br_i.first_i + 1;
//    br.zero_count += br_i.zero_count;
    br.samples += br_i.last_i - br_i.first_i;
    br.zero_count += br_i.zero_count - 1;
  }
  if(br.samples == 0) {
    return proto::Errors::NO;
  }
  *freq = float(br.zero_count / double(br.samples * adc_->period_mks/1000000.0));
  return proto::Errors::NO;
//  // Считываем и обрабатываем второй блок.
//  if(!ram_->Read(0, reinterpret_cast<uint8_t*>(buffer_), 2*kBufLen_)) {
//    return proto::Errors::RAM_ERROR;
//  }
//  FilterData(kFilterLevel, buffer_, kBufLen_);
//  blocks[1] = ProcessBlock(buffer_, kBufLen_);
//  // Считываем и обрабатываем третий блок.
//  if(!ram_->Read(2*kBufLen_, reinterpret_cast<uint8_t*>(buffer_), 2*kBufLen_)) {
//    return proto::Errors::RAM_ERROR;
//  }
//  FilterData(kFilterLevel, buffer_, kBufLen_);
//  blocks[2] = ProcessBlock(buffer_, kBufLen_);
  // Вычисляем частоту.
//  uint32_t samples = (blocks[0].last_i - blocks[0].first_i + 1)
//    + (blocks[1].last_i - blocks[1].first_i + 1)
//    + (blocks[2].last_i - blocks[2].first_i + 1);
//  if(samples == 0) {
//    return proto::Errors::NO;
//  }
//  uint32_t count = blocks[0].zero_count + blocks[1].zero_count + blocks[2].zero_count;
//  *freq = float(count / double(samples * adc_->period_mks/1000000.0));
//  return proto::Errors::NO;
}
int FAnswerReader::ReadBlockData() {
  for (unsigned short & i : buffer_) {
    i = board::AdcReadWithTimeout(adc_, 2);
    if (i == 0xFFFF) {
      return 1;
    }
  }
  return 0;
}
void FAnswerReader::FilterData(uint32_t level, uint16_t* data, uint32_t len) {
  int32_t sum;
  for (uint32_t i = 0; i < (len - level); ++i) {
    sum = 0;
    for (uint32_t j = 0; j < level; ++j)
      sum += data[i + j];
    data[i] = sum/level;
  }
}
FAnswerReader::BlockResult FAnswerReader::ProcessBlock(const uint16_t *data, uint32_t len) {
// Считаем средний уровень сигнала.
  float average = 0;
  for(uint32_t i=0; i < len; ++i)
    average += float(data[i]);
  average = average / len;
  // Считаем средний уровень максимумов.
  float average_max = 0;
  int max_count = 0;
  for (uint32_t i = 2; i < len; ++i) {
    if ((data[i - 2] < data[i - 1]) && (data[i] < data[i - 1]) && (float(data[i]) > average)) {
      max_count++;
      average_max += float(data[i - 1]);
    }
  }
  average_max = average_max / float(max_count);
  // Определяем порог между средним уровнем и максимумом.
  float threshold = average + (average_max - average) / 2;
  // Считаем кол-во переходов через средний уровень снизу вверх (т.е. нулей).
  // Очередной нуль берем только после достижения порога.
  BlockResult br = {};
  br.first_i = -1;
  br.last_i = -1;
  br.zero_count = 0;
  uint32_t zero_counter = 0;
  uint32_t dbg_data[100] = {};  // Для отладки
  bool find_zero = true;
  for (uint32_t i = 10; i < len - 10; ++i) {
    zero_counter++;
    if (find_zero) {
      if ((float(data[i - 1]) < average) && (float(data[i]) >= average)) {
        if(br.zero_count < 100 )
          dbg_data[br.zero_count ] = zero_counter;
        zero_counter = 0;
        br.zero_count++;
        if (br.first_i < 0)
          br.first_i = i;
        br.last_i = i;
        find_zero = false;
      }
    }
    if (float(data[i]) >= threshold) {
      find_zero = true;
    }
  }
  dbg_data[99] = 0;
  return br;
}
void FAnswerReader::OutData() {
  ram_->Read(0, reinterpret_cast<uint8_t*>(buffer_), 2*kBufLen_);
  FilterData(5, buffer_, kBufLen_);
  for(uint32_t i=0; i < kBufLen_; ++i){
    LOG_TRC("%u,", buffer_[i]);
    LOG_FLUSH();
    DelayMs(1);
  }
}
}