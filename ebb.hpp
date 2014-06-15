#pragma once
// expressly better bencoder
// Copyright (C) 2013-2014 Igor Kaplounenko
// Licensed under MIT License

#include <array>
#include <cassert>
#include <cinttypes>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <tuple>
#include <vector>

#if defined(BOOST_PP_VARIADICS)
#if !BOOST_PP_VARIADICS
#error BOOST_PP_VARIADICS cannot be defined as 0 when using Ebb
#endif
#else
#define BOOST_PP_VARIADICS 1
#endif 
#include <boost/preprocessor/arithmetic/dec.hpp>
#include <boost/preprocessor/comparison/not_equal.hpp>
#include <boost/preprocessor/control/iif.hpp>
#include <boost/preprocessor/facilities/empty.hpp>
#include <boost/preprocessor/punctuation/comma_if.hpp>
#include <boost/preprocessor/seq/cat.hpp>
#include <boost/preprocessor/seq/first_n.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/seq/for_each_i.hpp>
#include <boost/preprocessor/seq/seq.hpp>
#include <boost/preprocessor/seq/to_tuple.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/tuple/elem.hpp>
#include <boost/preprocessor/tuple/enum.hpp>
#include <boost/preprocessor/tuple/size.hpp>
#include <boost/preprocessor/tuple/to_seq.hpp>

#ifndef PRId64
#if defined MSC_VER || defined __MINGW32_
#define PRId64 "I64d"
#else
#define PRId64 "lld"
#endif
#endif

namespace ebb {
	namespace detail {
		// http://stackoverflow.com/questions/7858817/unpacking-a-tuple-to-call-a-matching-function-pointer?lq=1
		template<size_t...> struct seq {};
		template<size_t N, size_t... S> struct gen_seq : gen_seq<N-1, N-1, S...> {};
		// generated sequence specialization / endpoint
		template<size_t... S> struct gen_seq<0, S...> { const static seq<S...> value; };

		// for enforcing lexicografic key order
		constexpr bool is_less_than(char const* left, char const* right) {
			return (*left < *right ||
					(*left == *right && *left && is_less_than(left+1, right+1)));
		}

		constexpr bool is_valid_key_order(char const* remaining) { return true; }

		template<typename... T>
		constexpr bool is_valid_key_order(char const* left, char const* right,
				T... remaining) {
			return is_less_than(left, right) ?
				is_valid_key_order(right, remaining...) : false;
		}

		// for list and dict start/end tokens, i.e. 'd', 'l', 'e'
		struct bencode_token {
			const unsigned char token;
		}; 

		struct bencoded_dict {
			unsigned char const* bdecode(unsigned char const* buffer, size_t &len) {
				assert(false);
				return NULL;
			}
			unsigned char* bencode(unsigned char* buffer, size_t len) {
				assert(false);
				return NULL;
			}
		};


		template<typename> struct is_unsigned_char_array : std::false_type {};
		template<size_t N> struct is_unsigned_char_array<std::array<unsigned char, N>>
			: std::true_type {};
		template<size_t N> struct is_unsigned_char_array<std::array<unsigned char, N>&>
			: std::true_type {};
		template<size_t N> struct is_unsigned_char_array<
			const std::array<unsigned char, N>> : std::true_type {};
		template<size_t N> struct is_unsigned_char_array<
			const std::array<unsigned char, N>&> : std::true_type {};
		template<typename A> void assert_valid_key_type() {
			static_assert(std::is_convertible<A, std::vector<unsigned char>>::value
				|| std::is_convertible<A, std::vector<char>>::value
				|| std::is_convertible<A, const char*>::value
				|| std::is_convertible<A, std::string>::value
				|| is_unsigned_char_array<A>::value
				, "Bencoded dictionary key must be a char*, an std::vector<unsigned char>"
				", or an std::array<unsigned char, N>.");
		}
		template<typename Head> void assert_valid_key_types() {
			assert_valid_key_type<Head>();
		}
		template<typename Head, typename Middle, typename... Tail>
			void assert_valid_key_types() {
			assert_valid_key_type<Head>();
			assert_valid_key_types<Middle, Tail...>();
		}
	}

