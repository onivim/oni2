open EditorCoreTypes;
open Revery;
open Revery.Draw;

open Oni_Core;

open Helpers;

module FontIcon = Oni_Components.FontIcon;
module Diagnostic = Feature_LanguageSupport.Diagnostic;
module Definition = Feature_LanguageSupport.Definition;

let renderLine =
    (
      ~transform,
      ~buffer,
      ~gutterWidth,
      ~leftVisibleColumn,
      ~scrollY,
      ~editorFont,
      ~theme: Theme.t,
      ~diagnosticsMap,
      ~selectionRanges,
      ~matchingPairs,
      ~bufferHighlights,
      item,
      _offset,
    ) => {
  let index = Index.fromZeroBased(item);
  let renderDiagnostics = (d: Diagnostic.t) =>
    DrawPrimitives.drawUnderline(
      ~buffer,
      ~gutterWidth,
      ~leftVisibleColumn,
      ~transform,
      ~scrollY,
      ~editorFont,
      ~color=Colors.red,
      d.range,
    );

  /* Draw error markers */
  switch (IntMap.find_opt(item, diagnosticsMap)) {
  | None => ()
  | Some(v) => List.iter(renderDiagnostics, v)
  };

  switch (Hashtbl.find_opt(selectionRanges, index)) {
  | None => ()
  | Some(v) =>
    List.iter(
      DrawPrimitives.renderRange(
        ~buffer,
        ~gutterWidth,
        ~leftVisibleColumn,
        ~transform,
        ~scrollY,
        ~editorFont,
        ~color=theme.editorSelectionBackground,
      ),
      v,
    )
  };

  /* Draw match highlights */
  let matchColor = theme.editorSelectionBackground;
  switch (matchingPairs) {
  | None => ()
  | Some((startPos, endPos)) =>
    DrawPrimitives.renderRange(
      ~buffer,
      ~gutterWidth,
      ~leftVisibleColumn,
      ~transform,
      ~scrollY,
      ~editorFont,
      ~color=matchColor,
      Range.{start: startPos, stop: startPos},
    );
    DrawPrimitives.renderRange(
      ~buffer,
      ~gutterWidth,
      ~leftVisibleColumn,
      ~transform,
      ~scrollY,
      ~editorFont,
      ~color=matchColor,
      Range.{start: endPos, stop: endPos},
    );
  };

  /* Draw search highlights */
  BufferHighlights.getHighlightsByLine(
    ~bufferId=Buffer.getId(buffer),
    ~line=index,
    bufferHighlights,
  )
  |> List.iter(
       DrawPrimitives.renderRange(
         ~buffer,
         ~editorFont,
         ~gutterWidth,
         ~leftVisibleColumn,
         ~transform,
         ~scrollY,
         ~offset=2.0,
         ~color=theme.editorFindMatchBackground,
       ),
     );
};

let renderEmbellishments =
    (
      ~scrollY,
      ~rowHeight,
      ~height,
      ~count,
      ~transform,
      ~buffer,
      ~editorFont,
      ~gutterWidth,
      ~leftVisibleColumn,
      ~theme,
      ~diagnosticsMap,
      ~selectionRanges,
      ~matchingPairs,
      ~bufferHighlights,
    ) =>
  ImmediateList.render(
    ~scrollY,
    ~rowHeight,
    ~height=float(height),
    ~count,
    ~render=
      renderLine(
        ~transform,
        ~buffer,
        ~scrollY,
        ~editorFont,
        ~gutterWidth,
        ~leftVisibleColumn,
        ~theme,
        ~diagnosticsMap,
        ~selectionRanges,
        ~matchingPairs,
        ~bufferHighlights,
      ),
    (),
  );

let renderDefinition =
    (
      ~leftVisibleColumn,
      ~layout: EditorLayout.t,
      ~cursorPosition: Location.t,
      ~buffer,
      ~gutterWidth,
      ~transform,
      ~scrollY,
      ~editorFont,
      ~bufferHighlights,
      ~theme,
      ~matchingPairs,
      ~bufferSyntaxHighlights,
    ) =>
  getTokenAtPosition(
    ~buffer,
    ~bufferHighlights,
    ~cursorLine=Index.toZeroBased(cursorPosition.line),
    ~theme,
    ~matchingPairs,
    ~bufferSyntaxHighlights,
    ~startIndex=leftVisibleColumn,
    ~endIndex=leftVisibleColumn + layout.bufferWidthInCharacters,
    cursorPosition,
  )
  |> Utility.Option.iter((token: BufferViewTokenizer.t) => {
       let range =
         Range.{
           start:
             Location.{
               line: cursorPosition.line,
               column: token.startPosition,
             },
           stop:
             Location.{line: cursorPosition.line, column: token.endPosition},
         };
       DrawPrimitives.drawUnderline(
         ~buffer,
         ~editorFont,
         ~gutterWidth,
         ~leftVisibleColumn,
         ~transform,
         ~scrollY,
         ~color=token.color,
         range,
       );
     });

