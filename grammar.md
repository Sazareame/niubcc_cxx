## Former Grammar

**Program**   -> **Function**

**Function**  -> int *identifier* (){**Statement**}

**Statement** -> return **Expr**;

**Expr**       -> *int* | **Unary** | (**Expr**)

**Unary**     -> [- | ~] **Expr**

*identifier*  -> 

*int*         ->

## ASDL of TACKY

**Program**(**Funcdef**)

**Funcdef**(*identifier*, **Inst**+)

**Inst** -> **Return**(**Val**) | **Unary**(*op*, **Val**, **Val**)

**Val** -> **Constant**(*int*) | **Var**(*identifier*)

*op* -> 