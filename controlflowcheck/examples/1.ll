define void @test() {
entry:
  br label %middle
middle:
  br label %exit
exit:
  ret void
}
