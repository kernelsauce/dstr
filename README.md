dstr
====

<b>Reference counted dynamic string and string containers</b>

Use it to ease the work with strings in C.

Author: John Abrahamsen <JhnAbrhmsn@gmail.com>.

<b>Its simple:</b>
	
	dstr *str = dstr_with_initial("concat me");
	dstr *str2 = dstr_with_initial(" to me");
	
	dstr_append(str, str2);
	dstr_print(str);
	dstr_decref(str);
	dstr_decref(str2);

Containers
----------

Implementations of both linked lists, dstr_list, and vectors, dstr_vector, are provided.

<b>It's also simple:</b>
	
	dstr_vector *vector = dstr_vector_new();
	dstr_vector_push_back_decref(vector, dstr_with_initial("First string"));
	dstr_vector_push_back_decref(vector, dstr_with_initial("Second string"));
	
	while(!dstr_vector_is_empty(vector)){
		dstr_print(dstr_vector_back(vector));
		dstr_vector_pop_back(vector);
	}
	
	dstr_vector_decref(vector);
	
No more memory management is needed. dstr library provides functions to containers
that will either decrement reference or not, depending on usage.
	
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


