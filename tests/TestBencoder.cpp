// Copyright (C) 2013 Igor Kaplounenko
// Licensed under MIT License

#include "ebb.hpp"

#include "gtest/gtest.h"

using namespace ebb;

TEST(ebb, integer) {
	unsigned char output[1024];
	unsigned char* last = bencoder(output, 1024)(
			3000ll
			);
	ASSERT_NE(static_cast<unsigned char*>(NULL), last);
	*last = '\0';
	EXPECT_STREQ("i3000e", reinterpret_cast<char*>(output));
}

TEST(ebb, integer_bounds) {
	unsigned char output[4];
	unsigned char* last = bencoder(output, 4)(
			3000ll
			);
	EXPECT_EQ(static_cast<unsigned char*>(NULL), last);
}

TEST(ebb, string) {
	unsigned char output[1024];
	unsigned char* last = bencoder(output, 1024)(
			"asdf"
			);
	ASSERT_NE(static_cast<unsigned char*>(NULL), last);
	*last = '\0';
	EXPECT_STREQ("4:asdf", reinterpret_cast<char*>(output));
}

TEST(ebb, string_bounds) {
	unsigned char output[4];
	unsigned char* last = bencoder(output, 4)(
			"asdf"
			);
	EXPECT_EQ(static_cast<unsigned char*>(NULL), last);
}

TEST(ebb, array) {
	unsigned char output[1024];
	std::array<unsigned char, 3> data1 = {{'\1', '\0', '\2'}};
	unsigned char* last = bencoder(output, 1024)(
			data1
			);
	ASSERT_NE(static_cast<unsigned char*>(NULL), last);
	EXPECT_EQ(0, memcmp("3:\x01\x00\x02", output, 3));
}

TEST(ebb, array_bounds) {
	unsigned char output[4];
	std::array<unsigned char, 3> data1 = {{'\1', '\0', '\2'}};
	unsigned char* last = bencoder(output, 4)(
			data1
			);
	EXPECT_EQ(static_cast<unsigned char*>(NULL), last);
}

TEST(ebb, vector) {
	unsigned char output[1024];
	std::vector<unsigned char> data1 = {{'\1', '\0', '\2'}};
	unsigned char* last = bencoder(output, 1024)(
			data1
			);
	ASSERT_NE(static_cast<unsigned char*>(NULL), last);
	EXPECT_EQ(0, memcmp("3:\x01\x00\x02", output, 3));
}

TEST(ebb, vector_bounds) {
	unsigned char output[4];
	std::vector<unsigned char> data1 = {{'\1', '\0', '\2'}};
	unsigned char* last = bencoder(output, 4)(
			data1
			);
	EXPECT_EQ(static_cast<unsigned char*>(NULL), last);
}

TEST(ebb, list) {
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

TEST(ebb, dict) {
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

TEST(ebb, key_types) {
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

TEST(ebb, fancy) {
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
