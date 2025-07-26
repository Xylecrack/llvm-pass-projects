; fanin_ifelse.ll
define i32 @fanin_ifelse(i32 %x) {
entry:
  %cmp = icmp sgt i32 %x, 0
  br i1 %cmp, label %if.then, label %if.else

if.then:
  %add = add i32 %x, 1
  br label %if.end

if.else:
  %sub = sub i32 %x, 1
  br label %if.end

if.end:
  %phi = phi i32 [ %add, %if.then ], [ %sub, %if.else ]
  %mul = mul i32 %phi, 2
  ret i32 %mul
}