	const static detail::bencode_token bdict_begin = {'d'};
	const static detail::bencode_token bdict_end = {'e'};
	const static detail::bencode_token blist_begin = {'l'};
	const static detail::bencode_token blist_end = {'e'};

	
	template<typename A, typename B> std::tuple<A, B> k_v(A &&a, B &&b) {
		detail::assert_valid_key_type<A>();
		return std::forward_as_tuple(a, b);
	}

	template<typename... Arguments> std::tuple<detail::bencode_token, Arguments...,
		detail::bencode_token> blist(Arguments&&... remaining) {
		return std::forward_as_tuple(blist_begin, remaining..., blist_end);
	}

	template<typename... A, typename... B> std::tuple<detail::bencode_token,
		std::tuple<A,B>..., detail::bencode_token> bdict(
				std::tuple<A, B>&&... remaining) {
		detail::assert_valid_key_types<A...>();
		return std::forward_as_tuple(bdict_begin, remaining..., bdict_end);
	}

	class bencoder {
		private:
			unsigned char* buffer;
			size_t len;
		public:
			bencoder(unsigned char* buffer, size_t len) : buffer(buffer), len(len) {};
			template<size_t Size> bencoder(std::array<unsigned char, Size>& buffer) :
				buffer(buffer.data()), len(buffer.size()) {};
			template<typename... Arguments> unsigned char* operator()
				(Arguments&&... remaining) {
					assert(buffer);
					return bencode(remaining...);
				}

		private:
			unsigned char* bencode() {
				return buffer;
			}

			template<typename... Arguments> unsigned char* bencode(char const *value,
					Arguments&&... remaining) {
				long written = snprintf(reinterpret_cast<char*>(buffer), len,
						"%" PRId64 ":%s", std::int64_t(strlen(value)), value);
				if (written > len) {
					buffer = NULL;
					return NULL;
				}
				buffer += written;
				len -= written;
				return bencode(remaining...);
			}

			template<typename... Arguments> unsigned char* bencode(
					std::vector<unsigned char> const &value, Arguments&&... remaining) {
				return bencode_listish(value, remaining...);
			}

			template<typename... Arguments> unsigned char* bencode(
					std::vector<char> const &value, Arguments&&... remaining) {
				return bencode_listish(value, remaining...);
			}

			template<typename... Arguments> unsigned char* bencode(
					std::string const &value, Arguments&&... remaining) {
				return bencode_listish(value, remaining...);
			}

			template<typename T, typename... Arguments> typename
					std::enable_if<std::is_integral<T>::value, unsigned char*>::type
					bencode(T value, Arguments&&... remaining) {
				long written = snprintf(reinterpret_cast<char*>(buffer), len,
						"i%" PRId64 "e", std::int64_t(value));
				if (written > len) {
					buffer = NULL;
					return NULL;
				}
				buffer += written;
				len -= written;
				return bencode(remaining...);
			}

			template<typename T, typename... Arguments> typename std::enable_if<
					std::is_base_of<detail::bencoded_dict, T>::value,
					unsigned char*>::type bencode(T const &value,
							Arguments&&... remaining) {
				unsigned char* result = value.bencode(buffer, len);
				if (result == NULL) {
					buffer = NULL;
					return NULL;
				}
				len -= result - buffer;
				buffer = result;
				return bencode(remaining...);
			}
			
			template<size_t N, typename... Arguments> unsigned char* bencode(
					std::array<unsigned char const, N> const &value,
					Arguments&&... remaining) {
				return bencode_listish(value, remaining...);
			}

			template<size_t N, typename... Arguments> unsigned char* bencode(
					std::array<unsigned char, N> const &value, Arguments&&... remaining) {
				return bencode_listish(value, remaining...);
			}

