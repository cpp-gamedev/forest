#include <forest/forest.hpp>
#include <cstdio>
#include <functional>
#include <iostream>
#include <iterator>

constexpr auto foo = forest::literal<64>("<rgb=500><b><i>hello</b></i></rgb> world");

void with_temp_file(std::function<void(std::FILE*)> const fn) {
	constexpr auto tf_name = "temp.txt";

	auto temp_file = std::fopen(tf_name, "w+");

	if (temp_file == nullptr) { return; }

	fn(temp_file);

	std::fclose(temp_file);
	temp_file = nullptr;
	std::remove(tf_name);
}

int main() {
	std::cout << foo << '\n';

	std::cout << forest::literal<71>("I <strike>copy</strike> <b>study</b> code from <u>stackoverflow.com</u>") << '\n';

	// TERMINAL SUPPORT MAY VARY
	std::cout << forest::literal<26>("<blink>Hey listen!</blink>") << '\n';

	std::cout << forest::literal<21>("<dim>Loading...</dim>") << '\n';
	//

	std::cout << forest::literal<70>("<rgb=150>G<reset>O</reset></rgb> <rgb=150><invert>Team!</invert></rgb>") << '\n';

	std::cout << forest::format("<i><u>Formatted</u></i>") << '\n';

	auto str = std::string("<invert>forest</invert> <dim>v");
	str += forest::version_v;
	str += "</dim>\n";
	forest::print(str);

	// Uncomment to clear everything written so far
	// std::cout << forest::literal<24>("<clear>Goodbye..</clear>") << '\n';

	std::string ft{};
	forest::format_to(std::back_inserter(ft), "<b><rgb=155>Hello from the string!</rgb></b>");
	std::cout << ft << '\n';

	with_temp_file([](std::FILE* const tf) {
		forest::print_to(tf, "<rgb=505><b>Hello from the file!</b></rgb>");

		// Required when printing to FILE* that is not stdout/stderr
		// As forest::print_to leaves the file pointer at the end of the file
		// which is the end of the written text
		// Can also be a call to std::fseek(tf, 0, SEEK_SET)
		std::rewind(tf);

		constexpr int BUFFER_SIZE{256};
		char buffer[BUFFER_SIZE]{};
		while (std::fgets(buffer, BUFFER_SIZE, tf) != nullptr) { std::cout << buffer << '\n'; }
	});

	forest::print_to(stdout, "<rgb=050><b>Hello from stdout</b></rgb>\n");

	forest::print_to(stderr, "<rgb=500><b>Hello from stderr!</b></rgb>\n");
}
