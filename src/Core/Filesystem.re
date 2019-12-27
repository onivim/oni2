/**
   Filesystem

   module for handling various Oni2 filesystem interactions in a
   safe way using monadic operators

   reference (source of inspiration): https://medium.com/@huund/making-a-directory-in-ocaml-53ceca84979f
 */
type t('a) = result('a, string);

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
    try(getgid() |> return) {
    | Not_found => error("No Group ID found")
    }
  );

let getUserId = () =>
  Unix.(
    try(getuid() |> return) {
    | Not_found => error("Cannot find user")
    }
  );

let stat = path =>
  Unix.(
    try(Some(stat(path)) |> return) {
    | Unix_error(ENOENT, _, _) => return(None)
    }
  );

let chmod = (path, ~perm=userReadWriteExecute, ()) =>
  Unix.(
    try(chmod(path, perm) |> return) {
    | Unix_error(_, _, _) => error("can't set permissions for '%s'", path)
    }
  );

let chown = (path, uid, gid) =>
  Unix.(
    try(chown(path, uid, gid) |> return) {
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
    try(mkdir(path, perm) |> return) {
    | Unix_error(err, _, _) =>
      error(
        "can't create directory '%s' because '%s",
        path,
        error_message(err),
      )
    }
  );

/**
 * Create a temporary directory
 * Adapted from: https://discuss.ocaml.org/t/how-to-create-a-temporary-directory-in-ocaml/1815/4
 */
let rand_digits = () => {
  let rand = Random.State.(bits(make_self_init()) land 0xFFFFFF);
  Printf.sprintf("%06x", rand);
};

let mkTempDir = (~prefix="mktempdir", ()) => {
  let tmp = Filename.get_temp_dir_name();

  let raise_err = msg => raise(Sys_error(msg));
  let rec loop = count =>
    if (count < 0) {
      raise_err("mkTempDir: Too many failing attempts");
    } else {
      let dir = Printf.sprintf("%s/%s%s", tmp, prefix, rand_digits());
      try(
        {
          Unix.mkdir(dir, 0o700);
          dir;
        }
      ) {
      | Unix.Unix_error(Unix.EEXIST, _, _) => loop(count - 1)
      | Unix.Unix_error(Unix.EINTR, _, _) => loop(count)
      | Unix.Unix_error(e, _, _) =>
        raise_err("mkTempDir: " ++ Unix.error_message(e))
      };
    };
  loop(1000);
};

let rmdir = path =>
  Unix.(
    try(rmdir(path) |> return) {
    | Unix_error(err, _, _) =>
      error(
        "can't remove directory '%s' because: '%s'",
        path,
        error_message(err),
      )
    }
  );

let unsafeFindHome = () =>
  Revery.(
    switch (Sys.getenv_opt("HOME"), Environment.os) {
    | (Some(dir), _) => dir
    | (None, Environment.Windows) => Sys.getenv("LOCALAPPDATA")
    | (None, _) => failwith("Could not find HOME dir")
    }
  );

let getOniDirectory = home =>
  Revery.(
    switch (Environment.os) {
    | Environment.Windows => Utility.join([home, "Oni2"]) |> return
    | _ => Utility.join([home, ".config", "oni2"]) |> return
    }
  );

let getHomeDirectory = () =>
  Unix.(
    try(unsafeFindHome() |> return) {
    | Unix_error(err, _, _) =>
      error("Cannot find home because: '%s'", error_message(err))
    | Failure(reason) => error("The OS could not be identified %s", reason)
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
    | bytes => Unix.write(destFile, buffer, 0, bytes) |> ignore |> copy_loop
    };
  copy_loop();
  Unix.close(sourceFile);
  Unix.close(destFile);
};

let copy = (source, dest) =>
  Unix.(
    try(copyFile(source, dest) |> return) {
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

let createOniConfiguration = (~configDir, ~file) => {
  let userConfigPath = Utility.join([configDir, file]);

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

let getPath = (dir, file) => return(Utility.join([dir, file]));

let createConfigIfNecessary = (configDir, file) =>
  Utility.join([configDir, file])
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
  getHomeDirectory()
  >>= getOniDirectory
  >>= (dir => getPath(dir, "extensions"))
  >>= getOrCreateConfigFolder;

let getOrCreateConfigFile = filename =>
  /* Get Oni Directory */
  getHomeDirectory()
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
  );
