/*
 * IconTheme.re
 *
 * Typing / schema for icon themes
 */

open Revery;

open Oni_Core;
open Oni_Core_Kernel;

module FontSource = {
  [@deriving (show({with_path: false}), yojson({strict: false}))]
  type t = {
    path: string,
    /* format: string, */
  };
};

[@deriving (show({with_path: false}), yojson({strict: false}))]
type fontSources = list(FontSource.t);

module Font = {
  [@deriving (show({with_path: false}), yojson({strict: false}))]
  type t = {
    id: string,
    src: list(FontSource.t),
    weight: string,
    style: string,
    size: string,
  };
};

[@deriving (show({with_path: false}), yojson({strict: false}))]
type fonts = list(Font.t);

module IconDefinition = {
  [@deriving (show({with_path: false}), yojson({strict: false}))]
  type raw = {
    fontCharacter: string,
    fontColor: string,
  };

  type t = {
    /* id: string, */
    fontCharacter: int,
    fontColor: Color.t,
  };

  let parseId: string => int =
    v => {
      let len = String.length(v);
      let sub = String.sub(v, 1, len - 1);
      "0x" ++ sub |> int_of_string;
    };

  let of_raw: raw => t =
    raw => {
      {
        /* id: raw.id, */
        fontCharacter: parseId(raw.fontCharacter),
        fontColor: Color.hex(raw.fontColor),
      };
    };
};

type t = {
  fonts: list(Font.t),
  iconDefinitions: StringMap.t(IconDefinition.t),
  file: string,
  fileExtensions: StringMap.t(string),
  fileNames: StringMap.t(string),
  languageIds: StringMap.t(string),
  /* TODO: Light mode */
};

let normalizeExtension = s =>
  if (String.length(s) > 1 && Char.equal(s.[0], '.')) {
    String.sub(s, 1, String.length(s) - 1);
  } else {
    s;
  };

let getIconForFile: (t, string, string) => option(IconDefinition.t) =
  (iconTheme: t, fileName: string, languageId: string) => {
    let id =
      switch (StringMap.find_opt(fileName, iconTheme.fileNames)) {
      | Some(v) => v
      | None =>
        switch (
          StringMap.find_opt(
            normalizeExtension(Rench.Path.extname(fileName)),
            iconTheme.fileExtensions,
          )
        ) {
        | Some(v) => v
        | None =>
          switch (StringMap.find_opt(languageId, iconTheme.languageIds)) {
          | Some(v) => v
          | None => iconTheme.file
          }
        }
      };

    StringMap.find_opt(id, iconTheme.iconDefinitions);
  };

let create = () => {
  fonts: [],
  iconDefinitions: StringMap.empty,
  file: "",
  fileExtensions: StringMap.empty,
  fileNames: StringMap.empty,
  languageIds: StringMap.empty,
};

let assocToStringMap = json => {
  switch (json) {
  | `Assoc(v) =>
    List.fold_left(
      (prev, curr) =>
        switch (curr) {
        | (s, `String(v)) => StringMap.add(s, v, prev)
        | _ => prev
        },
      StringMap.empty,
      v,
    )
  | _ => StringMap.empty
  };
};

let getOrEmpty = (v: result(list('a), 'b)) => {
  switch (v) {
  | Ok(v) => v
  | Error(_) => []
  };
};

let ofJson = (json: Yojson.Safe.t) => {
  open Yojson.Safe.Util;
  let fonts = json |> member("fonts") |> fonts_of_yojson |> getOrEmpty;
  let icons = json |> member("iconDefinitions") |> to_assoc;
  let file = json |> member("file") |> to_string;
  let extensionsJson = json |> member("fileExtensions");
  let fileNamesJson = json |> member("fileNames");
  let languageIdsJson = json |> member("languageIds");

  let toIconMap:
    list((string, Yojson.Safe.t)) => StringMap.t(IconDefinition.t) =
    icons => {
      List.fold_left(
        (prev, curr) => {
          let (id, jsonItem) = curr;
          switch (IconDefinition.raw_of_yojson(jsonItem)) {
          | Ok(v) =>
            let icon = IconDefinition.of_raw(v);
            StringMap.add(id, icon, prev);
          | Error(_) => prev
          };
        },
        StringMap.empty,
        icons,
      );
    };

  let iconDefinitions = icons |> toIconMap;
  let fileExtensions = assocToStringMap(extensionsJson);
  let fileNames = assocToStringMap(fileNamesJson);
  let languageIds = assocToStringMap(languageIdsJson);

  Some({
    fonts,
    iconDefinitions,
    file,
    fileExtensions,
    fileNames,
    languageIds,
  });
};
