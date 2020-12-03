function! s:hello1()
    echo "hello1"
endfunction

nnoremap <silent> <Plug>Hello1 :<C-U>call <SID>hello1()<CR>
