" init.vim
" Entry point for Oni interop
" Sets defaults for Oni2, and wires up some RPC for autocmds

if exists("g:loaded_oni_interop_plugin")
    finish
endif

set hidden
set nocursorline
set nobackup
set nowritebackup
set noswapfile

syntax off

let g:loaded_oni_interop_plugin = 1

function OniNotify(args)
    call rpcnotify(1, "oni_plugin_notify", a:args)
endfunction

function OniNotifyAutocmd(eventName)
    let context = OniGetContext()
    call OniNotify(["autocmd", a:eventName, context])
endfunction

augroup OniEventListeners
    autocmd!
    autocmd! BufWritePre * :call OniNotifyAutocmd("BufWritePre")
    autocmd! BufWritePost * :call OniNotifyAutocmd("BufWritePost")
    autocmd! BufEnter * :call OniNotifyAutocmd("BufEnter")
    autocmd! BufRead * :call OniNotifyAutocmd("BufRead")
    autocmd! BufWinEnter * :call OniNotifyAutocmd("BufWinEnter")
    autocmd! BufDelete * :call OniNotifyAutocmd("BufDelete")
    autocmd! BufUnload * :call OniNotifyAutocmd("BufUnload")
    autocmd! BufWipeout * :call OniNotifyAutocmd("BufWipeout")
    autocmd! CursorMoved * :call OniNotifyAutocmd("CursorMoved")
    autocmd! CursorMovedI * :call OniNotifyAutocmd("CursorMovedI")
augroup END

function OniGetContext()
    let bufferNumber = bufnr("%")
    let line = line(".")
    let column = col(".")

    let context = [bufferNumber, line, column]

    return context
endfunction


function OniGetBufferContext(bufnum)
    let l:context = {}
    let l:bufpath = bufname(a:bufnum)

    if strlen(l:bufpath)
        let l:context.bufferNumber = a:bufnum
        let l:context.bufferFullPath = expand("#".a:bufnum.":p")
        let l:context.filetype = getbufvar(a:bufnum, "&filetype")
        let l:context.buftype = getbufvar(a:bufnum, "&buftype")
        let l:context.modified = getbufvar(a:bufnum, "&mod")
        let l:context.hidden = getbufvar(a:bufnum, "&hidden")

        return l:context
    elseif -1 < index(['nofile','acwrite'], getbufvar(a:bufnum, '&buftype')) " scratch buffer
        return
    endif
endfunction