			template<typename... Arguments> unsigned char* bencode(
					detail::bencode_token const value, Arguments&&... remaining) {
				if (len == 0) {
					buffer = NULL;
					return NULL;
				}
				*buffer = value.token;
				buffer++;
				len--;
				return bencode(remaining...);
			}

			template<typename... TupleTypes, typename... Arguments>
				unsigned char* bencode(std::tuple<TupleTypes...> const &value,
						Arguments... remaining) {
				return bencode(detail::gen_seq<sizeof...(TupleTypes)>::value,
						value, remaining...);
			}

			template<size_t... S, typename... TupleTypes, typename... Arguments>
				unsigned char* bencode(detail::seq<S...>,
						std::tuple<TupleTypes...> const &value, Arguments... remaining) {
				return bencode(std::get<S>(value)..., remaining...);
			}

			template<typename T, typename... Arguments>
			unsigned char* bencode_listish(T const& value, Arguments&&... remaining) {
				if (value.size() > INT64_MAX) {
					buffer = NULL;
					return NULL;
				}
				long written = snprintf(reinterpret_cast<char*>(buffer), len,
						"%" PRId64 ":" , std::int64_t(value.size()));
				if (written + value.size() > len) {
					buffer = NULL;
					return NULL;
				}
				std::memcpy(buffer + written, &value[0], value.size());
				buffer += written + value.size();
				len -= written + value.size();
				return bencode(remaining...);
			}

			//TODO: array to tuple, perhaps?
			template<typename T, size_t S, typename... Arguments> unsigned char*
				bencode(std::array<T, S> const& value, Arguments&&... remaining) {
				bencode(blist_begin);
				for(auto const& v : value) {
					if (bencode(v) == NULL) {
						return NULL;
					}
				}
				return bencode(blist_end, remaining...);
			}

			template<typename T, typename... Arguments> unsigned char*
				bencode(std::vector<T> const& value, Arguments&&... remaining) {
				bencode(blist_begin);
				for(auto const& v : value) {
					if (bencode(v) == NULL) {
						return NULL;
					}
				}
				return bencode(blist_end, remaining...);
			}
	};

	template<typename T> typename std::enable_if<std::is_integral<T>::value,
		unsigned char const*>::type bdecode(unsigned char const* buffer,
				size_t &len, T &value) {
		// int has to start with 'i' and followed by at least one digit and 'e'
		if (len < 3 || buffer[0] != 'i') {
			return NULL;
		}
		unsigned char* end;
		static_assert(sizeof(long long) == sizeof(std::int64_t),
				"long long must be int64_t");
		std::int64_t val = std::strtoll(reinterpret_cast<char const*>(buffer + 1),
				reinterpret_cast<char**>(&end), 10);
		// we read nothing, or we read past the end of the buffer, or int is not
		// terminated by 'e'
		if (buffer + 1 == end || buffer + len <= end || *end != 'e') {
			return NULL;
		}
		value = T(val);
		if (value != val) {
			return NULL;
		}
		len -= (end - buffer + 1);
		return end + 1;
	}

	template<typename T> typename std::enable_if<
		std::is_base_of<std::string, T>::value ||
		std::is_base_of<std::vector<unsigned char>, T>::value,
		unsigned char const*>::type bdecode(unsigned char const* buffer,
				size_t &len, T &value) {
		if (len < 2) {
			return NULL;
		}
		unsigned char* end;
		std::int64_t value_len = std::strtoll(reinterpret_cast<char const*>(buffer),
				reinterpret_cast<char **>(&end), 10);
		// we read nothing, or our string has a negative length, or the string
		// extends past the end of the buffer
		if (buffer == end || *end != ':' || value_len < 0
				|| buffer + len < end + value_len + 1) {
			return NULL;
		}
		value.resize(value_len);
		std::copy(end + 1, end + value_len + 1, value.begin());
		len -= (end + value_len + 1 - buffer);
		return end + value_len + 1;
	}

