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

set shiftwidth=1
set tabstop=1

let g:loaded_oni_interop_plugin = 1

function OniNotify(args)
    call rpcnotify(1, "oni_plugin_notify", a:args)
endfunction

function OniNotifyAutocmd(eventName)
    let context = OniGetContext()
    call OniNotify(["autocmd", a:eventName, context])
endfunction

function OniCommand(commandName)
    call OniNotify(["command", a:commandName])
endfunction


function OniUpdateVisualRange()
    let visualMode = mode()

    if visualMode == "\<C-v>"
        let visualMode = "vb"
    end

    if visualMode == "v" || visualMode == "V" || visualMode == "vb"
        let [selectionStartLine, selectionStartColumn] = getpos("v")[1:2]
        let [selectionEndLine, selectionEndColumn] = getpos(".")[1:2]
    else
        let [selectionStartLine, selectionStartColumn, selectionEndLine, selectionEndColumn] = [1, 1, 1, 1]
    end

    let visualRange = [visualMode, selectionStartLine, selectionStartColumn, selectionEndLine, selectionEndColumn]

    call OniNotify(["oni_visual_range", visualRange])
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
    autocmd! TextChanged * :call OniNotifyAutocmd("TextChanged")
    autocmd! TextChangedI * :call OniNotifyAutocmd("TextChangedI")
    autocmd! TextChangedP * :call OniNotifyAutocmd("TextChangedP")
augroup END

function OniGetContext()
    let bufferNumber = bufnr("%")
    let line = line(".")
    let column = virtcol(".")
    let modified = getbufvar(bufferNumber, "&modified")

    let context = [bufferNumber, line, column, modified]

    return context
endfunction

function OniGetBufferContext(bufnum)
    let l:context = {}
    let l:context.bufferNumber = a:bufnum
    let l:context.bufferFullPath = expand("#".a:bufnum.":p")
    let l:context.buftype = getbufvar(a:bufnum, "&buftype")
    let l:context.modified = getbufvar(a:bufnum, "&mod")
    let l:context.hidden = getbufvar(a:bufnum, "&hidden")

    return l:context
endfunction

nnoremap <silent> zz :<C-u>call OniCommand('oni.editorView.scrollToCursor')<CR>
nnoremap <silent> zb :<C-u>call OniCommand('oni.editorView.scrollToCursorBottom')<CR>
nnoremap <silent> zt :<C-u>call OniCommand('oni.editorView.scrollToCursorTop')<CR>
nnoremap <silent> H  :<C-u>call OniCommand('oni.editorView.moveCursorToTop')<CR>
nnoremap <silent> M  :<C-u>call OniCommand('oni.editorView.moveCursorToMiddle')<CR>
nnoremap <silent> L  :<C-u>call OniCommand('oni.editorView.moveCursorToBottom')<CR>
