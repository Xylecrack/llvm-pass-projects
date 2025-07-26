define i32 @sum_to_n(i32 %n) {
entry:
  %cmp = icmp sgt i32 %n, 0
  br i1 %cmp, label %loop, label %exit

loop:
  %i = phi i32 [0, %entry], [%i_next, %body]
  %sum = phi i32 [0, %entry], [%sum_next, %body]
  %cond = icmp slt i32 %i, %n
  br i1 %cond, label %body, label %after_loop

body:
  %sum_next = add i32 %sum, %i
  %i_next = add i32 %i, 1
  br label %loop

after_loop:
  br label %exit

exit:
  %final_sum = phi i32 [0, %entry], [%sum, %after_loop]
  ret i32 %final_sum
}
