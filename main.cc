#include "src/card.h"
#include "src/card-set.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <random>
#include <vector>
#include <unordered_set>
#include <bit>

// Up to 4 jokers; 3-/4- or 5- run;
std::vector<int> run_table[5][3][1 << 5];
// Up to 4 jokers; 3 or 4 of a kind; 2^4 ways to have binary string of length 4.
std::vector<int> kind_table[5][2][1 << 4];

// Interesting card counts are 0 and 1. Doesn't really matter if it's 1 or 2.
std::vector<int> prepare_kind_table(int j_cnt, int cards_to_pick,
                                    const std::vector<int>& card_counts) {
    // We can't use 4 jokers in triplet.
    if (j_cnt > cards_to_pick) {
        return {};
    }

    const int free_to_pick_cards = cards_to_pick - j_cnt;
    const int unused_cards = 4 - free_to_pick_cards - j_cnt;
    std::vector<int> selectors;
    for (int i = 0; i < unused_cards + j_cnt; ++i) {
        selectors.push_back(0);
    }
    for (int i = 0; i < free_to_pick_cards; ++i) {
        selectors.push_back(1);
    }

    std::vector<int> out;
    do {
        int bit_sel = 0;
        bool all_picked_available = true;
        for (int i = 0; i < 4; ++i) {
            if (selectors[i] == 1 && card_counts[i] == 0) {
                all_picked_available = false;
                break;
            }
            bit_sel <<= 1;
            if (selectors[i] == 1) bit_sel |= 1;
        }
        if (all_picked_available) out.push_back(bit_sel);
    } while (std::next_permutation(selectors.begin(), selectors.end()));
    return out;
}

std::vector<int> prepare_run_table(int j_cnt,
                                   const std::vector<int>& counters) {
    const int run_len = counters.size();
    // Unexpected run length.
    if (run_len != 3 && run_len != 4 && run_len != 5) {
        return {};
    }
    const int missing_cards = std::count_if(counters.begin(), counters.end(),
                                            [](const int x) { return x == 0; });
    // Too many missing cards. We are not able to cover them with jokers.
    if (missing_cards > j_cnt) {
        return {};
    }

    std::vector<int> selectors;
    const int free_to_pick_cards = run_len - j_cnt;
    for (int i = 0; i < j_cnt; ++i) {
        selectors.push_back(0);
    }
    for (int i = 0; i < free_to_pick_cards; ++i) {
        selectors.push_back(1);
    }

    std::vector<int> out;
    do {
        int bit_sel = 0;
        bool all_picked_available = true;
        for (int i = 0; i < run_len; ++i) {
            if (selectors[i] == 1 && counters[i] == 0) {
                all_picked_available = false;
                break;
            }
            bit_sel <<= 1;
            if (selectors[i] == 1) bit_sel |= 1;
        }
        if (all_picked_available) out.push_back(bit_sel);
    } while (std::next_permutation(selectors.begin(), selectors.end()));
    return out;
}

