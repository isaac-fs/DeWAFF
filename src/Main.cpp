#include "ProgramInterface.hpp"

int main(int argc, char** argv) {
    ProgramInterface interface(argc, argv);
    return interface.run();
}