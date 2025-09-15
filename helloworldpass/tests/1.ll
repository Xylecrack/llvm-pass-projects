
define dso_local i32 @foo(i32 noundef %0) #0 {
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  store i32 %0, ptr %2, align 4
  store i32 0, ptr %3, align 4
  br label %4

4:                                                ; preds = %8, %1
  %5 = load i32, ptr %2, align 4
  %6 = add nsw i32 %5, -1
  store i32 %6, ptr %2, align 4
  %7 = icmp ne i32 %5, 0
  br i1 %7, label %8, label %11

8:                                                ; preds = %4
  %9 = load i32, ptr %2, align 4
  %10 = add nsw i32 %9, 1
  store i32 %10, ptr %3, align 4
  br label %4

11:                                               ; preds = %4
  %12 = load i32, ptr %3, align 4
  ret i32 %12
}


