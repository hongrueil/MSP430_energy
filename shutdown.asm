    .cdecls C, list, "msp430.h" ; this allows us to use C headers

    .text                       ; locates code in 'text' section

    .global shutdown_asm                ; declares 'add' as global

shutdown_asm: .asmfunc
	;bic #GIE, sr
	dint
	nop
	mov.b #PMMPW_H, &PMMCTL0_H
	bis.b #PMMREGOFF, &PMMCTL0_L
	bic.b #SVSHE, &PMMCTL0_L
	mov.b #000h, &PMMCTL0_H
	bis #CPUOFF+OSCOFF+SCG0+SCG1, sr
	nop

	.endasmfunc

	.end
