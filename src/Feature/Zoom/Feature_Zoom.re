open Oni_Core;

[@deriving show]
type msg = unit;

module Log = (val Log.withNamespace("Feature.Zoom"));

type model = {
  // Injected dependencies
  getZoom: unit => float,
  setZoom: float => unit,
  zoom: float,
};

let initial = (~getZoom, ~setZoom) => {getZoom, setZoom, zoom: 1.0};

type outmsg =
  | Effect(Isolinear.Effect.t(msg))
  | Nothing;

let zoom = ({zoom, _}) => zoom;

let update = (_msg, model) => (model, Nothing);

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
  open Oni_Core;
  open Config.Schema;

  let zoom = setting("ui.zoom", float, ~default=1.0);
};

let configurationChanged = (~config: Config.resolver, model) => {
  let zoom = Configuration.zoom.get(config);
  let model' = {...model, zoom};
  (model', Effects.setZoom(model'));
};

module Contributions = {
  let isMacCondition = "isMac" |> WhenExpr.parse;
  let keybindings =
    Feature_Input.Schema.[
      bind(
        ~key="<D-+>",
        ~command="workbench.action.zoomIn",
        ~condition=isMacCondition,
      ),
      bind(
        ~key="<C-+>",
        ~command="workbench.action.zoomIn",
        ~condition=WhenExpr.Value(True),
      ),
      bind(
        ~key="<D-->",
        ~command="workbench.action.zoomOut",
        ~condition=isMacCondition,
      ),
      bind(
        ~key="<C-->",
        ~command="workbench.action.zoomOut",
        ~condition=WhenExpr.Value(True),
      ),
      bind(
        ~key="<D-0>",
        ~command="workbench.action.zoomReset",
        ~condition=isMacCondition,
      ),
      bind(
        ~key="<C-0>",
        ~command="workbench.action.zoomReset",
        ~condition=WhenExpr.Value(True),
      ),
    ];

  let configuration = Configuration.[zoom.spec];
};
