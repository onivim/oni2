open Oni_Core;

module Constants = {
  let zoomStep = 0.2;
  let defaultZoomValue = 1.0;
  let minZoomValue = 0.2;
  let maxZoomValue = 2.8;
};

[@deriving show]
type command =
  | ZoomIn
  | ZoomOut
  | ZoomReset;

[@deriving show]
type msg =
  | Command(command);

module Log = (val Log.withNamespace("Feature.Zoom"));

type model = {
  // Injected dependencies
  getZoom: unit => float,
  setZoom: float => unit,
  zoom: float,
};

let initial = (~getZoom, ~setZoom) => {
  getZoom,
  setZoom,
  zoom: Constants.defaultZoomValue,
};

type outmsg =
  | Effect(Isolinear.Effect.t(msg))
  | UpdateConfiguration(ConfigurationTransformer.t)
  | Nothing;

let zoom = ({zoom, _}) => zoom;

module Internal = {
  let updateZoom = (model, zoomFn) => {
    let currentZoomValue = model.zoom;

    let calculatedZoomValue = zoomFn(currentZoomValue);
    let newZoomValue =
      Utility.IntEx.clamp(
        calculatedZoomValue,
        ~hi=Constants.maxZoomValue,
        ~lo=Constants.minZoomValue,
      );

    if (newZoomValue != currentZoomValue) {
      (
        {...model, zoom: newZoomValue},
        UpdateConfiguration(
          ConfigurationTransformer.setField("ui.zoom", `Float(newZoomValue)),
        ),
      );
    } else {
      (model, Nothing);
    };
  };

  let zoomIn = model => updateZoom(model, zoom => zoom +. Constants.zoomStep);

  let zoomOut = model => updateZoom(model, zoom => zoom -. Constants.zoomStep);

  let zoomReset = model =>
    updateZoom(model, _zoom => Constants.defaultZoomValue);
};

let update = (msg, model) =>
  switch (msg) {
  | Command(ZoomIn) => Internal.zoomIn(model)
  | Command(ZoomOut) => Internal.zoomOut(model)
  | Command(ZoomReset) => Internal.zoomReset(model)
  };

module Effects = {
  let setZoom = model =>
    Isolinear.Effect.create(~name="zoom.synchronize", () => {
      let currentZoom = model.getZoom();

      if (model.zoom != currentZoom) {
        Log.infof(m => m("Setting zoom: %f", model.zoom));
        model.setZoom(model.zoom);
      };
    });
};

module Configuration = {
  open Config.Schema;

  let zoom = setting("ui.zoom", float, ~default=Constants.defaultZoomValue);
};

let configurationChanged = (~config: Config.resolver, model) => {
  let zoom = Configuration.zoom.get(config);
  let model' = {...model, zoom};
  (model', Effects.setZoom(model'));
};

module Commands = {
  open Feature_Commands.Schema;

  let zoomIn =
    define(
      ~category="View",
      ~title="Zoom In",
      "workbench.action.zoomIn",
      Command(ZoomIn),
    );

  let zoomOut =
    define(
      ~category="View",
      ~title="Zoom Out",
      "workbench.action.zoomOut",
      Command(ZoomOut),
    );

  let zoomReset =
    define(
      ~category="View",
      ~title="Zoom Reset",
      "workbench.action.zoomReset",
      Command(ZoomReset),
    );
};

module Contributions = {
  let isMacCondition = "isMac" |> WhenExpr.parse;
  let keybindings =
    Feature_Input.Schema.[
      bind(
        ~key="<D-+>",
        ~command=Commands.zoomIn.id,
        ~condition=isMacCondition,
      ),
      bind(
        ~key="<C-+>",
        ~command=Commands.zoomIn.id,
        ~condition=WhenExpr.Value(True),
      ),
      bind(
        ~key="<D-->",
        ~command=Commands.zoomOut.id,
        ~condition=isMacCondition,
      ),
      bind(
        ~key="<C-->",
        ~command=Commands.zoomOut.id,
        ~condition=WhenExpr.Value(True),
      ),
      bind(
        ~key="<D-0>",
        ~command=Commands.zoomReset.id,
        ~condition=isMacCondition,
      ),
      bind(
        ~key="<C-0>",
        ~command=Commands.zoomReset.id,
        ~condition=WhenExpr.Value(True),
      ),
    ];

  let commands = Commands.[zoomIn, zoomOut, zoomReset];

  let configuration = Configuration.[zoom.spec];
};
