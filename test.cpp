#include "stylizer/core/core.hpp"

int main() {
    stylizer::auto_release context = stylizer::context::create();
    std::cout << "Hello World" << std::endl;
}