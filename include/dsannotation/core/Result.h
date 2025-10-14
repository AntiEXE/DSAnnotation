#pragma once

#include <string>
#include <string>
#include <utility>
#include <variant>

namespace dsannotation::core {

template <typename T, typename E = std::string>
class Result {
public:
    static Result success(T value) {
        return Result(std::move(value));
    }

    static Result error(E error) {
        return Result(std::move(error));
    }

    bool hasValue() const noexcept { return std::holds_alternative<T>(data_); }
    bool hasError() const noexcept { return std::holds_alternative<E>(data_); }

    const T& value() const { return std::get<T>(data_); }
    T& value() { return std::get<T>(data_); }

    const E& error() const { return std::get<E>(data_); }
    E& error() { return std::get<E>(data_); }

private:
    explicit Result(T value) : data_(std::move(value)) {}
    explicit Result(E error) : data_(std::move(error)) {}

    std::variant<T, E> data_;
};

} // namespace dsannotation::core
