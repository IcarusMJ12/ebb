#include "ebb.hpp"

#include "gtest/gtest.h"

using namespace ebb;

TEST(ebb, bencode) {
	unsigned char output[1024];
	std::array<unsigned char, 3> data1 = {{'e', 'f', 'g'}};
	std::vector<unsigned char> data2 = {{'h', 'i', 'j'}};
	const unsigned char* expected = reinterpret_cast<const unsigned char*>
		("d1:a1:bi1ei2e4:listl1:a1:b1:c1:d3:efg3:hijee");
	unsigned char* last = bencoder(output, 1024)(
			benc_dict(
				benc_k_v("a", "b"),
				benc_k_v(1, 2),
				benc_k_v("list", benc_list(
						"a", "b", "c", "d",
						data1,
						data2
						))
				));
	EXPECT_NE(static_cast<unsigned char*>(NULL), last);
	*last = '\0';
	EXPECT_STREQ(reinterpret_cast<const char*>(expected),
			reinterpret_cast<const char*>(output));
}

TEST(ebb, nested_list) {
	unsigned char output[1024];
	unsigned char* last = bencoder(output, 1024)
		(benc_list(
							 "a", "b", "c", "d"
							)
		);
	EXPECT_NE(static_cast<unsigned char*>(NULL), last);
	*last = '\0';
	EXPECT_STREQ("l1:a1:b1:c1:de", reinterpret_cast<const char*>(output));
}

TEST(ebb, bounds) {
	unsigned char output[8];
	unsigned char* last = bencoder(output, 8)
		("abcdefgh");
	EXPECT_EQ(NULL, last);
}
