#include <forest/forest.hpp>
#include <iostream>

constexpr auto foo = forest::literal<64>("<rgb=500><b><i>hello</b></i></rgb> world");

int main() {
	std::cout << foo << '\n';
	auto str = std::string("<invert>forest</invert> <dim>v");
	str += forest::version_v;
	str += "</dim>\n";
	forest::print(str);
}
