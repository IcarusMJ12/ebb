// Copyright (C) 2013-2014 Igor Kaplounenko
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

using namespace ebb;

EBB_MAKE_BENCODED_DICT(benc_dict,
		(std::array<unsigned char, 1>, array)
		(int, integer)
		(std::int64_t, integer64)
		(std::string, string)
		(std::vector<unsigned char>, vector))

EBB_MAKE_BENCODED_DICT(benc_dict_nested,
		(std::array<benc_dict, 2>, dicts))

EBB_MAKE_BENCODED_DICT(benc_dict_escaped,
		(int, spacesQ20andQ20QQ))

TEST(bencode, integer) {
	unsigned char output[1024];
	unsigned char* last = bencoder(output, 1024)(
			3000ll
			);
	ASSERT_NE(static_cast<unsigned char*>(NULL), last);
	*last = '\0';
	EXPECT_STREQ("i3000e", reinterpret_cast<char*>(output));
}

TEST(bencode, integer_bounds) {
	unsigned char output[4];
	unsigned char* last = bencoder(output, 4)(
			3000ll
			);
	EXPECT_EQ(static_cast<unsigned char*>(NULL), last);
}

TEST(bencode, string) {
	unsigned char output[1024];
	unsigned char* last = bencoder(output, 1024)(
			"asdf"
			);
	ASSERT_NE(static_cast<unsigned char*>(NULL), last);
	*last = '\0';
	EXPECT_STREQ("4:asdf", reinterpret_cast<char*>(output));
}

TEST(bencode, string_bounds) {
	unsigned char output[4];
	unsigned char* last = bencoder(output, 4)(
			"asdf"
			);
	EXPECT_EQ(static_cast<unsigned char*>(NULL), last);
}

TEST(bencode, array) {
	unsigned char output[1024];
	std::array<unsigned char, 3> data1 = {{'\1', '\0', '\2'}};
	unsigned char* last = bencoder(output, 1024)(
			data1
			);
	ASSERT_NE(static_cast<unsigned char*>(NULL), last);
	EXPECT_EQ(0, memcmp("3:\x01\x00\x02", output, 3));
}

TEST(bencode, const_array) {
	unsigned char output[1024];
	std::array<const unsigned char, 3> data1 = {{'\1', '\0', '\2'}};
	unsigned char* last = bencoder(output, 1024)(
			data1
			);
	ASSERT_NE(static_cast<unsigned char*>(NULL), last);
	EXPECT_EQ(0, memcmp("3:\x01\x00\x02", output, 3));
}

TEST(bencode, array_bounds) {
	unsigned char output[4];
	std::array<unsigned char, 3> data1 = {{'\1', '\0', '\2'}};
	unsigned char* last = bencoder(output, 4)(
			data1
			);
	EXPECT_EQ(static_cast<unsigned char*>(NULL), last);
}

TEST(bencode, vector) {
	unsigned char output[1024];
	std::vector<unsigned char> data1 = {{'\1', '\0', '\2'}};
	unsigned char* last = bencoder(output, 1024)(
			data1
			);
	ASSERT_NE(static_cast<unsigned char*>(NULL), last);
	EXPECT_EQ(0, memcmp("3:\x01\x00\x02", output, 3));
}

TEST(bencode, vector_bounds) {
	unsigned char output[4];
	std::vector<unsigned char> data1 = {{'\1', '\0', '\2'}};
	unsigned char* last = bencoder(output, 4)(
			data1
			);
	EXPECT_EQ(static_cast<unsigned char*>(NULL), last);
}

TEST(bencode, list) {
	unsigned char output[1024];
	unsigned char* last = bencoder(output, 1024)(
			blist(
				"a", "b", "c", "d"
				)
		);
	ASSERT_NE(static_cast<unsigned char*>(NULL), last);
	*last = '\0';
	EXPECT_STREQ("l1:a1:b1:c1:de", reinterpret_cast<const char*>(output));
}

TEST(bencode, dict) {
	unsigned char output[1024];
	unsigned char* last = bencoder(output, 1024)(
			bdict(
				k_v("1", 2),
				k_v("3", "4")
				)
			);
	ASSERT_NE(static_cast<unsigned char*>(NULL), last);
	*last = '\0';
	EXPECT_STREQ("d1:1i2e1:31:4e", reinterpret_cast<const char*>(output));
}

