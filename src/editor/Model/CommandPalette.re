let create = setItems => {
  let commands: list(Actions.menuCommand) = [
    {
      category: Some("Preferences"),
      name: "Open configuration file",
      command: () => Actions.OpenConfigFile("configuration.json"),
      icon: None,
    },
    {
      category: Some("Preferences"),
      name: "Open keybindings file",
      command: () => Actions.OpenConfigFile("keybindings.json"),
      icon: None,
    },
    {
      category: Some("Preferences"),
      name: "Reload configuration",
      command: () => Actions.ConfigurationReload,
      icon: None,
    },
    {
      category: Some("View"),
      name: "Close Editor",
      command: () => Actions.Command("view.closeEditor"),
      icon: None,
    },
    {
      category: Some("View"),
      name: "Split Editor Vertically",
      command: () => Actions.Command("view.splitVertical"),
      icon: None,
    },
    {
      category: Some("View"),
      name: "Split Editor Horizontally",
      command: () => Actions.Command("view.splitHorizontal"),
      icon: None,
    },
    {
      category: Some("View"),
      name: "Toggle Zen Mode",
      command: () => Actions.ToggleZenMode,
      icon: None,
    },
    {
      category: Some("View"),
      name: "Rotate Windows (Forwards)",
      command: () => Actions.Command("view.rotateForward"),
      icon: None,
    },
    {
      category: Some("View"),
      name: "Rotate Windows (Backwards)",
      command: () => Actions.Command("view.rotateBackward"),
      icon: None,
    },
  ];

  setItems(commands);

  () => ();
};
