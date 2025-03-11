#include "src/card.h"
#include "src/card-set.h"
#include "src/solver.h"

#include <gtest/gtest.h>

TEST(Solver, Simple) {
    CardSet test_cards;
    test_cards.AddCard(Card(Color::Hearts, 0));
    test_cards.AddCard(Card(Color::Hearts, 1));
    test_cards.AddCard(Card(Color::Hearts, 2));
    int unused_call_counter = 0;
    EXPECT_TRUE(Solver::Solve(test_cards, unused_call_counter));

    test_cards.AddCard(Card(Color::Hearts, 6));
    EXPECT_FALSE(Solver::Solve(test_cards, unused_call_counter));
}

TEST(Solver, EmptyHand) {
    CardSet test_cards;
    int unused_call_counter = 0;
    EXPECT_TRUE(Solver::Solve(test_cards, unused_call_counter));
}