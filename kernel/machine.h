#pragma once

#define COUNTER_FREQ_IN_HZ 0x3b9aca0

#define GIC_BASE (0x8000000)

#define GICD_BASE (GIC_BASE + 0x00000)
#define GICC_BASE (GIC_BASE + 0x10000)

/* GICD Registers */

// Distributor Control Register
#define GICD_CTLR (GICD_BASE + 0x000)
// Interrupt Controller Type Register
#define GICD_TYPER (GICD_BASE + 0x004)
// Distributor Implementer Identification Register
#define GICD_IIDR (GICD_BASE + 0x008)

// Interrupt Group Registers
#define GICD_IGROUPRn (GICD_BASE + 0x080)
// Interrupt Set-Enable Registers
#define GICD_ISENABLERn (GICD_BASE + 0x100)
// Interrupt Clear-Enable Registers
#define GICD_ICENABLERn (GICD_BASE + 0x180)
// Interrupt Set-Pending Registers
#define GICD_ISPENDRn (GICD_BASE + 0x200)
// Interrupt Clear-Pending Registers
#define GICD_ICPENDRn (GICD_BASE + 0x280)
// GICv2 Interrupt Set-Active Registers
#define GICD_ISACTIVERnd (GICD_BASE + 0x300)
// Interrupt Clear-Active Registers
#define GICD_ICACTIVERne (GICD_BASE + 0x380)
// Interrupt Priority Registers
#define GICD_IPRIORITYRn (GICD_BASE + 0x400)
// Interrupt Processor Targets Registers
#define GICD_ITARGETSRn (GICD_BASE + 0x800)
// Interrupt Configuration Registers
#define GICD_ICFGRn (GICD_BASE + 0xC00)
#define GICD_PPISR  (GICD_BASE + 0xD00) // not found in gicv2 doc
// Non-secure Access Control Registers, optional
#define GICD_NSACRne (GICD_BASE + 0xE00)
// Software Generated Interrupt Register
#define GICD_SGIR (GICD_BASE + 0xF00)
// SGI Clear-Pending Registers
#define GICD_CPENDSGIRne (GICD_BASE + 0xF10)
// SGI Set-Pending Registers
#define GICD_SPENDSGIRne (GICD_BASE + 0xF20)

/* GICC Registers */
// CPU Interface Control Register
#define GICC_CTLR (GICC_BASE + 0x0000)
// Interrupt Priority Mask Register
#define GICC_PMR (GICC_BASE + 00004)
// Binary Point Register
#define GICC_BPR (GICC_BASE + 0x0008)
// Interrupt Acknowledge Register
#define GICC_IAR (GICC_BASE + 0x000C)
// End of Interrupt Register
#define GICC_EOIR (GICC_BASE + 0x0010)
// Running Priority Register
#define GICC_RPR (GICC_BASE + 0x0014)
// Highest Priority Pending Interrupt Register
#define GICC_HPPIR (GICC_BASE + 0x0018)
// Aliased Binary Point Register
#define GICC_ABPRb (GICC_BASE + 0x001C)
// Aliased Interrupt Acknowledge Register
#define GICC_AIARc (GICC_BASE + 0x0020)
// Aliased End of Interrupt Register
#define GICC_AEOIRc (GICC_BASE + 0x0024)
// Aliased Highest Priority Pending Interrupt Register
#define GICC_AHPPIRc (GICC_BASE + 0x0028)
// Active Priorities Registers
#define GICC_APRnc (GICC_BASE + 0x00D0)
// Non-secure Active Priorities Registers
#define GICC_NSAPRnc (GICC_ABPRb + 0x00E0)
// CPU Interface Identification Register
#define GICC_IIDR (GICC_BASE + 0x00FC)
// Deactivate Interrupt Register
#define GICC_DIRc (GICC_BASE + 0x1000)

#define GICV_CTLR (GICV_BASE + 0x0) // ??

/* register value */
#define GICD_CTL_ENABLE         0x1
#define GICD_CTL_DISABLE        0x0
#define GICD_INT_ACTLOW_LVLTRIG 0x0
#define GICD_INT_EN_CLR_X32     0xffffffff
#define GICD_INT_EN_SET_SGI     0x0000ffff
#define GICD_INT_EN_CLR_PPI     0xffff0000
#define GICD_INT_DEF_PRI        0xa0
#define GICD_INT_DEF_PRI_X4                                    \
	((GICD_INT_DEF_PRI << 24) | (GICD_INT_DEF_PRI << 16) | \
	 (GICD_INT_DEF_PRI << 8) | GICD_INT_DEF_PRI)

/* Register bits */
#define GICD_TYPE_LINE_NR      0x01F
#define GICD_TYPE_CPU_NR_SHIFT 5
#define GICD_TYPE_CPU_NR       0x0E0
#define GICD_TYPE_SEC          0x400

#define GICC_ENABLE            0x1
#define GICC_INT_PRI_THRESHOLD 0xf0

#define GICC_CTRL_EOImodeNS_SHIFT 9
#define GICC_CTRL_EOImodeNS       (1 << GICC_CTRL_EOImodeNS_SHIFT)

#define GICC_IAR_INT_ID_MASK 0x3ff
#define GICC_INT_SPURIOUS    1023
#define GICC_DIS_BYPASS_MASK 0x1e0

#define GIC_PRI_IRQ 0xA0
#define GIC_PRI_IPI 0x90

/* GICD_SGIR defination */
#define GICD_SGIR_SGIINTID_SHIFT   0
#define GICD_SGIR_CPULIST_SHIFT    16
#define GICD_SGIR_LISTFILTER_SHIFT 24
#define GICD_SGIR_VAL(listfilter, cpulist, sgi)         \
	(((listfilter) << GICD_SGIR_LISTFILTER_SHIFT) | \
	 ((cpulist) << GICD_SGIR_CPULIST_SHIFT) |       \
	 ((sgi) << GICD_SGIR_SGIINTID_SHIFT))

#define GIC_INTID_EL1_PHYS_TIMER 30
#define GIC_INTID_EL3_PHYS_TIMER 29
#define GIC_INTID_VIRT_TIMER     27
#define GIC_INTID_EL2_PHYS_TIMER 26
