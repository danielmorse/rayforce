/*
 *   Copyright (c) 2024 Anton Kundenko <singaraiona@gmail.com>
 *   All rights reserved.

 *   Permission is hereby granted, free of charge, to any person obtaining a copy
 *   of this software and associated documentation files (the "Software"), to deal
 *   in the Software without restriction, including without limitation the rights
 *   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *   copies of the Software, and to permit persons to whom the Software is
 *   furnished to do so, subject to the following conditions:

 *   The above copyright notice and this permission notice shall be included in all
 *   copies or substantial portions of the Software.

 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *   SOFTWARE.
 */

#define TEST_ASSERT_EQ(lhs, rhs)                                                                                       \
    {                                                                                                                  \
        obj_p le = eval_str(lhs);                                                                                      \
        obj_p lns = obj_fmt(le, B8_TRUE);                                                                              \
        if (IS_ERROR(le)) {                                                                                            \
            obj_p fmt = str_fmt(-1, "Input error: %s\n -- at: %s:%d", AS_C8(lns), __FILE__, __LINE__);                 \
            TEST_ASSERT(0, AS_C8(lns));                                                                                \
            drop_obj(lns);                                                                                             \
            drop_obj(fmt);                                                                                             \
        } else {                                                                                                       \
            obj_p re = eval_str(rhs);                                                                                  \
            obj_p rns = obj_fmt(re, B8_TRUE);                                                                          \
            obj_p fmt = str_fmt(-1, "Expected %s, got %s\n -- at: %s:%d", AS_C8(rns), AS_C8(lns), __FILE__, __LINE__); \
            TEST_ASSERT(str_cmp(AS_C8(lns), lns->len, AS_C8(rns), rns->len) == 0, AS_C8(fmt));                         \
            drop_obj(fmt);                                                                                             \
            drop_obj(re);                                                                                              \
            drop_obj(le);                                                                                              \
            drop_obj(lns);                                                                                             \
            drop_obj(rns);                                                                                             \
        }                                                                                                              \
    }

test_result_t test_lang_basic() {
    TEST_ASSERT_EQ("null", "null");
    TEST_ASSERT_EQ("0x1a", "0x1a");
    TEST_ASSERT_EQ("[0x1a 0x1b]", "[0x1a 0x1b]");
    TEST_ASSERT_EQ("true", "true");
    TEST_ASSERT_EQ("false", "false");
    TEST_ASSERT_EQ("1", "1");
    TEST_ASSERT_EQ("1.1", "1.10");
    TEST_ASSERT_EQ("\"\"", "\"\"");
    TEST_ASSERT_EQ("'asd", "'asd");
    TEST_ASSERT_EQ("'", "0Ns");
    TEST_ASSERT_EQ("(as 'String ')", "\"\"");
    TEST_ASSERT_EQ("(as 'String ' )", "\"\"");
    TEST_ASSERT_EQ("\"asd\"", "\"asd\"");
    TEST_ASSERT_EQ("{a: \"asd\" b: 1 c: [1 2 3]}", "{a: \"asd\" b: 1 c: [1 2 3]}");
    TEST_ASSERT_EQ("{a: \"asd\" b: 1 c: [1 2 3] d: {e: 1 f: 2}}", "{a: \"asd\" b: 1 c: [1 2 3] d: {e: 1 f: 2}}");
    TEST_ASSERT_EQ("{a: \"asd\" b: 1 c: [1 2 3] d: {e: 1 f: 2 g: {h: 1 i: 2}}}",
                   "{a: \"asd\" b: 1 c: [1 2 3] d: {e: 1 f: 2 g: {h: 1 i: 2}}}");
    TEST_ASSERT_EQ("(list 1 2 3 \"asd\")", "(list 1 2 3 \"asd\")");
    TEST_ASSERT_EQ("(list 1 2 3 \"asd\" (list 1 2 3))", "(list 1 2 3 \"asd\" (list 1 2 3))");
    TEST_ASSERT_EQ("(list 1 2 3 \"asd\" (list 1 2 3 (list 1 2 3)))", "(list 1 2 3 \"asd\" (list 1 2 3 (list 1 2 3)))");
    TEST_ASSERT_EQ("(list 1 2 3)", "(list 1 2 3)");
    TEST_ASSERT_EQ("(enlist 1 2 3)", "[1 2 3]");

    PASS();
}

