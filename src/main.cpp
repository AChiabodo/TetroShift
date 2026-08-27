#include "core/GameApp.hpp"
#include <iostream>
#include <exception>

int main() {
    try {
        TetroShift::GameApp app;
        app.Run();
    } catch (const std::exception& ex) {
        std::cerr << "Fatal Exception in TetroShift: " << ex.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown Fatal Exception in TetroShift" << std::endl;
        return 1;
    }

    return 0;
}
