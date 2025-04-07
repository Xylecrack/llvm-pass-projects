; ModuleID = './examples/notcont3.opt.ll'
source_filename = "examples/notcont3.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define dso_local void @bar2(ptr noundef %0, i32 noundef %1) #0 {
  %3 = sext i32 %1 to i64
  br label %4

4:                                                ; preds = %10, %2
  %indvars.iv = phi i64 [ %indvars.iv.next, %10 ], [ %3, %2 ]
  %indvars.iv.next = add nsw i64 %indvars.iv, -1
  %5 = icmp sgt i64 %indvars.iv, 0
  br i1 %5, label %6, label %11

6:                                                ; preds = %4
  %7 = shl nsw i64 %indvars.iv.next, 1
  %8 = getelementptr inbounds i32, ptr %0, i64 %7
  %9 = trunc nsw i64 %indvars.iv.next to i32
  store i32 %9, ptr %8, align 4
  br label %10

10:                                               ; preds = %6
  br label %4, !llvm.loop !6

11:                                               ; preds = %4
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
