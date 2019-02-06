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

function OniNotifyEvent(eventName)
    let context = OniGetContext()
    call OniNotify(["event", a:eventName, context])
endfunction

augroup OniEventListeners
    autocmd!
    autocmd! BufWritePre * :call OniNotifyEvent("BufWritePre")
    autocmd! BufWritePost * :call OniNotifyEvent("BufWritePost")
    autocmd! BufEnter * :call OniNotifyEvent("BufEnter")
    autocmd! BufRead * :call OniNotifyEvent("BufRead")
    autocmd! BufWinEnter * :call OniNotifyEvent("BufWinEnter")
    autocmd! BufDelete * :call OniNotifyEvent("BufDelete")
    autocmd! BufUnload * :call OniNotifyEvent("BufUnload")
    autocmd! BufWipeout * :call OniNotifyEvent("BufWipeout")
    autocmd! CursorMoved * :call OniNotifyEvent("CursorMoved")
    autocmd! CursorMovedI * :call OniNotifyEvent("CursorMovedI")
augroup END

function OniGetContext()
let context = {}
let context.bufferNumber = bufnr("%")
let context.line = line(".")
let context.column = col(".")

if exists("b:last_change_tick")
    let context.version = b:last_change_tick
endif

return context
endfunction
