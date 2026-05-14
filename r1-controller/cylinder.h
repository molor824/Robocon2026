#pragma once

#include "shifter.h"

namespace Cylinder {
  constexpr int RAISE_BIT = 0, EXTEND_BIT = 1, GRAB_BIT = 2, WLIFT_BIT = 3, WTILT_BIT = 4, WGRAB1_BIT = 5, WGRAB2_BIT = 6;

  bool raise = 0, extend = 0, grab = 0, wlift = 0, wtilt = 0, wgrab1 = 0, wgrab2 = 0;

  void sync() {
    Shifter::data &= ~(1 << RAISE_BIT | 1 << EXTEND_BIT | 1 << GRAB_BIT | 1 << WLIFT_BIT | 1 << WTILT_BIT | 1 << WGRAB1_BIT | 1 << WGRAB2_BIT);
    Shifter::data |= (raise != 0) << RAISE_BIT 
      | (extend != 0) << EXTEND_BIT 
      | (grab != 0) << GRAB_BIT
      | (wlift != 0) << WLIFT_BIT
      | (wtilt != 0) << WTILT_BIT
      | (wgrab1 != 0) << WGRAB1_BIT
      | (wgrab2 != 0) << WGRAB2_BIT;
    Shifter::sync();
  }
}
