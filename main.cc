#include "src/card.h"
#include "src/card-set.h"
#include "src/solver.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <random>
#include <vector>
#include <unordered_set>
#include <bit>

int main() {
    CardSet test_cards;
    test_cards.AddCard(Card(Color::Hearts, 0));
    test_cards.AddCard(Card(Color::Hearts, 1));
    test_cards.AddCard(Card(Color::Hearts, 2));
    //
    test_cards.AddCard(Card(Color::Hearts, 2));
    test_cards.AddCard(Card(Color::Hearts, 3));
    test_cards.AddCard(Card(Color::Hearts, 4));
    //
    test_cards.AddCard(Card(Color::Hearts, 4));
    test_cards.AddCard(Card(Color::Hearts, 5));
    test_cards.AddCard(Card(Color::Hearts, 6));
    test_cards.AddCard(Card(Color::Hearts, 7));
    test_cards.AddCard(Card(Color::Hearts, 8));
    //
    test_cards.AddCard(Card(Color::Spades, 8));
    test_cards.AddCard(Card(Color::Diamonds, 8));
    test_cards.AddCard(Card(Color::Clubs, 8));
    // Joker
    test_cards.AddCard(Card());

    if (int call_counter = 0; Solver::Solve(test_cards, call_counter)) {
        std::cout << "Solution exists! " << call_counter;
    } else {
        std::cout << "Solution DOES NOT exists!";
    }

    std::vector<Card> all_cards;
    for (int i = 0; i < 4; ++i) {
        all_cards.push_back(Card());
    }
    for (Color color : kAllColorsList) {
        for (int val = 0; val < 13; ++val) {
            all_cards.push_back(Card(color, val));
            all_cards.push_back(Card(color, val));
        }
    }

    std::cout << "\n\n";
    constexpr int kRandomCardsToUse = 8 * 13 + 4 - 10;

    std::seed_seq seed {2, 1, 0, 4};
    std::mt19937 g(seed);
    std::shuffle(all_cards.begin(), all_cards.end(), g);
    std::sort(all_cards.begin(), all_cards.begin() + kRandomCardsToUse);

    std::cout << "Available cards:\n";
    CardSet rand_set;
    for (int i = 0; i < kRandomCardsToUse; ++i) {
        rand_set.AddCard(all_cards[i]);
        std::cout << all_cards[i] << " ";
    }
    std::cout << "\n";

    int call_counter = 0;
    Solver::Solve(rand_set, call_counter);
    std::cout << "Made " << call_counter << " calls.\n";
    std::cout << "Cache useful " << Solver::GetCacheUsefulnessCount() << " times.\n";

    return 0;
}