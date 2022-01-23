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

  augroup closepop
    autocmd InsertLeave * :call popup_close(g:POPUP_ID)
    autocmd InsertLeave * :let POPUP_ID = 0
  augroup END  

  " Functions----------------------------------

  function PrevElem()
  if len(s:parsed_list) > 1
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
  endif
  endfunction

  function NextElem()
  if len(s:parsed_list) > 1 
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
  endif
  endfunction

  function Select()
  if len(s:parsed_list) != 0
    "let temp_char = getline('.')[col('.')-1] 
    execute "normal! a" . s:parsed_list[0] . "\<Esc>"
  endif
  endfunction

  function Close_Popup()
    if g:POPUP_ID != 0
      call popup_close(g:POPUP_ID)
      let g:POPUP_ID = 0
    endif
  endfunction

  function Get_Last_Token()
    let l:index = 0
    let l:last_occurance = -1
    
    while l:index < len(getline('.'))
      if getline('.')[l:index] == '.' || getline('.')[l:index] == '>'
        let l:last_occurance = l:index
      elseif getline('.')[l:index] == '='
        let l:last_occurance = -1
      endif
      let l:index += 1
    endwhile
    return l:last_occurance
  endfunction

  function Get_Last_Word()
    let l:last_word = "" 
    let l:col = Get_Last_Token()
    while l:col <= col('.')
      if getline(".")[l:col] != ' '
        let l:last_word = l:last_word . getline(".")[l:col]
      endif
      let l:col += 1
    endwhile
    return l:last_word
  endfunction

  function Parse_Engine_String()
    let s:current_row = line(".")
    if (Get_Last_Token() != -1) 
      let s:current_col = Get_Last_Token()
    else
      let s:current_col = col('.')
    endif
    let l:current_word = Get_Last_Word()
    echom l:current_word
    let s:engine_string = libcall("libclangpletion_unix.so", "complete", s:file_name . "." . s:extension . "\n" . s:current_row . "\n" . s:current_col . "\n" . l:current_word)
    let s:parsed_list = split(s:engine_string, "\n")
  endfunction

  " Keybinding Changes--------------------------

  
  inoremap <expr> <Del> g:POPUP_ID != 0 ? "<esc>:call Close_Popup()<cr>i<Right><Del>" : "\<Del>"
  inoremap <expr> <BS> g:POPUP_ID != 0 ? "<esc>:call Close_Popup()<cr>i<Right><BS>" : "\<BS>"
  inoremap <expr> <Down> g:POPUP_ID != 0 ? "<esc>:call NextElem()<cr>i<Right>" : "\<Down>"
  inoremap <expr> <Up> g:POPUP_ID != 0 ? "<esc>:call PrevElem()<cr>i<Right>" : "\<UP>"
  inoremap <expr> <TAB> g:POPUP_ID != 0 ? "<esc>:call Select()<cr>i<Right>" : "\<TAB>"
  inoremap <Right> <esc>:call Close_Popup()<cr>i<Right><Right>
  inoremap <Left> <esc>:call Close_Popup()<cr>i
endif
