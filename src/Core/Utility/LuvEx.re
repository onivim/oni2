module Process = {
  let spawn =
    Luv.Process.spawn(
      ~windows_hide=true,
      ~windows_hide_console=true,
      ~windows_hide_gui=true,
    );
};
