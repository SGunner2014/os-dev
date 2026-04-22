global load_paging_directory

; Loads a paging directory table
; stack: [esp + 4] -> pdx addr
;        [esp + 0] -> return address
load_paging_directory:
    mov eax, [esp + 4]
    mov cr3, eax
    ret
