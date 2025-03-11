#pragma once

#include "card.h"

#include <array>
#include <bit>

struct CardSet {
    CardSet() : jokers_cnt_(0), per_color_counters_({0, 0, 0, 0}) {}

    void AddCard(Card c) {
        if (c.is_joker()) {
            ++jokers_cnt_;
            return;
        }
        const int color_idx = static_cast<int>(c.color());
        const uint32_t sel = Contains(c) ? 0b10 : 0b01;
        per_color_counters_[color_idx] |= sel << (2 * c.val());
    }
    void RemoveCard(Card c) {
        const int color_idx = static_cast<int>(c.color());
        const uint32_t sel = (Count(c) == 2) ? 0b10 : 0b01;
        per_color_counters_[color_idx] -= sel << (2 * c.val());
    }
    void AddJokers(int count) { jokers_cnt_ += count; }
    void RemoveJokers(int count) { jokers_cnt_ -= count; }

    bool Contains(Card c) const {
        const int color_idx = static_cast<int>(c.color());
        const uint32_t sel = 0b11 << (2 * c.val());
        return per_color_counters_[color_idx] & sel;
    }
    int Count(Card c) const {
        const int color_idx = static_cast<int>(c.color());
        const uint32_t sel = 0b11 << (2 * c.val());
        return __builtin_popcount(per_color_counters_[color_idx] & sel);
    }
    int size() const {
        int size = jokers_cnt_;
        for (uint32_t x : per_color_counters_) {
            size += __builtin_popcount(x);
        }
        return size;
    }
    int empty() const { 
        uint32_t sum = jokers_cnt_;
        for (uint32_t x : per_color_counters_) {
            sum += x;
        }
        return sum == 0;
    }
    int jokers_count() const { return jokers_cnt_; }

    bool operator==(const CardSet& other) const
    {
        return this->jokers_cnt_ == other.jokers_cnt_ && this->per_color_counters_ == other.per_color_counters_;
    }
    struct HashFunction
    {
        size_t operator()(const CardSet& point) const
        {
            const auto IntHasher = std::hash<uint32_t>();
            size_t combined_hash = IntHasher(point.jokers_cnt_);
            int idx = 0;
            for (uint32_t x : point.per_color_counters_) {
                combined_hash ^= std::rotl(IntHasher(x), ++idx) ;
            }
            return combined_hash;
        }
    };

   private:
    int jokers_cnt_;
    std::array<uint32_t, 4> per_color_counters_;
};