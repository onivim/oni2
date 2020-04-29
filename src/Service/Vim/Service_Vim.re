let forceReload = () =>
  Isolinear.Effect.create(~name="vim.discardChanges", () =>
    ignore(Vim.command("e!"): Vim.Context.t)
  );

let forceOverwrite = () =>
  Isolinear.Effect.create(~name="vim.forceOverwrite", () =>
    ignore(Vim.command("w!"): Vim.Context.t)
  );

let reload = () =>
  Isolinear.Effect.create(~name="vim.reload", () => {
    ignore(Vim.command("e"): Vim.Context.t)
  });

let saveAllAndQuit = () =>
  Isolinear.Effect.create(~name="lifecycle.saveAllAndQuit", () =>
    ignore(Vim.command("xa"): Vim.Context.t)
  );

let quitAll = () =>
  Isolinear.Effect.create(~name="lifecycle.saveAllAndQuit", () =>
    ignore(Vim.command("qa!"): Vim.Context.t)
  )
