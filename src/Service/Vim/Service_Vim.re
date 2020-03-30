let forceReload = () =>
  Isolinear.Effect.create(~name="vim.discardChanges", () =>
    Vim.command("e!")
  );

let forceOverwrite = () =>
  Isolinear.Effect.create(~name="vim.forceOverwrite", () =>
    Vim.command("w!")
  );

let saveAllAndQuit = () =>
  Isolinear.Effect.create(~name="lifecycle.saveAllAndQuit", () =>
    Vim.command("xa")
  );

let quitAll = () =>
  Isolinear.Effect.create(~name="lifecycle.saveAllAndQuit", () =>
    Vim.command("qa!")
  );
