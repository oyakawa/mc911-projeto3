

@.formatting.string = private constant [4 x i8] c"%d\0A\00"

define i32 @main() {
entry0:
  %tmp0 = alloca i32
  store i32 12, i32* %tmp0
  %tmp2 = load i32* %tmp0
  %tmp3 = add i32 %tmp2, 22
  ret i32 0
}