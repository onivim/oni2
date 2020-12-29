
# 1. Environment

## 1.1 Validate PATH set correctly on OSX (#1161)

- Run release Onivim from Finder (NOT terminal)
- Run `:echo $PATH`
- Validate full shell path is available

__Pass:__
- [ ] OSX

## 1.2 Validate launches from dock in OSX (#2659)

- Update .zshrc to have blocking input: (`read var`, `echo $var`)
- Update .zshrc to have canary entry in `PATH`
- Run Onivim 2 from dock
- Validate Onivim 2 launches and PATH is correct

__Pass:__
- [ ] OSX

# 2. First-Run Experience

Test cases covering launching and using Onivim without any persistence or configuration.

## 2.1 No directory set [OSX|Win|Linux]

- Clear the Onivim 2 configuration folder (`rm -rf ~/.config/oni2`)
- Launch Onivim
- Verify explorer shows 'No folder opened'
- Verify Control+P/Command+P shows the 'Welcome' buffer
- Verify Control+Shift+P/Command+P shows the command palette

__Pass:__
- [ ] Win
- [ ] OSX
- [ ] Linux

## 2.2 Home directory set [OSX]

This case is related to #2742 - previous builds of Onivim may have persisted
the startup folder as `~/Documents`. This is problematic because Onivim may
not have permission to read that folder.

- Launch a developer build of Onivim
- `:cd ~/Documents` (change to documents folder)
- Re-launch developer build of Onivim - verify `~/Documents` is the current workspace.
- Launch the release build of Onivim _from Finder_
- Verify explorer shows 'No folder opened'
- Verify Control+P/Command+P shows only the 'Welcome' buffer
- Verify Control+Shift+P/Command+Shift+P shows the command palette

__Pass:__
- [ ] OSX

# 3. SCM

## 3.1 Verify diff markers show in modified git file (#2857)

- Launch Onivim in oni2 directory
- Modify `README.md`
- Verify diff markers show 

__Pass:__
- [ ] Win
- [ ] OSX
- [ ] Linux

# 4. Editing

## 4.1 Verify H/L/M behavior on editor surface

- Launch Onivim in oni2 directory
- Open `README.md`
- Set line wrap (`:set wrap`)
- In normal mode, verify `H` goes to top buffer line
- In normal mode, verify `L` goes to bottom buffer line
- In normal mode, verify `M` goes to middle buffer line
- In normal mode, verify `3H` goes to third buffer line 
- In normal mode, verify `4L` goes to fourth-from-bottom buffer line
- In normal mode, verify `1000H` goes to the same position as `L`
- In normal mode, verify `1000L` goes to the same position as `H`

__Pass:__
- [ ] Win
- [ ] OSX
- [ ] Linux
