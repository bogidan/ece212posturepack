
#ifndef INCLUDED_CORE_H
#define INCLUDED_CORE_H

#include <utility>

#define ONCE (([]()->bool{ static bool once = true; return once ? once = false, true : false; })())
#define RUN_ONCE(_Exec) do{if(ONCE){ _Exec; }}while(false)


// Types
#include <stdint.h>
typedef unsigned long ulong_t;
typedef void  nil;
typedef float  f32;
typedef double f64;
typedef uint8_t  u08, u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef  int8_t  s08, s8;
typedef  int16_t s16;
typedef  int32_t s32;
typedef  int64_t s64;

typedef          long      sl_;
typedef          long long sll;
typedef unsigned long      ul_;
typedef unsigned long long ull;


// Defines
#define lengthOf(_Arr) (sizeof(_Arr)/sizeof(*_Arr))
#define halfOf(_Arr)   (lengthOf(_Arr) >> 1)
#define wrapOf(_Arr)   (lengthOf(_Arr) - 1)

// Helpers

template<typename T> T minimum(T a, T b) { return a < b ? a : b; }
template<typename T> T maximum(T a, T b) { return a > b ? a : b; }
template<typename T> T clamp(T val, T minimum, T maximum) {
  assert( !(maximum < minimum) );
  return (val < minimum) ? minimum : (maximum < val) ? maximum : val;
}



void PrintContinue();

template<typename T>
void PrintContinue(const T& arg) {
  Serial.print(' ');
  Serial.print(arg);
}
template<typename T, typename... Args>
void PrintContinue(const T& arg, const Args&... args) {
  Serial.print(' ');
  Serial.print(arg);
  return PrintContinue(args...);
}

namespace core {
  // Template Variable Argument core::Print(...)
  template<typename T, typename... Args>
  void Print(const T& arg, const Args&... args) {
    Serial.print(arg); // No Space before first Print
    PrintContinue(args...);
  }

  // Template Variable Argument core::Println(...)
  template<typename T, typename... Args>
  void Println(const T& arg, const Args&... args) {
    Serial.print(arg); // No Space before first Print
    PrintContinue(args...);
    Serial.println();
  }
};

// register_pin keeps track of used pins and detects pin overlap.
int register_pin(int pin, const char* module);

template<typename ...Args>
nil _println(Args&& ...args) {
  core::Println(std::forward<Args>(args)...);
}

template<typename ...Args>
nil _print(Args&& ...args) {
  core::Print(std::forward<Args>(args)...);
}

#endif
