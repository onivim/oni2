let forceReload = () =>
  Isolinear.Effect.create(~name="vim.discardChanges", () =>
    Vim.command("e!") |> ignore
  );

let forceOverwrite = () =>
  Isolinear.Effect.create(~name="vim.forceOverwrite", () =>
    Vim.command("w!") |> ignore
  );

let reload = () =>
  Isolinear.Effect.create(~name="vim.reload", () => {
    Vim.command("e") |> ignore
  });

let saveAllAndQuit = () =>
  Isolinear.Effect.create(~name="lifecycle.saveAllAndQuit", () =>
    Vim.command("xa") |> ignore
  );

let quitAll = () =>
  Isolinear.Effect.create(~name="lifecycle.saveAllAndQuit", () =>
    Vim.command("qa!") |> ignore
  );
