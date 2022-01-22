let list = ["First", "Second"] + [getline(".")]
let win_id = popup_create(list, #{
                                \ pos: 'botleft',
                                \ line: 'cursor-1',
                                \ col: 'cursor',
                                \ moved: 'WORD',
                                \ scrollbar: 'true',
				                        \ })

let current_row = line(".")
let current_col = col(".")
let file_name = expand('%:t:r')
let extension = expand('%:e')

echo "Current Row: " . current_row
echo "Current Col: " . current_col
echo "Current File Name: " . file_name
echo "File Ext: " . extension
" echo "Before the if"


" while current_col > 3
"   if current_col > 0
"     echo "The current column is greater than 0"
"   elseif current_col == col("$") - 1
"     echo Fuck
"   endif
" endwhile

" Use liball((libname), (funcname), (arg))
