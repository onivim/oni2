open Vim;

let resetBuffer = filePath => {
  /* Reset clipboard provider */
  Clipboard.setProvider(_ => None);

  let _ = Vim.command("clear-undo");

  let _ = input("<esc>");
  let _ = input("<esc>");
  let _context = command("e!");
  let ret = Buffer.openFile(filePath);

  /* Move cursor to initial location */
  let _ = input("g");
  let _ = input("g");
  let _ = input("0");
  ret;
};

let input = sz => Vim.input(sz) |> ignore;
