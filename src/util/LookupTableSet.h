#pragma once
#include <vector>


namespace fb {
    template<typename T>
    class LookupTableSet {

    public:
        explicit LookupTableSet(size_t max_value) : lookup_table_(max_value, false){};


        void emplace(T value) {

            if (lookup_table_[value]) {
                return;
            }
            contents.emplace_back(value);
            lookup_table_[value] = true;
        }

        auto begin() { return contents.begin(); }
        auto end() { return contents.end(); }

        auto size() { return contents.size(); }


    private:
        std::vector<bool> lookup_table_;
        std::vector<T> contents{};
    };
}// namespace fb
