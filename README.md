dstr
====

<b>Reference counted dynamic string and string containers</b>

Use it to ease the work with strings in C. Doxygen documentation is available by generating it from source.

Author: John Abrahamsen <JhnAbrhmsn@gmail.com>.

<b>Its simple:</b>
	
```C
	dstr *str = dstr_with_initial("concat me");
	dstr *str2 = dstr_with_initial(" to me");
	
	dstr_append(str, str2);
        printf("%s", dstr_from_dstr_to_cstr(str));
	dstr_decref(str);
	dstr_decref(str2);
```

Containers
----------

Implementations of both linked lists, dstr_list, and vectors, dstr_vector, are provided.

<b>It's also simple:</b>
	
```C
	dstr_vector *vector = dstr_vector_new();
	dstr_vector_push_back_decref(vector, dstr_with_initial("First string"));
	dstr_vector_push_back_decref(vector, dstr_with_initial("Second string"));
	
	while(!dstr_vector_is_empty(vector)){
		dstr_print(dstr_vector_back(vector));
		dstr_vector_pop_back(vector);
	}
	
	dstr_vector_decref(vector);
```
	
No more memory management is needed. dstr library provides functions for containers
that will either decrement reference or not, depending on usage.
	
Performance
-----------

All benchmarks are done on Ubuntu 11.04 and Lenovo W510.

Strings:

```C
    dstr *str = dstr_new();
    int i;

    for (i = 0; i < 1000000; i++){
        dstr_append_cstr(str, "concat me onto something...");
    }

    dstr_decref(str);
```

Without DST_MEM_CLEAR defined (zero bytes previous used string buffers before
freeing):

  Test: test_some_concating ... time used for 1000000 appends: 0 seconds 50 milliseconds. passed

With DST_MEM_CLEAR:

  Test: test_some_concating ... time used for 1000000 appends: 0 seconds 660 milliseconds. passed

Containers:

```C
    dstr *str = dstr_with_initial("append me");
    dstr_vector *vec = dstr_vector_new();
    int i;

    for (i = 0; i < 1000000; i++){
        dstr_vector_push_back(vec, str);
    }

    dstr_vector_decref(vec);
    dstr_decref(str);
```

Without DSTR_MEM_SECURITY  (boundary protection for operations get/set/remove operations):
  - Test: test_vector_append_speed ... time used for 1000000 push_back to vector: 0 seconds 60 milliseconds. passed
  - Test: test_vector_append_speed_no_prealloc ... time used for 1000000 push_back to vector: 0 seconds 70 milliseconds. passed
  - Test: test_vector_append_front_speed ... time used for 20000 push_front to vector: 2 seconds 140 milliseconds. passed
  - Test: test_vector_list_speed ... time used for 1000000 insertion to list: 0 seconds 110 milliseconds. passed

With DSTR_MEM_SECURITY defined:
  - Test: test_vector_append_speed ... time used for 1000000 push_back to vector: 0 seconds 70 milliseconds. passed
  - Test: test_vector_append_speed_no_prealloc ... time used for 1000000 push_back to vector: 0 seconds 70 milliseconds. passed
  - Test: test_vector_append_front_speed ... time used for 20000 push_front to vector: 2 seconds 140 milliseconds. passed
  - Test: test_vector_list_speed ... time used for 1000000 insertion to list: 0 seconds 120 milliseconds. passed


License
-------

Copyright 2012 John Abrahamsen <jhnabrhmsn@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. 


