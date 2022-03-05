let s:lib_loc = expand('<sfile>:p:h')
let s:lib_name = ""
let s:parsed_list = []
let s:engine_string = ""
let s:help_string = ""
let s:current_row = line(".")
let s:current_col = col(".")
let s:file_name = expand('%')
let s:file_contents = ""
let s:extension = expand('%:e')

let g:POPUP_ID = 0
let g:HELPER_ID = 0

let s:operators = [ '(', '[', '>', '!', '~', '+', '-', '*', '&', '/', '%', '<', '=', '^', '|', '&', '?', ':', '.', ',' ]

if has('win32unix') || has ('win32')
  let s:plugin_loc = expand('<sfile>:p:h:h')[1] . ":" . strpart(expand('<sfile>:p:h:h'), 2)
  let s:lib_name = "libclangpletion.dll"
else
  let s:plugin_loc =  expand('<sfile>:p:h:h')
  let s:lib_name = "libclangpletion.so"
endif

if s:extension == "c"

  " Autocommand Groups-------------------------

  augroup popups
   autocmd TextChangedI * :call popup_close(g:POPUP_ID)
   autocmd TextChangedI * :call Parse_Engine_String()
   autocmd TextChangedI * :call Open_Popup()

   autocmd TextChangedI * :call Update_Function_Helper()
  augroup END

  augroup closepop
    autocmd InsertLeave * :call Close_Popup()
    autocmd InsertLeave * :call Close_Helper()
  augroup END

  autocmd VimLeave * :call libcall(s:lib_loc . "/" . s:lib_name, "free_memory", "")

  " Functions----------------------------------

  " ===== PrevElem() =====
  "
  " Updates the code completion popup options to switch the currently
  " selected option to the previous option in the list
  "
  " ======================
  function PrevElem()
  if len(s:parsed_list) > 1
    let last_index = len(s:parsed_list) - 1

    " Temporarily store the last element of the code completion list
    let temp = s:parsed_list[last_index]

    " Iterate through the code completion list, moving each element down one
    " position in the list, and bringing the last element in the list to the
    " top
    let i = last_index
    while i >= 0
      if i == 0
        " Move the last element to the top of the list
        let s:parsed_list[i] = temp
      else
        " Move all other elements down the list by 1 position
        let next_index = i - 1
        let s:parsed_list[i] = s:parsed_list[next_index]
      endif
      let i -= 1
    endwhile

    " Close and reopen the code completion popup to reflect the updated
    " positions
    call popup_close(g:POPUP_ID)
    let g:POPUP_ID = popup_create(s:parsed_list, #{line:"cursor+1", col: "cursor"})
  endif
  endfunction

  " ===== NextElem() =====
  "
  " Updates the code completion popup options to switch the currently selected
  " option to the next option in the list
  "
  " ======================
  function NextElem()
    if len(s:parsed_list) > 1
      let last_index = len(s:parsed_list) - 1

      " Temporarily store the first element of the code completion list
      let temp = s:parsed_list[0]

      " Iterate through the code completion list, moving each element up one
      " position in the list, and bringing the first element in the list to
      " the bottom
      let i = 0
      while i < len(s:parsed_list)
        if i == last_index
          " Move the first element in the list to the bottom of the list
          let s:parsed_list[i] = temp
        else
          " Move every other element up the list by 1 position
          let next_index = i + 1
          let s:parsed_list[i] = s:parsed_list[next_index]
        endif
        let i += 1
      endwhile

      " Close and reopen the code completion popup to reflect the updated
      " positions
      call popup_close(g:POPUP_ID)
      let g:POPUP_ID = popup_create(s:parsed_list, #{line:"cursor+1", col: "cursor"})
    endif
  endfunction

  " ===== Select() =====
  "
  " Selects an option from the code completion list and places it into the
  " current buffer
  "
  " ====================
  function Select()
    if len(s:parsed_list) != 0
      " Execute macro that deletes the previous word then places the first
      " option of the code completion list into the buffer
      execute "normal! bdwa" . s:parsed_list[0] . "\<Esc>"
    endif
  endfunction

  " ===== Get_Last_Token() =====
  "
  " Retrieves the latest position before the cursor where any type of C
  " operator is located, which is the position at which code completion will
  " be analyzed from. Return value is zero indexed.
  "
  " ============================
  function Get_Last_Token()
    let l:index = 0
    let l:last_occurance = -1

    let l:cur_line = getline(".")
    while l:index < col('.') - 1
      for i in s:operators
        if l:cur_line[l:index] == i
          let l:last_occurance = l:index + 1
          break
        endif
      endfor

      let l:index += 1
    endwhile
    return l:last_occurance
  endfunction

  " ===== Get_Last_Word() =====
  "
  " Returns the current word that is being typed into the buffer, stretching
  " from the most recent C operator to the position of the cursor
  "
  " ===========================
  function Get_Last_Word()
    let l:last_word = ""
    let l:col = Get_Last_Token()
    while l:col < col('.') - 1
      if getline(".")[l:col] != ' '
        let l:last_word = l:last_word . getline(".")[l:col]
      endif
      let l:col += 1
    endwhile
    return l:last_word
  endfunction

  " ===== Get_File_Contents() =====
  "
  " Returns the total file contents of the current buffer
  "
  " ===============================
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

  " ===== Parse_Engine_String() =====
  "
  " Generates the code completion list
  "
  " =================================
  function Parse_Engine_String()
    let s:current_row = line(".")
    if (Get_Last_Token() != -1)
      let s:current_col = Get_Last_Token() - 1
    else
      let s:current_col = col('.') - 1
    endif
    let l:current_word = Get_Last_Word()
    let s:file_contents = Get_File_Contents()
    if (len(l:current_word) > 0)
      let s:engine_string = libcall(s:lib_loc . "/" . s:lib_name, "complete", s:plugin_loc . "\n" . s:file_name . "\n" . s:current_row . "\n" . s:current_col . "\n" . l:current_word . s:file_contents)
    else
      let s:engine_string = ""
    endif
    let s:parsed_list = split(s:engine_string, "\n")
  endfunction

  " ====== Open_Popup() =====
  "
  " Opens the code completion popup menu
  "
  " =========================
  function Open_Popup()
    if len(s:parsed_list) > 0
      let g:POPUP_ID=popup_create(s:parsed_list, #{line:"cursor+1", col: "cursor"})
    endif
  endfunction

  " ===== Get_Func_Beginning() =====
  "
  " Returns the starting position of the current function that is being called
  " by the user, if it exists. Return value is zero indexed.
  "
  " ================================
  function Get_Func_Beginning()
    let l:index = col('.')
    let l:func_beginning = 0

    let l:opening_paren_found = 0
    let l:closing_paren_found = 0

    let l:cur_line = getline(".")
    while l:index >= 0
      if l:cur_line[l:index] == '(' && l:opening_paren_found == 0 && l:closing_paren_found == 0
        let l:opening_paren_found = 1
      elseif l:cur_line[l:index] == ')' && l:opening_paren_found == 0
        let l:closing_paren_found += 1
      elseif l:cur_line[l:index] == '(' && l:closing_paren_found > 0
        let l:closing_paren_found += -1
      elseif l:opening_paren_found == 1
        echom l:cur_line[l:index]
        for i in s:operators
          if l:cur_line[l:index] == i
            let l:func_beginning = l:index + 1
            break
          endif
        endfor

        if l:func_beginning != 0
          break
        endif
      endif
      let l:index += -1
    endwhile

    if (l:opening_paren_found == 1)
      return l:func_beginning
    else
      return col('.') - 1
    endif
  endfunction

  " ===== Get_Current_Function() =====
  "
  " Returns the name of the current function that is being typed by the user
  "
  " ==================================
  function Get_Current_Function()
    let l:current_function = ""
    let l:function_found = 0

    let l:col = Get_Func_Beginning()
    echom l:col
    while l:col < col('.') - 1
      if getline(".")[l:col] == '('
        let l:function_found = 1
        break
      endif

      if getline(".")[l:col] != ' '
        let l:current_function = l:current_function . getline(".")[l:col]
      endif
      let l:col += 1
    endwhile

    if l:function_found == 1
      return l:current_function
    else
      return ""
    endif
  endfunction

  " ===== Update_Function_Helper() =====
  "
  " Polls updates for generation of the function helper and handles when to
  " open and close the function helper
  "
  " ====================================
  function Update_Function_Helper()
    if g:HELPER_ID == 0
      call Gen_Function_Helper()
      call Open_Helper()
    elseif getline(".")[col('.') - 2] == ')' || getline(".")[col('.') - 2] == '('
      call popup_close(g:HELPER_ID)
      let g:HELPER_ID = 0
    endif
  endfunction

  " ===== Gen_Function_Helper() =====
  "
  " Generates the text of the function helper popup
  "
  " =================================
  function Gen_Function_Helper()
    let l:current_function = Get_Current_Function()
    if len(l:current_function) > 0
      let s:help_string = libcall(s:lib_loc . "/" . s:lib_name, "function_helper", s:file_name . "\n" . l:current_function . s:file_contents)
    else
      let s:help_string = ""
    endif
  endfunction

  function Open_Helper()
    let l:func_beginning = Get_Func_Beginning()
    let l:func_name = Get_Current_Function()
    let l:name_len = len(l:func_name)

    let l:col = l:func_beginning + l:name_len + 2

    if len(s:help_string) > 0
      let g:HELPER_ID=popup_create(s:help_string, #{line:"cursor-1", col:(l:col + 4)})
    endif
  endfunction

  " ===== Close_Popups() =====
  "
  " Close both code completion and function popup menus
  "
  " ==========================
  function Close_Popups()
    call Close_Popup()
    call Close_Helper()
  endfunction

  " ===== Close_Popup() =====
  "
  " Close code completion popup menu
  "
  " =========================
  function Close_Popup()
    if g:POPUP_ID != 0
      call popup_close(g:POPUP_ID)
      let g:POPUP_ID = 0
    endif
  endfunction

  " ===== Close_Helper() =====
  "
  " Close function helper popup menu
  "
  " ==========================
  function Close_Helper()
    if g:HELPER_ID != 0
      call popup_close(g:HELPER_ID)
      let g:HELPER_ID = 0
    endif
  endfunction

  " Keybinding Changes--------------------------


  inoremap <expr> <Del> g:POPUP_ID != 0 ? "<esc>:call Close_Popup()<cr>i<Right><Del>" : "\<Del>"
  inoremap <expr> <BS> g:POPUP_ID != 0 ? "<esc>:call Close_Popup()<cr>i<Right><BS>" : "\<BS>"
  inoremap <expr> <Down> g:POPUP_ID != 0 ? "<esc>:call NextElem()<cr>i<Right>" : "<esc>:call Close_Helper()<cr>i<Right><Down>"
  inoremap <expr> <Up> g:POPUP_ID != 0 ? "<esc>:call PrevElem()<cr>i<Right>" : "<esc>:call Close_Helper()<cr>i<Right><UP>"
  inoremap <expr> <TAB> g:POPUP_ID != 0 ? "<esc>:call Select()<cr>i<Right>" : "\<TAB>"
  inoremap <Right> <esc>:call Close_Popup()<cr>i<Right><Right>
  inoremap <Left> <esc>:call Close_Popup()<cr>i
endif
