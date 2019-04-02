open MenuCommand;

/* let openConfigurationFile = (effects: Effects.t(Actions.t), name) => */
/*   switch (Filesystem.createOniConfigFile(name)) { */
/*   | Ok(path) => effects.openFile(~path, ()) */
/*   | Error(e) => print_endline(e) */
/*   }; */
let create = setItems => {
  let commands: list(MenuCommand.t) = [
    {
      category: None,
      name: "Open configuration file",
      /* command: () => openConfigurationFile(effects, "configuration.json"), */
      command: () => print_endline("configuration.json"),
      icon: None,
    },
    {
      category: None,
      name: "Open keybindings file",
      /* command: () => openConfigurationFile(effects, "keybindings.json"), */
      command: () => print_endline("keybindings.json"),
      icon: None,
    },
  ];

  setItems(commands);

  () => ();
};
