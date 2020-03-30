let discardChanges = () =>
  Isolinear.Effect.create(~name="vim.discardChanges", () => {
    let _ = Vim.input("<esc>");
    let _ = Vim.input("<esc>");
    let _ = Vim.input(":");
    let _ = Vim.input("e");
    let _ = Vim.input("!");
    let _ = Vim.input("<CR>");
    ();
  });

let forceOverwrite = () =>
  Isolinear.Effect.create(~name="vim.forceOverwrite", () => {
    let _ = Vim.input("<esc>");
    let _ = Vim.input("<esc>");
    let _ = Vim.input(":");
    let _ = Vim.input("w");
    let _ = Vim.input("!");
    let _ = Vim.input("<CR>");
    ();
  });

let saveAllAndQuit = () =>
  Isolinear.Effect.create(~name="lifecycle.saveAllAndQuit", () => {
    Vim.input("<ESC>") |> (ignore: list(Vim.Cursor.t) => unit);
    Vim.input("<ESC>") |> (ignore: list(Vim.Cursor.t) => unit);
    Vim.input(":") |> (ignore: list(Vim.Cursor.t) => unit);
    Vim.input("x") |> (ignore: list(Vim.Cursor.t) => unit);
    Vim.input("a") |> (ignore: list(Vim.Cursor.t) => unit);
    Vim.input("<CR>") |> (ignore: list(Vim.Cursor.t) => unit);
  });

let quitAll = () =>
  Isolinear.Effect.create(~name="lifecycle.saveAllAndQuit", () => {
    Vim.input("<ESC>") |> (ignore: list(Vim.Cursor.t) => unit);
    Vim.input("<ESC>") |> (ignore: list(Vim.Cursor.t) => unit);
    Vim.input(":") |> (ignore: list(Vim.Cursor.t) => unit);
    Vim.input("q") |> (ignore: list(Vim.Cursor.t) => unit);
    Vim.input("a") |> (ignore: list(Vim.Cursor.t) => unit);
    Vim.input("!") |> (ignore: list(Vim.Cursor.t) => unit);
    Vim.input("<CR>") |> (ignore: list(Vim.Cursor.t) => unit);
  });
