#pragma once

#include "base.hpp"
#include "traits.hpp"

namespace coll {
struct MapArgsTag {};

template<typename M>
struct MapArgs {
  using TagType = MapArgsTag;

  template<typename Input>
  using MapperResultType = typename traits::invocation<M, Input>::result_t;

  M mapper;
};

template<typename M>
MapArgs<M> map(M mapper) { return {mapper}; }

template<typename Parent, typename Args>
struct Map {
  using InputType = typename traits::remove_cvr_t<Parent>::OutputType;
  using OutputType = typename Args::template MapperResultType<InputType>;

  Parent parent;
  Args args;

  template<typename Child>
  struct Execution : public Child {
    Args args;

    template<typename ... X>
    Execution(const Args& args, X&& ... x):
      Child(std::forward<X>(x)...),
      args(args) {
    }

    inline void process(InputType e) {
      Child::process(args.mapper(std::forward<InputType>(e)));
    }
  };

  template<ExecutionType ET, typename Child, typename ... X>
  inline decltype(auto) wrap(X&& ... x) {
    return parent.template wrap<ET, Execution<Child>>(
      args, std::forward<X>(x) ...
    );
  }
};

template<typename Parent, typename Args,
  typename P = traits::remove_cvr_t<Parent>,
  typename A = traits::remove_cvr_t<Args>,
  std::enable_if_t<std::is_same<typename A::TagType, MapArgsTag>::value>* = nullptr,
  std::enable_if_t<traits::is_pipe_operator<P>::value>* = nullptr>
inline Map<P, A>
operator | (Parent&& parent, Args&& args) {
  return {std::forward<Parent>(parent), std::forward<Args>(args)};
}
} // namespace coll
