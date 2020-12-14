open Oni_Core;
open Oni_Core.Utility;

module Log = (
  val Oni_Core.Log.withNamespace("Oni2.LanguageSupport.CodeLens")
);

// MODEL

type codeLens = {
  handle: int,
  lens: Exthost.CodeLens.t,
  uniqueId: string,
};

let lineNumber = ({lens, _}) =>
  Exthost.OneBasedRange.(lens.range.startLineNumber - 1);

let uniqueId = ({uniqueId, _}) => uniqueId;

let textFromExthost = (lens: Exthost.CodeLens.t) => {
  Exthost.Command.(
    lens.command
    |> OptionEx.flatMap(command => command.label)
    |> Option.map(Exthost.Label.toString)
    |> Option.value(~default="(null)")
  );
};

let text = ({lens, _}: codeLens) => textFromExthost(lens);

type provider = {
  handle: int,
  selector: Exthost.DocumentSelector.t,
};

type handleToLenses = IntMap.t(list(codeLens));

type model = {
  providers: list(provider),
  bufferToLenses: IntMap.t(handleToLenses),
  bufferToUnresolvedLenses: IntMap.t(list((int, Exthost.CodeLens.t))),
};

type outmsg =
  | Nothing
  | CodeLensesChanged({
      bufferId: int,
      lenses: list(codeLens),
    });

let get = (~bufferId, {bufferToLenses, _}) => {
  bufferToLenses
  |> IntMap.find_opt(bufferId)
  |> Option.value(~default=IntMap.empty)
  |> IntMap.bindings
  |> List.map(snd)
  |> List.flatten;
};

let initial = {
  providers: [],
  bufferToLenses: IntMap.empty,
  bufferToUnresolvedLenses: IntMap.empty,
};

[@deriving show]
type msg =
  | CodeLensesError(string)
  | CodeLensesReceived({
      handle: int,
      bufferId: int,
      lenses: list(Exthost.CodeLens.t),
    })
  | CodeLensResolved({
      handle: int,
      bufferId: int,
      oldLens: Exthost.CodeLens.t,
      resolvedLens: Exthost.CodeLens.t,
    })
  | CodeLensResolveFailed({
      handle: int,
      bufferId: int,
      lens: Exthost.CodeLens.t,
      msg: string,
    });

let register = (~handle: int, ~selector, model) => {
  ...model,
  providers: [{handle, selector}, ...model.providers],
};

let unregister = (~handle: int, model) => {
  ...model,
  providers: model.providers |> List.filter(prov => prov.handle != handle),
};

// UPDATE

let addLenses = (handle, bufferId, lenses, handleToLenses) => {
  let internalLenses =
    lenses
    |> List.map(lens =>
         {
           lens,
           handle,
           uniqueId:
             Printf.sprintf(
               "%d%d%d",
               handle,
               bufferId,
               Hashtbl.hash(textFromExthost(lens)),
             ),
         }
       );

  let sort = (lenses: list(codeLens)) =>
    lenses
    |> Base.List.dedup_and_sort(~compare=(lensA, lensB) => {
         Exthost.CodeLens.(
           {
             lensA.lens.range.startLineNumber - lensB.lens.range.startLineNumber;
           }
         )
       });

  handleToLenses
  |> IntMap.update(
       handle,
       fun
       | None => internalLenses |> sort |> Option.some
       | Some(prev) => prev @ internalLenses |> sort |> Option.some,
     );
};

let resolveLens = (~bufferId, ~handle, ~oldLens, ~resolvedLens, model) => {
  let bufferToUnresolvedLenses' =
    model.bufferToUnresolvedLenses
    |> IntMap.update(
         bufferId,
         fun
         | None => None
         | Some(lenses) => {
             lenses
             |> List.filter(((h, lens)) =>
                  handle != h
                  || Exthost.CodeLens.(lens.cacheId != oldLens.cacheId)
                )
             |> Option.some;
           },
       );

  let bufferToLenses' =
    model.bufferToLenses
    |> IntMap.update(
         bufferId,
         fun
         | None =>
           IntMap.empty
           |> addLenses(handle, bufferId, [resolvedLens])
           |> Option.some
         | Some(map) =>
           map |> addLenses(handle, bufferId, [resolvedLens]) |> Option.some,
       );

  {
    ...model,
    bufferToUnresolvedLenses: bufferToUnresolvedLenses',
    bufferToLenses: bufferToLenses',
  };
};

