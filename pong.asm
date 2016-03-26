;Assembly from Project 1
;Includes code for function prototypes:
;                switchSign 
section .text
global _switchSign
_switchSign:
	mulsd xmm0, [sign]
	retsd
section .data
sign: dq -1.0