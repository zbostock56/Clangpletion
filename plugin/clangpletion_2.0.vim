" ========== Functions ==========

function Gen_Lib_Name()
 if has('win32unix') || has('win32')
    return "libclangpletion.dll"
  else
    return "libclangpletion.so"
  endif
endfunction

function Gen_Temp_Dir()
  let l:raw_path = expand("<sfile>:h:h") . "/tmp"

  if has('win32unix') || has('win32')
    let l:dirs = split(l:raw_path, '/')
    let l:win_path = l:dirs[0] . ':'

    let l:index = 1
    while (l:index < len(l:dirs))
      let l:win_path = l:win_path . "/" . l:dirs[l:index]

      let l:index = l:index + 1
    endwhile

    return l:win_path
  else
    return l:raw_path
  endif
endfunction

function Gen_OS_Specific_Dir(dir)
  let l:raw_path = a:dir

  if (has('win32unix') || has('win32')) && len(l:raw_path) > 0
    let l:dirs = split(l:raw_path, '/')
    let l:win_path = l:dirs[0] . ':'

    let l:index = 1
    while (l:index < len(l:dirs))
      let l:win_path = l:win_path . "/" . l:dirs[l:index]

      let l:index = l:index + 1
    endwhile

    return l:win_path
  else
    return l:raw_path
  endif
endfunction

function Populate_Include_List()
  let l:include_arr = split(system("bash " . s:lib_loc . "/get_default_includes.sh"), "::")

  if len(l:include_arr) > 0
    let s:include_list = "-I" . l:include_arr[0]
  else
    let s:include_list = ""
  endif

  let l:index = 1
  while l:index < len(l:include_arr)
    let s:include_list = s:include_list . "\n" . "-I" . l:include_arr[l:index]
    let l:index = l:index + 1
  endwhile
endfunction

function Add_Include_Path(path)
  if len(s:include_list) > 0
    let s:include_list = s:include_list . "-I" . a:path . "\n"
  else
    let s:include_list = "-I" . a:path . "\n"
  endif
endfunction

function Init()
  let s:filename = expand("%:t")
  let s:file_dir = Gen_OS_Specific_Dir(expand('%:p'))
  let s:include_dir = s:temp_dir . "/i_" . s:filename
  let s:func_helper = ""
  let s:cur_func_name = ""

  call Populate_Include_List()
endfunction

function Get_Last_Token(line)
  let l:token_pos = -1
  let l:index = 0

  while l:index < len(a:line)
    let l:cur_op = 0
    if Is_Operator(a:line[l:index]) == 1
      let l:token_pos = l:index
    endif

    let l:index = l:index + 1
  endwhile

  return l:token_pos + 1
endfunction

function Get_Word(start_col)
  let l:line = getline(line('.'))
  let l:word = ""

  let l:index = a:start_col - 1

  while (l:index < col('.') - 1)
    let l:word = l:word . l:line[l:index]

    let l:index = l:index + 1
  endwhile

  return l:word
endfunction

function Get_Func_Beginning_Index()
  let l:line = getline(line('.'))
  let l:index = col('.') - 2

  let l:paren_counter = 0

  let l:found_name = 0
  while l:found_name == 0 && l:index > 0
    if l:line[l:index] == ")"
      let l:paren_counter = l:paren_counter + 1
    elseif l:line[l:index] == "(" && l:paren_counter == 0
      let l:found_name = 1
    elseif l:line[l:index] == "("
      let l:paren_counter = l:paren_counter - 1
    endif
    let l:index = l:index - 1
  endwhile

  if l:found_name == 1
    while l:index >= 0
      if Is_Operator(l:line[l:index]) == 1
        return l:index + 1
      endif

      let l:index = l:index - 1
    endwhile
    return 0
  else
    return -1
  endif
endfunction

function Get_File_Contents()
  let l:lines = getline(0, "$")

  let l:contents = l:lines[0]

  let l:index = 1
  while l:index < len(l:lines)
    let l:contents = l:contents . "\n" . l:lines[l:index]

    let l:index = l:index + 1
  endwhile

  return l:contents
endfunction

function Update_File()
  let l:contents = Get_File_Contents()

  let l:init = libcall(s:lib_loc . "/" . s:lib_name, "init", s:temp_dir . "\n" . s:filename . "\n" . l:contents)
endfunction

function Func_Helper()
  let l:row = line('.')
  let l:col = Get_Func_Beginning_Index() + 1

  let l:line = getline(l:row)

  let l:func_name = ""
  if l:col != 0
    let l:index = l:col - 1
    while l:index < len(l:line)
      if l:line[l:index] == "("
        break
      endif
      let l:func_name = l:func_name . l:line[l:index]
      let l:index = l:index + 1
    endwhile
  endif

  if len(l:func_name) > 0 && l:func_name != s:cur_func_name
    let s:func_helper = libcall(s:lib_loc . "/" . s:lib_name, "func_helper", s:file_dir . "\n" . s:temp_dir . "\n" . s:filename . "\n" . l:func_name . "\n" . l:row . "\n" . l:col . "\n" . s:include_list)
    let s:cur_func_name = l:func_name
    call Refresh_Func_Popup()
  elseif len(l:func_name) == 0 && l:func_name != s:cur_func_name
    let s:func_helper = ""
    let s:cur_func_name = ""
    call Close_Func_Popup()
  elseif len(l:func_name) > 0 && l:func_name == s:cur_func_name
    call Refresh_Func_Popup()
  endif
endfunction

