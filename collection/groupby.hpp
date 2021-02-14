#pragma once

#include <vector>
#include <utility>

#include "base.hpp"
#include "container_utils.hpp"
#include "groupby_adjacent.hpp"
#include "reference.hpp"

namespace coll {
template<typename KeyBy,
  typename ValueBy = Identity::type,
  typename Aggregator = ContainerBuilder<std::vector>,
  typename AggregateTo = DefaultContainerInserter::type,
  bool CacheByRef = false,
  bool Adjacenct = false>
struct GroupBy {
  constexpr static std::string_view name = "groupby";

  KeyBy keyby;
  ValueBy valby = Identity::value;
  Aggregator aggregator;
  AggregateTo aggregate_to = DefaultContainerInserter::value;

  // used by user
  template<typename AnotherAggregator, typename AnotherAggregateTo>
  inline GroupBy<KeyBy, ValueBy, AnotherAggregator, AnotherAggregateTo, CacheByRef, Adjacenct>
  aggregate(AnotherAggregator a, AnotherAggregateTo b) {
    return {std::forward<KeyBy>(keyby), std::forward<ValueBy>(valby),
            std::forward<AnotherAggregator>(a), std::forward<AnotherAggregateTo>(b)};
  }

  template<typename AnotherAggregator>
  inline GroupBy<KeyBy, ValueBy, AnotherAggregator, DefaultContainerInserter::type, CacheByRef, Adjacenct>
  aggregate(AnotherAggregator a) {
    return {std::forward<KeyBy>(keyby), std::forward<ValueBy>(valby),
            std::forward<AnotherAggregator>(a), DefaultContainerInserter::value};
  }

  inline GroupBy<KeyBy, ValueBy, Aggregator, AggregateTo, true, Adjacenct>
  cache_by_ref() {
    return {std::forward<KeyBy>(keyby), std::forward<ValueBy>(valby),
            std::forward<Aggregator>(aggregator), std::forward<AggregateTo>(aggregate_to)};
  }

  inline GroupBy<KeyBy, ValueBy, Aggregator, AggregateTo, CacheByRef, true>
  adjacent() {
    return {std::forward<KeyBy>(keyby), std::forward<ValueBy>(valby),
            std::forward<Aggregator>(aggregator), std::forward<AggregateTo>(aggregate_to)};
  }

  template<typename AnotherValueBy>
  inline GroupBy<KeyBy, AnotherValueBy, Aggregator, AggregateTo, CacheByRef, Adjacenct>
  valueby(AnotherValueBy another_valueby) {
    return {std::forward<KeyBy>(keyby), std::forward<AnotherValueBy>(another_valueby),
            std::forward<Aggregator>(aggregator), std::forward<AggregateTo>(aggregate_to)};
  }

  inline auto count() {
    return aggregate(size_t(0), [](auto& cnt, auto&&) { ++cnt; });
  }

  // used by operator
  constexpr static bool is_adjacent = Adjacenct;

  template<typename Input>
  using KeyType = traits::remove_cvr_t<typename traits::invocation<KeyBy, traits::remove_vr_t<Input>&>::result_t>;

  template<typename Input>
  using ValueType = typename traits::invocation<ValueBy, traits::remove_vr_t<Input>&>::result_t;

  template<typename Input,
    typename Val = ValueType<Input>>
  using RefOrVal = std::conditional_t<CacheByRef,
    Reference<traits::remove_vr_t<Val>>,
    traits::remove_cvr_t<Val>
  >;

  template<typename Input, typename Elem = RefOrVal<Input>>
  inline constexpr bool is_builder() const {
    return traits::is_builder<Aggregator, Elem>::value;
  }

  template<typename Input>
  using AggregatorType = decltype(
    std::declval<GroupBy<KeyBy, ValueBy, Aggregator, AggregateTo, CacheByRef, Adjacenct>&>()
      .template get_aggregator<Input>()
  );

  template<typename Input, typename Elem = RefOrVal<Input>>
  inline decltype(auto) get_aggregator() {
    if constexpr (traits::is_builder<Aggregator, Elem>::value) {
      return aggregator(Type<Elem>{});
    } else {
      return aggregator;
    }
  }
};

template<typename KeyBy>
inline GroupBy<KeyBy> groupby(KeyBy keyby) {
  return {std::forward<KeyBy>(keyby)};
}

inline auto groupby() {
  return groupby(Identity::value);
}

template<typename Parent, typename Args,
  std::enable_if_t<Args::name == "groupby" && !Args::is_adjacent>* = nullptr,
  std::enable_if_t<traits::is_pipe_operator<Parent>::value>* = nullptr>
inline auto operator | (Parent&& parent, Args&& args) {
  using InputType = typename traits::remove_cvr_t<Parent>::OutputType;
  using KeyType = typename Args::template KeyType<InputType>;
  using AggregatorType = typename Args::template AggregatorType<InputType>;
  if constexpr (args.template is_builder<InputType>()) {
    return parent | aggregate(std::unordered_map<KeyType, AggregatorType>(),
                      [&](auto& map, InputType e) {
                        auto&& key = args.keyby(e);
                        auto iter = map.find(key);
                        if (iter == map.end()) {
                          iter = map.emplace(key, args.template get_aggregator<InputType>()).first;
                        }
                        args.aggregate_to(iter->second, args.valby(e));
                      });
  } else {
    return parent | aggregate(std::unordered_map<KeyType, AggregatorType>(),
                      [&](auto& map, InputType e) {
                        args.aggregate_to(map[args.keyby(e)], args.valby(e));
                      });
  }
}

// Override for adjacenct groupby
template<typename Parent, typename Args,
  std::enable_if_t<Args::name == "groupby" && Args::is_adjacent>* = nullptr,
  std::enable_if_t<traits::is_pipe_operator<Parent>::value>* = nullptr>
inline GroupByAdjacent<Parent, Args> 
operator | (Parent&& parent, Args&& args) {
  return {std::forward<Parent>(parent), std::forward<Args>(args)};
}
} // namespace coll
