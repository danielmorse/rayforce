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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include "../core/mmap.h"
#include "../core/rayforce.h"
#include "../core/format.h"
#include "../core/unary.h"
#include "../core/vm.h"
#include "../core/vector.h"
#include "../core/parse.h"
#include "../core/runtime.h"
#include "../core/cc.h"
#include "../core/debuginfo.h"
#include "../core/dict.h"
#include "../core/alloc.h"

#define LINE_SIZE 2048
#define PROMPT "> "
#define VERSION "0.0.1"
#define LOGO "\n\
  ▒█▀▀█ █▀▀█ █░░█ ▒█▀▀▀ █▀▀█ █▀▀█ █▀▀ █▀▀ | Version: %s\n\
  ▒█▄▄▀ █▄▄█ █▄▄█ ▒█▀▀▀ █░░█ █▄▄▀ █░░ █▀▀ | Documentation: https://singaraiona.github.io/rayforce\n\
  ▒█░▒█ ▀░░▀ ▄▄▄█ ▒█░░░ ▀▀▀▀ ▀░▀▀ ▀▀▀ ▀▀▀ | Official: https://github.com/singaraiona/rayforce\n\n"

static volatile bool_t running = true;

null_t usage()
{
    printf("%s%s%s", BOLD, YELLOW, "Usage: rayforce [-f] [file...]\n");
    exit(EXIT_FAILURE);
}

null_t print_logo()
{
    str_t logo = str_fmt(0, LOGO, VERSION);
    printf("%s%s%s", DARK_CYAN, logo, RESET);
    rf_free(logo);
}

null_t print_error(rf_object_t *error, str_t filename, str_t source, u32_t len)
{
    const str_t PADDING = "                                                  ";
    u16_t line_number = 0, i, l;
    str_t start = source;
    str_t end = NULL;
    str_t error_desc, lf = "", msg, p;
    span_t span = error->adt->span;

    switch (error->adt->code)
    {
    case ERR_INIT:
        error_desc = "init";
        break;
    case ERR_PARSE:
        error_desc = "parse";
        break;
    case ERR_FORMAT:
        error_desc = "format";
        break;
    case ERR_TYPE:
        error_desc = "type";
        break;
    case ERR_LENGTH:
        error_desc = "length";
        break;
    case ERR_INDEX:
        error_desc = "index";
        break;
    case ERR_ALLOC:
        error_desc = "alloc";
        break;
    case ERR_IO:
        error_desc = "io";
        break;
    case ERR_NOT_FOUND:
        error_desc = "not found";
        break;
    case ERR_NOT_EXIST:
        error_desc = "not exist";
        break;
    case ERR_NOT_IMPLEMENTED:
        error_desc = "not implemented";
        break;
    case ERR_STACK_OVERFLOW:
        error_desc = "stack overflow";
        break;
    case ERR_THROW:
        error_desc = "throw";
        break;
    default:
        error_desc = "unknown";
    }

    if (!source)
    {
        printf("%s** [E%.3d] error%s: %s\n %s-->%s %s:%d:%d\n    %s %s %s\n", TOMATO, error->adt->code, RESET,
               error_desc, CYAN, RESET, filename, span.end_line, span.end_column, TOMATO, as_string(error), RESET);
        return;
    }

    printf("%s** [E%.3d] error%s: %s\n %s-->%s %s:%d:%d\n    %s|%s\n", TOMATO, error->adt->code, RESET,
           error_desc, CYAN, RESET, filename, span.end_line, span.end_column, CYAN, RESET);

    while (1)
    {
        end = strchr(start, '\n');
        if (end == NULL)
        {
            end = source + len;
            lf = "\n";
        }

        u32_t line_len = end - start + 1;

        if (line_number >= span.start_line && line_number <= span.end_line)
        {
            printf("%.3d %s|%s %.*s", line_number, CYAN, RESET, line_len, start);

            // Print the arrow or span for the error
            if (span.start_line == span.end_line)
            {
                printf("%s    %s|%s ", lf, CYAN, RESET);
                for (i = 0; i < span.start_column; i++)
                    printf(" ");

                for (i = span.start_column; i <= span.end_column; i++)
                    printf("%s^%s", TOMATO, RESET);

                l = 0;
                msg = as_string(error);
                p = strtok(msg, "\n");
                while (p != NULL)
                {
                    if (!l)
                        printf("%*.*s %s%s%s\n", l, l, PADDING, TOMATO, p, RESET);
                    else
                        printf("%*.*s %s%s%s\n", l, l, PADDING, YELLOW, p, RESET);
                    p = strtok(NULL, "\n");
                    l = span.end_column - span.start_column + 8;
                }
            }
            else
            {
                if (line_number == span.start_line)
                {
                    printf("    %s|%s ", CYAN, RESET);
                    for (i = 0; i < span.start_column; i++)
                        printf(" ");

                    printf("%s^%s\n", TOMATO, RESET);
                }
                else if (line_number == span.end_line)
                {
                    for (i = 0; i < span.end_column + 6; i++)
                        printf(" ");

                    printf("%s^ %s%s\n", TOMATO, as_string(error), RESET);
                }
            }
        }

        if (line_number > span.end_line)
            break;

        line_number++;
        start = end + 1;
    }
}

