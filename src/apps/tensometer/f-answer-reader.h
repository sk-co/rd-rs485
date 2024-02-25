#ifndef RD_RS485_SRC_APPS_TENSOMETER_F_ANSWER_READER_H_
#define RD_RS485_SRC_APPS_TENSOMETER_F_ANSWER_READER_H_

#include "adc-def.h"
#include "board-gpio.h"
#include "protocol.h"
#include "ram_23k256.h"

namespace app {

class FAnswerReader {
 public:
  static constexpr auto kBufLen_ = 9000;
  static uint16_t buffer_[kBufLen_];

  FAnswerReader(PinNames pin_name_out_en, PinNames pin_name_in_en,
                Adc_t *adc, AdcChannelId_t channel_id, ext::Ram23k256 *ram);
  proto::Errors ReadData();
  proto::Errors CalculateFreq(float *freq);
  void OutData();

 private:
  struct BlockResult {
    int32_t first_i;
    int32_t last_i;
    uint32_t zero_count;
    uint32_t samples;
    bool is_good_value;
  };
  struct DataStat {
    uint32_t ind_start;
    float median;
    float max;
    float min;
    float max_thr;
    float min_thr;
  };
  int ReadBlockData();
  void FilterData(uint32_t level, uint16_t* data, uint32_t len);
  BlockResult ProcessBlock(const uint16_t* data, uint32_t len);
  bool ValidatePeriods(uint32_t first_ind, uint32_t len);
  DataStat GetStat(const uint16_t *data, uint32_t len);

  Gpio_t pin_out_en_ = {};
  Gpio_t pin_in_en_ = {};
  Adc_t *adc_ = nullptr;
  AdcChannelId_t channel_id_ = {};
  ext::Ram23k256 *ram_ = nullptr;

  static constexpr auto kPeriodsLen_ = 200;
  uint16_t periods[kPeriodsLen_] = {};
};

}

#endif //RD_RS485_SRC_APPS_TENSOMETER_F_ANSWER_READER_H_
