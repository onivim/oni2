" A Neovim plugin that implements GUI helper commands
if !has('nvim') || exists('g:GuiLoaded')
  finish
endif
let g:GuiLoaded = 1

" Get the channel to the last UI in the uis_list
function! s:get_last_ui_chan()
	let uis = nvim_list_uis()
	if len(uis) == 0
		echoerr "No UIs are attached"
		return -1
	endif

	" Use the last UI in the list
	return uis[-1].chan
endfunction

" Close the GUI
function! GuiClose() abort
  call rpcnotify(0, 'Gui', 'Close')
endfunction

" Notify the GUI when exiting Neovim
augroup nvim_gui_shim
  autocmd!
  autocmd VimLeave * call GuiClose()
augroup END

" A replacement for foreground()
function! GuiForeground() abort
  call rpcnotify(0, 'Gui', 'Foreground')
endfunction

" Set maximized state for GUI window (1 is enabled, 0 disabled)
function! GuiWindowMaximized(enabled) abort
  call rpcnotify(0, 'Gui', 'WindowMaximized', a:enabled)
endfunction

" Set fullscreen state for GUI window (1 is enabled, 0 disabled)
function! GuiWindowFullScreen(enabled) abort
  call rpcnotify(0, 'Gui', 'WindowFullScreen', a:enabled)
endfunction

" Set GUI font
function! GuiFont(fname, ...) abort
  let force = get(a:000, 0, 0)
  call rpcnotify(0, 'Gui', 'Font', a:fname, force)
endfunction

" Set additional linespace
function! GuiLinespace(height) abort
  call rpcnotify(0, 'Gui', 'Linespace', a:height)
endfunction

" Configure mouse hide behaviour (1 is enabled, 0 disabled)
function! GuiMousehide(enabled) abort
  call rpcnotify(0, 'Gui', 'Mousehide', a:enabled)
endfunction

" The GuiFont command. For compatibility there is also Guifont
function s:GuiFontCommand(fname, bang) abort
  if a:fname ==# ''
    if exists('g:GuiFont')
      echo g:GuiFont
    else
      echo 'No GuiFont is set'
    endif
  else
    call GuiFont(a:fname, a:bang ==# '!')
  endif
endfunction
command! -nargs=? -bang Guifont call s:GuiFontCommand("<args>", "<bang>")
command! -nargs=? -bang GuiFont call s:GuiFontCommand("<args>", "<bang>")

function s:GuiLinespaceCommand(height) abort
  if a:height ==# ''
    if exists('g:GuiLinespace')
      echo g:GuiLinespace
    else
      echo 'No GuiLinespace is set'
    endif
  else
    call GuiLinespace(a:height)
  endif
endfunction
command! -nargs=? GuiLinespace call s:GuiLinespaceCommand("<args>")

function! s:GuiTabline(enable) abort
	call rpcnotify(0, 'Gui', 'Option', 'Tabline', a:enable)
endfunction
command! -nargs=1 GuiTabline call s:GuiTabline(<args>)

function! s:GuiPopupmenu(enable) abort
	call rpcnotify(0, 'Gui', 'Option', 'Popupmenu', a:enable)
endfunction
command! -nargs=1 GuiPopupmenu call s:GuiPopupmenu(<args>)

" GuiDrop('file1', 'file2', ...) is similar to :drop file1 file2 ...
" but it calls fnameescape() over all arguments
function GuiDrop(...)
	let l:fnames = deepcopy(a:000)
	let l:args = map(l:fnames, 'fnameescape(v:val)')
	exec 'drop '.join(l:args, ' ')
	if !has('nvim-0.2')
		doautocmd BufEnter
	endif
endfunction

function GuiName()
	if !has('nvim-0.3')
		return ''
	endif

	" Use the last UI in the list
	let ui_chan = s:get_last_ui_chan()
  if (ui_chan == -1)
    return
  endif

	let info = nvim_get_chan_info(ui_chan)
	return get(info.client, 'name', '')
endfunction

function s:ui_has_clipboard(idx, ui_info)
	let info = nvim_get_chan_info(a:ui_info.chan)
	let client_info = get(info, 'client', {})

	if get(client_info, 'type', 0) == 'ui'
		let attrs = get(client_info, 'attributes', {})
		return has_key(attrs, 'gui-clipboard')
	else
		return 0
	endif
endfunction

"Enable a GUI provided clipboard
function GuiClipboard()
	if !has("nvim-0.3.2")
		echoerr "UI clipboard requires nvim >=0.3.2"
		return
	endif

	let uis = nvim_list_uis()
	call filter(uis, funcref('s:ui_has_clipboard'))
	if len(uis) == 0
		echoerr "No UIs with clipboard support are attached"
		return
	endif
	let ui_chan = uis[-1].chan

    let g:clipboard = {
          \   'name': 'custom',
          \   'copy': {
          \      '+': {lines, regtype -> rpcnotify(ui_chan, 'Gui', 'SetClipboard', lines, regtype, '+')},
          \      '*': {lines, regtype -> rpcnotify(ui_chan, 'Gui', 'SetClipboard', lines, regtype, '*')},
          \    },
          \   'paste': {
          \      '+': {-> rpcrequest(ui_chan, 'Gui', 'GetClipboard', '+')},
          \      '*': {-> rpcrequest(ui_chan, 'Gui', 'GetClipboard', '*')},
          \   },
          \ }
	call provider#clipboard#Executable()
endfunction

" Directory autocommands for Treeview
" TODO : for the time being, the chdir events are broadcasted to all
"        UIs. Being able to select a "correct" UI would be an improvement.
"        But what is "the correct UI" ?
augroup guiDirEvents
    autocmd!
    autocmd DirChanged * call rpcnotify(0, 'Dir', getcwd())
    autocmd WinEnter * call rpcnotify(0, 'Dir', getcwd())
augroup END


" Notifies the TreeView widget of a Show or Hide event
function! s:TreeViewShowHide(show)
    call rpcnotify(0, 'Gui', 'TreeView', 'ShowHide', a:show)
endfunction

command! GuiTreeviewShow call <SID>TreeViewShowHide(1)
command! GuiTreeviewHide call <SID>TreeViewShowHide(0)
noremap <script> <Plug>GuiTreeviewShow :call <SID>TreeViewShowHide(1)
noremap <script> <Plug>GuiTreeviewHide :call <SID>TreeViewShowHide(0)
anoremenu <script> Gui.Treeview.Show :call <SID>TreeViewShowHide(1)
anoremenu <script> Gui.Treeview.Hide :call <SID>TreeViewShowHide(0)

" Notifies the TreeView widget of a Toggle event
function! s:TreeViewToggle()
    call rpcnotify(0, 'Gui', 'TreeView', 'Toggle')
endfunction

command! GuiTreeviewToggle call <SID>TreeViewToggle()
noremap <script> <Plug>GuiTreeviewToggle :call <SID>TreeViewToggle()
anoremenu <script> Gui.Treeview.Toggle :call <SID>TreeViewShowToggle()