	template<size_t S> unsigned char const* bdecode(unsigned char const* buffer,
			size_t &len, std::array<unsigned char, S> &value) {
		if (len < 2) {
			return NULL;
		}
		unsigned char* end;
		std::int64_t value_len = std::strtoll(reinterpret_cast<char const*>(buffer),
				reinterpret_cast<char **>(&end), 10);
		// we read nothing, or our string has a negative length, or the string
		// extends past the end of the buffer
		if (buffer == end || *end != ':' || value_len < 0
				|| buffer + len < end + value_len + 1 || value_len != value.size()) {
			return NULL;
		}
		std::copy(end + 1, end + value_len + 1, value.begin());
		len -= (end + value_len + 1 - buffer);
		return end + value_len + 1;
	}

	template<typename T, size_t S> const unsigned char* bdecode(
			unsigned char const* buffer, size_t &len, std::array<T, S> &value) {
		if (len < 2 || *buffer != 'l') return NULL;
		len--;
		buffer++;
		for (T& i : value) {
			buffer = i.bdecode(buffer, len);
			if (buffer == NULL) return NULL;
		}
		if (len == 0 || *buffer != 'e') return NULL;
		len--;
		return buffer + 1;
	}

	template<typename T> typename std::enable_if<std::is_base_of<
		detail::bencoded_dict, T>::value, const unsigned char*>::type
		bdecode(unsigned char const* buffer, size_t &len, T &value) {
		return value.bdecode(buffer, len);
	}

	template<typename T> typename std::enable_if<!std::is_same<
		T, unsigned char>::value, unsigned char const*>::type bdecode(
			unsigned char const* buffer, size_t &len, std::vector<T> &value) {
		if (len < 2 || *buffer != 'l') {
			return NULL;
		}
		buffer++;
		len--;
		value.clear();
		while(*buffer != 'e') {
			T element;
			buffer = bdecode(buffer, len, element);
			if (buffer == NULL || len == 0) {
				value.clear();
				return NULL;
			}
			value.push_back(element);
		}
		len--;
		return buffer + 1;
	}

	inline unsigned char const* bdecode_expect(unsigned char const* buffer,
			size_t &len, char const* value) {
		if (len < 3) {
			return NULL;
		}
		unsigned char* end;
		std::int64_t value_len = std::strtoll(reinterpret_cast<char const*>(buffer),
				reinterpret_cast<char **>(&end), 10);
		// we read nothing, or our string has a negative length, or the string
		// extends past the end of the buffer
		if (buffer == end || *end != ':' || value_len < 0
				|| buffer + len < end + value_len + 1
				|| value_len != std::strlen(value)
				|| (std::memcmp(end + 1, value, strlen(value)) != 0)) {
			return NULL;
		}
		len -= (end + value_len + 1 - buffer);
		return end + value_len + 1;
	}
}

// here be dragons
// long story short, this wraps boost sequence elements in an extra set of
// parentheses, turning each one into a tuple
#define EBB_ADD_PAREN_1(...) ((__VA_ARGS__)) EBB_ADD_PAREN_2
#define EBB_ADD_PAREN_2(...) ((__VA_ARGS__)) EBB_ADD_PAREN_1
#define EBB_ADD_PAREN_1_END
#define EBB_ADD_PAREN_2_END
#define EBB_PARENIFY(A) BOOST_PP_CAT(EBB_ADD_PAREN_1 A,_END)

#define EBB_GET_ARG_TYPE(TUPLE) \
	BOOST_PP_TUPLE_ENUM(BOOST_PP_SEQ_TO_TUPLE(BOOST_PP_SEQ_FIRST_N(BOOST_PP_DEC( \
					BOOST_PP_TUPLE_SIZE(TUPLE)), BOOST_PP_TUPLE_TO_SEQ(TUPLE))))
#define EBB_GET_ARG_NAME(TUPLE) \
	BOOST_PP_TUPLE_ELEM(BOOST_PP_DEC(BOOST_PP_TUPLE_SIZE(TUPLE)), TUPLE)

