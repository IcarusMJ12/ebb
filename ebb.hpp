#pragma once
// expressly better bencoder

#include <cassert>
#include <cstdio>
#include <array>
#include <tuple>
#include <vector>

namespace ebb {
	namespace internal {
		// stolen almost verbatim from:
		// http://stackoverflow.com/questions/7858817/unpacking-a-tuple-to-call-a-matching-function-pointer?lq=1
		template<int...> struct seq {};
		template<int N, int... S> struct gen_seq : gen_seq<N-1, N-1, S...> {};
		// generated sequence specialization / endpoint
		template<int... S> struct gen_seq<0, S...> { typedef seq<S...> type; };

		struct bencode_token {
			const unsigned char token;
		}; 
	}

	struct internal::bencode_token dict_begin = {'d'};
	struct internal::bencode_token dict_end = {'e'};
	struct internal::bencode_token list_begin = {'l'};
	struct internal::bencode_token list_end = {'e'};

	
	template<typename... Arguments> std::tuple<Arguments...> benc_k_v(Arguments&&... remaining) {
		return std::forward_as_tuple(remaining...);
	}

	template<typename... Arguments> std::tuple<internal::bencode_token, Arguments..., internal::bencode_token> benc_list(Arguments&&... remaining) {
		return std::forward_as_tuple(list_begin, remaining..., list_end);
	}

	template<typename... Arguments> std::tuple<internal::bencode_token, Arguments..., internal::bencode_token> benc_dict(Arguments&&... remaining) {
		return std::forward_as_tuple(dict_begin, remaining..., dict_end);
	}

	class bencoder {
		private:
			unsigned char* buffer;
			long long len;
		public:
			bencoder(unsigned char* buffer, long long len = -1):
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

			template<typename... Arguments> unsigned char* bencode(long long value, Arguments&&... remaining) {
				long written = snprintf(reinterpret_cast<char*>(buffer), len, "i%llie", value);
				if (written > len) {
					buffer = NULL;
					return NULL;
				}
				buffer += written;
				len -= written;
				return bencode(remaining...);
			}

			template<typename... Arguments> unsigned char* bencode(char const *value, Arguments&&... remaining) {
				long written = snprintf(reinterpret_cast<char*>(buffer), len, "%lu:%s", strlen(value), value);
				if (written > len) {
					buffer = NULL;
					return NULL;
				}
				buffer += written;
				len -= written;
				return bencode(remaining...);
			}

			template<typename... Arguments> unsigned char* bencode(std::vector<unsigned char> const &value, Arguments&&... remaining) {
				long written = snprintf(reinterpret_cast<char*>(buffer), len, "%lu:", value.size());
				if (written + value.size() > len) {
					buffer = NULL;
					return NULL;
				}
				memcpy(buffer + written, &(value[0]), value.size());
				buffer += written + value.size();
				len -= written + value.size();
				return bencode(remaining...);
			}

			template<size_t N, typename... Arguments> unsigned char* bencode(std::array<unsigned char, N> const &value, Arguments&&... remaining) {
				long written = snprintf(reinterpret_cast<char*>(buffer), len, "%lu:", N);
				if (written + N > len) {
					buffer = NULL;
					return NULL;
				}
				memcpy(buffer + written, &(value[0]), N);
				buffer += written + N;
				len -= written + N;
				return bencode(remaining...);
			}

			template<typename... Arguments> unsigned char* bencode(internal::bencode_token const value, Arguments&&... remaining) {
				if (len == 0) {
					buffer = NULL;
					return NULL;
				}
				*buffer = value.token;
				buffer++;
				len--;
				return bencode(remaining...);
			}

			template<typename... TupleTypes, typename... Arguments> unsigned char* bencode(std::tuple<TupleTypes...> const &value, Arguments... remaining) {
				return bencode(typename internal::gen_seq<sizeof...(TupleTypes)>::type(), value, remaining...);
			}

			template<int... S, typename... TupleTypes, typename... Arguments> unsigned char* bencode(internal::seq<S...>, std::tuple<TupleTypes...> const &value, Arguments... remaining) {
				return bencode(std::get<S>(value)..., remaining...);
			}
	};
}
