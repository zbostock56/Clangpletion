let s:list = ["First", "Second", "FUCK", "Hello", "test", "haha"]


let s:current_row = line(".")
let s:current_col = col(".")
let file_name = expand('%:t:r')
let extension = expand('%:e')
let s:replacement = "Hello" " update to the replacement text that the user selects

let g:POPUP_ID = 0
highlight Pmenu ctermbg=gray guibg=gray

augroup popups
  autocmd TextChangedI * :call popup_close(g:POPUP_ID)
  autocmd TextChangedI * :let g:POPUP_ID=popup_create(s:list, #{line:"cursor+1", col: "cursor+2"})
augroup END

function PrevElem()
  let last_index = len(s:list) - 1
  let temp = s:list[last_index]
  let i = last_index
  while i >= 0
    if i == 0
      let s:list[i] = temp
    else
      let next_index = i - 1
      let s:list[i] = s:list[next_index]
    endif
    let i -= 1
  endwhile
  call popup_close(g:POPUP_ID)
  let g:POPUP_ID = popup_create(s:list, #{line:"cursor+1", col: "cursor+2"})
endfunction

function NextElem()
  let last_index = len(s:list) - 1
  let temp = s:list[0]
  let i = 0
  while i < len(s:list)
    if i == last_index
      let s:list[i] = temp
    else
      let next_index = i + 1
      let s:list[i] = s:list[next_index]
    endif
    let i += 1
  endwhile
  call popup_close(g:POPUP_ID)
  let g:POPUP_ID = popup_create(s:list, #{line:"cursor+1", col: "cursor+2"})
endfunction

function Select()
  "let temp_char = getline('.')[col('.')-1] 
  execute "normal! i" . s:list[0]
endfunction

function Close_Popup()
  if g:POPUP_ID != 0
    call popup_close(g:POPUP_ID)
    let g:POPUP_ID = 0
  endif
endfunction

"inoremap <expr> <esc> POPUP_ID != 0 ? "\<esc>:call popup_close(POPUP_ID)<cr>:let POPUP_ID=0<cr>li" : "\<esc>"
inoremap -r <esc>:call Close_Popup()<cr>i<Right>
inoremap <Down> <esc>:call NextElem()<cr>i<Right>
inoremap <Up> <esc>:call PrevElem()<cr>i<Right>
inoremap <TAB> <esc>bdw:call Select()<cr>i<Right>
inoremap <Right> <esc>:call Close_Popup()<cr>i<Right><Right>
inoremap <Left> <esc>:call Close_Popup()<cr>i
"inoremap <expr> <Down> POPUP_ID != 0 ?  : "\<Down>" 
" inoremap <Down> call NextElem() 
" inoremap <expr> <Up> POPUP_ID != 0 ?  : "\<Up>"
" inoremap <expr> <BS> POPUP_ID != 0 ? "\<esc>:call popup_close(POPUP_ID)<cr>:let POPUP_ID=0<cr>li" : "\<BS>"
" inoremap <expr> <Del> POPUP_ID != 0 ? "\<esc>:call popup_close(POPUP_ID)<cr>:let POPUP_ID=0<cr>li" : "\<Del>"




" Use liball((libname), (funcname), (arg))
