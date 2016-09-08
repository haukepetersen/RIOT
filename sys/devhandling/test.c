

#include <stdio.h>

#define I1NAME(x)      .i.foo = x

typedef struct {
    const char *foo;
    const char *bar;
} inner_t;

typedef struct {
    inner_t i;
    const char *blubb;
} outer_t;

const outer_t test = {
    I1NAME("foo"),
    .i.bar = "bar",
    .blubb = "blubb"
};

int main(void)
{
    printf("Hello\n");

    printf("i1 = %s\n", test.i.foo);

    return 0;
}


