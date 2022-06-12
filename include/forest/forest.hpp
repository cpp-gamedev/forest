#include <cstdio>
#include <iterator>
#include <string>

namespace forest {
///
/// \brief Interpolate escape sequences through text and write characters to out
///
template <std::output_iterator<char> Out>
constexpr Out format_to(Out out, std::string_view const text, std::size_t capacity = std::string_view::npos);
///
/// \brief Interpolate escape sequences through text
///
std::string format(std::string_view const text);
///
/// \brief Print text to out after interpolating escape sequences
///
void print_to(std::FILE* out, std::string_view const text);
///
/// \brief Print text to stdout after interpolating escape sequences
///
void print(std::string_view const text);

///
/// \brief Compile-time alternative for literals
///
template <std::size_t Capacity>
struct literal {
	char buffer[Capacity]{};

	consteval literal(std::string_view const text) { format_to(buffer, text, Capacity); }

	constexpr std::string_view get() const { return buffer; }
	constexpr operator std::string_view() const { return get(); }
};

namespace detail {
using u8 = unsigned char;

enum class style_type : u8 { reset, bold, italic, underline, dim, blink, invert, strike, clear, count_ };
inline constexpr std::string_view style_src_v[] = {
	"reset", "b", "i", "u", "dim", "blink", "invert", "strike", "clear",
};
static_assert(std::size(style_src_v) == static_cast<std::size_t>(style_type::count_));

template <typename T>
struct attribute_t {
	T t{};
	bool undo{};
};

using style_t = attribute_t<style_type>;
using rgb_t = attribute_t<u8>;

constexpr rgb_t make_rgb(float r, float g, float b) {
	auto const ir = static_cast<int>(r * 5.0f);
	auto const ig = static_cast<int>(g * 5.0f);
	auto const ib = static_cast<int>(b * 5.0f);
	return {static_cast<u8>(36 * ir + 6 * ig + ib)};
}

constexpr bool to_style(style_type& out, std::string_view const str) {
	for (std::size_t i = 0; i < static_cast<std::size_t>(style_type::count_); ++i) {
		if (style_src_v[i] == str) {
			out = static_cast<style_type>(i);
			return true;
		}
	}
	return false;
}

constexpr float clamp(float const a, float const lo, float const hi) {
	if (a < lo) { return lo; }
	if (a > hi) { return hi; }
	return a;
}

constexpr bool to_rgb(u8& out, std::string_view const str) {
	if (str.size() != 3) { return false; }
	auto const r = clamp(static_cast<float>(str[0] - '0') / 5.0f, 0.0f, 1.0f);
	auto const g = clamp(static_cast<float>(str[1] - '0') / 5.0f, 0.0f, 1.0f);
	auto const b = clamp(static_cast<float>(str[2] - '0') / 5.0f, 0.0f, 1.0f);
	out = make_rgb(r, g, b).t;
	return true;
}

enum class token_type : u8 { text, style, rgb };

struct token_t {
	std::string_view text{};
	style_t style{};
	rgb_t rgb{};
	token_type type{};
};

struct scanner_t {
	std::string_view text{};

	constexpr std::string_view attribute(std::size_t open) {
		auto const close = text.find('>', open);
		if (close == std::string_view::npos) {
			text = {};
			return {};
		}
		auto ret = text.substr(open + 1, close - open - 1);
		text = text.substr(close + 1);
		return ret;
	}

	constexpr bool style(style_t& out_style, std::string_view token) {
		if (token.empty()) { return false; }
		if (token.front() == '/') {
			out_style.undo = true;
			token = token.substr(1);
			if (token.empty()) { return false; }
		} else {
			out_style.undo = false;
		}
		if (!to_style(out_style.t, token)) { return false; }
		return true;
	}

	constexpr bool rgb(rgb_t& out_rgb, std::string_view token) {
		if (token.empty()) { return false; }
		if (token.front() == '/') {
			out_rgb.undo = true;
			token = token.substr(1);
			if (token != "rgb") { return false; }
			return true;
		} else {
			out_rgb.undo = false;
		}
		if (!token.starts_with("rgb=")) { return false; }
		token = token.substr(4);
		if (!to_rgb(out_rgb.t, token)) { return false; }
		return true;
	}

