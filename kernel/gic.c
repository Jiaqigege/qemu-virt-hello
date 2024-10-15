#include "gic.h"
#include "types.h"
#include "machine.h"
#include "tools.h"
#include "printf.h"
#include "timer.h"

static u32 gicv2_get_cpumask()
{
	u32 mask, i;
	for (i = mask = 0; i < 32; i += 4) {
		// 一个核的时候全部指向cpu0，大于1个核的时候全部指向cpu1
		mask = get32(GICD_ITARGETSRn + i);
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

	/* Disable the distributor */
	put32(GICD_CTLR, GICD_CTL_DISABLE);
	printf("GICD_IGROUPRn: %x\n", get32(GICD_IGROUPRn));

	/* Print GIC information */
	type = get32(GICD_TYPER);
	nr_lines = ((type & GICD_TYPE_LINE_NR) + 1) * 32;
	nr_cpus = 1 + ((type & GICD_TYPE_CPU_NR) >> GICD_TYPE_CPU_NR_SHIFT);
	printf("GICv2: %d irq(s), %d cpu(s)\n", nr_lines, nr_cpus);

	/* Set all global interrupts to this CPU only */
	cpumask = gicv2_get_cpumask();
	cpumask |= cpumask << 8;
	cpumask |= cpumask << 16;
	printf("cpumask: %u 0x%x\n", cpumask, cpumask);
	printf("Set GICD_ITARGETSRn\n");
	for (i = 32; i < nr_lines; i += 4) {
		put32(GICD_ITARGETSRn + i * 4 / 4, cpumask);
		printf("[0x%lx] = 0x%x\n", GICD_ITARGETSRn + i * 4 / 4,
		       get32(GICD_ITARGETSRn + i * 4 / 4));
	}

	/* Set all global interrupts to be level triggered, active low */
	printf("Set GICD_ICFGRn\n");
	for (i = 32; i < nr_lines; i += 16) {
		put32(GICD_ICFGRn + i / 4, GICD_INT_ACTLOW_LVLTRIG);
		printf("[0x%lx] = 0x%x\n", GICD_ICFGRn + i / 4,
		       get32(GICD_ICFGRn + i / 4));
	}

	/* Set priority on all global interrupts */
	printf("Set GICD_IPRIORITYRn\n");
	for (i = 32; i < nr_lines; i += 4) {
		put32(GICD_IPRIORITYRn + i, GICD_INT_DEF_PRI_X4);
		printf("[0x%lx] = 0x%x\n", GICD_IPRIORITYRn + i,
		       get32(GICD_IPRIORITYRn + i));
	}

	/*
	 * Deactivate and disable all SPIs. Leave the PPI and SGIs
	 * alone as they are in the redistributor registers on GICv3.
	 */
	printf("Deactivate and disable all SPIs\n");
	for (i = 32; i < nr_lines; i += 32) {
		put32(GICD_ICACTIVERne + i / 8, GICD_INT_EN_CLR_X32);
		put32(GICD_ICENABLERn + i / 8, GICD_INT_EN_CLR_X32);
	}

	/* Turn on the distributor */
	put32(GICD_CTLR, GICD_CTL_ENABLE);
	printf("GICD enabled, GICD_CTLR: 0x%x\n", get32(GICD_CTLR));
}

static void gicv2_cpu_init()
{
	int i;
	u32 bypass;

	/*
	 * Deal with the banked PPI and SGI interrupts - disable all
	 * private interrupts. Make sure everything is deactivated.
	 */
	printf("Deactivate and disable all PPIs\n");
	for (i = 0; i < 32; i += 32) {
		put32(GICD_ICACTIVERne + i / 8, GICD_INT_EN_CLR_X32);
		put32(GICD_ICENABLERn + i / 8, GICD_INT_EN_CLR_X32);
	}

	/* Set priority on PPI and SGI interrupts */
	printf("Set GICD_IPRIORITYRn\n");
	for (i = 0; i < 32; i += 4) {
		put32(GICD_IPRIORITYRn + i * 4 / 4, GICD_INT_DEF_PRI_X4);
		printf("[0x%lx] = 0x%x\n", GICD_IPRIORITYRn + i * 4 / 4,
		       get32(GICD_IPRIORITYRn + i * 4 / 4));
	}

	/* Ensure all SGI interrupts are now enabled */
	printf("Enable all SGI\n");
	put32(GICD_ISENABLERn, GICD_INT_EN_SET_SGI);
	printf("GICD_ISENABLERn: 0x%x\n", get32(GICD_ISENABLERn));

	/* Don't mask by priority */
	put32(GICC_PMR, GICC_INT_PRI_THRESHOLD);
	printf("GICC_PMR: 0x%x\n", get32(GICC_PMR));
	/* Finest granularity of priority */
	put32(GICC_BPR, 0);
	printf("GICC_BPR: 0x%x\n", get32(GICC_BPR));
	printf("Set GICC_APRnc\n");
	for (i = 0; i < 4; i++) {
		put32(GICC_APRnc + i * 4, 0);
		printf("[0x%lx] = 0x%x\n", GICC_APRnc + i * 4,
		       get32(GICC_APRnc + i * 4));
	}

	/* Turn on delivery */
	bypass = get32(GICC_CTLR);
	bypass &= GICC_DIS_BYPASS_MASK;
	put32(GICC_CTLR, bypass | GICC_CTRL_EOImodeNS | GICC_ENABLE);
	printf("GICC enabled, GICC_CTLR: 0x%x\n", get32(GICC_CTLR));
}

void gicv2_init()
{
	u32 cpuid = 0; // only core 0 is used

	if (cpuid == 0)
		gicv2_dist_init();

	/* init the cpu interface (GICC) */
	gicv2_cpu_init();

	/* enable the timer's irq */
	put32(GICD_ISENABLERn, GICD_INT_EN_CLR_PPI);
	printf("GICD_ISENABLERn: %x\n", get32(GICD_ISENABLERn));
}

void gicv2_handle_irq()
{
	unsigned int irqnr = 0;
	unsigned int irqstat = 0;

	irqstat = get32(GICC_IAR);
	irqnr = irqstat & 0x3ff;

	printf("irq %d\n", irqnr);
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
