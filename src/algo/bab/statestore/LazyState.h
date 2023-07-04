#pragma once
#include "algo/bab/states/State.h"
#include <memory>
#include <variant>

namespace fb {

    template<typename State>
        requires std::derived_from<State, CommonState>
    class LazyState {
    public:
        using complex_ptr = std::unique_ptr<State>;
        using common_ptr = std::unique_ptr<CommonState>;

        explicit LazyState(State &&state) : data(std::make_unique<State>(std::move(state))) {}
        explicit LazyState(CommonState &&common_state) : data(std::make_unique<CommonState>(std::move(common_state))) {}

        LazyState(LazyState<State> &&other) noexcept = default;
        LazyState &operator=(LazyState<State> &&other) noexcept = default;


        template<typename Initializer>
        State &getCompleteState(Initializer const &init) {
            if (isLoaded()) {
                return *std::get<complex_ptr>(data);
            }
            auto common = std::get<common_ptr>(std::move(data));
            data = std::make_unique<State>(std::move(*common));
            init.initialize(*std::get<complex_ptr>(data));
            return *std::get<complex_ptr>(data);
        }

        CommonState const &getCommonState() const {
            if (isLoaded()) {
                return *std::get<complex_ptr>(data);
            } else {
                return *std::get<common_ptr>(data);
            }
        }

        bool isLoaded() const { return std::holds_alternative<complex_ptr>(data); }

        bool free() {
            if (!isLoaded()) {
                return false;
            }
            auto state = std::get<complex_ptr>(std::move(data));
            data = std::make_unique<CommonState>(std::move(*state));
            return true;
        }

    private:
        std::variant<complex_ptr, common_ptr> data;
    };

}// namespace fb