	constexpr bool next(token_t& out_token) {
		if (text.empty()) { return false; }
		out_token.type = token_type::text;
		auto const open = text.find('<');
		if (open == std::string_view::npos) {
			// no attributes in text
			out_token.text = text;
			text = {};
			return true;
		}

		if (open > 0) {
			// remaining text before attribute
			out_token.text = text.substr(0, open);
			text = text.substr(open);
			return true;
		}

		// attribute
		auto token = attribute(open);
		if (style(out_token.style, token)) {
			out_token.type = token_type::style;
			return true;
		}
		if (rgb(out_token.rgb, token)) {
			out_token.type = token_type::rgb;
			return true;
		}

		// skip
		return next(out_token);
	}
};

template <std::output_iterator<char> Out>
struct pen_t {
	Out out;

	std::size_t written{};

	static constexpr std::string_view attribute_start_v = "\x1b[";
	static constexpr std::string_view style_dst_v[] = {
		"0m", "1m", "3m", "4m", "2m", "5m", "7m", "9m", "2J",
	};
	static_assert(std::size(style_dst_v) == static_cast<std::size_t>(style_type::count_));

	static constexpr std::string_view style_dst(style_type style) { return style_dst_v[static_cast<std::size_t>(style)]; }

	static constexpr std::uint32_t max_place(std::uint32_t const a) {
		for (std::uint32_t place = 10000; place > 1; place /= 10) {
			if (a >= place) { return place; }
		}
		return 1;
	}

	constexpr pen_t& write_digit(std::uint32_t const digit) {
		auto const ch = static_cast<char>(digit + '0');
		++written;
		return write(std::string_view(&ch, 1));
	}

	constexpr pen_t& write(std::string_view const text) {
		for (char const ch : text) {
			*out++ = ch;
			++written;
		}
		return *this;
	}

	constexpr pen_t& write(std::uint32_t num) {
		for (std::uint32_t place = max_place(num); place > 1; place /= 10) {
			write_digit(num / place);
			num %= place;
		}
		write_digit(num);
		return *this;
	}

	constexpr pen_t& write(rgb_t rgb) {
		write(attribute_start_v);
		if (rgb.undo) { return write("39m"); }
		write("38;5;");
		write(static_cast<std::uint32_t>(rgb.t) + 16U);
		return write("m");
	}

	constexpr pen_t& write(style_t style) {
		if (style.t == style_type::clear) { write({style_type::reset}); }
		write(attribute_start_v);
		if (style.undo) {
			if (style.t == style_type::bold) { style.t = style_type::dim; }
			write("2");
		}
		return write(style_dst(style.t));
	}
};
} // namespace detail
} // namespace forest

template <std::output_iterator<char> Out>
constexpr Out forest::format_to(Out out, std::string_view const text, std::size_t capacity) {
	auto pen = detail::pen_t<Out>{out};
	auto scanner = detail::scanner_t{text};
	auto scan = detail::token_t{};
	auto vacant = [capacity, &pen] { return capacity == std::string_view::npos || pen.written < capacity; };
	while (scanner.next(scan) && vacant()) {
		switch (scan.type) {
		case detail::token_type::rgb: pen.write(scan.rgb); break;
		case detail::token_type::style: pen.write(scan.style); break;
		default:
		case detail::token_type::text: pen.write(scan.text); break;
		}
	}
	return pen.out;
}

namespace forest {
inline constexpr auto reset_v = literal<8>("<reset>");
inline constexpr auto clear_v = literal<8>("<clear>");
} // namespace forest

inline std::string forest::format(std::string_view const text) {
	auto str = std::string{};
	format_to(std::back_inserter(str), text);
	return str;
}

inline void forest::print_to(std::FILE* out, std::string_view const text) { std::fprintf(out, "%s%s\n", format(text).c_str(), reset_v.get().data()); }
inline void forest::print(std::string_view const text) { print_to(stdout, text); }
