#include "tools.h"

int main(int argc, char **argv) {
    try {
        Tools::sample_main(argc, argv);
        return 0;
    }
    catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    catch (...) {
        std::cerr << "uncaught exception!" << std::endl;
        return 1;
    }
}
