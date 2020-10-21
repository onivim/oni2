/**
   Filesystem

   module for handling various Oni2 filesystem interactions in a
   safe way using monadic operators

   reference (source of inspiration): https://medium.com/@huund/making-a-directory-in-ocaml-53ceca84979f
 */
module Path = Utility.Path;
open Kernel;
module Log = (val Log.withNamespace("Oni2.Filesystem"));

type t('a) = result('a, string);

module Internal = {
  let getUserDataDirectoryExn = () =>
    Revery.(
      switch (Environment.os) {
      | Environment.Windows => Sys.getenv("LOCALAPPDATA")
      | _ =>
        switch (Sys.getenv_opt("HOME")) {
        | Some(dir) => dir
        | None => failwith("Could not find user data directory")
        }
      }
    );
};

/** [on_success] executes [f] unless we already hit an error. In
  that case the error is passed on. */
let onSuccess = (t: t('a), f: 'a => t('b)) =>
  switch (t) {
  | Ok(x) => f(x)
  | Error(str) => Error(str)
  };

/** [on_error] ignores the current error and executes [f]. If
  there is no error, [f] is not executed and the result is
  passed on. */
let onError = (t, f) =>
  switch (t) {
  | Error(str) => f(str)
  | Ok(x) => Ok(x)
  };

/**
   Helper functions ============================================
 */
let _always = (_t, f) => f();

/* This informs of an error and passes the error string wrapped in an Error to the next function*/
let error = fmt => Printf.ksprintf(msg => Error(msg), fmt);

let _inform = fmt => Printf.ksprintf(msg => Ok(msg), fmt);

let return = x => Ok(x);

let _fail = msg => Error(msg);

/**
   Infix Operators: These are essentially aliases to the
   onError and onSuccess functions.
   This syntax allows us to use the monadic FS operators like
   mkdir in a chainable fashion

   mkdir
    //= error("something went wrong")
    >>= (_ => return())

   in this example mkdir returns either an Ok(value) or an Error(value)
   the result is passed to both functions sequentially if it is an error
   the error handler is called if it is an OK this is ignored an the Ok
   value is passed to the success handler

   The advantage is that it allows for a concise way to describe a whole
   range of error and OK scenarios
 */
let (>>=) = onSuccess;
let (/\/=) = onError;
/* let ( *> ) = always; */

/**
   Permissions ==================================================
 */
let userReadWriteExecute = 0o777;

/**
   Safe Unix functions ==========================================
 */

let isFile = st =>
  switch (st.Unix.st_kind) {
  | Unix.S_REG => return()
  | _ => error("not a file")
  };

let isDir = st =>
  switch (st.Unix.st_kind) {
  | Unix.S_DIR => return()
  | _ => error("not a directory")
  };

let stat = path =>
  Unix.(
    try(Some(stat(path)) |> return) {
    | Unix_error(ENOENT, _, _) => return(None)
    }
  );

/**
  Permissions default to read/write permissions

  The permissions of the directory should ideally be
  drwxr-xr-x -> this means the user can read, write and execute
  but group and others can only read and execute. Without
  the correct permission creating the subfolders or files
  will not work correctly
*/
let mkdir = (path, ~perm=userReadWriteExecute, ()) =>
  Unix.(
    try(mkdir(path, perm) |> return) {
    | Unix_error(err, _, _) =>
      error(
        "can't create directory '%s' because '%s",
        path,
        error_message(err),
      )
    }
  );

let getOniDirectory = dataDirectory =>
  Revery.(
    switch (Environment.os) {
    | Environment.Windows => Path.join([dataDirectory, "Oni2"]) |> return
    | _ => Path.join([dataDirectory, ".config", "oni2"]) |> return
    }
  );

let getUserDataDirectory = () =>
  Unix.(
    try(Internal.getUserDataDirectoryExn() |> return) {
    | Unix_error(err, _, _) =>
      error("Cannot find data directory because: '%s'", error_message(err))
    | Failure(reason) => error("The OS could not be identified %s", reason)
    }
  );

let createOniConfiguration = (~configDir, ~file) => {
  let userConfigPath = Path.join([configDir, file]);

  let configFile = open_out(userConfigPath);
  let configString = ConfigurationDefaults.getDefaultConfigString(file);

  switch (configString) {
  | Some(c) => output_string(configFile, c)
  | None => ()
  };

  switch (close_out(configFile)) {
  | exception (Sys_error(msg)) =>
    error("Failed whilst making config %s, due to %s", file, msg)
  | v => return(v)
  };
};

let getPath = (dir, file) => return(Path.join([dir, file]));

let createConfigIfNecessary = (configDir, file) =>
  Path.join([configDir, file])
  |> (
    configPath =>
      stat(configDir)
      >>= (
        fun
        | Some(dirStats) =>
          isDir(dirStats)
          /\/= (
            _ =>
              mkdir(configDir, ())
              >>= (() => createOniConfiguration(~file, ~configDir))
          )
          /* config directory already exists */
          >>= (
            () =>
              createOniConfiguration(~configDir, ~file)
              /\/= error("Error creating configuration files because: %s")
              >>= (() => return(configPath))
          )
        | None =>
          mkdir(configDir, ())
          >>= (() => createOniConfiguration(~configDir, ~file))
          >>= (() => return(configPath))
      )
  );

let getOrCreateConfigFolder = configDir =>
  stat(configDir)
  >>= (
    fun
    | Some(dirStats) =>
      /*
        Check the stats, if its a folder we can stop.
        If its not, we need to make a folder.
       */
      isDir(dirStats)
      /\/= (_ => mkdir(configDir, ()))
      >>= (() => return(configDir))
    | None =>
      /*
       No folder was present, lets make one.
       */
      mkdir(configDir, ()) >>= (() => return(configDir))
  );

let getExtensionsFolder = () =>
  getUserDataDirectory()
  >>= getOniDirectory
  >>= getOrCreateConfigFolder
  >>= (dir => getPath(dir, "extensions"))
  >>= getOrCreateConfigFolder;

let getStoreFolder = () =>
  getUserDataDirectory()
  >>= getOniDirectory
  >>= (dir => getPath(dir, "store"))
  >>= getOrCreateConfigFolder;

let getGlobalStorageFolder = () =>
  getUserDataDirectory()
  >>= getOniDirectory
  >>= (dir => getPath(dir, "global"))
  >>= getOrCreateConfigFolder;

let rec getOrCreateConfigFile = (~overridePath=?, filename) => {
  switch (overridePath) {
  | Some(path) =>
    switch (Sys.file_exists(path)) {
    | exception ex =>
      Log.error("Error loading configuration file at: " ++ path);
      Log.error("  " ++ Printexc.to_string(ex));
      getOrCreateConfigFile(filename);

    | false =>
      Log.error("Error loading configuration file at: " ++ path);
      getOrCreateConfigFile(filename);

    | true => Ok(path)
    }
  | None =>
    /* Get Oni Directory */
    getUserDataDirectory()
    >>= getOniDirectory
    >>= (
      configDir =>
        getPath(configDir, filename)
        /* Check whether the config file already exists */
        >>= stat
        >>= (
          fun
          | Some(existingFileStats) =>
            /* is the the thing that exists a file */
            isFile(existingFileStats)
            >>= (
              () =>
                getPath(configDir, filename)
                /* if it exists but is not a file attempt to create a file */
                /\/= (_ => createConfigIfNecessary(configDir, filename))
            )
          /* if the file does not exist try and create it */
          | None => createConfigIfNecessary(configDir, filename)
        )
    )
  };
};
