#include "../helpers/compile_helper.hpp"

TEST(StructMethods, PrintMethod) {
  BirdTest::TestOptions options;
  options.code = "  struct Person { \
                        name: str;\
                        fn say_name() {\
                            print self.name \
                        }\
                    }";

  options.after_interpret = [&](Interpreter &interpreter) {};

  options.after_compile = [&](std::string &output, CodeGen &codegen) {};

  ASSERT_TRUE(BirdTest::compile(options));
}