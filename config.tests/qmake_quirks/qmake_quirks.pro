QMAKE_QUIRKS=

foo="bar"
bar=bar
!equals(foo,$$bar):QMAKE_QUIRKS+=keep_quotes

foo=$$quote(\n)
equals(foo,\n):QMAKE_QUIRKS+=quote_is_escape_expand

defineTest(foo) {
    count(1,1):QMAKE_QUIRKS+=func_in_join
    export(QMAKE_QUIRKS)
}
foo(foo bar)

defineReplace(bar) {
    foo=foo bar
    return($$foo)
}
foo=$$bar()
count(foo,1):QMAKE_QUIRKS+=func_out_join

foo="foo bar"
contains(QMAKE_QUIRKS,keep_quotes) {
    foo~=s/^"//
    foo~=s/"$//
}
bar=$$foo
count(bar,2):QMAKE_QUIRKS+=var_split

foo=foo bar
bar=$$foo
count(bar,1):QMAKE_QUIRKS+=var_join

foo=foo
bar=bar
baz=$$foo$$bar
equals(baz,foo\$$bar):QMAKE_QUIRKS+=var_parse

defineTest(cfoo) {
    !count(1,1):QMAKE_QUIRKS+=combine_func_args
    !count(ARGS,2):QMAKE_QUIRKS+=fucked_func_args
}
cfoo(1,2)

message(QMAKE_QUIRKS: $$QMAKE_QUIRKS)