let update = (msg, model) =>
  switch (msg) {
  // TODO
  | CodeLensResolveFailed(_) => (model, Nothing)

  // TODO
  | CodeLensResolved({bufferId, handle, oldLens, resolvedLens}) =>
    let model' =
      model |> resolveLens(~handle, ~bufferId, ~oldLens, ~resolvedLens);

    let lenses = get(~bufferId, model');
    (model', CodeLensesChanged({bufferId, lenses}));

  | CodeLensesError(_) => (model, Nothing)
  | CodeLensesReceived({handle, bufferId, lenses}) =>
    let resolvedLenses =
      lenses
      |> List.filter((lens: Exthost.CodeLens.t) =>
           Option.is_some(lens.command)
         );

    let unresolvedLenses =
      lenses
      |> List.filter((lens: Exthost.CodeLens.t) =>
           Option.is_none(lens.command)
         )
      |> List.map(lens => (handle, lens));

    let bufferToLenses =
      model.bufferToLenses
      |> IntMap.update(
           bufferId,
           fun
           | None =>
             IntMap.empty
             |> addLenses(handle, bufferId, resolvedLenses)
             |> Option.some
           | Some(existing) =>
             existing
             |> addLenses(handle, bufferId, resolvedLenses)
             |> Option.some,
         );

    let bufferToUnresolvedLenses =
      model.bufferToUnresolvedLenses
      |> IntMap.update(
           bufferId,
           fun
           | None => Some(unresolvedLenses)
           | Some(cur) => Some(cur @ unresolvedLenses),
         );
    let model' = {...model, bufferToLenses, bufferToUnresolvedLenses};
    let lenses = get(~bufferId, model');
    (model', CodeLensesChanged({bufferId, lenses}));
  };

// CONFIGURATION

module VimSettings = {
  open Config.Schema;
  open VimSetting.Schema;

  let codeLens =
    vim("codelens", codeLensSetting => {
      codeLensSetting
      |> VimSetting.decode_value_opt(bool)
      |> Option.value(~default=false)
    });
};

module Configuration = {
  open Config.Schema;

  module Experimental = {
    let enabled =
      setting(
        ~vim=VimSettings.codeLens,
        "experimental.editor.codeLens",
        bool,
        ~default=false,
      );
  };
};

// SUBSCRIPTION

module Sub = {
  let create = (~visibleBuffers, ~visibleBuffersAndRanges, ~client, model) => {
    let codeLenses =
      visibleBuffers
      |> List.map(buffer => {
           model.providers
           |> List.filter(({selector, _}) =>
                Exthost.DocumentSelector.matchesBuffer(~buffer, selector)
              )
           |> List.map(({handle, _}) => {
                let toMsg =
                  fun
                  | Error(msg) => CodeLensesError(msg)
                  | Ok(lenses) =>
                    CodeLensesReceived({
                      handle,
                      bufferId: buffer |> Oni_Core.Buffer.getId,
                      lenses,
                    });

                Service_Exthost.Sub.codeLenses(
                  ~handle,
                  ~buffer,
                  ~toMsg,
                  client,
                );
              })
         })
      |> List.flatten;

    let codeLensResolve =
      visibleBuffersAndRanges
      |> List.map(((bufferId, ranges: list(EditorCoreTypes.Range.t))) => {
           let lenses =
             model.bufferToUnresolvedLenses
             |> IntMap.find_opt(bufferId)
             |> Option.value(~default=[]);

           lenses
           |> List.filter_map(((handle, lens)) => {
                let toMsg = maybeLens => {
                  switch (maybeLens) {
                  | Ok(resolvedLens) =>
                    CodeLensResolved({
                      handle,
                      bufferId,
                      oldLens: lens,
                      resolvedLens,
                    })
                  | Error(msg) =>
                    Log.errorf(m => m("Codelens resolve failed: %s", msg));
                    CodeLensResolveFailed({handle, bufferId, lens, msg});
                  };
                };

                if (ranges
                    |> List.exists(range => {
                         let startLine =
                           Exthost.(CodeLens.(lens.range.startLineNumber));
                         EditorCoreTypes.(
                           Range.contains(
                             Location.{
                               line: Index.fromOneBased(startLine),
                               column: Index.zero,
                             },
                             range,
                           )
                         );
                       })) {
                  Some(
                    Service_Exthost.Sub.codeLens(
                      ~toMsg,
                      ~handle,
                      ~lens,
                      client,
                    ),
                  );
                } else {
                  None;
                };
              });
         })
      |> List.flatten;

    codeLenses @ codeLensResolve |> Isolinear.Sub.batch;
  };
};

let sub = (~config, ~visibleBuffers, ~visibleBuffersAndRanges, ~client, model) =>
  if (Configuration.Experimental.enabled.get(config)) {
    Sub.create(~visibleBuffers, ~visibleBuffersAndRanges, ~client, model);
  } else {
    Isolinear.Sub.none;
  };

// COLORS

module Colors = {
  open Revery;
  open ColorTheme.Schema;

  let foreground =
    define("editorCodeLens.foreground", color(Color.hex("#999999")) |> all);
};

// CONTRIBUTIONS

module Contributions = {
  let colors = Colors.[foreground];

  let configuration = Configuration.[Experimental.enabled.spec];
};

// VIEW

module View = {
  module CodeLensColors = Colors;
  open Revery.UI;
  let make = (~leftMargin, ~theme, ~uiFont: Oni_Core.UiFont.t, ~codeLens, ()) => {
    let foregroundColor = CodeLensColors.foreground.from(theme);
    let text = text(codeLens);
    <View
      style=Style.[
        marginTop(4),
        marginBottom(0),
        marginLeft(leftMargin),
        flexGrow(1),
        flexShrink(0),
      ]>
      <Text
        text
        fontFamily={uiFont.family}
        fontSize={uiFont.size}
        style=Style.[
          color(foregroundColor),
          flexGrow(1),
          textWrap(Revery.TextWrapping.Wrap),
        ]
      />
    </View>;
  };
};
