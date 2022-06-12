#include <forest/forest.hpp>
#include <iostream>

constexpr auto foo = forest::literal<64>("<rgb=500><b><i>hello</b></i></rgb> world");

int main() {
	std::cout << foo << '\n';
	forest::print("<invert>forest</invert>\n");
}
