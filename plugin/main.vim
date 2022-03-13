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
let g:selected_option = -1

let g:HELPER_ID = 0

let s:operators = [ '(', '[', '>', '!', '~', '+', '-', '*', '&', '/', '%', '<', '=', '^', '|', '&', '?', ':', '.', ',', ' ' ]

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

  " ===== Select_Option() =====
  "
  " Selects an option from the code completion list and places it into the
  " current buffer
  "
  " ====================
  function Select_Option(id, index)
    " Ensure the current option is valid and the length of completion list is
    " not 0
    if (a:index > 0) && (len(s:parsed_list) != 0)
      " Record the end of the current word that needs to be replaced
      let l:end = col('.')
      " Move the cursor to the beginning of the word to be replaced
      execute "normal! a \<Esc>b"
      " Record the beginning of the current word that needs to be replaced
      let l:beginning = col('.')

      " Iterate through the word, deleting the letters to be replaced
      let l:i = 0
      while l:i < (l:end - l:beginning)
        execute "normal! \<Del>"
        let l:i += 1
      endwhile

      " Insert the completion string
      execute "normal! i" . s:parsed_list[a:index - 1] . "\<Del>\<Del>"
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
    " Records the last occurance of a C operator. Initially set to -1 to
    " denote the beginning of the line
    let l:last_occurance = -1

    " Increment through the line until the current cursor position is reached,
    " checking if each character is a valid C operator
    let l:cur_line = getline(".")
    while l:index < col('.') - 1
      for i in s:operators
        " If a C operator has been detected update the last occurance variable
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
    " Records the word that the cursor is currently on
    let l:last_word = ""

    " Generate the position at which the word will begin, which is given by
    " the position of the most recent C operator before the cursor
    let l:col = Get_Last_Token()

    " Increment from the position of the last C operator to the position of
    " the cursor, updating the last word variable
    while l:col < col('.') - 1
      " Ignore whitespace
      if getline(".")[l:col] != ' '
        " Update the last word to include each character between the last
        " operator and the cursor
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
    " Retrieves the total file contents of the buffer, with each line being a
    " string in an array
    let l:line_list = getline(1, "$")

    " Records the actual file contents of the buffer
    let l:content_str = ""

    " Increment through the list of lines of the file, updating the content
    " string variable
    let l:index = 0
    while l:index < len(l:line_list)
      " For each line in the line list, append the line to the content string
      " variable with a new line before it
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
    " Retireve the current row position of the cursor
    let s:current_row = line(".")

    " Retrieve the column at which to execute code completion. Code completion
    " is intended to be executed after the most recent C operator before the
    " cursor
    if (Get_Last_Token() != -1)
      " If there is a most recent C operator, record the current column as the
      " location at the C operator
      let s:current_col = Get_Last_Token() - 1
    else
      " If there is not a most recent C operator, record the current column at
      " the column position of the cursor
      let s:current_col = col('.') - 1
    endif

    " Retrieve the current word at the cursor
    let l:current_word = Get_Last_Word()

    " Retrieve the file contents of the buffer
    let s:file_contents = Get_File_Contents()

    " If the word at the cursor is not blank, generate a code completion list
    if (len(l:current_word) > 0)
      " Call the external C library to generate a code completion list string,
      " passing the appropriate arguments
      let s:engine_string = libcall(s:lib_loc . "/" . s:lib_name, "complete", s:plugin_loc . "\n" . s:file_name . "\n" . s:current_row . "\n" . s:current_col . "\n" . l:current_word . s:file_contents)
    else
      let s:engine_string = ""
    endif

    " Once the code completion string is generated, split the string into a
    " list at each new line character, generating a finalized code completion
    " list
    let s:parsed_list = split(s:engine_string, "\n")
  endfunction

  " ====== Open_Popup() =====
  "
  " Opens the code completion popup menu
  "
  " =========================
  function Open_Popup()
    if len(s:parsed_list) > 0
      let g:selected_option = 1
      let g:POPUP_ID = popup_menu(s:parsed_list, #{
            \ pos: "topleft",
            \ line: "cursor+1",
            \ col: "cursor",
            \ border: [ 0, 0, 0, 0 ],
            \ mapping: 1,
            \ filter: 'Code_Comp_Filter',
            \ callback: 'Select_Option'
            \ })

      "let g:POPUP_ID=popup_create(s:parsed_list, #{line:"cursor+1", col: "cursor"})
    endif
  endfunction

  " ===== Code_Comp_Filter() =====
  "
  " Provides additional input filtering for the code completion popup menu
  "
  " ==============================
  function Code_Comp_Filter(id, key)
    if (a:key == "\<Left>") || (a:key == "\<Right>")
      call Close_Popup()
      return 1
    elseif (a:key == "\<Down>")
      if g:selected_option < len(s:parsed_list)
        let g:selected_option += 1
      endif

      return popup_filter_menu(a:id, a:key)
    elseif (a:key == "\<Up>")
      if g:selected_option > 1
        let g:selected_option -= 1
      endif

      return popup_filter_menu(a:id, a:key)
    else
      return 0
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

    " Records the position at which the most recent function before the cursor
    " begins
    let l:func_beginning = 0

    " Records whether the opening parenthesis after the function name has been
    " found, indicating that the the argument list has been fully traversed
    let l:opening_paren_found = 0

    " Records if a closing parethesis has been found, indicating that a
    " complementary opening parenthesis must be also found before the opening
    " parenthesis after the function name has been found
    let l:closing_paren_found = 0

    " Climb backward through the line, starting at the current cursor position
    " and heading to the very end of the line, looking for the beginning of
    " the function call
    let l:cur_line = getline(".")
    while l:index >= 0
      if l:cur_line[l:index] == '(' && l:opening_paren_found == 0 && l:closing_paren_found == 0
        " If the current character is an opening parenthesis, no other opening
        " parenthesis have been found, and there are no un-paired closing
        " parenthesis, indicate that the opening parenthesis at the beginning of
        " the function argument list has been found
        let l:opening_paren_found = 1

      elseif l:cur_line[l:index] == ')' && l:opening_paren_found == 0
        " If the current character is a closing parenthesis, and no other
        " opening parenthesis have been found, increment the number of un-paired
        " closing parenthesis by 1
        let l:closing_paren_found += 1

      elseif l:cur_line[l:index] == '(' && l:closing_paren_found > 0
        " If the current character if a closing parenthesis, and there exist
        " unpaired closing parenthesis, decrement the number of un-paired
        " closed parenthesis by 1, as the current character is the complement
        " to a found closed parenthesis

        let l:closing_paren_found += -1

      elseif l:opening_paren_found == 1
        " If the opening parenthesis that indicates the end of the argument
        " list has been found, begin looking for the most recent C operator,
        " which denotes the beginning of the function call
        echom l:cur_line[l:index]
        for i in s:operators
          " If the current character is a C operator, update the func
          " beginning variable to the position immediately after the C
          " operator, indicating the beginning of the function call
          if l:cur_line[l:index] == i
            let l:func_beginning = l:index + 1
            break

          endif
        endfor

        " If the most recent C operator has been found, indicating the
        " beginning of the function call, break from the loop and return
        if l:func_beginning != 0
          break
        endif
      endif
      let l:index += -1
    endwhile

    if (l:opening_paren_found == 1)
      " If the opening parenthesis, indicating the beginning of the current
      " call's argument list has been found, return the location at which the
      " function call begins
      return l:func_beginning
    else
      " If no opening parenthesis has been found, return the current cursor
      " position, indicating there is no current function call
      return col('.') - 1
    endif
  endfunction

  " ===== Get_Current_Function() =====
  "
  " Returns the name of the current function that is being typed by the user
  "
  " ==================================
  function Get_Current_Function()
    " Records the name of the current function call
    let l:current_function = ""

    " Records whether or not the function name has been fully populated
    let l:function_found = 0

    " Retrieve the beginning of the current function call
    let l:col = Get_Func_Beginning()

    " Iterate from the beginning of the current function call to the position
    " of the cursor, updating the current function variable
    while l:col < col('.') - 1
      " If the opening parenthesis denoting the beginning of the function
      " argument list has been found, break from the loop, as the name of the
      " function has been fully populated
      if getline(".")[l:col] == '('
        let l:function_found = 1
        break
      endif

      " Ignore whitespace
      if getline(".")[l:col] != ' '
        " Update the current function variable with each character
        let l:current_function = l:current_function . getline(".")[l:col]
      endif
      let l:col += 1
    endwhile

    if l:function_found == 1
      " If the function name has been fully populated, return the name of the
      " function
      return l:current_function
    else
      " If the function name has not been fully populated, return a blank
      " string, indicating there is no current function being typed
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
      " If helper ID is blank, indicating no function helper, try to generate
      " a function helper
      call Gen_Function_Helper()
      call Open_Helper()
    elseif getline(".")[col('.') - 2] == ')' || getline(".")[col('.') - 2] == '('
      " If the current function helper is open and a opening of closing
      " parenthesis is typed, denoting a new function call, or the close of
      " the current function call, close the old function helper
      call Close_Helper()
    else
      " Otherwise, close and reopen the function helper to update it's
      " position along with the cursor
      call Close_Helper()
      call Open_Helper()
    endif
  endfunction

  " ===== Gen_Function_Helper() =====
  "
  " Generates the text of the function helper popup
  "
  " =================================
  function Gen_Function_Helper()
    " Retrieve the name of the current function (if any)
    let l:current_function = Get_Current_Function()

    let s:current_row = line(".")
    let s:current_col = col('.') - 1

    if len(l:current_function) > 0
      " If the current function exists, make a call to the external C library to
      " generate a function helper for the function
      let s:help_string = libcall(s:lib_loc . "/" . s:lib_name, "function_helper", s:file_name . "\n" . s:current_row . "\n" . s:current_col . "\n". l:current_function . s:file_contents)
    else
      " If the current function does not exist, simply update the help string
      " to be blank
      let s:help_string = ""
    endif
  endfunction

  " ===== Open_Helper() =====
  "
  " Opens the function helper
  "
  " =========================
  function Open_Helper()
    let l:num_cols = winwidth(0)
    let l:cur_col = col('.')
    let l:x_offset = 0
    let l:y_offset = 1

    " Determines the appropriate column and line offset for the location of
    " the function helper based on the window and cursor position
    if (l:cur_col + len(s:help_string) > (l:num_cols - 4)) && (l:cur_col - len(s:help_string) > 0)
      let l:x_offset = len(s:help_string)
    elseif l:cur_col + len(s:help_string) > (l:num_cols - 4)
      let l:x_offset = len(s:help_string)
      let l:y_offset = (len(s:help_string) / l:num_cols) + 1
    endif

    if len(s:help_string) > 0
      let g:HELPER_ID=popup_create(s:help_string, #{line:"cursor-" . l:y_offset, col:"cursor-" . l:x_offset})
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

  inoremap <expr> <Tab> g:POPUP_ID != 0 ? "<esc>:call Select_Option(g:POPUP_ID, g:selected_option)<cr>i<Right>" : "\<Tab>"
  inoremap <expr> <Down> g:POPUP_ID != 0 ? "\<Down>" : "<esc>:call Close_Helper()<cr>i<Right><Down>"
  inoremap <expr> <Up> g:POPUP_ID != 0 ? "\<Up>" : "<esc>:call Close_Helper()<cr>i<Right><Up>"
endif
