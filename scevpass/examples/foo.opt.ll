; ModuleID = './examples/foo.opt.ll'
source_filename = "examples/foo.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define dso_local void @tricky_disc(ptr noundef %0, i32 noundef %1) #0 {
  br label %3

3:                                                ; preds = %4, %2
  %.01 = phi i32 [ 0, %2 ], [ %11, %4 ]
  %.0 = phi i32 [ 0, %2 ], [ %13, %4 ]
  %.not = icmp eq i32 %.01, %1
  br i1 %.not, label %14, label %4

4:                                                ; preds = %3
  %5 = mul nuw nsw i32 %.0, 3
  %6 = and i32 %.0, 1
  %7 = sub nsw i32 %5, %6
  %8 = sext i32 %7 to i64
  %9 = getelementptr inbounds i32, ptr %0, i64 %8
  store i32 %.01, ptr %9, align 4
  %10 = and i32 %.01, 268435455
  %11 = add nuw nsw i32 %10, 1
  %12 = add nuw nsw i32 %.0, 2
  %13 = and i32 %12, 4095
  br label %3, !llvm.loop !6

14:                                               ; preds = %3
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
