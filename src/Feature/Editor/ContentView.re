open EditorCoreTypes;

open Oni_Core;
open Utility;

open Helpers;

module Diagnostic = Feature_Diagnostics.Diagnostic;

let renderDiagnostics =
    (
      ~context: Draw.context,
      ~colors: Colors.t,
      ~diagnosticsMap,
      ~languageConfiguration,
      viewLine,
      _offset,
    ) => {
  let topViewLine = Editor.getTopViewLine(context.editor);
  let bottomViewLine = Editor.getBottomViewLine(context.editor);

  // Since this drawing logic is per-buffer-line, we only want to draw
  // each buffer once. So, we'll draw the _primary_ view line (the
  // first one for a buffer line), or the top/bottom if we're in the middle
  // of a buffer line.
  let shouldRenderViewLine =
    Editor.viewLineIsPrimary(viewLine, context.editor)
    || viewLine == topViewLine
    || viewLine == bottomViewLine;

  if (shouldRenderViewLine) {
    let index = Editor.viewLineToBufferLine(viewLine, context.editor);
    let item = EditorCoreTypes.LineNumber.toZeroBased(index);

    let renderDiagnostics = (colors: Colors.t, diagnostic: Diagnostic.t) => {
      // If at single character, get the token under the cursor
      let range =
        if (diagnostic.range.start == diagnostic.range.stop) {
          Editor.getTokenAt(
            ~languageConfiguration,
            diagnostic.range.start,
            context.editor,
          )
          |> Option.map(range
               // In addition, bump the end position out - the range returned from the editor is inclusive,
               // but the squiggly rendering is exclusive.
               =>
                 CharacterRange.{
                   start: range.start,
                   stop: {
                     ...range.stop,
                     character: CharacterIndex.(range.stop.character + 1),
                   },
                 }
               )
          |> Option.value(~default=diagnostic.range);
        } else {
          diagnostic.range;
        };

      let color =
        Exthost.Diagnostic.Severity.(
          switch (diagnostic.severity) {
          | Error => colors.errorForeground
          | Warning => colors.warningForeground
          | Hint => colors.hintForeground
          | Info => colors.infoForeground
          }
        );

      if (diagnostic.isDeprecated
          && Editor.shouldShowDeprecated(context.editor)) {
        let color = colors.editorForeground;
        Draw.strikethrough(~context, ~color, range);
      } else if (diagnostic.isUnused
                 && Editor.shouldShowUnused(context.editor)) {
        let invAlpha = colors.unnecessaryCodeOpacity |> Revery.Color.getAlpha;
        let alpha = 1.0 -. invAlpha;
        let color =
          colors.editorBackground |> Revery.Color.multiplyAlpha(alpha);
        Draw.rangeCharacter(~context, ~color, range);
      } else {
        Draw.squiggly(~context, ~color, range);
      };
    };

    /* Draw error markers */
    switch (IntMap.find_opt(item, diagnosticsMap)) {
    | None => ()
    | Some(diagnostics) => List.iter(renderDiagnostics(colors), diagnostics)
    };
  };
};

let renderLine =
    (
      ~context: Draw.context,
      ~buffer,
      ~colors: Colors.t,
      ~selectionRanges,
      ~matchingPairs: option((CharacterPosition.t, CharacterPosition.t)),
      ~vim,
      ~languageSupport,
      viewLine,
      _offset,
    ) => {
  let topViewLine = Editor.getTopViewLine(context.editor);
  let bottomViewLine = Editor.getBottomViewLine(context.editor);

  // Since this drawing logic is per-buffer-line, we only want to draw
  // each buffer once. So, we'll draw the _primary_ view line (the
  // first one for a buffer line), or the top/bottom if we're in the middle
  // of a buffer line.
  let shouldRenderViewLine =
    Editor.viewLineIsPrimary(viewLine, context.editor)
    || viewLine == topViewLine
    || viewLine == bottomViewLine;

  if (shouldRenderViewLine) {
    let index = Editor.viewLineToBufferLine(viewLine, context.editor);

    switch (Hashtbl.find_opt(selectionRanges, index)) {
    | None => ()
    | Some(selections) =>
      List.iter(
        Draw.rangeByte(~context, ~color=colors.selectionBackground),
        selections,
      )
    };

    /* Draw match highlights */
    switch (matchingPairs) {
    | None => ()
    | Some((startPos, endPos)) =>
      Draw.rangeCharacter(
        ~context,
        ~color=colors.selectionBackground,
        CharacterRange.{start: startPos, stop: startPos},
      );
      Draw.rangeCharacter(
        ~context,
        ~color=colors.selectionBackground,
        CharacterRange.{start: endPos, stop: endPos},
      );
    };

    let bufferId = Buffer.getId(buffer);
    /* Draw search highlights */
    Feature_Vim.getSearchHighlightsByLine(~bufferId, ~line=index, vim)
    |> List.iter(
         Draw.rangeByte(
           ~context,
           ~padding=1.,
           ~color=colors.findMatchBackground,
         ),
       );

    /* Draw document highlights */
    Feature_LanguageSupport.DocumentHighlights.getByLine(
      ~bufferId,
      ~line=index |> EditorCoreTypes.LineNumber.toZeroBased,
      languageSupport,
    )
    |> List.iter(
         Draw.rangeCharacter(
           ~context,
           ~padding=1.,
           ~color=colors.wordHighlightBackground,
         ),
       );
  };
};

