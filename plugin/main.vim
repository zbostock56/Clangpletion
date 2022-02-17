let s:lib_loc = expand('<sfile>:p:h')
let s:lib_name = ""
let s:parsed_list = []
let s:engine_string = ""
let s:help_string = ""
let s:current_row = line(".")
let s:current_col = col(".")
let s:file_name = expand('%')
let s:extension = expand('%:e')

let g:POPUP_ID = 0
let g:HELPER_ID = 0

if has('win32unix') || has ('win32')
  let s:plugin_loc = expand('<sfile>:p:h:h')[1] . ":" . strpart(expand('<sfile>:p:h:h'), 2)
  let s:lib_name = "libclangpletion.dll"
else
  let s:plugin_loc =  expand('<sfile>:p:h:h')
  let s:lib_name = "libclangpletion.so"
endif

if s:extension == "c"
  highlight Pmenu ctermbg=white guibg=black

  " Autocommand Groups-------------------------

  augroup popups
   autocmd TextChangedI * :call popup_close(g:POPUP_ID)
   autocmd TextChangedI * :call Parse_Engine_String()
   autocmd TextChangedI * :call Open_Popup()
 "  autocmd TextChangedI * :call Open_Helper()
  augroup END

  augroup closepop
    autocmd InsertLeave * :call popup_close(g:POPUP_ID)
  "  autocmd InsertLeave * :call popup_close(g:HELPER_ID)
    autocmd InsertLeave * :let POPUP_ID = 0
  "  autocmd InsertLeave * :let HELPER_ID = 0
  augroup END

  autocmd VimLeave * :call libcall(s:lib_loc . "/" . s:lib_name, "free_memory", "")

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
    execute "normal! bdwa" . s:parsed_list[0] . "\<Esc>"
  endif
  endfunction

  function Close_Popup()
    if g:POPUP_ID != 0
      call popup_close(g:POPUP_ID)
      let g:POPUP_ID = 0
    endif
  endfunction

  function Close_Helper()
    if g:HELPER_ID != 0
      call popup_close(g:HELPER_ID)
      let g:HELPER_ID = 0
    endif
  endfunction

  function Get_Last_Token()
    let l:index = 0
    let l:last_occurance = -1

    while l:index < len(getline('.'))
      if getline('.')[l:index] == '.' || getline('.')[l:index] == '>' || getline('.')[l:index] == '(' || getline('.')[l:index] == '=' || getline('.')[l:index] == ','
        let l:last_occurance = l:index + 1
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

  function Get_File_Contents()
    let l:line_list = getline(1, "$")
    let l:content_str = ""
    let l:index = 0
    while l:index < len(l:line_list)
      let l:content_str = l:content_str . "\n" . l:line_list[l:index]
      let l:index += 1
    endwhile
    return l:content_str
  endfunction

  function Parse_Engine_String()
    let s:current_row = line(".")
    if (Get_Last_Token() != -1)
      let s:current_col = Get_Last_Token() - 1
    else
      let s:current_col = col('.') - 1
    endif
    let l:current_word = Get_Last_Word()
    let l:file_contents = Get_File_Contents()
    if (len(l:current_word) > 0)
      let s:engine_string = libcall(s:lib_loc . "/" . s:lib_name, "complete", s:plugin_loc . "\n" . s:file_name . "\n" . s:current_row . "\n" . s:current_col . "\n" . l:current_word . l:file_contents)
    else
      let s:engine_string = ""
    endif
    let s:parsed_list = split(s:engine_string, "\n")
  endfunction

  function Open_Popup()
    if len(s:parsed_list) > 0
      let g:POPUP_ID=popup_create(s:parsed_list, #{line:"cursor+1", col: "cursor"})
    endif
  endfunction

  function Open_Helper()
    let l:current_word = Get_Last_Word()
    echom l:current_word
    if (len(l:current_word) > 0)
      let s:help_string = libcall(s:lib_loc . "/" . s:lib_name, "function_helper", l:current_word)
    else
      let s:help_string = ""
    endif

    if len(s:help_string) > 0
      let g:HELPER_ID=popup_create(s:help_string, #{line:"cursor-1", col:"cursor"})
    endif
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
