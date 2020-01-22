/**
 * Cli.re
 *
 * Module for handling command-line arguments for Oni2
 */
open Rench;

module CoreLog = Log;
module Log = (val Log.withNamespace("Oni2.Core.Cli"));

type t = {
  folder: string,
  filesToOpen: list(string),
  forceScaleFactor: option(float),
  syntaxHighlightService: bool,
  overriddenExtensionsDir: option(string),
  shouldClose: bool,
  shouldLoadExtensions: bool,
  shouldSyntaxHighlight: bool,
  shouldLoadConfiguration: bool,
};

let create = (~folder, ~filesToOpen, ()) => {
  folder,
  filesToOpen,
  forceScaleFactor: None,
  syntaxHighlightService: false,
  overriddenExtensionsDir: None,
  shouldClose: false,
  shouldLoadExtensions: true,
  shouldSyntaxHighlight: true,
  shouldLoadConfiguration: true,
};

module Internal = {
  let filterPsnArgument = args => {
    let psnRegex = Str.regexp("^-psn.*");
    let f = s => {
      !Str.string_match(psnRegex, s, 0);
    };

    args |> Array.to_list |> List.filter(f) |> Array.of_list;
  };
  
  let usage = () => Printf.sprintf({|
  Onivim 2 (%s)

  Usage:

    oni2 [options][paths...]

  Options:
  |}, BuildInfo.version);
}

let newline = "\n";

let show = (v: t) => {
  let files =
    List.fold_left((curr, p) => curr ++ newline ++ p, "", v.filesToOpen);

  "Folder: " ++ v.folder ++ newline ++ "Files: " ++ files;
};

let noop = () => ();

let setWorkingDirectory = s => {
  Log.debug("--working-directory - chdir: " ++ s);
  Sys.chdir(s);
};

