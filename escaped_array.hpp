#pragma once
// expressly better bencoder
// Copyright (C) 2014 Igor Kaplounenko
// Licensed under MIT License

#include <array>

// the Q is silent
// you may override it to any non-hexadecimal by defining it before including
// this header; note that only alpha characters are permitted, or underscore if
// you are okay with never having the first character be escaped
#ifndef EBB_ESCAPE_CHAR
#define EBB_ESCAPE_CHAR 'Q'
#endif

namespace ebb {
	constexpr size_t escaped_len(char const* cstr);

	namespace detail {
		constexpr bool is_hex(char c) {
			return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') ||
				(c >= 'a' && c <= 'f');
		}

		static_assert(!is_hex(EBB_ESCAPE_CHAR),
				"EBB_ESCAPE_CHAR must not be a hexadecimal digit");

		constexpr size_t escaped_group_len(char const* cstr) {
			return (*cstr == EBB_ESCAPE_CHAR) ? 1 + ebb::escaped_len(cstr + 1) :
				1 + ebb::escaped_len(cstr + 2);
		}

		constexpr char hex2dec(char c) {
			return (c >= '0' && c <= '9') ? c - '0' :
				(c >= 'A' && c <= 'F') ? c - 'A' + 10 :
				c - 'a' + 10;
		}

		constexpr char numify(char const* cstr) {
			return *cstr == EBB_ESCAPE_CHAR ? EBB_ESCAPE_CHAR :
				hex2dec(*cstr)*16 + hex2dec(*(cstr+1));
		}

		template<size_t size_ret, size_t size, typename... args> constexpr
		typename std::enable_if<size_ret == sizeof...(args),
						 std::array<char, size_ret> >::type
		escape(char const* cstr, args... head) {
			return {{ head... }};
		}

		template<size_t size_ret, size_t size, typename... args> constexpr
		typename std::enable_if<(size == 0 && size_ret > sizeof...(args)),
						 std::array<char, size_ret> >::type
		escape(char const* cstr, args... head) {
			return escape<size_ret, size>(cstr, head..., '\0');
		}

		template<size_t size_ret, size_t size, typename... args> constexpr
		typename std::enable_if<size_ret != sizeof...(args) && size == 1,
						 std::array<char, size_ret> >::type
		escape(char const* cstr, args... head) {
			return escape<size_ret, size - 1>(cstr, head..., *cstr);
		}

		template<size_t size_ret, size_t size, typename... args> constexpr
		typename std::enable_if<size_ret != sizeof...(args) && size == 2,
						 std::array<char, size_ret> >::type
		escape(char const* cstr, args... head) {
			return (*cstr == EBB_ESCAPE_CHAR && *(cstr + 1) == EBB_ESCAPE_CHAR) ?
				escape<size_ret, size - 2>(cstr + 2, head..., EBB_ESCAPE_CHAR) :
				escape<size_ret, size - 1>(cstr + 1, head..., *cstr);
		}

		template<size_t size_ret, size_t size, typename... args> constexpr
		typename std::enable_if<(size_ret != sizeof...(args) && size > 2),
						 std::array<char, size_ret> >::type
		escape(char const* cstr, args... head) {
			return (*cstr == EBB_ESCAPE_CHAR) ? (
					(*(cstr + 1) == EBB_ESCAPE_CHAR) ?
					escape<size_ret, size - 2>(cstr + 2, head..., EBB_ESCAPE_CHAR) :
					escape<size_ret, size - 3>(cstr + 3, head..., numify(cstr + 1))) :
				escape<size_ret, size - 1>(cstr + 1, head..., *cstr);
		}

		constexpr char const* next_token(char const* cstr) {
			return (*cstr != EBB_ESCAPE_CHAR) ? cstr + 1 :
				(*(cstr + 1) == EBB_ESCAPE_CHAR) ? cstr + 2 : cstr + 3;
		}

		constexpr char escape_chars(char const* cstr) {
			return (*cstr != EBB_ESCAPE_CHAR) ? *cstr : detail::numify(cstr + 1);
		}
	}

	constexpr size_t escaped_len(char const* cstr) {
		return (*cstr == '\0') ? 0 :
			(*cstr == EBB_ESCAPE_CHAR) ? detail::escaped_group_len(cstr + 1) :
			1 + escaped_len(cstr + 1);
	}

	constexpr bool is_valid_escaped_string(char const* cstr) {
		return (*cstr == '\0') ? true :
			(*cstr != EBB_ESCAPE_CHAR) ? is_valid_escaped_string(cstr + 1) :
			(*(cstr + 1) == EBB_ESCAPE_CHAR) ? is_valid_escaped_string(cstr + 2) :
			(detail::is_hex(*(cstr + 1)) && detail::is_hex(*(cstr + 2))) ?
			is_valid_escaped_string(cstr + 3) : false;
	}

	constexpr bool is_escaped_less_than(char const* a, char const* b) {
		return (*b == '\0' || detail::escape_chars(a) > detail::escape_chars(b)) ?
			false :
			(*a == '\0' || detail::escape_chars(a) < detail::escape_chars(b)) ?
			true :
			is_escaped_less_than(detail::next_token(a), detail::next_token(b));
	}

	template<size_t size_ret, size_t size> constexpr std::array<char, size_ret>
	escape(char const (&cstr)[size]) {
		return detail::escape<size_ret, size - 1>(cstr);
	}
}

// be sure to static_assert is_valid_escaped_string before calling this, or you
// *WILL* get garbage if the string is invalid
#define EBB_ESCAPE(str) ebb::escape<ebb::escaped_len(str)>(str)