std::unordered_set<CardSet, CardSet::HashFunction> UNSOLVABLE_CARDSET_CACHE;
int cache_useful = 0;
bool Solve(CardSet& cs, int& call_counter) {
    ++call_counter;
    if (UNSOLVABLE_CARDSET_CACHE.count(cs) == 1) {
        ++cache_useful;
        return false;
    }
    if (cs.empty()) return true;
    for (int jokers_to_use = 0; jokers_to_use <= cs.jokers_count();
         ++jokers_to_use) {
        // Run handling.
        for (int run_len : {5, 4, 3}) {
            for (Color color : kAllColorsList) {
                // A 2 3 4 5 6 7 8 9 10   J  Q  K
                // 0 1 2 3 4 5 6 7 8  9  10 11 12
                for (int idx = 0; idx + run_len <= 14; ++idx) {
                    uint32_t entry = 0;
                    for (int i = 0; i < run_len; ++i) {
                        entry <<= 1;
                        entry +=
                            cs.Contains(Card(color, (idx + i) % 13)) ? 1 : 0;
                    }
                    const auto& options =
                        run_table[jokers_to_use][run_len - 3][entry];
                    if (options.empty()) continue;
                    cs.RemoveJokers(jokers_to_use);
                    bool solved = false;
                    int success_option;
                    for (const int encoded_option : options) {
                        for (int i = 0; i < run_len; ++i) {
                            if (encoded_option & (1 << (run_len - i - 1))) {
                                cs.RemoveCard(Card(color, (idx + i) % 13));
                            }
                        }
                        const bool success = Solve(cs, call_counter);
                        for (int i = 0; i < run_len; ++i) {
                            if (encoded_option & (1 << (run_len - i - 1))) {
                                cs.AddCard(Card(color, (idx + i) % 13));
                            }
                        }
                        if (success) {
                            success_option = encoded_option;
                            solved = true;
                            break;
                        }
                    }
                    cs.AddJokers(jokers_to_use);
                    if (solved) {
                        std::cout << "Solution step: run(" << run_len << "): ";
                        for (int i = 0; i < run_len; ++i) {
                            if (success_option & (1 << (run_len - i - 1))) {
                                const int remaining =
                                    cs.Count(Card(color, (idx + i) % 13));
                                std::cout
                                    << kColorNames[remaining - 1]
                                                  [static_cast<int>(color)]
                                    << kValueNames[(idx + i) % 13] << ",";
                            }
                        }
                        std::cout << " ... with " << jokers_to_use << " jokers";
                        std::cout << "\n";
                        return true;
                    }
                }
            }
        }
        // Kind handling.
        for (int kind_count : {4, 3}) {
            // A 2 3 4 5 6 7 8 9 10   J  Q  K
            // 0 1 2 3 4 5 6 7 8  9  10 11 12
            for (int value = 0; value < 13; ++value) {
                uint32_t entry = 0;
                for (Color color : kAllColorsList) {
                    entry <<= 1;
                    entry += cs.Contains(Card(color, value)) ? 1 : 0;
                }
                const auto& options =
                    kind_table[jokers_to_use][kind_count - 3][entry];
                if (options.empty()) continue;
                cs.RemoveJokers(jokers_to_use);
                bool solved = false;
                int success_option;
                for (int encoded_option : options) {
                    for (int i = 0; i < 4; ++i) {
                        if (encoded_option & (1 << (3 - i))) {
                            cs.RemoveCard(Card(static_cast<Color>(i), value));
                        }
                    }
                    const bool success = Solve(cs, call_counter);
                    for (int i = 0; i < 4; ++i) {
                        if (encoded_option & (1 << (3 - i))) {
                            cs.AddCard(Card(static_cast<Color>(i), value));
                        }
                    }
                    if (success) {
                        success_option = encoded_option;
                        solved = true;
                        break;
                    }
                }
                cs.AddJokers(jokers_to_use);
                if (solved) {
                    std::cout << "Solution step: kind(" << kind_count << "): ";
                    for (int i = 0; i < 4; ++i) {
                        if (success_option & (1 << (4 - i - 1))) {
                            const int remaining =
                                cs.Count(Card(static_cast<Color>(i), value));
                            std::cout << kColorNames[remaining - 1][i]
                                      << kValueNames[value] << ",";
                        }
                    }
                    std::cout << " ... with " << jokers_to_use << " jokers";
                    std::cout << "\n";
                    return true;
                }
            }  // value cycle end
        }  // kind cycle end
    }  // jokers cycle end
    UNSOLVABLE_CARDSET_CACHE.insert(cs);
    return false;
}

void PrepareTables() {
    for (int joker_count = 0; joker_count < 5; ++joker_count) {
        for (int cards_to_pick : {3, 4}) {
            for (int comb = 0; comb < (1 << 4); ++comb) {
                const std::vector<int> counts = {comb & 8, comb & 4, comb & 2,
                                                 comb & 1};
                kind_table[joker_count][cards_to_pick - 3][comb] =
                    prepare_kind_table(joker_count, cards_to_pick, counts);
            }
        }
    }

    for (int joker_count = 0; joker_count < 5; ++joker_count) {
        for (int run_len : {3, 4, 5}) {
            for (int comb = 0; comb < (1 << run_len); ++comb) {
                std::vector<int> counts;
                for (int i = 0; i < run_len; ++i) {
                    const int sel = 1 << (run_len - i - 1);
                    counts.push_back(comb & sel);
                }
                run_table[joker_count][run_len - 3][comb] =
                    prepare_run_table(joker_count, counts);
            }
        }
    }
}

int main() {
    PrepareTables();

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

    if (int call_counter = 0; Solve(test_cards, call_counter)) {
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
    Solve(rand_set, call_counter);
    std::cout << "Made " << call_counter << " calls.\n";
    std::cout << "Cache useful " << cache_useful << " times.\n";

    return 0;
}