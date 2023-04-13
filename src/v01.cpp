#include <ios>
#include <iostream>
#include <vector>

#ifdef LOCAL
#define log if (true) std::cerr
#else
#define log if (false) std::cerr
#endif

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    std::vector<int> values(100);
    for (int i = 0; i < 100; i++) {
        std::cin >> values[i];
    }

    for (int i = 0; i < 100; i++) {
        int cell;
        std::cin >> cell;

        if (i == 99) {
            break;
        }

        std::cout << "F" << std::endl;
    }

    return 0;
}
