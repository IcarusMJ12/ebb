// Copyright (C) 2014 Igor Kaplounenko
// Licensed under MIT License

#ifdef _WIN32
#include <stdarg.h>
#include <stdio.h>
inline int snprintf(char* buf, int len, char const* fmt, ...)
{
	va_list lp;
	va_start(lp, fmt);
	int ret = _vsnprintf(buf, len, fmt, lp);
	va_end(lp);
	if (ret < 0) { buf[len-1] = 0; ret = len+1; }
	return ret;
}
#endif

#include "ebb.hpp"

#include "gtest/gtest.h"

#define UCAST(A) reinterpret_cast<const unsigned char*>(A)

using namespace ebb;

EBB_MAKE_BENCODED_DICT(nested_dict,
		(int, i))

EBB_MAKE_BENCODED_DICT(various_dict,
		(std::array<unsigned char, 3>, array)
		(int, integer)
		(std::int64_t, integer64)
		(std::array<nested_dict, 3>, nested_array)
		(std::vector<nested_dict>, nested_vector)
		(std::string, string)
		(std::vector<unsigned char>, vector))

EBB_MAKE_BENCODED_DICT(benc_dict_escaped,
		(int, spacesQ20andQ20QQ))

EBB_MAKE_BENCODED_DICT(benc_dict_map,
		(std::map<std::string, std::string>, something))

TEST(bdecode, integer) {
	std::int64_t out;
	const char* input = "i8e";
	size_t len = strlen(input);
	const unsigned char* result;

	result = bdecode(UCAST(input), len, out);
	EXPECT_TRUE(result);
	EXPECT_EQ(8, out);
	EXPECT_EQ('\0', *result);
	EXPECT_EQ(0, len);
	EXPECT_EQ(UCAST(input) + strlen(input), result);
}

TEST(bdecode, integer_truncated) {
	std::int64_t out;
	const char* input = "i88";
	size_t len = strlen(input);
	const unsigned char* result;

	result = bdecode(UCAST(input), len, out);
	EXPECT_FALSE(result);
}

TEST(bdecode, string) {
	std::string out;
	const char* input = "5:array";
	size_t len = strlen(input);
	const unsigned char* result;

	result = bdecode(UCAST(input), len, out);
	EXPECT_TRUE(result);
	EXPECT_STREQ("array", out.c_str());
	EXPECT_EQ('\0', *result);
	EXPECT_EQ(0, len);
	EXPECT_EQ(UCAST(input) + strlen(input), result);
}

TEST(bdecode, string_truncated) {
	std::string out;
	const char* input = "5:arr";
	size_t len = strlen(input);
	const unsigned char* result;

	result = bdecode(UCAST(input), len, out);
	EXPECT_FALSE(result);
}

TEST(bdecode, vector) {
	std::vector<std::int64_t> out;
	const char* input = "li1ei2ei3ei4ee";
	size_t len = strlen(input);
	const unsigned char* result;

	result = bdecode(UCAST(input), len, out);
	ASSERT_TRUE(result);
	for(int i = 0; i < 4; i++) {
		EXPECT_EQ(i + 1, out[i]);
	}
	EXPECT_EQ('\0', *result);
	EXPECT_EQ(0, len);
	EXPECT_EQ(UCAST(input) + strlen(input), result);
}

TEST(bdecode, vector_truncated) {
	std::vector<std::int64_t> out;
	const char* input = "li1ei2ei3ei4e";
	size_t len = strlen(input);
	const unsigned char* result;

	result = bdecode(UCAST(input), len, out);
	EXPECT_FALSE(result);
}

TEST(bdecode, dict_various) {
	std::array<unsigned char, 1024> buf;
	unsigned char* last = bencoder(buf)(
			bdict(
				k_v("array", "abc"),
				k_v("integer", 1),
				k_v("integer64", 2),
				k_v("nested_array", blist(
						bdict(k_v("i", 7)),
						bdict(k_v("i", 8)),
						bdict(k_v("i", 9)))),
				k_v("nested_vector", blist(
						bdict(k_v("i", 5)),
						bdict(k_v("i", 6)))),
				k_v("string", "def"),
				k_v("vector", "ghi")
				));
	assert(last != nullptr);
	*last = '\0';

	various_dict d;
	const unsigned char* ret = d.bdecode(buf);
	EXPECT_TRUE(ret);
	EXPECT_FALSE(memcmp(d.array.data(), "abc", d.array.size()));
	EXPECT_EQ(1, d.integer);
	EXPECT_EQ(2, d.integer64);
	EXPECT_EQ(7, d.nested_array[0].i);
	EXPECT_EQ(8, d.nested_array[1].i);
	EXPECT_EQ(9, d.nested_array[2].i);
	ASSERT_EQ(2, d.nested_vector.size());
	EXPECT_EQ(5, d.nested_vector[0].i);
	EXPECT_EQ(6, d.nested_vector[1].i);
	EXPECT_STREQ("def", d.string.c_str());
	EXPECT_FALSE(memcmp(&d.vector[0], "ghi", d.vector.size()));
}

TEST(bdecode, dict_escaped) {
	std::array<unsigned char, 1024> buf;
	unsigned char* last = bencoder(buf)(
			bdict(
				k_v("spaces and Q", 1)
				));
	assert(last != nullptr);
	*last = '\0';

	benc_dict_escaped d;
	const unsigned char* ret = d.bdecode(buf);
	EXPECT_TRUE(ret);
	EXPECT_EQ(1, d.spacesQ20andQ20QQ);
}

TEST(bdecode, dict_map) {
	std::array<unsigned char, 1024> buf;
	unsigned char* last = bencoder(buf)(
			bdict(
				k_v("something", bdict(
						k_v("one", "two"),
						k_v("three", "four")
						))));
	assert(last != nullptr);
	*last = '\0';

	benc_dict_map d;
	const unsigned char* ret = d.bdecode(buf);
	EXPECT_TRUE(ret);
	EXPECT_TRUE(d.something["one"] == "two");
	EXPECT_TRUE(d.something["three"] == "four");
}
