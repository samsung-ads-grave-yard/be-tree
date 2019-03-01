/**
 * The MIT License (MIT).
 *
 * https://github.com/clibs/red-black-tree
 *
 * Copyright (c) 2003-2014 Julienne Walker (happyfrosty@hotmail.com) [Author]
 * Copyright (c) 2014 clibs, Jonathan Barronville (jonathan@scrapum.photos) [Maintainer], and contributors.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once

#include <stddef.h>

struct jsw_rbtree;

typedef int (*cmp_f) (const void *p1, const void *p2);

struct jsw_rbtree* jsw_rbnew (cmp_f cmp);
void jsw_rbdelete(struct jsw_rbtree* tree);
void* jsw_rbfind(struct jsw_rbtree* tree, void* data);
int jsw_rbinsert(struct jsw_rbtree* tree, void* data);
int jsw_rberase(struct jsw_rbtree* tree, void* data);
size_t jsw_rbsize(struct jsw_rbtree* tree);