function Auto_Complete()
  let l:row = line('.')
  let l:line = getline(l:row)
  let l:col = Get_Last_Token(l:line) + 1

  let l:word = Get_Word(l:col)

  if len(l:word) > 0 || (l:col > 1 && (l:line[l:col - 2] == '.' || l:line[l:col - 2] == '>'))
    let s:comp_str = split(libcall(s:lib_loc . "/" . s:lib_name, "code_complete", s:file_dir . "\n" . s:temp_dir . "\n" . s:filename . "\n" . l:word . "\n" . l:row . "\n" . l:col . "\n" . s:include_list), "\n")
    let s:current_selection = 0
  else
    let s:comp_str = ""
  endif
endfunction

function Refresh_Func_Popup()
  call Close_Func_Popup()
  if len(s:func_helper) > 0
    let s:func_popup_id = popup_create(s:func_helper, #{
                            \ line: "cursor-1",
                            \ col: "cursor",
                            \ filter: "Func_Filter"
                            \ })
  endif
endfunction

function Refresh_Popup()
  call Close_Popup()
  if (len(s:comp_str) > 0)
    let s:autocomp_popup_id = popup_create(s:comp_str, #{
                                \ line: "cursor+1",
                                \ col: "cursor",
                                \ highlight: "CodeComp",
                                \ border: [0, 1, 0, 1],
                                \ borderhighlight: [ "CodeComp" ],
                                \ filter: 'Autocomp_Filter'
                                \ })
    call prop_add(s:current_selection + 1, 1, #{
          \ bufnr: winbufnr(s:autocomp_popup_id),
          \ end_col: len(s:comp_str[s:current_selection]) + 1,
          \ type: 'current_option'
          \ })
  endif
endfunction

function Select_Autocomp_Option()
  let l:option = s:comp_str[s:current_selection]
  let l:line = line('.')
  let l:col = Get_Last_Token(getline(l:line)) + 1
  let l:word = Get_Word(l:col)

  let l:deletes = ""
  let l:i = 0
  while l:i < len(l:word)
    let l:deletes = l:deletes . "\<Del>"
    let l:i = l:i + 1
  endwhile

  echom l:line
  echom l:col
  call cursor(l:line, l:col)

  execute "normal! i" . l:option
  call cursor(l:line, l:col + len(l:option))
  execute "normal! i" . l:deletes
  call cursor(l:line, l:col + len(l:option))

  call Close_Popup()
endfunction

function Close_Func_Popup()
  if s:func_popup_id != 0
    call popup_close(s:func_popup_id)
    let s:func_popup_id = 0
  endif
endfunction

function Close_Popup()
  if s:autocomp_popup_id != 0
    call popup_close(s:autocomp_popup_id)
    let s:autocomp_popup_id = 0
  endif
endfunction

function Autocomp_Filter(winid, key)
  if a:key == "\<Up>"
    if s:current_selection == 0
      let s:current_selection = len(s:comp_str) - 1
    else
      let s:current_selection = s:current_selection - 1
    endif
    call Refresh_Popup()
    return 1
  elseif a:key == "\<Down>"
    if s:current_selection == len(s:comp_str) - 1
      let s:current_selection = 0
    else
      let s:current_selection = s:current_selection + 1
    endif
    call Refresh_Popup()
    return 1
  elseif a:key == "\<Left>" || a:key == "\<Right>"
    call Close_Popup()
    return 0
  elseif a:key == "\<Tab>"
    call Select_Autocomp_Option()
    return 1
  else
    return 0
  endif
endfunction

function Func_Filter(winid, key)
  if a:key == "\<Up>" || a:key == "\<Down>"
    if s:autocomp_popup_id == 0
      call Close_Func_Popup()
    endif
    return 0
  elseif a:key == "\<Left>" || a:key == "\<Right>"
    call Close_Func_Popup()
    return 0
  else
    return 0
  endif
endfunction

function Is_Operator(char)
  let l:cur_op = 0

  while l:cur_op < len(s:operators)
    if a:char == s:operators[l:cur_op]
      return 1
    endif
    let l:cur_op = l:cur_op + 1
  endwhile
  return 0
endfunction
" ========== Variables ==========

let s:operators = [ '(', '[', '>', '!', '~', '+', '-', '*', '&', '/', '%', '<', '=', '^', '|', '?', ':', '.', ',', ' ' ]
let s:lib_loc = expand('<sfile>:p:h')
let s:lib_name = Gen_Lib_Name()
let s:temp_dir = Gen_OS_Specific_Dir(expand("<sfile>:h:h") . "/tmp")
let s:file_dir = Gen_OS_Specific_Dir(expand('%:p'))
let s:autocomp_popup_id = 0
let s:func_popup_id = 0

" ========== Auto commands ==========

augroup ChangedText
  autocmd TextChangedI *.[ch] :call Update_File()
  autocmd TextChangedI *.[ch] :call Func_Helper()
  autocmd TextChangedI *.[ch] :call Auto_Complete()
  autocmd TextChangedI *.[ch] :call Refresh_Popup()
augroup END

augroup BufChange
  autocmd BufEnter *.[ch] :call Close_Func_Popup()
  autocmd BufEnter *.[ch] :call Close_Popup()
  autocmd BufEnter *.[ch] :call Init()
augroup END

augroup LeaveInsert
  autocmd InsertLeave *.[ch] :call Close_Func_Popup()
  autocmd InsertLeave *.[ch] :call Close_Popup()
augroup END

" ========== Highlighting ==========

highlight CodeComp ctermbg=white ctermfg=black
highlight CurrentOption ctermbg=black ctermfg=white

" ========== Initialization ==========

if expand("%:e") == "c"
  call Init()
endif

call prop_type_add('current_option', #{
      \ highlight: 'CurrentOption'
      \ })