let renderWhitespace =
    (
      ~editorFont: EditorFont.t,
      ~x: float,
      ~y: float,
      ~transform,
      ~count: int,
      ~theme: Theme.t,
      (),
    ) => {
  let size = 2.;
  let xOffset = editorFont.measuredWidth /. 2. -. 1.;
  let yOffset = editorFont.measuredHeight /. 2. -. 1.;

  for (i in 0 to count - 1) {
    let xPos = x +. editorFont.measuredWidth *. float(i);

    Shapes.drawRect(
      ~transform,
      ~x=xPos +. xOffset,
      ~y=y +. yOffset,
      ~width=size,
      ~height=size,
      ~color=theme.editorWhitespaceForeground,
      (),
    );
  };
};

let renderTokens =
    (
      editorFont: EditorFont.t,
      gutterWidth: float,
      theme: Theme.t,
      tokens,
      xOffset: float,
      yOffset: float,
      transform,
      whitespaceSetting: ConfigurationValues.editorRenderWhitespace,
    ) => {
  let yF = yOffset;
  let xF = xOffset;

  let f = (token: BufferViewTokenizer.t) => {
    let x =
      gutterWidth
      +. editorFont.measuredWidth
      *. float(Index.toZeroBased(token.startPosition))
      -. xF;
    let y = yF;

    let backgroundColor = token.backgroundColor;

    switch (token.tokenType) {
    | Text =>
      Revery.Draw.Text.drawString(
        ~window=Revery.UI.getActiveWindow(),
        ~transform,
        ~x,
        ~y,
        ~backgroundColor,
        ~color=token.color,
        ~fontFamily=editorFont.fontFile,
        ~fontSize=editorFont.fontSize,
        token.text,
      )
    | Tab =>
      Revery.Draw.Text.drawString(
        ~window=Revery.UI.getActiveWindow(),
        ~transform,
        ~x=x +. editorFont.measuredWidth /. 4.,
        ~y=y +. editorFont.measuredHeight /. 4.,
        ~backgroundColor,
        ~color=theme.editorWhitespaceForeground,
        ~fontFamily="FontAwesome5FreeSolid.otf",
        ~fontSize=10,
        FontIcon.codeToIcon(0xf30b),
      )
    | Whitespace =>
      renderWhitespace(
        ~editorFont,
        ~x,
        ~y,
        ~transform,
        ~count=String.length(token.text),
        ~theme,
        (),
      )
    };
  };

  tokens |> WhitespaceTokenFilter.filter(whitespaceSetting) |> List.iter(f);
};

let renderText =
    (
      ~scrollY,
      ~rowHeight,
      ~height,
      ~count,
      ~selectionRanges,
      ~buffer,
      ~bufferHighlights,
      ~cursorLine,
      ~theme,
      ~matchingPairs,
      ~bufferSyntaxHighlights,
      ~leftVisibleColumn,
      ~layout: EditorLayout.t,
      ~editorFont,
      ~gutterWidth,
      ~editor: Editor.t,
      ~transform,
      ~shouldRenderWhitespace,
    ) =>
  ImmediateList.render(
    ~scrollY,
    ~rowHeight,
    ~height=float(height),
    ~count,
    ~render=
      (item, offset) => {
        let index = Index.fromZeroBased(item);
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
            ~buffer,
            ~bufferHighlights,
            ~cursorLine,
            ~theme,
            ~matchingPairs,
            ~bufferSyntaxHighlights,
            ~selection=selectionRange,
            leftVisibleColumn,
            leftVisibleColumn + layout.bufferWidthInCharacters,
            item,
          );

        let _ =
          renderTokens(
            editorFont,
            gutterWidth,
            theme,
            tokens,
            editor.scrollX,
            offset,
            transform,
            shouldRenderWhitespace,
          );
        ();
      },
    (),
  );

let render =
    (
      ~scrollY,
      ~rowHeight,
      ~height,
      ~count,
      ~transform,
      ~buffer,
      ~editor,
      ~editorFont,
      ~gutterWidth,
      ~leftVisibleColumn,
      ~theme,
      ~diagnosticsMap,
      ~selectionRanges,
      ~matchingPairs,
      ~bufferHighlights,
      ~cursorPosition: Location.t,
      ~definition,
      ~layout,
      ~bufferSyntaxHighlights,
      ~shouldRenderWhitespace,
    ) => {
  renderEmbellishments(
    ~scrollY,
    ~rowHeight,
    ~height,
    ~count,
    ~transform,
    ~buffer,
    ~editorFont,
    ~gutterWidth,
    ~leftVisibleColumn,
    ~theme,
    ~diagnosticsMap,
    ~selectionRanges,
    ~matchingPairs,
    ~bufferHighlights,
  );

  if (Definition.isAvailable(
        Buffer.getId(buffer),
        cursorPosition,
        definition,
      )) {
    renderDefinition(
      ~leftVisibleColumn,
      ~layout,
      ~cursorPosition,
      ~buffer,
      ~editorFont,
      ~gutterWidth,
      ~transform,
      ~scrollY,
      ~bufferHighlights,
      ~theme,
      ~matchingPairs,
      ~bufferSyntaxHighlights,
    );
  };

  renderText(
    ~scrollY,
    ~rowHeight,
    ~height,
    ~count,
    ~selectionRanges,
    ~buffer,
    ~bufferHighlights,
    ~cursorLine=Index.toZeroBased(cursorPosition.line),
    ~theme,
    ~matchingPairs,
    ~bufferSyntaxHighlights,
    ~leftVisibleColumn,
    ~layout,
    ~gutterWidth,
    ~editor,
    ~editorFont,
    ~transform,
    ~shouldRenderWhitespace,
  );
};
