#include "f-answer-reader.h"
#include <cmath>
#include <algorithm>
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
}
proto::Errors FAnswerReader::CalculateFreq(float *freq) {
  *freq = 0;
  FilterData(conf::kFilterLevel, buffer_, kBufLen_);
  FilterData(5, buffer_, kBufLen_);
  BlockResult br = {};
  for(uint32_t i=0; i < 16; ++i){
    BlockResult br_i = ProcessBlock(buffer_ + i*100, kBufLen_ - i*200);
    br.samples += br_i.last_i - br_i.first_i;
    br.zero_count += br_i.zero_count - 1;
//    LOG_TRC("s:%u,z:%u,f:%f\n", br_i.last_i - br_i.first_i, br_i.zero_count - 1,
//            (br_i.zero_count - 1)/float(br_i.last_i - br_i.first_i));
//    LOG_FLUSH();
  }
  if(br.samples == 0) {
    return proto::Errors::NO;
  }
  *freq = float(br.zero_count / double(br.samples * adc_->period_mks/1000000.0));
  return proto::Errors::NO;
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
//FAnswerReader::BlockResult FAnswerReader::ProcessBlock(const uint16_t *data, uint32_t len) {
//  // Считаем средний уровень сигнала.
//  float average = 0;
//  for(uint32_t i=0; i < len; ++i)
//    average += float(data[i]);
//  average = average / len;
//  // Считаем средний уровень максимумов.
//  float average_max = 0;
//  int max_count = 0;
//  for (uint32_t i = 2; i < len; ++i) {
//    if ((data[i - 2] < data[i - 1]) && (data[i] < data[i - 1]) && (float(data[i]) > average)) {
//      max_count++;
//      average_max += float(data[i - 1]);
//    }
//  }
//  average_max = average_max / float(max_count);
//  // Определяем порог между средним уровнем и максимумом.
//  float threshold = average + (average_max - average) / 2;
//  // Считаем кол-во переходов через средний уровень снизу вверх (т.е. нулей).
//  // Очередной нуль берем только после достижения порога.
//  BlockResult br = {};
//  br.first_i = -1;
//  br.last_i = -1;
//  br.zero_count = 0;
//  uint32_t zero_counter = 0;
//  uint32_t dbg_data[100] = {};  // Для отладки
//  bool find_zero = true;
//  for (uint32_t i = 10; i < len - 10; ++i) {
//    zero_counter++;
//    if (find_zero) {
//      if ((float(data[i - 1]) < average) && (float(data[i]) >= average)) {
//        if(br.zero_count < 100 )
//          dbg_data[br.zero_count ] = zero_counter;
//        zero_counter = 0;
//        br.zero_count++;
//        if (br.first_i < 0)
//          br.first_i = i;
//        br.last_i = i;
//        find_zero = false;
//      }
//    }
//    if (float(data[i]) >= threshold) {
//      find_zero = true;
//    }
//  }
//  dbg_data[99] = 0;
//  return br;
//}
FAnswerReader::BlockResult FAnswerReader::ProcessBlock(const uint16_t *data, uint32_t len) {
  struct StepInfo {
    uint32_t ind_start;
    float median;
    float max;
    float min;
    float max_thr;
    float min_thr;
  };
  StepInfo si[80] = {};
  uint32_t si_len = 0;
  constexpr auto kStepLen = 150;
  for(uint32_t i=0; i < (len - kStepLen); i+=kStepLen) {
    si[si_len].ind_start = i;
    for(uint32_t j=0; j < kStepLen; ++j)
      if(si[si_len].max < data[i+j])
        si[si_len].max = data[i+j];
    si[si_len].min = 0xFFFF;
    for(uint32_t j=0; j < kStepLen; ++j)
      if(si[si_len].min > data[i+j])
        si[si_len].min = data[i+j];
    si[si_len].median = si[si_len].min + (si[si_len].max - si[si_len].min)/2;
    si[si_len].max_thr = si[si_len].median + (si[si_len].max - si[si_len].median)/3;
    si[si_len].min_thr = si[si_len].median - (si[si_len].median - si[si_len].min)/3;
    si_len++;
  }
  // Считаем кол-во переходов через средний уровень снизу вверх (т.е. нулей).
  // Очередной нуль берем только после достижения порога.
  BlockResult br = {};
  br.first_i = -1;
  br.last_i = -1;
  br.zero_count = 0;
  uint32_t zero_counter = 0;
  bool find_zero = true;
  uint32_t si_ind = 0;
  for (uint32_t i = 10; i < len - 10; ++i) {
    if(i > (si_ind+1)*kStepLen)
      si_ind++;
    zero_counter++;
    if (find_zero) {
      if ((float(data[i - 1]) <= si[si_ind].median) && (float(data[i+1]) >= si[si_ind].median)) {
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
    if (float(data[i]) < si[si_ind].min_thr) {
      find_zero = true;
    }
  }
  dbg_data[99] = 0;
  return br;
}
void FAnswerReader::OutData() {
  for(unsigned short i : buffer_){
    LOG_TRC("%u,", i);
    LOG_FLUSH();
    DelayMks(100);
  }
  LOG_TRC("dbg_data:\r\n");
  LOG_FLUSH();
  for(uint32_t i: dbg_data){
    LOG_TRC("%u,", i);
    LOG_FLUSH();
    DelayMs(1);
  }
}
}