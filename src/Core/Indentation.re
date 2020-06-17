/*
 * Indentation.re
 *
 * Helpers for dealing with indentation level
 */

let getLevel = (settings: IndentationSettings.t, text: string) => {
  let tabSize = settings.tabSize;

  let spaceCount = ref(0);
  let indentLevel = ref(0);

  let allWhitespace = ref(true);
  let i = ref(0);
  let len = String.length(text);

  while (i^ < len && allWhitespace^) {
    let c = text.[i^];

    switch (c) {
    | '\t' =>
      incr(indentLevel);
      spaceCount := 0;
    | ' ' =>
      if (spaceCount^ == 0) {
        incr(indentLevel);
      };
      incr(spaceCount);
      if (spaceCount^ == tabSize) {
        spaceCount := 0;
      };
    | _ => allWhitespace := false
    };

    incr(i);
  };

  allWhitespace^ ? 0 : indentLevel^;
};

let getForBuffer = (~buffer, configuration: Configuration.t) => {
  let bufferIndentation = Buffer.getIndentation(buffer);
  switch (bufferIndentation) {
  | None => IndentationSettings.ofConfiguration(configuration)
  | Some(indentation) => indentation
  };
};
