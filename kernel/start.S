#include "asm.h"
#include "machine.h"

.section ".text.boot"

BEGIN_FUNC(_start)
	// https://developer.arm.com/documentation/ddi0601/2024-06/AArch64-Registers/MPIDR-EL1--Multiprocessor-Affinity-Register?lang=en
	// mpidr_el1[7:0] 表示当前核在集群内的标识符，mpidr_el1[15:8] 表示集群的标识符。
	// 因此，mpidr_el1[7:0] 可以用于标识当前处理器核的 ID。
	// 下面的代码用于判断当前处理器是否为 0 号核，如果不是，则跳转到 hang 处挂起。

	// mrs(Move Register from Special register)
	// mpidr(Multiprocessor Affinity Register) 
	// x1 = mpidr_el1
	// x1 = x1 & 0b11
	mrs	x1, mpidr_el1
	and	x1, x1, #3

	# 如果核 ID 不为 0，则跳转到 hang
	cmp	x1, #0
	# not equal (x1 != 0) 跳转
	bne	hang

primary:
	bl	arm64_elX_to_el1

	// set exception vector
	// https://developer.arm.com/documentation/ddi0601/2024-06/AArch64-Registers/VBAR-EL1--Vector-Base-Address-Register--EL1-?lang=en
	// msr(Move to System Register)
	// vbar_el1(Vector Base Address Register) 存储EL1模式下的异常向量表的基地址
	// 将中断向量表地址存储到vbar_el1
	adr	x0, exception_vector
	msr	vbar_el1, x0

	// setup stack
	// 将stack地址加载到x0并设置栈指针sp
	ldr	x0, =stack
	mov	sp, x0

	// jump to C code
	b	main
	// never return

hang:
	// Wait For Interrupt
	// 低功耗状态，等待中断唤醒
	wfi
	b	hang
END_FUNC(_start)
