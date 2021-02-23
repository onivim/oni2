/**
   Filesystem

   module for handling various Oni2 filesystem interactions in a
   safe way using monadic operators

   reference (source of inspiration): https://medium.com/@huund/making-a-directory-in-ocaml-53ceca84979f
 */
module Path = Utility.Path;
module OptionEx = Utility.OptionEx;
module ResultEx = Utility.ResultEx;
open Kernel;
module Log = (val Log.withNamespace("Oni2.Filesystem"));

type t('a) = result('a, string);

module Internal = {
  let getUserDataDirectoryExn = () =>
    Revery.(
      switch (Environment.os) {
      | Environment.Windows(_) =>
        Sys.getenv_opt("LOCALAPPDATA")
        |> OptionEx.flatMap(FpExp.absoluteCurrentPlatform)
        |> Option.get
      | _ =>
        switch (Sys.getenv_opt("HOME")) {
        | Some(dir) => FpExp.absoluteCurrentPlatform(dir) |> Option.get
        | None => failwith("Could not find user data directory")
        }
      }
    );
};

/* This informs of an error and passes the error string wrapped in an Error to the next function*/
let error = fmt => Printf.ksprintf(msg => Error(msg), fmt);

let return = x => Ok(x);

/**
   Permissions ==================================================
 */
let userReadWriteExecute = 0o777;

/**
   Safe Unix functions ==========================================
 */

let stat = path =>
  Unix.(
    try(Some(stat(path |> FpExp.toString))) {
    | Unix_error(ENOENT, _, _) => None
    }
  );

let isFile = path =>
  path
  |> stat
  |> Option.map(stats => {
       switch (stats.Unix.st_kind) {
       | Unix.S_REG => true
       | _ => false
       }
     })
  |> Option.value(~default=false);

let isDir = path =>
  path
  |> stat
  |> Option.map(stats => {
       switch (stats.Unix.st_kind) {
       | Unix.S_DIR => true
       | _ => false
       }
     })
  |> Option.value(~default=false);

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
    try(mkdir(path |> FpExp.toString, perm) |> return) {
    | Unix_error(err, _, _) =>
      error(
        "can't create directory '%s' because '%s",
        path |> FpExp.toString,
        error_message(err),
      )
    }
  );

let mkdirp = path => {
  let rec loop = curr =>
    if (isDir(curr)) {
      Ok();
    } else if (isFile(curr)) {
      Error(
        "Can't create directory because file exists: " ++ FpExp.toString(curr),
      );
    } else if (!FpExp.hasParentDir(curr)) {
      Ok();
    } else {
      // Create parent directory, if necesssary
      loop(FpExp.dirName(curr)) |> ResultEx.flatMap(mkdir(curr));
    };

  loop(path) |> Result.map(_ => path);
};

let getOniDirectory = dataDirectory =>
  Revery.(
    switch (Environment.os) {
    | Environment.Windows(_) => FpExp.append(dataDirectory, "Oni2") |> return
    | _ => FpExp.At.(dataDirectory / ".config" / "oni2") |> return
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

let getOrCreateOniConfiguration = (~configDir, ~file) => {
  mkdirp(configDir)
  |> ResultEx.flatMap(_ =>
       if (isFile(FpExp.append(configDir, file))) {
         // Already created
         Ok();
       } else {
         let userConfigPath = FpExp.append(configDir, file) |> FpExp.toString;

         let configFile = open_out(userConfigPath);
         let configString =
           ConfigurationDefaults.getDefaultConfigString(file);

         switch (configString) {
         | Some(c) => output_string(configFile, c)
         | None => ()
         };

         switch (close_out(configFile)) {
         | exception (Sys_error(msg)) =>
           error("Failed whilst making config %s, due to %s", file, msg)
         | v => return(v)
         };
       }
     );
};

let getOrCreateConfigFolder = mkdirp;

let getExtensionsFolder = () =>
  getUserDataDirectory()
  |> ResultEx.flatMap(getOniDirectory)
  |> Result.map(dir => FpExp.append(dir, "extensions"))
  |> ResultEx.flatMap(mkdirp);

let getStoreFolder = () =>
  getUserDataDirectory()
  |> ResultEx.flatMap(getOniDirectory)
  |> Result.map(dir => FpExp.append(dir, "store"))
  |> ResultEx.flatMap(mkdirp);

let getGlobalStorageFolder = () =>
  getUserDataDirectory()
  |> ResultEx.flatMap(getOniDirectory)
  |> Result.map(dir => FpExp.append(dir, "global"))
  |> ResultEx.flatMap(mkdirp);

let getWorkspaceStorageFolder = () =>
  getUserDataDirectory()
  |> ResultEx.flatMap(getOniDirectory)
  |> Result.map(dir => FpExp.append(dir, "workspace"))
  |> ResultEx.flatMap(mkdirp);

let getSnippetsFolder = () =>
  getUserDataDirectory()
  |> ResultEx.flatMap(getOniDirectory)
  |> Result.map(dir => FpExp.append(dir, "snippets"))
  |> ResultEx.flatMap(mkdirp);

let rec getOrCreateConfigFile = (~overridePath=?, filename) => {
  switch (overridePath) {
  | Some(path) =>
    let pathString = path |> FpExp.toString;
    switch (Sys.file_exists(pathString)) {
    | exception ex =>
      Log.error("Error loading configuration file at: " ++ pathString);
      Log.error("  " ++ Printexc.to_string(ex));
      getOrCreateConfigFile(filename);

    | false =>
      Log.error("Error loading configuration file at: " ++ pathString);
      getOrCreateConfigFile(filename);

    | true => Ok(path)
    };
  | None =>
    /* Get Oni Directory */
    getUserDataDirectory()
    |> ResultEx.flatMap(getOniDirectory)
    |> ResultEx.flatMap(configDir =>
         getOrCreateOniConfiguration(~configDir, ~file=filename)
         |> Result.map(() => FpExp.append(configDir, filename))
       )
  };
};
