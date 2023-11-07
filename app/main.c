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

#define _POSIX_C_SOURCE 1

#include "../core/runtime.h"
#include "../core/format.h"
#include "../core/util.h"
#include "../core/sys.h"

#define LINE_SIZE 2048
#define PROMPT "> "
#define LOGO "\
  RayforceDB: %d.%d %s\n\
  %s %d(MB)\n\
  Documentation: https://rayforcedb.com/\n\
  Github: https://github.com/singaraiona/rayforce\n"

nil_t print_logo()
{
  sys_info_t nfo = get_sys_info();
  str_t logo = str_fmt(0, LOGO, RAYFORCE_MAJOR_VERSION, RAYFORCE_MINOR_VERSION, __DATE__, nfo.cpu, nfo.mem);
  printf("%s%s%s", BOLD, logo, RESET);
  heap_free(logo);
}

i32_t main(i32_t argc, str_t argv[])
{
  i32_t code = -1;

  runtime_init(argc, argv);
  print_logo();
  code = runtime_run();
  runtime_cleanup();

  return code;
}
