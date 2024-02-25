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
  for(uint32_t i=0; i < 8; ++i){
    BlockResult br_i = ProcessBlock(buffer_ + i*100, kBufLen_ - i*200);
    if(!br_i.is_good_value)
      return proto::Errors::NO;
    br.samples += br_i.last_i - br_i.first_i - 1; // Вичитаем 1 ??, почему-то так точнее.
    br.zero_count += br_i.zero_count - 1;

    LOG_TRC("s:%u, z:%u, f:%f\n", br_i.last_i - br_i.first_i, br_i.zero_count - 1,
            (br_i.zero_count - 1)/(double(br_i.last_i - br_i.first_i - 1)*adc_->period_mks/1000000.0));
    LOG_FLUSH();

  }
  if(br.samples == 0) {
    return proto::Errors::NO;
  }
  *freq = float((br.zero_count) / double(br.samples * adc_->period_mks/1000000.0));
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
  // Вычисляем статистику по началу данных.
  DataStat stat = GetStat(data, 500);
  LOG_TRC("max1: %d, min1: %d\n", int(stat.max), int(stat.min));
  const float amp1 = stat.max - stat.min;
  // Считаем кол-во переходов через средний уровень снизу вверх (т.е. нулей).
  // Очередной нуль берем только после достижения порога.
  BlockResult br = {};
  br.first_i = -1;
  br.last_i = -1;
  br.zero_count = 0;
  br.is_good_value = false;
  volatile uint32_t zero_counter = 0;
  bool find_zero = false;
  memset(periods , 0, kPeriodsLen_*sizeof(periods[0]));
  int periods_ind = 0;
  int average_period = 0;
  int average_periods_count = 0;
  constexpr int kSkipPeriods = 6;
  for (uint32_t i = 5; i < len - 5; ++i) {
    zero_counter++;
    if (find_zero) {
      if ((float(data[i - 1]) <= stat.median) && (float(data[i+1]) >= stat.median)) {
        if(float(data[i]) < stat.median) {
          if((stat.median - float(data[i])) > (float(data[i+1]) - stat.median)){
            // Значит следующее значение ближе к нулю.
            continue;
          }
        }
        // По начальным 10 значениям определяем средний период колебаний.
        if(periods_ind > 2 && average_periods_count < 10) {
          average_period += zero_counter;
          average_periods_count++;
        }
        else if(average_periods_count == 10) {
          average_period = average_period/average_periods_count;
          average_periods_count++;
        }
        // Если период сильно меньше среднего, то продолжаем считать.
        if(average_periods_count > 10 && zero_counter < average_period*0.8)
          continue;
        // Добавляем период.
        if(periods_ind < kPeriodsLen_ ) {
          periods[periods_ind++] = zero_counter;
        }
        if(zero_counter > 100)
          zero_counter = 0;
        zero_counter = 0;
        br.zero_count++;
        if (br.first_i < 0)
          br.first_i = int32_t(i);
        br.last_i = int32_t(i);
        find_zero = false;

        if(kSkipPeriods > periods_ind){
          // Пропускаем первые несколько периодов, т.к. пока устаканиться средний уровень,
          // там бывают неверные данные.
          br.first_i = -1;
          br.zero_count--;
        }

        if(periods_ind >= 2) {
          // Пересчитываем статистику относительно последнего найденного периода.
          // Таким образом компенсируем смещение среднего уровня.
          stat = GetStat(data + (i - periods[periods_ind-1]), periods[periods_ind-1]);
          continue;
        }
      }
    }
    if (float(data[i]) < stat.min_thr) {
      find_zero = true;
    }
  }
  LOG_TRC("max2: %d, min2: %d\n", int(stat.max), int(stat.min));
  LOG_TRC("amp1: %0.3f, amp2: %0.3f\n", amp1, stat.max - stat.min);
  LOG_FLUSH();
  // Первые 5 найденных пропускаем, бывает там неточные данные.
  br.is_good_value = ValidatePeriods(5, periods_ind);
  return br;
}
void FAnswerReader::OutData() {
  for(unsigned short i : buffer_){
    LOG_TRC("%u,", i);
    LOG_FLUSH();
    DelayMks(100);
  }
//  LOG_TRC("dbg_data:\r\n");
//  LOG_FLUSH();
//  for(uint32_t i: periods){
//    LOG_TRC("%u,", i);
//    LOG_FLUSH();
//    DelayMs(1);
//  }
}
// Проверяем, что найденные периоды приблизительно одинаковые (отклонение 15%),
// иначе не гармоничный сигнал.
bool FAnswerReader::ValidatePeriods(uint32_t first_ind, uint32_t len) {
  // Первый и последний период игнорируем.
  int bad_values = 0;
  constexpr auto kMaxBadValues = 10;
  constexpr auto kCompareLen = 5;
  for(uint32_t i=first_ind; i < len- kCompareLen - 1; ++i) {
    bool is_find = false;
    for(uint32_t j=1; j < kCompareLen; ++j) {
      if (periods[i] >= periods[i+j]*0.85 && periods[i] <= periods[i+j]*1.15) {
        is_find = true;
        break;
      }
    }
    if(!is_find) {
      if(bad_values++ > kMaxBadValues) {
        return false;
      }
    }
  }
  return true;
}
FAnswerReader::DataStat FAnswerReader::GetStat(const uint16_t *data, uint32_t len) {
  DataStat stat = {};
  for(uint32_t j=0; j < len; ++j)
    if(stat.max < float(data[j]))
      stat.max = data[j];
  stat.min = 0xFFFF;
  for(uint32_t j=0; j < len; ++j)
    if(stat.min > float(data[j]))
      stat.min = data[j];
  stat.median = stat.min + (stat.max - stat.min)/2;
  stat.max_thr = stat.median + (stat.max - stat.median)/3;
  stat.min_thr = stat.median - (stat.median - stat.min)/3;
  return stat;
}
}