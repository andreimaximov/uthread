/**
 * Some literature on x86-64 context switching...
 *
 * https://eli.thegreenplace.net/2011/09/06/stack-frame-layout-on-x86-64/
 * https://samwho.co.uk/blog/2013/06/01/context-switching-on-x86/
 * https://github.com/freebsd/freebsd/blob/master/sys/x86/include/ucontext.h
 * https://en.wikipedia.org/wiki/X86_calling_conventions
 * https://en.wikipedia.org/wiki/FLAGS_register
 *
 * NOTE: We do not snapshot and restore all of the registers for performance
 * sake! As per the System V ABI, "If the callee wishes to use registers RBX,
 * RBP, and R12–R15, it must restore their original values before returning
 * control to the caller". Other registers are volatile so we can ignore them.
 */

.globl uthreadAsmContextSwap
uthreadAsmContextSwap:
  // Save callee preserved registers for the current context.
  movq %rbx, 0  (%rdi)
  movq %rbp, 8 (%rdi)
  movq %r12, 32 (%rdi)
  movq %r13, 40 (%rdi)
  movq %r14, 48 (%rdi)
  movq %r15, 56 (%rdi)

  // The top of the current stack should have the return address. Right under
  // that (at least as far as we can tell, compilers can do funky optimizations
  // with jmpq for tail calls) is the stack frame for the function we should
  // return to.
  leaq 8 (%rsp), %rax
  movq %rax    , 16 (%rdi)

  // Save the return address from the top of the stack.
  movq 0 (%rsp), %rax
  movq %rax    , 24 (%rdi)

  // Continue to load the target context.
  movq %rsi, %rdi

.globl uthreadAsmContextJump
uthreadAsmContextJump:
  // Load callee preserved registers for target context.
  movq 0  (%rdi), %rbx
  movq 8 (%rdi), %rbp
  movq 16 (%rdi), %rsp
  movq 32 (%rdi), %r12
  movq 40 (%rdi), %r13
  movq 48 (%rdi), %r14
  movq 56 (%rdi), %r15

  // The return address can be pushed to the top of the stack for return to pop.
  pushq 24 (%rdi)

  ret
