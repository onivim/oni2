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
      command: () => Actions.ViewCloseEditor,
      icon: None,
    },
  ];

  setItems(commands);

  () => ();
};
