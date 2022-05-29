.global setjmp
.global longjmp

setjmp:
	moveal 4(%sp),%a0
	movel 0(%sp),12(%a0)
	movel %sp,8(%a0)
	moveml %d2-%d7/%a2-%a6,20(%a0)
	clrl %d0
	rts

longjmp:
	moveal 4(%sp),%a0
	movel 8(%sp),%d0
	bne 1f
	movel &1,%d0
1:
	moveml 20(%a0),%d2-%d7/%a2-%a6
	moveal 8(%a0),%sp
	movel 12(%a0),%sp
	rts
