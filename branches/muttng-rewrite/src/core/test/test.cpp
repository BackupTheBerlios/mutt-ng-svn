#include <unit++/unit++.h>

using namespace unitpp;

#include "buffer_tests.h"
#include "hash_tests.h"

namespace {

	class core_tests : public suite {
		public:
			core_tests() : suite("core_tests") {
				suite::main().add("core_test_suite",new buffer_tests());
				suite::main().add("core_test_suite",new hash_tests());
			}
	};

	core_tests * theTest = new core_tests();

}
