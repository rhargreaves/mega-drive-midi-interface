.global setjmp
.global longjmp

setjmp:
	moveal sp@(4),a0
	movel sp@(0),a0@(12)
	movel sp,a0@(8)
	moveml d2-d7/a2-a6,a0@(20)
	clrl d0
	rts

longjmp:
	moveal sp@(4),a0
	movel sp@(8),d0
	bne 1f
	movel &1,d0
1:
	moveml a0@(20),d2-d7/a2-a6
	moveal a0@(8),sp
	movel a0@(12),sp@
	rts
