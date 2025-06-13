Here is a small C compiler wirtten in C++.

The main design follows the book *Writing a C Compiler*, by Nora Sandler.

I would like to note that the project is still a work in progress.

## Things Finished
- Simple framework, including a minimal *Lexer*, *Parser*, *Ir Generator* and *Code Generator*
- Unary Expression, including `-` and `~`. No pre/suffix dec/increment yet.
- Binary Expression, including arithmatic operation and bitwise operation.
- Logic Operation, including `&&`, `||`, `!`, relational operator and shortcutting.
- Local variable, assignment and declaration.
- Codegen is only for x86 platform, emmiting AT&T format asm.

## TODO List in th Near Future...
- Conditional statments.
- Suffix and prefix `--` and `++`
- Compound assignment such as `+=`

I would also like to implement a version in Rust, but it seems to be a very time-consuming task otz...