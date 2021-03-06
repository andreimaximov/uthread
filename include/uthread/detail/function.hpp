#pragma once

#include <memory>
#include <type_traits>

namespace uthread {
namespace detail {

// A substitute for std::function<void()> which (1) does not require lambda
// captures to be copyable and (2) does not allow exceptions to escape.
class FunctionBase {
 public:
  virtual ~FunctionBase() {}

  virtual void call() noexcept = 0;
};

template <typename F>
class Function : public FunctionBase {
 public:
  template <typename T>
  Function(T&& f) : f_{std::forward<T>(f)} {}

  void call() noexcept override { f_(); }

 private:
  F f_;
};

template <typename F>
using FunctionT = Function<typename std::decay<F>::type>;

template <typename F>
using FunctionReturnT = typename std::result_of<F()>::type;

template <typename F>
constexpr bool FunctionReturnsVoid =
    std::is_same<FunctionReturnT<F>, void>::value;

template <typename F>
constexpr bool FunctionReturnsRef =
    std::is_reference<FunctionReturnT<F>>::value;

// Uses a Function to type erase the underlying function f.
template <typename F>
static std::unique_ptr<FunctionBase> makef(F&& f) {
  return std::unique_ptr<FunctionBase>{new FunctionT<F>{std::forward<F>(f)}};
}

}  // namespace detail
}  // namespace uthread
