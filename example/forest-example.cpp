#include <forest/forest.hpp>
#include <cstdio>
#include <iostream>
#include <iterator>
#include <memory>

constexpr auto foo = forest::literal<64>("<rgb=500><b><i>hello</b></i></rgb> world");

template <typename F>
void with_temp_file(F const fn) {
	constexpr auto tf_name = "temp.txt";

	auto const file_deleter = [](std::FILE* const handle) -> void {
		std::fclose(handle);
		std::remove(tf_name);
	};

	auto temp_file = std::unique_ptr<std::FILE, decltype(file_deleter)>(std::fopen(tf_name, "w+"), file_deleter);

	if (temp_file == nullptr) { return; }

	fn(temp_file.get());
}

int main() {
	std::cout << foo << '\n';

	constexpr std::string_view strike_and_underline_styles = "I <strike>copy</strike> <b>study</b> code from <u>stackoverflow.com</u>";
	std::cout << forest::literal<strike_and_underline_styles.size()>(strike_and_underline_styles) << '\n';

	// TERMINAL SUPPORT MAY VARY
	constexpr std::string_view blink_style = "<blink>Hey listen!</blink>";
	std::cout << forest::literal<blink_style.size()>(blink_style) << '\n';

	constexpr std::string_view dim_style = "<dim>Loading...</dim>";
	std::cout << forest::literal<dim_style.size()>(dim_style) << '\n';
	//

	constexpr std::string_view reset_and_invert_styles = "<rgb=150>G<reset>O</reset></rgb> <rgb=150><invert>Team!</invert></rgb>";
	std::cout << forest::literal<reset_and_invert_styles.size()>(reset_and_invert_styles) << '\n';

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
