let s:parsed_list = []
let s:engine_string = "hello\nyes\nhi"
let s:current_row = line(".")
let s:current_col = col(".")
let file_name = expand('%:t:r')
let extension = expand('%:e')
let g:POPUP_ID = 0

highlight Pmenu ctermbg=gray guibg=gray

" Autocommand Groups-------------------------

augroup popups
  autocmd TextChangedI * :call popup_close(g:POPUP_ID)
  autocmd TextChangedI * :let g:POPUP_ID=popup_create(s:parsed_list, #{line:"cursor+1", col: "cursor+2"})
augroup END

" Functions----------------------------------

function PrevElem()
  let last_index = len(s:parsed_list) - 1
  let temp = s:parsed_list[last_index]
  let i = last_index
  while i >= 0
    if i == 0
      let s:parsed_list[i] = temp
    else
      let next_index = i - 1
      let s:parsed_list[i] = s:parsed_list[next_index]
    endif
    let i -= 1
  endwhile
  call popup_close(g:POPUP_ID)
  let g:POPUP_ID = popup_create(s:parsed_list, #{line:"cursor+1", col: "cursor+2"})
endfunction

function NextElem()
  let last_index = len(s:parsed_list) - 1
  let temp = s:parsed_list[0]
  let i = 0
  while i < len(s:parsed_list)
    if i == last_index
      let s:parsed_list[i] = temp
    else
      let next_index = i + 1
      let s:parsed_list[i] = s:parsed_list[next_index]
    endif
    let i += 1
  endwhile
  call popup_close(g:POPUP_ID)
  let g:POPUP_ID = popup_create(s:parsed_list, #{line:"cursor+1", col: "cursor+2"})
endfunction

function Select()
  "let temp_char = getline('.')[col('.')-1] 
  execute "normal! i" . s:parsed_list[0]
endfunction

function Close_Popup()
  if g:POPUP_ID != 0
    call popup_close(g:POPUP_ID)
    let g:POPUP_ID = 0
  endif
endfunction

function Parse_Engine_String() 
  echom s:engine_string
  let s:parsed_list = split(s:engine_string, "\n")
endfunction

:call Parse_Engine_String()

" Keybinding Changes--------------------------

inoremap -r <esc>:call Close_Popup()<cr>i<Right>
inoremap <Down> <esc>:call NextElem()<cr>i<Right>
inoremap <Up> <esc>:call PrevElem()<cr>i<Right>
inoremap <TAB> <esc>bdw:call Select()<cr>i<Right>
inoremap <Right> <esc>:call Close_Popup()<cr>i<Right><Right>
inoremap <Left> <esc>:call Close_Popup()<cr>i


" inoremap <expr> <esc> POPUP_ID != 0 ? "\<esc>:call popup_close(POPUP_ID)<cr>:let POPUP_ID=0<cr>li" : "\<esc>"
" inoremap <expr> <Down> POPUP_ID != 0 ?  : "\<Down>" 
" inoremap <Down> call NextElem() 
" inoremap <expr> <Up> POPUP_ID != 0 ?  : "\<Up>"
" inoremap <expr> <BS> POPUP_ID != 0 ? "\<esc>:call popup_close(POPUP_ID)<cr>:let POPUP_ID=0<cr>li" : "\<BS>"
" inoremap <expr> <Del> POPUP_ID != 0 ? "\<esc>:call popup_close(POPUP_ID)<cr>:let POPUP_ID=0<cr>li" : "\<Del>"
" Use liball((libname), (funcname), (arg))
