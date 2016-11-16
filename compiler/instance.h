#ifndef PROJECT_INSTANCE_H
#define PROJECT_INSTANCE_H


template<typename T>
struct extract_value_type {
  using value_type = T;
};

template<template<typename> class X, typename T>
struct extract_value_type<X<T>> {
  using value_type = T;
};

template<typename T, typename ... Args>
T New(Args ... args) {
  using value_type = typename extract_value_type<T>::value_type;
  return T(new value_type(args...));
};


template <typename T, typename P>
T* UseType(P* ptr) {
  static_cast<T*>(ptr);
}


#endif //PROJECT_INSTANCE_H