#define EBB_ENCODE_ARG(R, DATA, I, TUPLE) \
	BOOST_PP_COMMA_IF(I) BOOST_PP_STRINGIZE(EBB_GET_ARG_NAME(TUPLE)) \
	BOOST_PP_COMMA() this->EBB_GET_ARG_NAME(TUPLE)

#define EBB_STRINGIFY_ARG(R, DATA, I, TUPLE) \
	BOOST_PP_COMMA_IF(I) BOOST_PP_STRINGIZE(EBB_GET_ARG_NAME(TUPLE))

#define EBB_ATTR_DECL(R, DATA, TUPLE) \
	EBB_GET_ARG_TYPE(TUPLE) EBB_GET_ARG_NAME(TUPLE);

#define EBB_MAKE_BENCODED_ATTRIBUTES(ATTRIBUTES) \
	BOOST_PP_SEQ_FOR_EACH(EBB_ATTR_DECL, 0, EBB_PARENIFY(ATTRIBUTES))

#define EBB_MAKE_ARGS(ARGS) \
	BOOST_PP_SEQ_FOR_EACH_I(EBB_ENCODE_ARG, 0, EBB_PARENIFY(ARGS))

#define EBB_VALIDATE_ARGS(ARGS) \
	static_assert(ebb::detail::is_valid_key_order( \
				BOOST_PP_SEQ_FOR_EACH_I(EBB_STRINGIFY_ARG, 0, EBB_PARENIFY(ARGS))), \
			"dictionary keys are not lexicographically sorted");

#define EBB_MAKE_BENCODE(ATTRIBUTES) \
	unsigned char* bencode(unsigned char* buffer, size_t len) const { \
		EBB_VALIDATE_ARGS(ATTRIBUTES) \
		return ebb::bencoder(buffer, len)(ebb::bdict_begin, \
				EBB_MAKE_ARGS(ATTRIBUTES), ebb::bdict_end); \
	} \
	\
	template <size_t Size> \
	unsigned char* bencode(std::array<unsigned char, Size> &buffer) const { \
		return this->bencode(buffer.data(), buffer.size()); \
	}

#define EBB_BDECODE_NEXT(R, DATA, TUPLE) \
	buf = ebb::bdecode_expect(buf, len, \
			BOOST_PP_STRINGIZE(EBB_GET_ARG_NAME(TUPLE))); \
	if (buf == NULL) return NULL; \
	buf = ebb::bdecode(buf, len, EBB_GET_ARG_NAME(TUPLE)); \
	if (buf == NULL) return NULL;

#define EBB_MAKE_BDECODE(ATTRIBUTES) \
	template<size_t S> unsigned char const* bdecode( \
			std::array<unsigned char, S> const& buffer) { \
		size_t len = buffer.size(); \
		return bdecode(buffer.data(), len); \
	} \
	\
	unsigned char const* bdecode(unsigned char const* buffer, size_t &len) { \
		unsigned char const* buf = buffer; \
		if (len < 2 || *buf != 'd') return NULL; \
		len--; \
		buf++; \
		BOOST_PP_SEQ_FOR_EACH(EBB_BDECODE_NEXT, 0, EBB_PARENIFY(ATTRIBUTES)) \
		if (len == 0 || *buf != 'e') return NULL; \
		len--; \
		buf++; \
		return buf; \
	}

/*	e.g.
 *
 *  EBB_MAKE_BENCODED_DICT(my_dict,
 *  		(int, a)(char const*, b)(std::array<unsigned char, 5>, c))
 *
 *  will create a dictionary { 'a': a, 'b' : b, 'c': c }
 */
#define EBB_MAKE_BENCODED_DICT(NAME, ATTRIBUTES) \
struct NAME : ebb::detail::bencoded_dict { \
	EBB_MAKE_BENCODED_ATTRIBUTES(ATTRIBUTES) \
	\
	EBB_MAKE_BENCODE(ATTRIBUTES) \
	\
	EBB_MAKE_BDECODE(ATTRIBUTES) \
};
