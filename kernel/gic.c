#include "gic.h"
#include "types.h"
#include "machine.h"
#include "tools.h"
#include "printf.h"
#include "timer.h"

static u32 gicv2_get_cpumask()
{
	// GICD_ITARGETSR0到GICD_ITARGETSR7 这几个寄存器是只读的，无法修改。
	// 当处理器读取这些寄存器时，寄存器的字段只返回与当前读取寄存器的处理器对应的值。(可以认为是CPU ID)
	u32 mask, i;
	for (i = mask = 0; i < 32; i += 4) {
		// 一个核的时候全部指向cpu0，大于1个核的时候全部指向cpu1
		mask = get32(GICD_ITARGETSRn + i);
		printf("a = %x\n", mask);
		mask |= mask >> 16;
		mask |= mask >> 8;
		if (mask)
			break;
	}
	return mask;
}

static void gicv2_dist_init()
{
	u32 cpumask;
	u32 type, nr_lines, nr_cpus;
	int i;

	green_println("Start initialize gicv2 distribution interface.");

	/* Disable the distributor */
	put32(GICD_CTLR, GICD_CTL_DISABLE);
	printf("GICD_IGROUPRn(Interrupt Group Registers): %x\n",
	       get32(GICD_IGROUPRn));

	/* Print GIC information */
	type = get32(GICD_TYPER);
	printf("GICD_TYPER(Interrupt Controller Type Register): 0x%x\n", type);
	// GIC支持的最大中断数
	nr_lines = ((type & GICD_TYPE_LINE_NR) + 1) * 32;
	// 已实现的CPU  interfaces的数量
	nr_cpus = 1 + ((type & GICD_TYPE_CPU_NR) >> GICD_TYPE_CPU_NR_SHIFT);
	printf("GICv2: %d irq(s), %d cpu(s)\n", nr_lines, nr_cpus);

	/* Set all global interrupts to this CPU only */
	cpumask = gicv2_get_cpumask(); // 获取CPU ID
	cpumask |= cpumask << 8;
	cpumask |= cpumask << 16;
	printf("cpumask: %u 0x%x\n", cpumask, cpumask);
	printf("Set GICD_ITARGETSRn(Interrupt Processor Targets Registers)\n");
	for (i = 32; i < nr_lines; i += 4) { //一个寄存器操作4个interrupt id
		put32(GICD_ITARGETSRn + REG32_SIZE * i / 4, cpumask);
		// printf("[0x%lx] = 0x%x\n", GICD_ITARGETSRn + REG32_SIZE + i / 4,
		//        get32(GICD_ITARGETSRn + REG32_SIZE + i / 4));
	}

	/* Set all global interrupts to be level triggered, active low */
	printf("Set GICD_ICFGRn(Interrupt Configuration Registers)\n");
	for (i = 32; i < nr_lines; i += 16) { // 1个寄存器操作16个interrupt id
		put32(GICD_ICFGRn + REG32_SIZE * i / 16,
		      GICD_INT_ACTLOW_LVLTRIG);
		// printf("[0x%lx] = 0x%x\n", GICD_ICFGRn + REG32_SIZE * i / 16,
		//        get32(GICD_ICFGRn + REG32_SIZE * i / 16));
	}

	/* Set priority on all global interrupts */
	printf("Set GICD_IPRIORITYRn(Interrupt Priority Registers)\n");
	for (i = 32; i < nr_lines; i += 4) { // 1个寄存器操作4个interrupt id
		// 8bit提供给优先级，但是一般只实现4bit
		// 设置为a0a0a0a0，即每个中断优先级为0xa = 10
		put32(GICD_IPRIORITYRn + REG32_SIZE * i / 4,
		      GICD_INT_DEF_PRI_X4);
		// printf("[0x%lx] = 0x%x\n",
		//        GICD_IPRIORITYRn + REG32_SIZE * i / 4,
		//        get32(GICD_IPRIORITYRn + REG32_SIZE * i / 4));
	}

	/*
	 * Deactivate and disable all SPIs. Leave the PPI and SGIs
	 * alone as they are in the redistributor registers on GICv3.
	 */
	printf("Deactivate and disable all SPIs\n");
	// 将所有SPI中断都停用（大于32的）
	for (i = 32; i < nr_lines; i += 32) { // 1个寄存器操作32个interrupt id
		// 写1 设置为deactive
		put32(GICD_ICACTIVERne + REG32_SIZE * i / 32,
		      GICD_INT_EN_CLR_X32);
		// 写1 禁止中断转发
		put32(GICD_ICENABLERn + REG32_SIZE * i / 32,
		      GICD_INT_EN_CLR_X32);
	}

	/* Turn on the distributor */
	// 启用中断转发（中断会根据优先级规则转发到CPU接口）
	put32(GICD_CTLR, GICD_CTL_ENABLE);
	printf("GICD enabled, GICD_CTLR: 0x%x\n", get32(GICD_CTLR));
	green_println("Complete initialize gicv2 distribution interface.");
	printf(NEWLINE);
}

