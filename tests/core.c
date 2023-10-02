/*
 *   Copyright (c) 2023 Anton Kundenko <singaraiona@gmail.com>
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

test_result_t test_str_match()
{
    TEST_ASSERT(str_match("brown", "br?*wn"), "brown\", \"br?*wn");
    TEST_ASSERT(str_match("broasdfasdfwn", "br?*wn"), "broasdfasdfwn\", \"br?*wn");
    TEST_ASSERT(str_match("browmwn", "br?*wn"), "browmwn\", \"br?*wn");
    TEST_ASSERT(str_match("brown", "[wertfb]rown"), "brown\", \"[wertfb]rown");
    TEST_ASSERT(str_match("brown", "[^wertf]rown"), "brown\", \"[^wertf]rown");
    TEST_ASSERT(!str_match("bro[wn", "[^wertf]ro[[wn"), "bro[wn\", \"[^wertf]ro[[wn");
    TEST_ASSERT(str_match("bro^wn", "[^wertf]ro^wn"), "bro^wn\", \"[^wertf]ro^wn");
    TEST_ASSERT(!str_match("brown", "br[?*]wn"), "brown\", \"br[?*]wn");

    PASS();
}
