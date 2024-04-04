segment data
	FSCANF_FMT:
	bytes "%lf\0"
	
	FILENAME:
	bytes "number.txt\0"
	
	OPENMODE:
	bytes "r\0"
	
	ERROR_MSG:
	bytes "Error. Not able to open the file.\0"
	
	PRINTF_FMT:
	bytes "{\"name\":\"Zili Luo\",\"email\":\"zili.luo@student.manchester.ac.uk\",\"number1\":%.2lf,\"number2\":%.3le}\0"

segment text
	; stack layout:
	; 0:	f64	num
	; 8:	FILE*	fptr
	load_imm	q r0, 16
	add		q sp, sp, r0
	
	; fptr = fopen("number.txt", "r")
	load_imm	q r0, FILENAME	; r0 = fopen("number.txt", "r")
	vtoreal		r0, r0
	load_imm	q r1, OPENMODE
	vtoreal		r1, r1
	libc_call	fopen
	load_imm	q r1, 8		; fptr = r0
	store_dir	q r0, r1, vmem
	
	; if r0 == NULL
	cmp		q r0, r0
	b		_error_end, nz
	load_imm	q r0, ERROR_MSG	; printf(ERROR_MSG)
	vtoreal		r0, r0
	libc_call	printf
	load_imm	q r0, 1		; exit(1)
	libc_call	exit
	_error_end:
	
	; fscanf(fptr, "%lf", &num);
	; r0 is already fptr
	load_imm	q r1, FSCANF_FMT
	vtoreal		r1, r1
	load_imm	q r2, 0	; &num
	vtoreal		r2, r2
	libc_call	fscanf
	
	; fclose(fptr)
	load_imm	q r0, 8
	load_dir	q r0, r0, vmem
	libc_call	fclose
	
	; printf(PRINTF_FMT, num, num)
	load_imm	q r0, PRINTF_FMT
	vtoreal		r0, r0
	load_imm	q r2, 0
	load_dir	q r1, r2, vmem
	load_dir	q r2, r2, vmem
	libc_call	printf
	
	brk