TEST(bencode, key_types) {
	unsigned char output[1024];
	const char* data0 = "abc";
	char* data0_1 = const_cast<char*>(data0);
	std::array<unsigned char, 3> data1 = {{'e', 'f', 'g'}};
	std::vector<unsigned char> data2 = {{'h', 'i', 'j'}};
	const std::array<unsigned char, 3> data1_1 = {{'e', 'f', 'g'}};
	const std::vector<unsigned char> data2_1 = {{'h', 'i', 'j'}};
	unsigned char* last = bencoder(output, 1024)(
			bdict(
				k_v(data0, 1),
				k_v(data0_1, 1),
				k_v(data1, 1),
				k_v(data1_1, 1),
				k_v(data2, 1),
				k_v(data2_1, 1)
				)
			);
	EXPECT_NE(static_cast<unsigned char*>(NULL), last);
}

TEST(bencode, multiple_writes) {
	unsigned char output[1024];
	const char* data0 = "abc";
	const char* data1 = "def";
	bencoder b(output, 1024);
	b(blist_begin, data0);
	unsigned char* last = b(data1, blist_end);
	ASSERT_NE(static_cast<unsigned char*>(NULL), last);
	*last = '\0';
	EXPECT_STREQ("l3:abc3:defe", reinterpret_cast<const char*>(output));
}

TEST(bencode, fancy) {
	unsigned char output[1024];
	std::array<unsigned char, 3> data1 = {{'e', 'f', 'g'}};
	std::vector<unsigned char> data2 = {{'h', 'i', 'j'}};
	const char* expected = "d1:a1:b1:1i2e4:listl1:a1:b1:c1:d3:efg3:hijee";
	unsigned char* last = bencoder(output, 1024)(
			bdict(
				k_v("a", "b"),
				k_v("1", 2),
				k_v("list", blist("a", "b", "c", "d", data1, data2))
				)
			);
	EXPECT_NE(static_cast<unsigned char*>(NULL), last);
	*last = '\0';
	EXPECT_STREQ(expected, reinterpret_cast<const char*>(output));
}

TEST(bencode, array_constructor) {
	std::array<unsigned char, 1024> output;
	std::array<unsigned char, 3> data1 = {{'e', 'f', 'g'}};
	std::vector<unsigned char> data2 = {{'h', 'i', 'j'}};
	const char* expected = "d1:a1:b1:1i2e4:listl1:a1:b1:c1:d3:efg3:hijee";
	unsigned char* last = bencoder(output)(
			bdict(
				k_v("a", "b"),
				k_v("1", 2),
				k_v("list", blist("a", "b", "c", "d", data1, data2))
				)
			);
	EXPECT_NE(static_cast<unsigned char*>(NULL), last);
	*last = '\0';
	EXPECT_STREQ(expected, reinterpret_cast<const char*>(output.data()));
}

TEST(bencode, bencoded_dict) {
	benc_dict d;
	d.integer = 1;
	d.integer64 = 2;
	d.string = "3";
	d.array[0] = '4';
	d.vector.push_back('5');

	const char* expected = "d5:array1:47:integeri1e9:integer64i2e6:string1:3"
		"6:vector1:5e";

	std::array<unsigned char, 1024> output;
	unsigned char* last = d.bencode(output);
	EXPECT_NE(static_cast<unsigned char*>(NULL), last);
	*last = '\0';
	EXPECT_STREQ(expected, reinterpret_cast<const char*>(output.data()));
}

TEST(bencode, bencoded_dict_nested) {
	benc_dict_nested d;
	for (auto& dict : d.dicts) {
		dict.integer = 1;
		dict.integer64 = 2;
		dict.string = "3";
		dict.array[0] = '4';
		dict.vector.push_back('5');
	}

	const char* expected = "d5:dictsl"
		"d5:array1:47:integeri1e9:integer64i2e6:string1:36:vector1:5e"
		"d5:array1:47:integeri1e9:integer64i2e6:string1:36:vector1:5e"
		"ee";

	std::array<unsigned char, 1024> output;
	unsigned char* last = d.bencode(output);
	EXPECT_NE(static_cast<unsigned char*>(NULL), last);
	*last = '\0';
	EXPECT_STREQ(expected, reinterpret_cast<const char*>(output.data()));
}

TEST(bencode, bencoded_dict_escaped) {
	benc_dict_escaped d;
	d.spacesQ20andQ20QQ = 1;

	const char* expected = "d12:spaces and Qi1ee";

	std::array<unsigned char, 1024> output;
	unsigned char* last = d.bencode(output);
	EXPECT_NE(static_cast<unsigned char*>(NULL), last);
	*last = '\0';
	EXPECT_STREQ(expected, reinterpret_cast<const char*>(output.data()));
}
