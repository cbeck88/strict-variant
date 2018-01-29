//  (C) Copyright 2016 - 2018 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <cstring>
#include <iostream>

typedef void (*test_func_t)();

struct test_record {
  const char * name;
  test_func_t func;

  void run() const {
    std::cout << "  TEST " << name << ": ... ";
    {
      int l = 50 - static_cast<int>(std::strlen(name));
      for (int i = 0; i < l; ++i) {
        std::cout << ' ';
      }
    }
    std::cout.flush();

    func();
  }

  void report_okay() const { std::cout << " passed" << std::endl; }

  void report_fail() const { std::cout << " FAILED" << std::endl; }
};