test_result_t test_lang_math() {
    TEST_ASSERT_EQ("(+ 1 2)", "3");
    TEST_ASSERT_EQ("(+ 1 2.2)", "3.20");
    TEST_ASSERT_EQ("(+ 1.1 2)", "3.10");
    TEST_ASSERT_EQ("(+ 1.1 2.2)", "3.30");
    TEST_ASSERT_EQ("(+ [1 2] 3)", "[4 5]");
    TEST_ASSERT_EQ("(+ 1 [2 3])", "[3 4]");
    TEST_ASSERT_EQ("(+ [1 2] [3 4])", "[4 6]");
    TEST_ASSERT_EQ("(+ [1.1 2.2] [3 4])", "[4.10 6.20]");
    TEST_ASSERT_EQ("(- 1 2)", "-1");
    TEST_ASSERT_EQ("(- [1 2] 3)", "[-2 -1]");
    TEST_ASSERT_EQ("(- 1 [2 3])", "[-1 -2]");
    TEST_ASSERT_EQ("(- [1 2] [3 4])", "[-2 -2]");
    TEST_ASSERT_EQ("(* 2 3)", "6");
    TEST_ASSERT_EQ("(* [2 3] 4)", "[8 12]");
    TEST_ASSERT_EQ("(* 2 [3 4])", "[6 8]");
    TEST_ASSERT_EQ("(* [2 3] [4 5])", "[8 15]");
    TEST_ASSERT_EQ("(/ 6 3)", "2");
    TEST_ASSERT_EQ("(/ [6 8] 2)", "[3 4]");
    TEST_ASSERT_EQ("(/ 6 [3 4])", "[2 1]");
    TEST_ASSERT_EQ("(/ [6 8] [2 4])", "[3 2]");
    TEST_ASSERT_EQ("(% 6 4)", "2");
    TEST_ASSERT_EQ("(% [6 8] 3)", "[0 2]");
    TEST_ASSERT_EQ("(% 6 [4 3])", "[2 0]");
    TEST_ASSERT_EQ("(% [6 8] [4 3])", "[2 2]");
    TEST_ASSERT_EQ("(div 6 3)", "2.00");
    TEST_ASSERT_EQ("(div [6 8] 4)", "[1.50 2.00]");
    TEST_ASSERT_EQ("(div 6 [3 4])", "[2.00 1.50]");
    TEST_ASSERT_EQ("(div [6 8] [4 3])", "[1.50 2.67]");
    TEST_ASSERT_EQ("((fn [x y] (+ x y)) 1 [2.3 4])", "[3.3 5.0]");
    TEST_ASSERT_EQ("(map count (list (list \"aaa\" \"bbb\")))", "[2]");
    TEST_ASSERT_EQ("(sum [-24 12 3])", "-9");
    TEST_ASSERT_EQ("(sum -24)", "-24");
    PASS();
}

test_result_t test_lang_query() {
    TEST_ASSERT_EQ(
        "(set t (table [sym price volume tape] (list [apl vod god] [102 99 203] [500 400 900] (list "
        "\"A\"\"B\"\"C\"))))",
        "(table [sym price volume tape] (list [apl vod god] [102 99 203] [500 400 900] (list \"A\"\"B\"\"C\")))");
    TEST_ASSERT_EQ("(at t 'sym)", "[apl vod god]");
    TEST_ASSERT_EQ("(at t 'price)", "[102 99 203]");
    TEST_ASSERT_EQ("(at t 'volume)", "[500 400 900]");
    TEST_ASSERT_EQ("(at t 'tape)", "(list \"A\"\"B\"\"C\")");
    TEST_ASSERT_EQ(
        "(set n 10)"
        "(set gds (take n (guid 3)))"
        "(set t (table [OrderId Symbol Price Size Tape Timestamp]"
        "(list gds"
        "(take n [apll good msfk ibmd amznt fbad baba])"
        "(as 'F64 (til n))"
        "(take n (+ 1 (til 3)))"
        "(map (fn [x] (as 'String x)) (take n (til 10)))"
        "(as 'Timestamp (til n)))))"
        "null",
        "null");
    TEST_ASSERT_EQ("(select {from: t by: Symbol})",
                   "(table [Symbol OrderId Price Size Tape Timestamp]"
                   "(list [apll good msfk ibmd amznt fbad baba]"
                   "(at gds (til 7)) [0 1 2 3 4 5 6.0] [1 2 3 1 2 3 1]"
                   "(list \"0\"\"1\"\"2\"\"3\"\"4\"\"5\"\"6\")"
                   "(at (at t 'Timestamp) (til 7))))");
    TEST_ASSERT_EQ("(select {from: t by: Symbol where: (== Price 3)})",
                   "(table [Symbol OrderId Price Size Tape Timestamp]"
                   "(list [ibmd] (at gds 3) [3.00] [1] (list \"3\") [2000.01.01D00:00:00.000000003]))");
    TEST_ASSERT_EQ("(select {from: t by: Symbol where: (== Price 99)})",
                   "(table [Symbol OrderId Price Size Tape Timestamp]"
                   "(list [] [] [] [] (list) []))");
    TEST_ASSERT_EQ("(select {s: (sum Price) from: t by: Symbol})",
                   "(table [Symbol s]"
                   "(list [apll good msfk ibmd amznt fbad baba]"
                   "[7.00 9.00 11.00 3.00 4.00 5.00 6.00]))");
    PASS();
}

