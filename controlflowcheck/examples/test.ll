; ModuleID = 'test'
source_filename = "test"

define i32 @main() {
entry:
  %a = alloca i32, align 4
  store i32 0, i32* %a, align 4
  %val = load i32, i32* %a, align 4
  %cmp = icmp eq i32 %val, 0
  br i1 %cmp, label %if.then, label %if.else

if.then:
  store i32 1, i32* %a, align 4
  br label %end

if.else:
  store i32 2, i32* %a, align 4
  br label %end

end:
  %res = load i32, i32* %a, align 4
  ret i32 %res
}
