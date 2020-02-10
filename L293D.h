
//           L293D
//         .-------.
// * 1,2EN |1    16| Vcc1
// *    1A |2    15| 4A    *
//      1Y |3    14| 4Y
//     GND |4    13| GND
//     GND |5    12| GND
//      2Y |6    11| 3Y
// *    2A |7    10| 3A    *
//    Vcc2 |8     9| 3,4EN *
//         .-------.
struct L293D_pinout {
  int pin_en12, pin_1A, pin_2A, pin_en34, pin_3A, pin_4A;
  enum mode {
    ON_OFF, PWM
  };
  mode mode_en12, mode_en34;
};

class L293D : L293D_pinout {
protected:
  L293D(const L293D_pinout& config)
    : L293D_pinout(config)
  {
    pinMode(pin_1A, OUTPUT);
    pinMode(pin_2A, OUTPUT);
  }
};
