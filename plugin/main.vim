let s:parsed_list = []
let s:engine_string = "hello\nyes\nhi"
let s:current_row = line(".")
let s:current_col = col(".")
let s:file_name = expand('%:t:r')
let s:extension = expand('%:e')
let g:POPUP_ID = 0
if s:extension == "c"
  highlight Pmenu ctermbg=white guibg=black

  " Autocommand Groups-------------------------

  augroup popups
   autocmd TextChangedI * :call popup_close(g:POPUP_ID)
   autocmd TextChangedI * :call Parse_Engine_String()
   autocmd TextChangedI * :let g:POPUP_ID=popup_create(s:parsed_list, #{line:"cursor+1", col: "cursor"})
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
   let g:POPUP_ID = popup_create(s:parsed_list, #{line:"cursor+1", col: "cursor"})
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
    let g:POPUP_ID = popup_create(s:parsed_list, #{line:"cursor+1", col: "cursor"})
  endfunction

  function Select()
    "let temp_char = getline('.')[col('.')-1] 
    execute "normal! i " . s:parsed_list[0]
  endfunction

  function Close_Popup()
    if g:POPUP_ID != 0
      call popup_close(g:POPUP_ID)
      let g:POPUP_ID = 0
    endif
  endfunction

  function Parse_Engine_String()
    let s:current_row = line(".")
    let s:current_col = col(".")
    let s:engine_string = libcall("libclangpletion_mac.so", "complete", s:file_name . "." . s:extension . "\n" . s:current_row . "\n" . s:current_col)
    let s:parsed_list = split(s:engine_string, "\n")
  endfunction

  " Keybinding Changes--------------------------

  inoremap <expr> <BS> g:POPUP_ID != 0 ? "<esc>:call Close_Popup()<cr>i<Right><BS>" : "\<BS>"
  inoremap <expr> <Down> g:POPUP_ID != 0 ? "<esc>:call NextElem()<cr>i<Right>" : "\<Down>"
  inoremap <expr> <Up> g:POPUP_ID != 0 ? "<esc>:call PrevElem()<cr>i<Right>" : "\<UP>"
  inoremap <expr> <TAB> g:POPUP_ID != 0 ? "<esc>bdw:call Select()<cr>i<Right>" : "\<TAB>"
  inoremap <Right> <esc>:call Close_Popup()<cr>i<Right><Right>
  inoremap <Left> <esc>:call Close_Popup()<cr>i
endif
