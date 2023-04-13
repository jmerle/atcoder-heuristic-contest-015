#include <array>
#include <cmath>
#include <ios>
#include <iostream>
#include <vector>

#ifdef LOCAL
#define log if (true) std::cerr
#else
#define log if (false) std::cerr
#endif

enum class Direction {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

struct Grid {
    std::array<int, 100> cells;

    Grid() : cells() {}

    Grid(const Grid &other) = default;

    int &at(int x, int y) {
        return cells[y * 10 + x];
    }

    int score(int valueSum) {
        Grid regions;
        std::vector<int> regionSizes;
        int nextRegion = 1;

        for (int y = 0; y < 10; y++) {
            for (int x = 0; x < 10; x++) {
                int value = at(x, y);
                if (value == 0) {
                    continue;
                }

                if (regions.at(x, y) == 0) {
                    if (x > 0 && at(x - 1, y) == value && regions.at(x - 1, y) != 0) {
                        regions.at(x, y) = regions.at(x - 1, y);
                    }

                    if (x < 9 && at(x + 1, y) == value && regions.at(x + 1, y) != 0) {
                        regions.at(x, y) = regions.at(x + 1, y);
                    }

                    if (y > 0 && at(x, y - 1) == value && regions.at(x, y - 1) != 0) {
                        regions.at(x, y) = regions.at(x, y - 1);
                    }

                    if (y < 9 && at(x, y + 1) == value && regions.at(x, y + 1) != 0) {
                        regions.at(x, y) = regions.at(x, y + 1);
                    }

                    if (regions.at(x, y) == 0) {
                        regions.at(x, y) = nextRegion;
                        nextRegion++;

                        regionSizes.push_back(0);
                    }
                }

                int region = regions.at(x, y);

                if (x > 0 && at(x - 1, y) == value) {
                    regions.at(x - 1, y) = region;
                }

                if (x < 9 && at(x + 1, y) == value) {
                    regions.at(x + 1, y) = region;
                }

                if (y > 0 && at(x, y - 1) == value) {
                    regions.at(x, y - 1) = region;
                }

                if (y < 9 && at(x, y + 1) == value) {
                    regions.at(x, y + 1) = region;
                }

                regionSizes[regions.at(x, y) - 1]++;
            }
        }

        int sizeSum = 0;
        for (int size : regionSizes) {
            sizeSum += size * size;
        }

        return (int) std::round(1e6 * ((double) sizeSum / (double) valueSum));
    }

    void apply(Direction direction) {
        switch (direction) {
            case Direction::FORWARD:
                for (int x = 0; x < 10; x++) {
                    int insert = 0;

                    for (int y = 0; y < 10; y++) {
                        if (at(x, y) != 0) {
                            if (y != insert) {
                                at(x, insert) = at(x, y);
                                at(x, y) = 0;
                            }

                            insert++;
                        }
                    }
                }
                break;
            case Direction::BACKWARD:
                for (int x = 0; x < 10; x++) {
                    int insert = 9;

                    for (int y = 9; y >= 0; y--) {
                        if (at(x, y) != 0) {
                            if (y != insert) {
                                at(x, insert) = at(x, y);
                                at(x, y) = 0;
                            }

                            insert--;
                        }
                    }
                }
                break;
            case Direction::LEFT:
                for (int y = 0; y < 10; y++) {
                    int insert = 0;

                    for (int x = 0; x < 10; x++) {
                        if (at(x, y) != 0) {
                            if (x != insert) {
                                at(insert, y) = at(x, y);
                                at(x, y) = 0;
                            }

                            insert++;
                        }
                    }
                }
                break;
            case Direction::RIGHT:
                for (int y = 0; y < 10; y++) {
                    int insert = 9;

                    for (int x = 9; x >= 0; x--) {
                        if (at(x, y) != 0) {
                            if (x != insert) {
                                at(insert, y) = at(x, y);
                                at(x, y) = 0;
                            }

                            insert--;
                        }
                    }
                }
                break;
        }
    }
};

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    std::vector<int> values(100);
    for (int i = 0; i < 100; i++) {
        std::cin >> values[i];
    }

    std::array<int, 3> valueCount{};
    for (int i = 0; i < 100; i++) {
        valueCount[values[i] - 1]++;
    }

    int valueSum = 0;
    for (int i = 0; i < 3; i++) {
        valueSum += valueCount[i] * valueCount[i];
    }

    Grid grid;

    for (int i = 0; i < 100; i++) {
        int cell;
        std::cin >> cell;

        if (i == 99) {
            break;
        }

        for (int j = 0; j < 100; j++) {
            if (grid.cells[j] == 0) {
                cell--;
                if (cell == 0) {
                    grid.cells[j] = values[i];
                    break;
                }
            }
        }

        Direction bestDirection = Direction::FORWARD;
        int bestScore = 0;

        for (auto direction : {Direction::FORWARD, Direction::BACKWARD, Direction::LEFT, Direction::RIGHT}) {
            Grid copy = grid;
            copy.apply(direction);

            int score = copy.score(valueSum);
            if (score > bestScore) {
                bestDirection = direction;
                bestScore = score;
            }
        }

        grid.apply(bestDirection);

        switch (bestDirection) {
            case Direction::FORWARD:
                std::cout << "F" << std::endl;
                break;
            case Direction::BACKWARD:
                std::cout << "B" << std::endl;
                break;
            case Direction::LEFT:
                std::cout << "L" << std::endl;
                break;
            case Direction::RIGHT:
                std::cout << "R" << std::endl;
                break;
        }
    }

    return 0;
}
