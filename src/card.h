#pragma once

#include <array>
#include <vector>
#include <fstream>
#include <algorithm>
#include <iostream>
#include <tuple>

static constexpr std::array<std::array<std::string_view, 4>, 2> kColorNames = {
    {{{"\u2665", "\u2663", "\u2660", "\u2666"}},
     {{"\u2661", "\u2667", "\u2664", "\u2662"}}}};
static constexpr std::array<std::string_view, 14> kValueNames = {
    "A", "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K", "Joker"};

enum class Color { Hearts = 0, Spades = 1, Diamonds = 2, Clubs = 3 };

constexpr Color kAllColorsList[] = {Color::Hearts, Color::Spades,
                                    Color::Diamonds, Color::Clubs};

class Card {
   public:
    Card() : is_joker_(true), col_(Color::Hearts), val_(0) {}
    Card(Color col, int val) : is_joker_(false), col_(col), val_(val) {}
    Card(Color col, std::string_view str_val) {
        if (str_val == "Joker") {
            is_joker_ = true;
            return;
        }
        const auto it = std::find(kValueNames.begin(), kValueNames.end(), str_val);
        if (it == kValueNames.end()) {
            std::cerr << "Incorrect card string value " << str_val << "\n";
            exit(1);
        }
        col_ = col;
        val_ = std::distance(kValueNames.begin(), it);
    }

    bool is_joker() const { return is_joker_; }
    Color color() const { return col_; }
    int val() const { return val_; }

    friend std::ostream& operator<<(std::ostream& os, const Card& card) {
        if (card.is_joker()) {
            os << "🃏︎";
        } else {
            os << kColorNames[0][static_cast<int>(card.col_)]
               << kValueNames[card.val_];
        }
        return os;
    }
    friend bool operator<(const Card& l, const Card& r) {
        if (l.is_joker() && r.is_joker()) return false;
        if (l.is_joker() || r.is_joker()) return l.is_joker();
        return std::tie(l.col_, l.val_) < std::tie(r.col_, r.val_);
    }

   private:
    bool is_joker_;
    Color col_;
    int val_;
};