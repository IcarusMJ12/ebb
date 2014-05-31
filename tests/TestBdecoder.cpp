// Copyright (C) 2013 Igor Kaplounenko
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
