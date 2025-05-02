#include "../helpers/compile_helper.hpp"

TEST(ForInTest, NestedIterationTest) {
  BirdTest::TestOptions options;
  options.code = "var arr = [ [ 1, 2, 3 ], [ 4, 5, 6 ] ];"
                 "for col in iter(arr) {"
                 "    for row in iter(col) {"
                 "        print row;"
                 "    }"
                 "}";

  options.after_compile = [&](std::string &output, CodeGen &code_gen) {
    ASSERT_EQ(output, "1\n2\n3\n4\n5\n6\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(ForInTest, ArrayOfTypeInt) {
  BirdTest::TestOptions options;
  options.code = "var arr = [ 1, 2, 3 ];"
                 "for x in iter(arr) {"
                 " print x;"
                 "}";

  options.after_compile = [&](std::string &output, CodeGen &code_gen) {
    ASSERT_EQ(output, "1\n2\n3\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(ForInTest, ArrayOfTypeFloat) {
  BirdTest::TestOptions options;
  options.code = "var arr = [ 1.1, 2.2, 3.3 ];"
                 "for x in iter(arr) {"
                 " print x;"
                 "}";

  options.after_compile = [&](std::string &output, CodeGen &code_gen) {
    ASSERT_EQ(output, "1.1\n2.2\n3.3\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(ForInTest, WithTypeStr) {
  BirdTest::TestOptions options;
  options.code = "var s = \"hello\";"
                 "for x in iter(s) {"
                 " print x;"
                 "}";

  options.after_compile = [&](std::string &output, CodeGen &code_gen) {
    ASSERT_EQ(output, "h\ne\nl\nl\no\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(ForInTest, ContinueTest) {
  BirdTest::TestOptions options;
  options.code = "var arr = [ 1, 2, 3 ];"
                 "for x in iter(arr) {"
                 "    if x == 2 {"
                 "        continue;"
                 "    }"
                 "    print x;"
                 "}";

  options.after_compile = [&](std::string &output, CodeGen &code_gen) {
    ASSERT_EQ(output, "1\n3\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(ForInTest, BreakTest) {
  BirdTest::TestOptions options;
  options.code = "var arr = [ 1, 2, 3 ];"
                 "for x in iter(arr) {"
                 "    if x == 2 {"
                 "        break;"
                 "    }"
                 "    print x;"
                 "}";

  options.after_compile = [&](std::string &output, CodeGen &code_gen) {
    ASSERT_EQ(output, "1\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(ForInTest, IterWithMultiDimensionalArrayArg) {
  BirdTest::TestOptions options;
  options.code = "for row in iter([ [ 1, 2, 3 ], [ 4, 5, 6 ] ]) {"
                 "    for element in iter(row) {"
                 "print element;"
                 "}"
                 "}";

  options.after_compile = [&](std::string &output, CodeGen &code_gen) {
    ASSERT_EQ(output, "1\n2\n3\n4\n5\n6\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(ForInTest, IterWithArrayArgTypeInt) {
  BirdTest::TestOptions options;
  options.code = "for x in iter([ 1, 2, 3 ]) {"
                 "    print x;"
                 "}";

  options.after_compile = [&](std::string &output, CodeGen &code_gen) {
    ASSERT_EQ(output, "1\n2\n3\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(ForInTest, IterWithArrayArgTypeFloat) {
  BirdTest::TestOptions options;
  options.code = "for x in iter([ 1.1, 2.2, 3.3 ]) {"
                 "    print x;"
                 "}";

  options.after_compile = [&](std::string &output, CodeGen &code_gen) {
    ASSERT_EQ(output, "1.1\n2.2\n3.3\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(ForInTest, IterWithStringArg) {
  BirdTest::TestOptions options;
  options.code = "for x in iter(\"hello\") {"
                 "    print x;"
                 "}";

  options.after_compile = [&](std::string &output, CodeGen &code_gen) {
    ASSERT_EQ(output, "h\ne\nl\nl\no\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(ForInTest, IterWithEmptyArray) {
  BirdTest::TestOptions options;
  options.code = "const foo: int[] = [];"
                 "for x in iter(foo) {"
                 "    print x;"
                 "}";

  options.after_compile = [&](std::string &output, CodeGen &code_gen) {
    ASSERT_EQ(output, "\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}