" TODO:
" - Conditional keymapping ("How to write a conditional mapping in Vim")
" - Autocommand to make the script run everytime the cursor position changes
" - make replace take the input of what the user selects out of the popup

" :execute set completeopt+=popup
let s:list = ["First", "Second", "FUCK"]


let s:current_row = line(".")
let s:current_col = col(".")
let file_name = expand('%:t:r')
let extension = expand('%:e')
let s:replacement = "Hello" " update to the replacement text that the user selects

let g:POPUP_ID = 0
let g:CURRENT_INDEX = -1
highlight Pmenu ctermbg=gray guibg=gray

" let len = 15
" let propId = 1000
" call prop_type_add('popupMarker', {})
" call prop_add(current_row, current_col, #{
"                 \ length: len,
"                 \ type: 'popupMarker',
"                 \ id: propId,
"                 \ })
" let winid = popup_menu(list, #{
"                 " \ pos: 'botleft',
"                 " \ textprop: 'popupMarker',
"                 " \ textpropid: propId,
"                 " \ border: [],
"                 " \ padding: [0,1,0,1],
"                 " \ close: 'click',
"                 \ callback: 'SelectedResults',
"                 \ })       
" call popup_menu(s:list, #{
"                           \ callback: 'SelectedResults',
"                           \ })

augroup popups
  autocmd TextChangedI * :call popup_close(g:POPUP_ID)
  autocmd TextChangedI * :let g:POPUP_ID=popup_create(s:list, #{line:"cursor+1", col: "cursor+2"})
augroup END

function NextElem() 
  let last_index = len(s:list) - 1
  let temp = s:list[0]
  let s:list[0] = s:list[last_index]
  let s:list[last_index] = temp
  echom s:list
endfunction

function Close_Popup()
  if g:POPUP_ID != 0
    call popup_close(g:POPUP_ID)
    let g:POPUP_ID = 0
  else
    execute "<C-[>"
  endif
endfunction

"inoremap <expr> <esc> POPUP_ID != 0 ? "\<esc>:call popup_close(POPUP_ID)<cr>:let POPUP_ID=0<cr>li" : "\<esc>"
inoremap <esc> <C-[>call Close_Popup()<cr>li
"inoremap <expr> <Down> POPUP_ID != 0 ?  : "\<Down>" 
" inoremap <Down> call NextElem() 
" inoremap <expr> <Up> POPUP_ID != 0 ?  : "\<Up>"

" inoremap <expr> <BS> POPUP_ID != 0 ? "\<esc>:call popup_close(POPUP_ID)<cr>:let POPUP_ID=0<cr>li" : "\<BS>"
" inoremap <expr> <Del> POPUP_ID != 0 ? "\<esc>:call popup_close(POPUP_ID)<cr>:let POPUP_ID=0<cr>li" : "\<Del>"

" The special prefix "a:" tells Vim that the variable is a function argument.
" Remove one word at a time using :dw
" function RemoveAndReplace(replacement)
"   call cursor(current_row, current_col)
"   :silent !dw " Remove the word that the cursor is on
"   :silent !i
"   :silent !a a:replacement
" :endfunction 

" inoremap <press SHIFT><press Ctrl-M> call RemoveAndReplace(replacement)
" inoremap <expr> <TAB> pumvisible() ? "\<C-y>" : "\<C-g>u\<CR>"
" inoremap <expr> <Esc> pumvisible() ? "\<C-e>" : "\<Esc>"
" inoremap <expr> <C-j> pumvisible() ? "\<C-n>" : "\<Down>"
" inoremap <expr> <C-k> pumvisible() ? "\<C-p>" : "\<Up>"


" Use liball((libname), (funcname), (arg))
