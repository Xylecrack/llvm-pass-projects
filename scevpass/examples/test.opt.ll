; ModuleID = './examples/test.opt.ll'
source_filename = "examples/test.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define dso_local void @foo(ptr noundef %0, i32 noundef %1) #0 {
  %3 = sext i32 %1 to i64
  br label %4

4:                                                ; preds = %17, %2
  %indvars.iv = phi i64 [ %indvars.iv.next, %17 ], [ 0, %2 ]
  %5 = icmp slt i64 %indvars.iv, %3
  br i1 %5, label %6, label %18

6:                                                ; preds = %4
  %7 = getelementptr inbounds i32, ptr %0, i64 %indvars.iv
  %8 = trunc nuw nsw i64 %indvars.iv to i32
  store i32 %8, ptr %7, align 4
  %9 = add nuw nsw i64 %indvars.iv, 1
  %10 = getelementptr inbounds i32, ptr %0, i64 %indvars.iv
  %11 = getelementptr inbounds i8, ptr %10, i64 4
  %12 = trunc nuw nsw i64 %9 to i32
  store i32 %12, ptr %11, align 4
  %13 = add nuw nsw i64 %indvars.iv, 2
  %14 = getelementptr inbounds i32, ptr %0, i64 %indvars.iv
  %15 = getelementptr inbounds i8, ptr %14, i64 8
  %16 = trunc nuw i64 %13 to i32
  store i32 %16, ptr %15, align 4
  br label %17

17:                                               ; preds = %6
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 3
  br label %4, !llvm.loop !6

18:                                               ; preds = %4
  ret void
}

attributes #0 = { noinline nounwind uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{i32 7, !"frame-pointer", i32 2}
!5 = !{!"Debian clang version 19.1.7 (1+b1)"}
!6 = distinct !{!6, !7}
!7 = !{!"llvm.loop.mustprogress"}
