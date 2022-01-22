" TODO:
" - Conditional keymapping ("How to write a conditional mapping in Vim")
" - Autocommand to make the script run everytime the cursor position changes
" - make replace take the input of what the user selects out of the popup

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
let replacement = "Hello" " update to the replacement text that the user selects


echo "Current Row: " . current_row
echo "Current Col: " . current_col
echo "Current File Name: " . file_name
echo "File Ext: " . extension

" The special prefix "a:" tells Vim that the variable is a function argument.
" Remove one word at a time using :dw
function RemoveAndReplace(replacement)
  call cursor(current_row, current_col)
  :execute dw " Remove the word that the cursor is on
  :execute i
  :execute a a:replacement
:endfunction 

inoremap <press SHIFT><press Ctrl-M> RemoveAndReplace(replacement)


" Use liball((libname), (funcname), (arg))