test_result_t test_lang_update() { PASS(); }

test_result_t test_lang_serde() {
    TEST_ASSERT_EQ("(de (ser null))", "null");

    PASS();
}

test_result_t test_lang_literals() {
    // Test basic character literals
    TEST_ASSERT_EQ("'a'", "'a'");
    TEST_ASSERT_EQ("'z'", "'z'");
    TEST_ASSERT_EQ("'0'", "'0'");
    TEST_ASSERT_EQ("'9'", "'9'");

    // Test standard escape sequences in character literals
    TEST_ASSERT_EQ("'\\n'", "'\\n'");
    TEST_ASSERT_EQ("'\\r'", "'\\r'");
    TEST_ASSERT_EQ("'\\t'", "'\\t'");
    TEST_ASSERT_EQ("'\\\\'", "'\\\\'");
    TEST_ASSERT_EQ("'\\''", "'\\''");

    // Test octal escape sequences in character literals
    TEST_ASSERT_EQ("'\\001'", "'\\001'");  // SOH (Start of Heading)
    TEST_ASSERT_EQ("'\\002'", "'\\002'");  // STX (Start of Text)
    TEST_ASSERT_EQ("'\\003'", "'\\003'");  // ETX (End of Text)
    TEST_ASSERT_EQ("'\\007'", "'\\007'");  // BEL (Bell)
    TEST_ASSERT_EQ("'\\010'", "'\\010'");  // BS (Backspace)
    TEST_ASSERT_EQ("'\\011'", "'\\011'");  // HT (Horizontal Tab) - same as '\t'
    TEST_ASSERT_EQ("'\\012'", "'\\012'");  // LF (Line Feed) - same as '\n'
    TEST_ASSERT_EQ("'\\015'", "'\\015'");  // CR (Carriage Return) - same as '\r'
    TEST_ASSERT_EQ("'\\032'", "'\\032'");  // SUB (Substitute)

    // Test empty quoted symbol (single quote)
    TEST_ASSERT_EQ("'", "0Ns");  // Empty quoted symbol should be parsed as a null symbol

    // Test basic string literals
    TEST_ASSERT_EQ("\"Hello, World!\"", "\"Hello, World!\"");
    TEST_ASSERT_EQ("\"\"", "\"\"");  // Empty string
    TEST_ASSERT_EQ("\"123\"", "\"123\"");

    // Test standard escape sequences in string literals
    TEST_ASSERT_EQ("\"Hello\\nWorld\"", "\"Hello\\nWorld\"");
    TEST_ASSERT_EQ("\"Hello\\rWorld\"", "\"Hello\\rWorld\"");
    TEST_ASSERT_EQ("\"Hello\\tWorld\"", "\"Hello\\tWorld\"");
    TEST_ASSERT_EQ("\"Hello\\\\World\"", "\"Hello\\\\World\"");
    TEST_ASSERT_EQ("\"Hello\\\"World\"", "\"Hello\\\"World\"");

    // Test octal escape sequences in string literals
    TEST_ASSERT_EQ("\"Hello\\001World\"", "\"Hello\\001World\"");  // SOH
    TEST_ASSERT_EQ("\"Hello\\002World\"", "\"Hello\\002World\"");  // STX
    TEST_ASSERT_EQ("\"Hello\\003World\"", "\"Hello\\003World\"");  // ETX
    TEST_ASSERT_EQ("\"Hello\\007World\"", "\"Hello\\007World\"");  // BEL
    TEST_ASSERT_EQ("\"Hello\\010World\"", "\"Hello\\010World\"");  // BS
    TEST_ASSERT_EQ("\"Hello\\011World\"", "\"Hello\\011World\"");  // HT
    TEST_ASSERT_EQ("\"Hello\\012World\"", "\"Hello\\012World\"");  // LF
    TEST_ASSERT_EQ("\"Hello\\015World\"", "\"Hello\\015World\"");  // CR

    // Test FIX protocol message with octal escapes
    TEST_ASSERT_EQ("\"8=FIX.4.2\\0019=006035=A49=CL156=TR34=152=20\"",
                   "\"8=FIX.4.2\\0019=006035=A49=CL156=TR34=152=20\"");

    // Test mixed escape sequences
    TEST_ASSERT_EQ("\"Mixed\\001\\n\\t\\015Escapes\"", "\"Mixed\\001\\n\\t\\015Escapes\"");

    PASS();
}