rf_object_t parse_cmdline(i32_t argc, str_t argv[])
{
    i32_t opt, len;
    rf_object_t keys = vector_symbol(0), vals = list(0), str;

    for (opt = 1; opt < argc && argv[opt][0] == '-'; opt++)
    {
        switch (argv[opt][1])
        {
        case 'f':
            opt++;

            if (argv[opt] == NULL)
                usage();

            vector_push(&keys, symbol("file"));
            len = strlen(argv[opt]);
            str = string(len);
            strncpy(as_string(&str), argv[opt], len);
            vector_push(&vals, str);
            break;
        default:
            usage();
        }
    }

    argv += opt;

    return dict(keys, vals);
}

null_t repl(str_t name, parser_t *parser, str_t buf, i32_t len)
{
    rf_object_t parsed, compiled, executed;
    str_t formatted = NULL;

    parsed = parse(parser, name, buf);
    // printf("%s\n", rf_object_fmt(&parsed));

    if (is_error(&parsed))
    {
        print_error(&parsed, name, buf, len);
        rf_object_free(&parsed);
        return;
    }

    compiled = cc_compile(&parsed, &parser->debuginfo);
    if (is_error(&compiled))
    {
        print_error(&compiled, name, buf, len);
        rf_object_free(&parsed);
        rf_object_free(&compiled);
        return;
    }

    // printf("%s\n", vm_code_fmt(&compiled));

    // release rc's of parsed asap
    rf_object_free(&parsed);

    executed = vm_exec(&runtime_get()->vm, &compiled);
    rf_object_free(&compiled);

    if (is_error(&executed))
        print_error(&executed, name, buf, len);
    else if (!is_null(&executed))
    {
        formatted = rf_object_fmt(&executed);
        if (formatted != NULL)
        {
            printf("%s\n", formatted);
            rf_free(formatted);
        }
    }

    rf_object_free(&executed);

    return;
}

null_t int_handler(i32_t sig)
{
    UNUSED(sig);
    running = false;
}

i32_t main(i32_t argc, str_t argv[])
{
#if defined(__linux__) || defined(__APPLE__) && defined(__MACH__)
    struct sigaction sa;
    sa.sa_handler = int_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGINT, &sa, NULL) == -1)
        perror("Error: cannot handle SIGINT");
#endif

    runtime_init(0);

    rf_object_t args = parse_cmdline(argc, argv), filename, symfile = symbol("file"), file;
    str_t line, ptr;
    parser_t parser = parser_new();

#if defined(__linux__) || defined(__APPLE__) && defined(__MACH__)
    print_logo();
#endif

    line = (str_t)mmap_malloc(LINE_SIZE);

    // load file
    filename = dict_get(&args, &symfile);
    if (!is_null(&filename))
    {
        file = rf_fread(&filename);
        if (file.type == TYPE_ERROR)
            print_error(&file, as_string(&filename), NULL, 0);
        else
            repl(as_string(&filename), &parser, as_string(&file), file.adt->len);

        rf_object_free(&file);
    }
    // --

    rf_object_free(&filename);

    while (running)
    {
        printf("%s%s%s", GREEN, PROMPT, RESET);
        ptr = fgets(line, LINE_SIZE, stdin);
        if (!ptr)
            break;

        repl("top-level", &parser, line, LINE_SIZE);
    }

    rf_object_free(&args);
    parser_free(&parser);
    mmap_free(line, LINE_SIZE);

    runtime_cleanup();

    return 0;
}
