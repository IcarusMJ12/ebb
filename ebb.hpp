#pragma once
// expressly better bencoder
// Copyright (C) 2013 Igor Kaplounenko
// Licensed under MIT License

#include <cinttypes>
#include <cstdint>
#include <cassert>
#include <cstdio>
#include <array>
#include <tuple>
#include <vector>
#include <cstring>
#include <string>

namespace ebb {
	namespace internal {
		// http://stackoverflow.com/questions/7858817/unpacking-a-tuple-to-call-a-matching-function-pointer?lq=1
		template<int...> struct seq {};
		template<int N, int... S> struct gen_seq : gen_seq<N-1, N-1, S...> {};
		// generated sequence specialization / endpoint
		template<int... S> struct gen_seq<0, S...> { typedef seq<S...> type; };

		// for list and dict start/end tokens, i.e. 'd', 'l', 'e'
		struct bencode_token {
			const unsigned char token;
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

	const static internal::bencode_token bdict_begin = {'d'};
	const static internal::bencode_token bdict_end = {'e'};
	const static internal::bencode_token blist_begin = {'l'};
	const static internal::bencode_token blist_end = {'e'};

	
	template<typename A, typename B> std::tuple<A, B> k_v(A &&a, B &&b) {
		internal::assert_valid_key_type<A>();
		return std::forward_as_tuple(a, b);
	}

	template<typename... Arguments> std::tuple<internal::bencode_token, Arguments...,
		internal::bencode_token> blist(Arguments&&... remaining) {
		return std::forward_as_tuple(blist_begin, remaining..., blist_end);
	}

	template<typename... A, typename... B> std::tuple<internal::bencode_token,
		std::tuple<A,B>..., internal::bencode_token> bdict(
				std::tuple<A, B>&&... remaining) {
		internal::assert_valid_key_types<A...>();
		return std::forward_as_tuple(bdict_begin, remaining..., bdict_end);
	}

	class bencoder {
		private:
			unsigned char* buffer;
			int64_t len;
		public:
			bencoder(unsigned char* buffer, int64_t len = -1):
				buffer(buffer), len(len) {};
			template<typename... Arguments> unsigned char* operator()
				(Arguments&&... remaining) {
					assert(buffer);
					return bencode(remaining...);
				}

		private:
			unsigned char* bencode() {
				return buffer;
			}

			template<typename... Arguments> unsigned char* bencode(int64_t value,
					Arguments&&... remaining) {
				long written = snprintf(reinterpret_cast<char*>(buffer), len,
						"i%" PRId64 "e", value);
				if (written > len) {
					buffer = NULL;
					return NULL;
				}
				buffer += written;
				len -= written;
				return bencode(remaining...);
			}

			template<typename... Arguments> unsigned char* bencode(char const *value,
					Arguments&&... remaining) {
				long written = snprintf(reinterpret_cast<char*>(buffer), len, "%d:%s",
						strlen(value), value);
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
				long written = snprintf(reinterpret_cast<char*>(buffer), len, "%zu:",
						value.size());
				if (written + value.size() > len) {
					buffer = NULL;
					return NULL;
				}
				std::memcpy(buffer + written, &value[0], value.size());
				buffer += written + value.size();
				len -= written + value.size();
				return bencode(remaining...);
			}

			template<typename... Arguments> unsigned char* bencode(
					std::vector<char> const &value, Arguments&&... remaining) {
				long written = snprintf(reinterpret_cast<char*>(buffer), len, "%zu:",
						value.size());
				if (written + value.size() > len) {
					buffer = NULL;
					return NULL;
				}
				std::memcpy(buffer + written, (unsigned char*)&value[0], value.size());
				buffer += written + value.size();
				len -= written + value.size();
				return bencode(remaining...);
			}

			template<typename... Arguments> unsigned char* bencode(
					std::string const &value, Arguments&&... remaining) {
				long written = snprintf(reinterpret_cast<char*>(buffer), len, "%zu:",
						value.size());
				if (written + value.size() > len) {
					buffer = NULL;
					return NULL;
				}
				std::memcpy(buffer + written, &(value[0]), value.size());
				buffer += written + value.size();
				len -= written + value.size();
				return bencode(remaining...);
			}

			template<size_t N, typename... Arguments> unsigned char* bencode(
					std::array<unsigned char, N> const &value, Arguments&&... remaining) {
				long written = snprintf(reinterpret_cast<char*>(buffer), len, "%zu:", N);
				if (written + N > len) {
					buffer = NULL;
					return NULL;
				}
				std::memcpy(buffer + written, &(value[0]), N);
				buffer += written + N;
				len -= written + N;
				return bencode(remaining...);
			}

			template<typename... Arguments> unsigned char* bencode(
					internal::bencode_token const value, Arguments&&... remaining) {
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
				return bencode(typename internal::gen_seq<sizeof...(TupleTypes)>::type(),
						value, remaining...);
			}

			template<int... S, typename... TupleTypes, typename... Arguments>
				unsigned char* bencode(internal::seq<S...>,
						std::tuple<TupleTypes...> const &value, Arguments... remaining) {
				return bencode(std::get<S>(value)..., remaining...);
			}
	};
}
