#include "asm.h"
#include "registers.h"

#define CURRENTEL_EL1           (0b01 << 2)
#define CURRENTEL_EL2           (0b10 << 2)

#define CPACR_EL1_FPEN          (0b11 << 20)
#define ID_AA64PFR0_EL1_GIC     (0b1111 << 24)

#define CNTHCTL_EL2_EL1PCEN     (1 << 1)
#define CNTHCTL_EL2_EL1PCTEN    (1 << 0)
#define CPTR_EL2_RES1           0x33ff
#define HCR_EL2_RW              (1 << 31)
#define ICC_SRE_EL2_SRE         (1 << 0)
#define ICC_SRE_EL2_ENABLE      (1 << 3)
#define ICC_SRE_EL3_SRE         (1 << 0)
#define ICC_SRE_EL3_ENABLE      (1 << 3)

#define SCR_EL3_HCE             (1 << 8)
#define SCR_EL3_NS              (1 << 0)
#define SCR_EL3_RW              (1 << 10)

#define SPSR_ELX_DAIF           (0b1111 << 6)
#define SPSR_ELX_EL1H           (0b0101)

#define ICH_HCR_EL2             S3_4_C12_C11_0
#define ICC_SRE_EL2             S3_4_C12_C9_5
#define ICC_SRE_EL3             S3_6_C12_C12_5

.macro enable_mmu sctlr tmp
    mrs     \tmp, \sctlr
    /* Enable MMU */
    orr     \tmp, \tmp, #SCTLR_EL1_M
    /* Disable alignment checking */
    bic     \tmp, \tmp, #SCTLR_EL1_A
    bic     \tmp, \tmp, #SCTLR_EL1_SA0
    bic     \tmp, \tmp, #SCTLR_EL1_SA
    orr     \tmp, \tmp, #SCTLR_EL1_nAA
    /* Data accesses Cacheable */
    orr     \tmp, \tmp, #SCTLR_EL1_C
    /* Instruction access Cacheable */
    orr     \tmp, \tmp, #SCTLR_EL1_I
    msr     \sctlr, \tmp
    isb
.endm

.macro disable_mmu sctlr tmp
    mrs     \tmp, \sctlr
    /* Disable MMU */
    bic     \tmp, \tmp, #SCTLR_EL1_M
    /* Disable alignment checking */
    bic     \tmp, \tmp, #SCTLR_EL1_A
    bic     \tmp, \tmp, #SCTLR_EL1_SA0
    bic     \tmp, \tmp, #SCTLR_EL1_SA
    orr     \tmp, \tmp, #SCTLR_EL1_nAA
    /* Disable Data Cache */
    bic     \tmp, \tmp, #SCTLR_EL1_C
    /* Disable Instruction Cache */
    bic     \tmp, \tmp, #SCTLR_EL1_I
    msr     \sctlr, \tmp
    isb
.endm

BEGIN_FUNC(arm64_elX_to_el1)
	// https://developer.arm.com/documentation/ddi0601/2024-06/AArch64-Registers/CurrentEL--Current-Exception-Level?lang=en
	// EL, bits [3:2] Current Exception Level.
	// 0b00 EL0 User Mode            最低的执行级别，通常用于运行用户空间的应用程序。
	// 0b01 EL1 Kernel Mode          用于操作系统内核的执行级别，具有更高的权限
	// 0b10 EL2 Hypervisor Mode      用于虚拟化场景，比如运行虚拟机管理程序（Hypervisor）
	// 0b11 EL3 Secure Monitor Mode  最高的执行级别，通常用于运行安全监控程序（Secure Monitor）
    mrs     x9, CurrentEL

    // Check the current exception level.
    cmp     x9, CURRENTEL_EL1
    beq     .Ltarget	// EL1 跳转
    cmp     x9, CURRENTEL_EL2
    beq     .Lin_el2	// EL2 跳转
    // Otherwise, we are in EL3. // 其他的认为是EL3

    // Set EL2 to 64bit and enable the HVC instruction.
    mrs     x9, scr_el3
    mov     x10, SCR_EL3_NS | SCR_EL3_HCE | SCR_EL3_RW
    orr     x9, x9, x10
    msr     scr_el3, x9

    // Set the return address and exception level.
    adr     x9, .Ltarget
    msr     elr_el3, x9
    mov     x9, SPSR_ELX_DAIF | SPSR_ELX_EL1H
    msr     spsr_el3, x9

.Lin_el2:
    // Disable EL1 timer traps and the timer offset.
    mrs     x9, cnthctl_el2
    orr     x9, x9, CNTHCTL_EL2_EL1PCEN | CNTHCTL_EL2_EL1PCTEN
    msr     cnthctl_el2, x9
    msr     cntvoff_el2, xzr

    // Disable EL2 coprocessor traps.
    mov     x9, CPTR_EL2_RES1
    msr     cptr_el2, x9

    msr     hstr_el2, xzr
    // Disable stage 2 translations.
    msr     vttbr_el2, xzr

    // Disable EL1 FPU traps.
    mov     x9, CPACR_EL1_FPEN
    msr     cpacr_el1, x9

    //// Check whether the GIC system registers are supported.
    //mrs     x9, id_aa64pfr0_el1
    //and     x9, x9, ID_AA64PFR0_EL1_GIC
    //cbz     x9, .Lno_gic_sr

    //// Enable the GIC system registers in EL3 and EL2, and allow their use in EL1.
    //mrs     x9, ICC_SRE_EL3
    //mov     x10, ICC_SRE_EL3_SRE
    //orr     x9, x9, x10
    //msr     ICC_SRE_EL3, x9
    //isb
    //mrs     x9, ICC_SRE_EL2
    //mov     x10, ICC_SRE_EL2_ENABLE | ICC_SRE_EL2_SRE
    //orr     x9, x9, x10
    //msr     ICC_SRE_EL2, x9

    //// Disable the GIC virtual CPU interface.
    //msr     ICH_HCR_EL2, xzr

.Lno_gic_sr:
    disable_mmu sctlr_el1, x9

    // Set EL1 to 64bit.
    mov     x9, HCR_EL2_RW
    msr     hcr_el2, x9

    // Set the return address and exception level.
    adr     x9, .Ltarget
    msr     elr_el2, x9
    mov     x9, #(SPSR_ELX_DAIF | SPSR_ELX_EL1H)
    msr     spsr_el2, x9

    isb
    eret

.Ltarget:
    ret
END_FUNC(arm64_elX_to_el1)

BEGIN_FUNC(put32)
    str     w1, [x0]
    ret
END_FUNC(put32)

BEGIN_FUNC(get32)
    ldr     w0, [x0]
    ret
END_FUNC(get32)

BEGIN_FUNC(delay)
    subs    x0, x0, #1
    bne     delay
    ret
END_FUNC(delay)
