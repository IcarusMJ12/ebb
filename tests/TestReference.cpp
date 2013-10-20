#include <cstdio>

#include "gtest/gtest.h"

TEST(reference, bencode) {
	unsigned char output[1024];
	const unsigned char* expected = reinterpret_cast<const unsigned char*>
		("d1:a1:bi1ei2e4:listl1:a1:b1:c1:dee");
	char* current = reinterpret_cast<char*>(output);
	current += sprintf(current, "d1:a1:b");
	current += sprintf(current, "i%ie", 1);
	current += sprintf(current, "i%ie", 2);
	current += sprintf(current, "4:listl1:a1:b1:c1:dee");
	EXPECT_STREQ(reinterpret_cast<const char*>(expected),
			reinterpret_cast<const char*>(output));
}
