/*
 * Indentation.re
 *
 * Helpers for dealing with indentation level
 */

Printexc.record_backtrace(true);

let getLevel = (settings: IndentationSettings.t, text: string) => {

  let tabSize = settings.tabSize;

  let spaceCount = ref(0);
  let indentLevel = ref(0);

  let allWhitespace = ref(true);
  let i = ref(0);
  let len = String.length(text);

  while(i^ < len && allWhitespace^) {

    let c = String.get(text, i^);

    switch (c) {
    | '\t' =>
        incr(indentLevel);
        spaceCount := 0;
    | ' ' =>
        incr(spaceCount)
        if (spaceCount^ == tabSize) {
            incr(indentLevel)
            spaceCount := 0;
        }
    | _ => allWhitespace := false
    };


    incr(i);
  }

  allWhitespace^ ? 0 : indentLevel^
};
