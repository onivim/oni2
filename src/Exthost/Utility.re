let getNodePath = () => {
  let ic =
    Sys.win32
      // HACK: Not sure why this command doesn't work on Linux / macOS, and vice versa...
      ? Unix.open_process_args_in(
          "node",
          [|"node", "-e", "console.log(process.execPath)"|],
        )
      : Unix.open_process_in("node -e 'console.log(process.execPath)'");
  let nodePath = input_line(ic);
  let _ = close_in(ic);
  nodePath |> String.trim;
};
