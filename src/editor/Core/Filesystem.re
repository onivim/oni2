type t('a) =
  | Ok('a)
  | Error(string);

/** [on_success] executes [f] unless we already hit an error. In
  that case the error is passed on. */

/** [on_success] executes [f] unless we already hit an error. In
  that case the error is passed on. */
let on_success = (t: t('a), f: 'a => t('b)) =>
  switch (t) {
  | Ok(x) => f(x)
  | Error(str) => Error(str)
  };

/** [on_error] ignores the current error and executes [f]. If
  there is no error, [f] is not executed and the result is
  passed on. */;

/** [on_error] ignores the current error and executes [f]. If
  there is no error, [f] is not executed and the result is
  passed on. */
let on_error = (t, f) =>
  switch (t) {
  | Error(str) => f(str)
  | Ok(x) => Ok(x)
  };

/**
   Helper functions ============================================
 */
let always = (_t, f) => f();

let ( *> ) = always;

/* This informs of an error and passes the error string wrapped in an Error to the next function*/
let error = fmt => Printf.ksprintf(msg => Error(msg), fmt);

let inform = fmt => Printf.ksprintf(msg => Ok(msg), fmt);

let return = x => Ok(x);

let _fail = msg => Error(msg);

let (>>=) = on_success;
let (/\/=) = on_error;

/**
   Permissions ==================================================
 */

let userReadWriteExecute = 0o777;

/**
   Safe Unix functions ==========================================
 */

let isDir = st =>
  switch (st.Unix.st_kind) {
  | Unix.S_DIR => return()
  | _ => error("not a directory")
  };

let hasOwner = (uid, st) =>
  if (st.Unix.st_uid == uid) {
    return();
  } else {
    error("expected uid = %d, found %d", uid, st.Unix.st_uid);
  };

let hasGroup = (gid, st) =>
  if (st.Unix.st_gid == gid) {
    return();
  } else {
    error("expected gid = %d, found %d", gid, st.Unix.st_gid);
  };

let hasPerm = (perm, st) =>
  if (st.Unix.st_perm == perm) {
    return();
  } else {
    error("expected permissions 0o%o, found 0o%o", perm, st.Unix.st_perm);
  };

let getGroupId = () =>
  Unix.(
    try (getgid() |> return) {
    | Not_found => error("No Group ID found")
    }
  );

let getUserId = () =>
  Unix.(
    try (getuid() |> return) {
    | Not_found => error("Cannot find user")
    }
  );

let stat = path =>
  Unix.(
    try (Some(stat(path)) |> return) {
    | Unix_error(ENOENT, _, _) => return(None)
    }
  );

let chmod = (path, ~perm=userReadWriteExecute, ()) =>
  Unix.(
    try (chmod(path, perm) |> return) {
    | Unix_error(_, _, _) => error("can't set permissions for '%s'", path)
    }
  );

let chown = (path, uid, gid) =>
  Unix.(
    try (chown(path, uid, gid) |> return) {
    | Unix_error(err, _, _) =>
      error(
        "can't set uid/gid for '%s' because '%s",
        path,
        error_message(err),
      )
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
    try (mkdir(path, perm) |> return) {
    | Unix_error(err, _, _) =>
      error(
        "can't create directory '%s' because '%s",
        path,
        error_message(err),
      )
    }
  );

let rmdir = path =>
  Unix.(
    try (rmdir(path) |> return) {
    | Unix_error(err, _, _) =>
      error(
        "can't remove directory '%s' because: '%s'",
        path,
        error_message(err),
      )
    }
  );

let getOniDirectory = home =>
  Utility.join([home, ".config", "oni2"]) |> return;

let getHomeDirectory = () =>
  Unix.(
    try (getenv("HOME") |> return) {
    | Unix_error(err, _, _) =>
      error("Cannot find home because: '%s'", error_message(err))
    }
  );

/**
   CopyFile:

   There is no native function to copy files (suprisingly)
   The reference below explains how this function works
   and why the seemingly arbitrary buffer size
   TLDR: efficiency

   NOTE: copyFile itself is not safe, copy (below, is safe)
   wraps the call with an exception handler that returns a wrapped
   value of Error or Ok

   reference: https://ocaml.github.io/ocamlunix/ocamlunix.html#sec33
 */
let copyFile = (source, dest) => {
  let bufferSize = 8192;
  let buffer = Bytes.create(bufferSize);

  let sourceFile = Unix.openfile(source, [O_RDONLY], 0);
  let destFile =
    Unix.openfile(dest, [O_WRONLY, O_CREAT, O_TRUNC], userReadWriteExecute);
  /**
    In the copy_loop function we do the copy by blocks of buffer_size bytes.
    We request buffer_size bytes to read. If read returns zero,
    we have reached the end of file and the copy is over.
    Otherwise we write the bytes we have read in the output file and start again.
   */
  let rec copy_loop = () =>
    switch (Unix.read(sourceFile, buffer, 0, bufferSize)) {
    | 0 => ()
    | bytes =>
      Unix.write(destFile, buffer, 0, bytes) |> ignore;
      copy_loop();
    };
  copy_loop();
  Unix.close(sourceFile);
  Unix.close(destFile);
};

let copy = (source, dest) =>
  Unix.(
    try (copyFile(source, dest) |> return) {
    | Unix_error(err, funcName, argument) =>
      error(
        "Failed to copy from %s to %s, encountered %s, whilst calling %s with %s",
        source,
        dest,
        error_message(err),
        funcName,
        argument,
      )
    }
  );

let createOniConfiguration = (configDir, file) => {
  open Utility;

  let assetDir = Revery.Environment.getWorkingDirectory();
  let configurationPath = join([assetDir, "assets", "configuration", file]);
  let userConfigPath = join([configDir, file]);

  copy(configurationPath, userConfigPath);
};

let createOniConfigFile = file =>
  getHomeDirectory()
  >>= getOniDirectory
  >>= (
    path =>
      stat(path)
      >>= (
        fun
        | Some(st) =>
          /**
           TODO:
           we should check if the config file we want to make exists
           if it does we should do nothing else
         */
          /* path already exists */
          isDir(st)
          *> (_ => inform("path already exists"))
          >>= (
            _ =>
              createOniConfiguration(path, file)
              /\/= error("Error creating configuration files because: %s")
              >>= (_ => return(Utility.join([path, file])))
          )
        | None =>
          mkdir(path, ())
          >>= (
            _ =>
              createOniConfiguration(path, file)
              >>= (_ => return(Utility.join([path, file])))
          )
      )
  );
