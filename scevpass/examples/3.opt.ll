; ModuleID = './examples/3.opt.ll'
source_filename = "examples/3.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define dso_local void @loop_unsorted(ptr noundef %0, i32 noundef %1) #0 {
  br label %3

3:                                                ; preds = %12, %2
  %indvars.iv = phi i64 [ %indvars.iv.next, %12 ], [ 0, %2 ]
  %exitcond = icmp ne i64 %indvars.iv, 3
  br i1 %exitcond, label %4, label %13

4:                                                ; preds = %3
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %5 = shl nuw nsw i64 %indvars.iv, 1
  %6 = or disjoint i64 %5, 1
  %7 = getelementptr inbounds i32, ptr %0, i64 %6
  %8 = trunc nuw nsw i64 %indvars.iv.next to i32
  store i32 %8, ptr %7, align 4
  %9 = shl nuw nsw i64 %indvars.iv, 1
  %10 = getelementptr inbounds i32, ptr %0, i64 %9
  %11 = trunc nuw nsw i64 %indvars.iv to i32
  store i32 %11, ptr %10, align 4
  br label %12

12:                                               ; preds = %4
  br label %3, !llvm.loop !6

13:                                               ; preds = %3
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
!5 = !{!"Debian clang version 19.1.7 (3)"}
!6 = distinct !{!6, !7}
!7 = !{!"llvm.loop.mustprogress"}