let renderEmbellishments =
    (
      ~context,
      ~buffer,
      ~colors,
      ~selectionRanges,
      ~matchingPairs,
      ~vim,
      ~languageSupport,
    ) =>
  Draw.renderImmediate(
    ~context,
    renderLine(
      ~context,
      ~buffer,
      ~colors,
      ~selectionRanges,
      ~matchingPairs,
      ~vim,
      ~languageSupport,
    ),
  );

let renderDefinition =
    (
      ~context,
      ~bufferId,
      ~languageConfiguration,
      ~languageSupport,
      ~cursorPosition: CharacterPosition.t,
      ~editor,
      ~colors: Colors.t,
      ~bufferSyntaxHighlights,
    ) => {
  Editor.getTokenAt(~languageConfiguration, cursorPosition, editor)
  |> Option.iter((range: CharacterRange.t) => {
       Feature_LanguageSupport.Definition.getAt(
         ~bufferId,
         ~range,
         languageSupport,
       )
       |> Option.iter(_ => {
            let color =
              Editor.characterToByte(range.start, editor)
              |> OptionEx.flatMap(bytePosition => {
                   Feature_Syntax.getAt(
                     ~bufferId,
                     ~bytePosition,
                     bufferSyntaxHighlights,
                   )
                 })
              |> Option.map((themeToken: Oni_Core.ThemeToken.t) =>
                   themeToken.foregroundColor
                 )
              |> Option.value(~default=colors.editorForeground);

            // Extend range by one character - the range
            // returned by getTokenAt is inclusive, but the
            // range underline uses is exclusive.

            let drawRange =
              CharacterRange.{
                start: range.start,
                stop: {
                  line: range.stop.line,
                  character: CharacterIndex.(range.stop.character + 1),
                },
              };

            Draw.underline(~context, ~color, drawRange);
          })
     });
};

let renderTokens =
    (
      ~offsetY,
      ~selection,
      ~context,
      ~colors,
      ~tokens,
      ~shouldRenderWhitespace,
    ) => {
  tokens
  |> WhitespaceTokenFilter.filter(~selection, shouldRenderWhitespace)
  |> List.iter(Draw.token(~offsetY, ~context, ~colors));
};

let renderText =
    (
      ~context,
      ~selectionRanges,
      ~editor,
      ~vim,
      ~cursorLine,
      ~colors,
      ~matchingPairs,
      ~bufferSyntaxHighlights,
      ~shouldRenderWhitespace,
    ) =>
  Draw.renderImmediate(
    ~context,
    (item, offsetY) => {
      let index = EditorCoreTypes.LineNumber.ofZeroBased(item);
      let selectionRange =
        switch (Hashtbl.find_opt(selectionRanges, index)) {
        | None => None
        | Some(v) =>
          switch (List.length(v)) {
          | 0 => None
          | _ => Some(List.hd(v))
          }
        };

      let tokens =
        getTokensForLine(
          ~editor,
          ~vim,
          ~cursorLine,
          ~colors,
          ~matchingPairs,
          ~bufferSyntaxHighlights,
          ~selection=selectionRange,
          ~scrollX=Editor.scrollX(editor),
          item,
        );

      renderTokens(
        ~selection=selectionRange,
        ~context,
        ~colors,
        ~tokens,
        ~shouldRenderWhitespace,
        ~offsetY,
      );
    },
  );

let render =
    (
      ~context,
      ~buffer,
      ~editor,
      ~colors,
      ~diagnosticsMap,
      ~selectionRanges,
      ~matchingPairs: option((CharacterPosition.t, CharacterPosition.t)),
      ~vim,
      ~cursorPosition: CharacterPosition.t,
      ~languageSupport,
      ~languageConfiguration,
      ~bufferSyntaxHighlights,
      ~shouldRenderWhitespace,
    ) => {
  renderEmbellishments(
    ~context,
    ~buffer,
    ~colors,
    ~selectionRanges,
    ~matchingPairs,
    ~vim,
    ~languageSupport,
  );

  let bufferId = Buffer.getId(buffer);
  if (Feature_LanguageSupport.Definition.isAvailable(
        ~bufferId,
        languageSupport,
      )) {
    renderDefinition(
      ~bufferId,
      ~languageSupport,
      ~languageConfiguration,
      ~context,
      ~editor,
      ~cursorPosition,
      ~colors,
      ~bufferSyntaxHighlights,
    );
  };

  renderText(
    ~context,
    ~selectionRanges,
    ~editor,
    ~vim,
    ~cursorLine=EditorCoreTypes.LineNumber.toZeroBased(cursorPosition.line),
    ~colors,
    ~matchingPairs,
    ~bufferSyntaxHighlights,
    ~shouldRenderWhitespace,
  );

  Draw.renderImmediate(
    ~context,
    renderDiagnostics(
      ~context,
      ~colors,
      ~diagnosticsMap,
      ~languageConfiguration,
    ),
  );
};
