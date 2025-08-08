
# ArrayAccessPass – LLVM Store Instruction Reordering Optimization

## Purpose

This LLVM pass detects and reorders store instructions inside loops based on their memory address offsets.
It uses ScalarEvolution to compute offsets and rearranges store instructions in increasing memory order
if there are no unsafe dependencies, improving spatial locality and potential performance.

---

## Example from `examples/1.opt.ll`

### Before Running the Pass

The original IR contains three store instructions targeting offsets from the base array pointer:

```llvm
  %5 = getelementptr inbounds i32, ptr %0, i64 %indvars.iv
  %6 = getelementptr inbounds i8, ptr %5, i64 8
  store i32 0, ptr %6, align 4         ; ---> Address(A[i]) + 8 bytes

  %7 = getelementptr inbounds i32, ptr %0, i64 %indvars.iv
  store i32 1, ptr %7, align 4         ; ---> Address(A[i])

  %8 = getelementptr inbounds i32, ptr %0, i64 %indvars.iv
  %9 = getelementptr inbounds i8, ptr %8, i64 4
  store i32 2, ptr %9, align 4         ; ---> Address(A[i]) + 4 bytes

  br label %10
```

These store instructions are unordered with respect to memory addresses.

## After Running the Pass

The pass analyzes the store targets, determines their address offsets using SCEV, and reorders them:

```llvm
  %5 = getelementptr inbounds i32, ptr %0, i64 %indvars.iv
  %6 = getelementptr inbounds i8, ptr %5, i64 8
  %7 = getelementptr inbounds i32, ptr %0, i64 %indvars.iv
  %8 = getelementptr inbounds i32, ptr %0, i64 %indvars.iv
  %9 = getelementptr inbounds i8, ptr %8, i64 4

  store i32 1, ptr %7, align 4         ; ---> Address(A[i])
  store i32 2, ptr %9, align 4         ; ---> Address(A[i]) + 4 bytes
  store i32 0, ptr %6, align 4         ; ---> Address(A[i]) + 8 bytes

  br label %10
```

The store instructions are now sorted by increasing offset from the base pointer.