let setRef: (ref(option('a)), 'a) => unit =
  (someRef, v) => someRef := Some(v);

let parse =
    (
      ~checkHealth,
      ~listExtensions,
      ~installExtension,
      ~uninstallExtension,
      ~printVersion,
    ) => {
  let args: ref(list(string)) = ref([]);

  let scaleFactor = ref(None);
  let syntaxHighlightService = ref(false);
  let extensionsDir = ref(None);
  let shouldClose = ref(false);

  let shouldLoadExtensions = ref(true);
  let shouldLoadConfiguration = ref(true);
  let shouldSyntaxHighlight = ref(true);

  let queuedJob = ref(None);

  let runAndExit = f =>
    queuedJob := Some(cli => {f(cli) |> exit});

  let runAndExitUnit = f =>
    Arg.Unit(() => queuedJob := Some(cli => {f(cli) |> exit}));

  let runAndExitString = f =>
    Arg.String(s => queuedJob := Some(cli => {f(s, cli) |> exit}));

  let disableExtensionLoading = () => shouldLoadExtensions := false;
  let disableLoadConfiguration = () => shouldLoadConfiguration := false;
  let disableSyntaxHighlight = () => shouldSyntaxHighlight := false;

  let incomingArgs = Sys.argv |> Internal.filterPsnArgument;

  let spec = Arg.[
      ("-f", Unit(Timber.App.enablePrinting), 
      " Stay attached to the foreground terminal.",
      ),
      ("--nofork", Unit(Timber.App.enablePrinting), 
      " Stay attached to the foreground terminal.",
      ),
      ("--debug", Unit(CoreLog.enableDebugLogging), 
      " Enable debug logging."
      ),
      ("--version", printVersion |> runAndExitUnit, " Print version information."),
      ("--no-log-colors", Unit(Timber.App.disableColors), " Turn off colors and rich formatting in logs."),
      ("--disable-extensions", Unit(disableExtensionLoading), " Turn off extension loading."),
      ("--disable-configuration", Unit(disableLoadConfiguration), " Do not load user configuration (use default configuration)."),
      ("--disable-syntax-highlighting", Unit(disableSyntaxHighlight), " Turn off syntax highlighting."),
      ("--log-file", String(Timber.App.setLogFile), " Specify a file for the output logs."),
      ("--log-filter", String(Timber.App.setNamespaceFilter), " Filter log output."),
      ("--checkhealth", checkHealth |> runAndExitUnit, " Check the health of the Oni2 editor."),
      ("--list-extensions", listExtensions |> runAndExitUnit, " List the currently installed extensions."),
      ("--install-extension", installExtension |> runAndExitString, " Install extension by specifying a path to the .vsix file"),
      ("--uninstall-extension", uninstallExtension |> runAndExitString, " Uninstall extension by specifying an extension id."),
      ("--working-directory", String(setWorkingDirectory), " Set the current working for Oni2."),
      (
        "--force-device-scale-factor",
        Float(f => scaleFactor := Some(f)),
        " Force the DPI scaling for the editor.",
      ),
      (
        "--syntax-highlight-service",
        Unit(() => syntaxHighlightService := true),
        "", // Internal option only
      ),
      ("--extensions-dir", String(setRef(extensionsDir)), " The folder to store/load VSCode extensions."),
      ("--force-device-scale-factor", Float(setRef(scaleFactor)), " Force the DPI scaling for the editor."),
    ];

  let handleAnonymousArgs = arg => args := [arg, ...args^];

  switch(Arg.parse_argv(incomingArgs,
    spec,
    handleAnonymousArgs,
    Internal.usage(),
  )) {
  | exception (Arg.Bad(err)) => runAndExit((_) => { prerr_endline(err); 1 })
  | exception (Arg.Help(msg)) => runAndExit((_) => { print_endline(msg); 0 })
  | _ => ()
  };

  if (!CoreLog.isPrintingEnabled()) {
    /* On Windows, detach the application from the console if we're not logging to console */
    Utility.freeConsole();
  };

  let paths = args^ |> List.rev;
  let workingDirectory = Environment.getWorkingDirectory();

  let stripTrailingPathCharacter = s => {
    let len = String.length(s);
    if (len > 1 && s.[len - 1] == '/') {
      String.sub(s, 0, len - 1);
    } else {
      s;
    };
  };

  let isAbsolutePathWithTilde = s =>
    switch (s.[0]) {
    | '~' => !Sys.win32
    | _ => false
    | exception (Invalid_argument(_)) => false
    };

  let resolvePath = p => {
    let p =
      if (Path.isAbsolute(p) || isAbsolutePathWithTilde(p)) {
        p;
      } else {
        Path.join(workingDirectory, p);
      };

    p |> Path.normalize |> stripTrailingPathCharacter;
  };

  let absolutePaths = List.map(resolvePath, paths);

  let isDirectory = p =>
    switch (Sys.is_directory(p)) {
    | v => v
    | exception (Sys_error(_)) => false
    };

  let directories = List.filter(isDirectory, absolutePaths);
  let filesToOpen = List.filter(p => !isDirectory(p), absolutePaths);

  let homeOrWorkingDirectory =
    switch (Sys.getenv_opt("HOME")) {
    | Some(homePath) => homePath
    | None => workingDirectory
    };

  /* Set the folder to be opened, based on 4 options:
         - If a folder(s) is given, use the first.
         - If no folders are given, but files are, use the dir of the first file.
         - If no files or folders are given, and the path is, the root, "/", try and use the home directory
         - If none of the other conditions are met, use the working directory
     */

  let folder =
    switch (directories, filesToOpen, workingDirectory) {
    | ([hd, ..._], _, _) => hd
    | ([], [hd, ..._], _) => Rench.Path.dirname(hd)
    | ([], [], "/") => homeOrWorkingDirectory
    | ([], [], workingDirectory) => workingDirectory
    };

  let cli = {
    folder,
    filesToOpen,
    forceScaleFactor: scaleFactor^,
    syntaxHighlightService: syntaxHighlightService^,
    overriddenExtensionsDir: extensionsDir^,
    shouldClose: shouldClose^,
    shouldSyntaxHighlight: shouldSyntaxHighlight^,
    shouldLoadExtensions: shouldLoadExtensions^,
    shouldLoadConfiguration: shouldLoadConfiguration^,
  };

  switch (queuedJob^) {
  | None => ()
  | Some(job) => job(cli)
  };

  cli;
};
