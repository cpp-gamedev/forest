/// forest
/// single-header library
/// Requirements: C++17

#include <cstdio>
#include <iterator>
#include <string>

#if __has_include(<forest/forest_version.hpp>)
#include <forest/forest_version.hpp>
#endif

namespace forest {
///
/// \brief Interpolate escape sequences through text and write characters to out
///
template <typename Out>
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
/// \brief Compute length of formatted string
///
constexpr std::size_t length(std::string_view const text);

///
/// \brief Compile-time alternative for literals
///
template <std::size_t Capacity>
struct literal {
	char buffer[Capacity + 1]{};

	constexpr literal(char const* text) : literal(std::string_view(text)) {}
	constexpr literal(std::string_view const text) { format_to(buffer, text, Capacity); }

	constexpr std::string_view get() const { return buffer; }
	constexpr operator std::string_view() const { return get(); }

	template <typename OstreamT>
	friend constexpr OstreamT& operator<<(OstreamT& out, literal const& l) {
		return out << l.get();
	}
};

namespace detail {
enum class style_t : std::uint8_t { reset, bold, italic, underline, dim, blink, invert, strike, clear, count_ };
inline constexpr std::string_view style_src_v[] = {
	"reset", "b", "i", "u", "dim", "blink", "invert", "strike", "clear",
};
static_assert(std::size(style_src_v) == static_cast<std::size_t>(style_t::count_));

enum class rgb_type : std::uint8_t { foreground, background };
struct rgb_t {
	std::uint8_t value{};
	rgb_type type{rgb_type::foreground};
};

enum class op_type : std::uint8_t { open, close };
template <typename T>
struct attr_op_t {
	T t{};
	op_type op{};
};

enum class attr_type : std::uint8_t { style, rgb };

constexpr std::uint8_t make_rgb(float r, float g, float b) {
	auto const ir = static_cast<int>(r * 5.0f);
	auto const ig = static_cast<int>(g * 5.0f);
	auto const ib = static_cast<int>(b * 5.0f);
	return static_cast<std::uint8_t>(36 * ir + 6 * ig + ib);
}

constexpr bool to_style(style_t& out, std::string_view const str) {
	for (std::size_t i = 0; i < static_cast<std::size_t>(style_t::count_); ++i) {
		if (style_src_v[i] == str) {
			out = static_cast<style_t>(i);
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

constexpr bool is_foreground(std::string_view str) { return str == "rgb"; }
constexpr bool is_background(std::string_view str) { return str == "bg" || str == "background"; }

constexpr bool to_rgb(rgb_t& out, std::string_view const str) {
	if (str.size() != 3) { return false; }
	auto const r = clamp(static_cast<float>(str[0] - '0') / 5.0f, 0.0f, 1.0f);
	auto const g = clamp(static_cast<float>(str[1] - '0') / 5.0f, 0.0f, 1.0f);
	auto const b = clamp(static_cast<float>(str[2] - '0') / 5.0f, 0.0f, 1.0f);
	out.value = make_rgb(r, g, b);
	return true;
}

struct tokenizer_t {
	std::string_view text{};

	constexpr std::pair<std::size_t, std::size_t> find_attribute() const {
		if (auto const open = text.find('<'); open != std::string_view::npos) { return {open, text.find('>', open + 1)}; }
		return {std::string_view::npos, std::string_view::npos};
	}

	constexpr bool try_attribute(std::string_view& out, std::size_t const close) {
		if (close != std::string_view::npos) {
			out = text.substr(0, close + 1);
			text = text.substr(close + 1);
			return true;
		}
		return false;
	}

	constexpr bool operator()(std::string_view& out) {
		if (text.empty()) { return false; }
		auto const [open, close] = find_attribute();

		if (open == 0 && close != std::string_view::npos) {
			if (try_attribute(out, close)) { return true; }
		}

		if (close != std::string_view::npos) {
			out = text.substr(0, open);
			text = text.substr(open);
		} else {
			out = text;
			text = {};
		}

		return true;
	}
};

struct key_value_t {
	std::string_view key{};
	std::string_view value{};

	static constexpr bool is_space(char const ch) { return ch == ' ' || ch == '\t' || ch == '\n'; }

	static constexpr std::string_view trim(std::string_view in) {
		while (!in.empty() && is_space(in.front())) { in = in.substr(1); }
		while (!in.empty() && is_space(in.back())) { in = in.substr(0, in.size() - 1); }
		return in;
	}

	static constexpr key_value_t make(std::string_view const str) {
		if (auto eq = str.find('='); eq != std::string_view::npos) { return {trim(str.substr(0, eq)), trim(str.substr(eq + 1))}; }
		return {str};
	}
};

struct attribute_t {
	style_t style{};
	rgb_t rgb{};
	attr_type type{};
	op_type op{};

	constexpr bool make_close(key_value_t const& kvp) {
		if (op != op_type::close) { return false; }
		if (is_background(kvp.key)) {
			type = attr_type::rgb;
			rgb.type = rgb_type::background;
			return true;
		}
		if (is_foreground(kvp.key)) {
			type = attr_type::rgb;
			rgb.type = rgb_type::foreground;
			return true;
		}
		return false;
	}

	constexpr bool make_key_value(key_value_t const& kvp) {
		if (op != op_type::open) { return false; }
		if (is_background(kvp.key) && to_rgb(rgb, kvp.value)) {
			type = attr_type::rgb;
			rgb.type = rgb_type::background;
			return true;
		}
		if (is_foreground(kvp.key) && to_rgb(rgb, kvp.value)) {
			type = attr_type::rgb;
			rgb.type = rgb_type::foreground;
			return true;
		}
		return false;
	}

	constexpr bool make_attribute(std::string_view const str) {
		// try style
		if (to_style(style, str)) {
			type = attr_type::style;
			return true;
		}

		// try assign
		auto const kvp = key_value_t::make(str);
		return kvp.value.empty() ? make_close(kvp) : make_key_value(kvp);
	}

	constexpr bool try_make(std::string_view str) {
		op = op_type::open;
		if (!str.empty() && str.front() == '/') {
			op = op_type::close;
			str = str.substr(1);
		}
		if (str.empty()) { return false; }
		return make_attribute(str);
	}
};

template <typename Out>
struct pen_t {
	Out out;
	std::size_t capacity{std::string_view::npos};

	std::size_t written{};

	static constexpr std::string_view attribute_start_v = "\x1b[";
	static constexpr std::string_view style_dst_v[] = {
		"0m", "1m", "3m", "4m", "2m", "5m", "7m", "9m", "2J",
	};
	static_assert(std::size(style_dst_v) == static_cast<std::size_t>(style_t::count_));

	static constexpr std::string_view style_dst(style_t style) { return style_dst_v[static_cast<std::size_t>(style)]; }

	static constexpr std::uint32_t max_place(std::uint32_t const a) {
		for (std::uint32_t place = 10000; place > 1; place /= 10) {
			if (a >= place) { return place; }
		}
		return 1;
	}

	static constexpr bool has_close(style_t const style) {
		switch (style) {
		case style_t::reset:
		case style_t::clear: return false;
		default: return true;
		}
	}

	constexpr bool full() const { return written + 1 >= capacity; }

	constexpr pen_t& write_digit(std::uint32_t const digit) {
		auto const ch = static_cast<char>(digit + '0');
		return write(std::string_view(&ch, 1));
	}

	constexpr pen_t& write(std::string_view const text) {
		for (char const ch : text) {
			if (full()) { return *this; }
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

	constexpr pen_t& write(attr_op_t<rgb_t> rgb) {
		write(attribute_start_v);
		if (rgb.t.type == rgb_type::foreground) {
			if (rgb.op == op_type::close) { return write("39m"); }
			write("38;5;");
		} else {
			if (rgb.op == op_type::close) { return write("49m"); }
			write("48;5;");
		}
		write(static_cast<std::uint32_t>(rgb.t.value) + 16U);
		return write("m");
	}

	constexpr pen_t& write(attr_op_t<style_t> style) {
		if (style.t == style_t::clear) { write({style_t::reset}); }
		write(attribute_start_v);
		if (style.op == op_type::close && has_close(style.t)) {
			if (style.t == style_t::bold) { style.t = style_t::dim; }
			write("2");
		}
		return write(style_dst(style.t));
	}
};

struct null_writer {
	using difference_type = std::ptrdiff_t;
	char discard{};
	constexpr char& operator*() { return discard; }
	constexpr null_writer& operator++() { return *this; }
	constexpr null_writer operator++(int) const { return *this; }
};

template <typename Out>
constexpr pen_t<Out>& write(pen_t<Out>& out_pen, std::string_view const text) {
	auto write_token = [&out_pen](std::string_view const token) -> pen_t<Out>& {
		if (token.front() == '<' && token.back() == '>') {
			auto attr = attribute_t{};
			if (attr.try_make(token.substr(1, token.size() - 2))) {
				switch (attr.type) {
				case attr_type::rgb: return out_pen.write({attr.rgb, attr.op});
				default: return out_pen.write({attr.style, attr.op});
				}
			}
		}
		return out_pen.write(token);
	};
	auto tokenizer = tokenizer_t{text};
	auto token = std::string_view{};
	while (!out_pen.full() && tokenizer(token)) { write_token(token); }
	return out_pen;
}
} // namespace detail
} // namespace forest

template <typename Out>
constexpr Out forest::format_to(Out out, std::string_view const text, std::size_t capacity) {
	auto pen = detail::pen_t<Out>{out, capacity};
	return detail::write(pen, text).out;
}

constexpr std::size_t forest::length(std::string_view const text) {
	auto pen = detail::pen_t<detail::null_writer>{};
	return detail::write(pen, text).written;
}

namespace forest {
inline constexpr auto reset_v = literal<8>("<reset>");
inline constexpr auto clear_v = literal<8>("<clear>");
} // namespace forest

inline std::string forest::format(std::string_view const text) {
	auto str = std::string{};
	str.reserve(length(text));
	format_to(std::back_inserter(str), text);
	return str;
}

inline void forest::print_to(std::FILE* out, std::string_view const text) { std::fprintf(out, "%s%s", format(text).c_str(), reset_v.get().data()); }
inline void forest::print(std::string_view const text) { print_to(stdout, text); }
