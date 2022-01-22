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
