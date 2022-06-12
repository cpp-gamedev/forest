#include <forest/forest.hpp>

constexpr auto foo = forest::literal<64>("<rgb=500><b><i>hello</b></i></rgb> world");

int main() { forest::print(foo); }