static void gicv2_cpu_init()
{
	int i;
	u32 bypass;

	green_println("Start initialize gicv2 cpu interface.");
	/*
	 * Deal with the banked PPI and SGI interrupts - disable all
	 * private interrupts. Make sure everything is deactivated.
	 */
	printf("Deactivate and disable all PPIs\n");
	for (i = 0; i < 32; i += 32) {
		put32(GICD_ICACTIVERne + REG32_SIZE * i / 32,
		      GICD_INT_EN_CLR_X32);
		put32(GICD_ICENABLERn + REG32_SIZE * i / 32,
		      GICD_INT_EN_CLR_X32);
	}

	/* Set priority on PPI and SGI interrupts */
	printf("Set GICD_IPRIORITYRn(Interrupt Priority Registers)\n");
	for (i = 0; i < 32; i += 4) {
		put32(GICD_IPRIORITYRn + REG32_SIZE * i / 4,
		      GICD_INT_DEF_PRI_X4);
		// printf("[0x%lx] = 0x%x\n", GICD_IPRIORITYRn + i * 4 / 4,
		//        get32(GICD_IPRIORITYRn + REG32_SIZE * i / 4));
	}

	/* Ensure all SGI interrupts are now enabled */
	printf("Enable all SGI\n");
	// 写1使能转发， [15：0] SGI
	put32(GICD_ISENABLERn, GICD_INT_EN_SET_SGI);
	printf("GICD_ISENABLERn(Interrupt Set-Enable Registers): 0x%x\n",
	       get32(GICD_ISENABLERn));

	/* Don't mask by priority */
	put32(GICC_PMR, GICC_INT_PRI_THRESHOLD);
	printf("GICC_PMR(Interrupt Priority Mask Register): 0x%x\n",
	       get32(GICC_PMR));

	/* Finest granularity of priority */
	put32(GICC_BPR, 0); // 16 level priority
	printf("GICC_BPR(Binary Point Register): 0x%x\n", get32(GICC_BPR));
	printf("Set GICC_APRnc(Active Priorities Registers)\n");
	for (i = 0; i < 4; i++) {
		put32(GICC_APRnc + i * REG32_SIZE, 0);
		// printf("[0x%lx] = 0x%x\n", GICC_APRnc + i * REG32_SIZE,
		//        get32(GICC_APRnc + i * REG32_SIZE));
	}

	/* Turn on delivery */
	bypass = get32(GICC_CTLR);
	bypass &= GICC_DIS_BYPASS_MASK;
	put32(GICC_CTLR, bypass | GICC_CTRL_EOImodeNS | GICC_ENABLE);
	printf("GICC enabled, GICC_CTLR: 0x%x\n", get32(GICC_CTLR));
	green_println("Complete initialize gicv2 cpu interface.");
	printf(NEWLINE);
}

void gicv2_init()
{
	u32 cpuid = 0; // only core 0 is used

	yellow_println("Start initialize gicv2.");

	if (cpuid == 0)
		gicv2_dist_init();

	/* init the cpu interface (GICC) */
	gicv2_cpu_init();

	/* enable the timer's irq */
	// enable ppi
	put32(GICD_ISENABLERn, GICD_INT_EN_CLR_PPI);
	printf("GICD_ISENABLERn(Interrupt Set-Enable Registers): %x\n",
	       get32(GICD_ISENABLERn));

	yellow_println("Complete initialize gicv2.");
}

void gicv2_handle_irq()
{
	unsigned int irqnr = 0;
	unsigned int irqstat = 0;

	irqstat = get32(GICC_IAR);
	irqnr = irqstat & 0x3ff;

	printf("------irq %d\n", irqnr);
	switch (irqnr) {
	case GIC_INTID_EL1_PHYS_TIMER:
	case GIC_INTID_VIRT_TIMER:
		handle_timer_irq();
		break;
	default:
		break;
	}
	put32(GICC_EOIR, irqstat);
	put32(GICC_DIRc, irqstat);
}
