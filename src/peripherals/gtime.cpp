#include "gtime.h"
#include "systime.h"

namespace {

//uint32_t time_ = 0;
//uint16_t millisec_ = 0;

}

namespace app::time {

uint32_t GetUTCTime(uint16_t *millisec) {
  SysTime_t sys_time = SysTimeGet();
  if(millisec) {
    *millisec = sys_time.SubSeconds;
  }
  return sys_time.Seconds;
}
// Устанавливаем время в формате UTC.
int SetUTCTime(uint32_t sec, uint16_t millisec) {
  SysTimeSet(SysTime_t{
    .Seconds = sec, .SubSeconds = int16_t(millisec)
  });
  return 0;
}
bool IsCorrectTime() {
  SysTime_t sys_time = SysTimeGet();
  constexpr uint32_t kDate_01_01_2020 = 1577836800u;
  return (sys_time.Seconds >= kDate_01_01_2020);
}
uint64_t GetUTCTime() {
  SysTime_t sys_time = SysTimeGet();
  return uint64_t(sys_time.Seconds)*1000 + sys_time.SubSeconds;
}
int SetUTCTime(uint64_t utc_ms) {
  SysTime_t sys_time{};
  sys_time.Seconds = utc_ms/1000;
  sys_time.SubSeconds = utc_ms - sys_time.Seconds*1000;
  SysTimeSet(sys_time);
  return 0;
}
}
