Welcome to ebb -- the Expressly Better Bencoder!
================================================

Just include ebb.hpp in your project and use the provided API to serialize your bencoded entities.

Sample syntax showcasing most of the features:

	#include <cassert>
	#include <cstdio>
	
	#include "ebb.hpp"
	
	using namespace ebb;
	
	int main() {
		unsigned char output[1024];
		std::array<unsigned char, 3> data1 = {{'e', 'f', 'g'}};
		std::vector<unsigned char> data2 = {{'h', 'i', 'j'}};
		unsigned char* last = bencoder(output, 1024)(
				bdict(
					// k_v is optional for clarity
					k_v("a", "b"),
					1, 2,
					k_v("list", blist("a", "b", "c", "d", data1, data2))
					)
				);
		assert(last);
		*last = '\0';
		// will print "d1:a1:bi1ei2e4:listl1:a1:b1:c1:d3:efg3:hijee"
		printf(reinterpret_cast<const char*>(output));
		return 0;
	}
