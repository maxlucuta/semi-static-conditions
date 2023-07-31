#ifndef TEMPLATES_HPP
#define TEMPLATES_HPP

template<typename... Types>
constexpr auto pack_size = [](auto... args) constexpr {
    return sizeof...(Types);
}(Types{}...);


#endif