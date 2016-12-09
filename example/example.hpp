#pragma once
#include <iostream>

struct A {
  A() { std::cout << "A()" << std::endl; }
  A(const A &) { std::cout << "A(const A &)" << std::endl; }
  A(A&&) { std::cout << "A(A&&)" << std::endl; }

  ~A() { std::cout << "~A()" << std::endl; }

  A & operator = (const A &) {
    std::cout << "operator = (const A &)" << std::endl;
    return *this;
  }

  A & operator = (A &&) {
    std::cout << "operator = (A &&)" << std::endl;
    return *this;
  }
};

struct B {
  B() { std::cout << "B()" << std::endl; }
  B(const B &) { std::cout << "B(const B &)" << std::endl; }
  B(B&&) { std::cout << "B(B&&)" << std::endl; }

  ~B() { std::cout << "~B()" << std::endl; }

  B & operator = (const B &) {
    std::cout << "operator = (const B &)" << std::endl;
    return *this;
  }

  B & operator = (B &&) {
    std::cout << "operator = (B &&)" << std::endl;
    return *this;
  }
};
