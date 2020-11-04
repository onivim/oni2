
# 1. Environment

## 1.1 Validate PATH set correctly on OSX (#1161)

- Run release Onivim from Finder (NOT terminal)
- Run `:echo $PATH`
- Validate full shell path is available

## 1.2 Validate launches from dock in OSX (#2659)

- Update .zshrc to have blocking input: (`read var`, `echo $var`)
- Update .zshrc to have canary entry in `PATH`
- Run Onivim 2 from dock
- Validate Onivim 2 launches and PATH is correct

