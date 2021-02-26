open Oni_Core;
open Oni_Model;
open Oni_Store;
open Feature_Editor;

module LineNumber = EditorCoreTypes.LineNumber;

Vim.init();

let config = (~vimSetting as _, _) => Config.NotSet;

/* Create a state with some editor size */
let simpleState = {
  let initialBuffer = {
    let Vim.BufferMetadata.{id, version, filePath, modified, _} =
      Vim.Buffer.openFile("untitled") |> Vim.BufferMetadata.ofBuffer;
    Buffer.ofMetadata(
      ~font=Font.default(),
      ~id,
      ~version,
      ~filePath,
      ~modified,
    );
  };

  let state =
    State.initial(
      ~cli=Oni_CLI.default,
      ~initialBuffer,
      ~initialBufferRenderers=BufferRenderers.initial,
      ~getUserSettings=() => Ok(Config.Settings.empty),
      ~keybindingsLoader=Feature_Input.KeybindingsLoader.none,
      ~extensionGlobalPersistence=Feature_Extensions.Persistence.initial,
      ~extensionWorkspacePersistence=Feature_Extensions.Persistence.initial,
      ~maybeWorkspace=None,
      ~workingDirectory=Sys.getcwd(),
      ~extensionsFolder=None,
      ~licenseKeyPersistence=None,
      ~titlebarHeight=0.,
      ~getZoom=() => 1.0,
      ~setZoom=_zoom => (),
    );

  Reducer.reduce(
    state,
    Actions.EditorGroupSizeChanged({id: 0, width: 3440, height: 1440}),
  );
};

let defaultFont: Service_Font.font = {
  let fontFamily = Revery.Font.Family.fromFile("JetBrainsMono-Regular.ttf");
  let fontSize = 10.;
  let fontWeight = Revery.Font.Weight.Normal;
  let smoothing = Revery.Font.Smoothing.default;
  let features = [];

  {
    fontFamily,
    fontSize,
    fontWeight,
    spaceWidth: 10.,
    underscoreWidth: 10.,
    avgCharWidth: 10.,
    maxCharWidth: 10.,
    measuredHeight: 10.,
    descenderHeight: 1.,
    smoothing,
    features,
    measurementCache:
      FontMeasurementCache.create(
        ~fontFamily,
        ~fontSize,
        ~fontWeight,
        ~smoothing,
        ~features,
      ),
  };
};

let simpleState =
  Reducer.reduce(
    simpleState,
    Actions.EditorFont(Service_Font.FontLoaded(defaultFont)),
  );

let thousandLines =
  Array.make(1000, "This is a buffer with a thousand lines!");

let defaultBuffer =
  Oni_Core.Buffer.ofLines(~font=defaultFont, ~id=0, thousandLines);
let defaultEditorBuffer =
  defaultBuffer |> Feature_Editor.EditorBuffer.ofBuffer;

let simpleEditor =
  Editor.create(~config, ~buffer=defaultEditorBuffer, ~preview=false, ())
  |> Editor.setSize(~pixelWidth=3440, ~pixelHeight=1440);

let createUpdateAction = (oldBuffer: Buffer.t, update: BufferUpdate.t) => {
  let newBuffer = Buffer.update(oldBuffer, update);
  Actions.Buffers(
    Feature_Buffers.Msg.updated(
      ~update,
      ~oldBuffer,
      ~newBuffer,
      ~triggerKey=None,
    ),
  );
};

let thousandLineBuffer = Buffer.ofLines(~font=Font.default(), thousandLines);

let thousandLineState =
  Reducer.reduce(
    simpleState,
    createUpdateAction(
      thousandLineBuffer,
      BufferUpdate.create(
        ~startLine=LineNumber.zero,
        ~endLine=LineNumber.ofZeroBased(1),
        ~lines=thousandLines,
        ~version=1,
        (),
      ),
    ),
  );

/* Apply indents so we go into deeper and deeper indentation, before returning back. */
let applyIndent = (line, lineNum, fileLen) =>
  if (lineNum < fileLen / 2) {
    String.make(lineNum, '\t') ++ line;
  } else {
    String.make(fileLen - lineNum, '\t') ++ line;
  };

let thousandLinesWithIndents =
  Array.mapi((i, l) => applyIndent(l, i, 1000), thousandLines);

let thousandLineStateWithIndents =
  Reducer.reduce(
    simpleState,
    createUpdateAction(
      thousandLineBuffer,
      BufferUpdate.create(
        ~startLine=LineNumber.zero,
        ~endLine=LineNumber.ofZeroBased(1),
        ~lines=thousandLinesWithIndents,
        ~version=1,
        (),
      ),
    ),
  );

let hundredThousandLines =
  Array.make(100000, "This is a buffer with a hundred thousand lines!");

let hundredThousandLineState =
  Reducer.reduce(
    simpleState,
    createUpdateAction(
      Buffer.ofLines(~font=Font.default(), [||]),
      BufferUpdate.create(
        ~startLine=LineNumber.zero,
        ~endLine=LineNumber.ofZeroBased(1),
        ~lines=hundredThousandLines,
        ~version=1,
        (),
      ),
    ),
  );
