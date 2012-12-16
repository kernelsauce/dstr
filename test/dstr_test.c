#include <CUnit/CUnit.h>
#include "dstr.h"

int main(){
    dstr *str = dstr_with_initial("test1234");
    dstr_print(str);

    return 0;
}
