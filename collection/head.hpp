#pragma once

#include "base.hpp"
#include "reference.hpp"

namespace coll {
template<bool Ref>
struct HeadArgs {
  constexpr static std::string_view name = "head";

  inline HeadArgs<true> ref() { return {}; }

  constexpr static bool use_ref = Ref;
};

template<typename Parent, typename Args>
struct Head {
  using InputType = typename traits::remove_cvr_t<Parent>::OutputType;
  using ResultType = std::conditional_t<Args::use_ref,
    Reference<typename traits::remove_vr_t<InputType>>,
    std::optional<typename traits::remove_cvr_t<InputType>>
  >;

  Parent parent;
  Args args;

  struct HeadProc {
    ResultType res;
    auto_val(control, default_control());

    inline void process(InputType e) {
      res = std::forward<InputType>(e);
      control.break_now = true;
    }

    inline void end() {}

    inline auto& result() { return res; }

    constexpr static ExecutionType execution_type = RunExecution;
    template<typename Exec, typename ... ArgT>
    static auto execution(ArgT&& ... args) {
      auto exec = Exec(std::forward<ArgT>(args)...);
      exec.process();
      exec.end();
      return std::move(exec.result());
    }
  };

  inline decltype(auto) head() {
    return parent.template wrap<HeadProc>();
  }
};

inline HeadArgs<false> head() { return {}; }

template<typename Parent, typename Args,
  std::enable_if_t<Args::name == "head">* = nullptr,
  std::enable_if_t<traits::is_coll_operator<Parent>::value>* = nullptr>
inline decltype(auto) operator | (Parent&& parent, Args&& args) {
  return Head<Parent, Args>{std::forward<Parent>(parent), std::forward<Args>(args)}.head();
}
} // namespace coll

