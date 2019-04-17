let create = setItems => {
  let commands: list(Actions.menuCommand) = [
    {
      category: None,
      name: "Open configuration file",
      command: () => Actions.OpenConfigFile("configuration.json"),
      icon: None,
    },
    {
      category: None,
      name: "Open keybindings file",
      command: () => Actions.OpenConfigFile("keybindings.json"),
      icon: None,
    },
    {
      category: None,
      name: "Reload configuration",
      command: () => Actions.ConfigurationReload,
      icon: None,
    },
  ];

  setItems(commands);

  () => ();
};
