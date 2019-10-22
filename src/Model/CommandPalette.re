let commands: array(Actions.menuItem) = [|
    {
      category: Some("Preferences"),
      name: "Open configuration file",
      command: () => Actions.OpenConfigFile("configuration.json"),
      icon: None,
      highlight: []
    },
    {
      category: Some("Preferences"),
      name: "Open keybindings file",
      command: () => Actions.OpenConfigFile("keybindings.json"),
      icon: None,
      highlight: []
    },
    {
      category: Some("Preferences"),
      name: "Reload configuration",
      command: () => Actions.ConfigurationReload,
      icon: None,
      highlight: []
    },
    {
      category: Some("View"),
      name: "Close Editor",
      command: () => Actions.Command("view.closeEditor"),
      icon: None,
      highlight: []
    },
    {
      category: Some("View"),
      name: "Split Editor Vertically",
      command: () => Actions.Command("view.splitVertical"),
      icon: None,
      highlight: []
    },
    {
      category: Some("View"),
      name: "Split Editor Horizontally",
      command: () => Actions.Command("view.splitHorizontal"),
      icon: None,
      highlight: []
    },
    {
      category: Some("View"),
      name: "Enable Zen Mode",
      command: () => Actions.EnableZenMode,
      icon: None,
      highlight: []
    },
    {
      category: Some("View"),
      name: "Disable Zen Mode",
      command: () => Actions.DisableZenMode,
      icon: None,
      highlight: []
    },
    {
      category: Some("Input"),
      name: "Enable Key Displayer",
      command: () => Actions.EnableKeyDisplayer,
      icon: None,
      highlight: []
    },
    {
      category: Some("Input"),
      name: "Disable Key Displayer",
      command: () => Actions.DisableKeyDisplayer,
      icon: None,
      highlight: []
    },
    {
      category: Some("View"),
      name: "Rotate Windows (Forwards)",
      command: () => Actions.Command("view.rotateForward"),
      icon: None,
      highlight: []
    },
    {
      category: Some("View"),
      name: "Rotate Windows (Backwards)",
      command: () => Actions.Command("view.rotateBackward"),
      icon: None,
      highlight: []
    },
    {
      category: Some("Editor"),
      name: "Copy Active Filepath To Clipboard",
      command: () => Actions.CopyActiveFilepathToClipboard,
      icon: None,
      highlight: []
    },
    /*{
        category: Some("Developer"),
        name: "Create massive menu",
        command: () => Actions.Command("developer.massiveMenu"),
        icon: None,
        highlight: []
      },*/
  |];