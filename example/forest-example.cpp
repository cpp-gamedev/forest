#include <forest/forest.hpp>

[[maybe_unused]] constexpr auto foo = forest::literal<64>("<rgb=500><b><i>hello</b></i></rgb> world");

int main() { forest::print(foo); }
