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

let error = fmt => Printf.kprintf(msg => Error(msg), fmt);

let return = x => Ok(x);

let _fail = msg => Error(msg);

let (>>=) = on_success;
let (/\/=) = on_error;

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

let chmod = (path, ~perm=0o640, ()) =>
  Unix.(
    try (chmod(path, perm) |> return) {
    | Unix_error(_, _, _) => error("can't set permissions for '%s'", path)
    }
  );

let chown = (path, uid, gid) =>
  Unix.(
    try (chown(path, uid, gid) |> return) {
    | Unix_error(_, _, _) => error("can't set uid/gid for '%s'", path)
    }
  );

/**
  Permissions default to read/write permissions
*/
let mkdir = (path, ~perm=0o640, ()) =>
  Unix.(
    try (mkdir(path, perm) |> return) {
    | Unix_error(_, _, _) => error("can't create directory '%s'", path)
    }
  );

let rmdir = path =>
  Unix.(
    try (rmdir(path) |> return) {
    | Unix_error(_, _, _) => error("can't remove directory '%s'", path)
    }
  );

let getOniDirectory = home =>
  home ++ Utility.join([".config", "oni2"]) |> return;

let getHomeDirectory = () =>
  Unix.(
    try (getenv("HOME") |> return) {
    | Unix_error(_, _, _) => error("Cannot find home")
    }
  );

/**
   CopyFile:

   There is no native function to copy files (suprisingly)

   reference: https://ocaml.github.io/ocamlunix/ocamlunix.html#sec33
 */
let copyFile = (source, dest) => {
  let buffer_size = 8192;
  let buffer = Bytes.create(buffer_size);

  let sourceFileDescriptor = Unix.openfile(source, [O_RDONLY], 0);
  let destFileDescriptor =
    Unix.openfile(dest, [O_WRONLY, O_CREAT, O_TRUNC], 438);
  /**
    In the copy_loop function we do the copy by blocks of buffer_size bytes.
    We request buffer_size bytes to read. If read returns zero,
    we have reached the end of file and the copy is over.
    Otherwise we write the bytes we have read in the output file and start again.
   */
  let rec copy_loop = () =>
    switch (Unix.read(sourceFileDescriptor, buffer, 0, buffer_size)) {
    | 0 => ()
    | bytes =>
      Unix.write(destFileDescriptor, buffer, 0, bytes) |> ignore;
      copy_loop();
    };
  copy_loop();
  Unix.close(sourceFileDescriptor);
  Unix.close(destFileDescriptor);
};

let copy = (source, dest) =>
  Unix.handle_unix_error(copyFile, source, dest) |> return;

let createOniConfiguration = configDir => {
  let defaultConfigDir = Revery.Environment.getWorkingDirectory();
  let configurationPath =
    defaultConfigDir
    ++ Utility.join(["assets", "configuration", "configuration.json"]);
  let userConfigPath = configDir ++ "configuration.json";
  copy(configurationPath, userConfigPath);
};

let createOniDirectory = () =>
  getHomeDirectory()
  >>= getOniDirectory
  >>= (
    path =>
      stat(path)
      >>= (
        fun
        | Some(st) =>
          /* path already exists */
          isDir(st)
          >>= (
            () =>
              error("Path %s already exists", path) >>= createOniConfiguration
          )
        | None => mkdir(path, ()) >>= (_ => createOniConfiguration(path))
      )
  );
