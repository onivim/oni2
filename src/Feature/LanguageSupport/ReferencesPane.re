open EditorCoreTypes;
open Oni_Core;

type location = {
  path: string,
  ranges: list(CharacterRange.t),
};

type model = {
  locationNodes: list(Tree.t(location, Oni_Components.LocationListItem.t)),
  locationsView:
    Component_VimTree.model(location, Oni_Components.LocationListItem.t),
};

let initial = {
  locationNodes: [],
  locationsView: Component_VimTree.create(~rowHeight=20),
};

[@deriving show]
type msg =
  | LocationsList(Component_VimTree.msg)
  | LocationFileLoaded({
      filePath: string,
      lines: array(string),
    })
  | KeyPress(string);

type outmsg =
  Component_VimTree.outmsg(location, Oni_Components.LocationListItem.t);

module Internal = {
  let locationsToReferences = (locations: list(Exthost.Location.t)) => {
    let map =
      List.fold_left(
        (acc, curr: Exthost.Location.t) => {
          let {range, uri}: Exthost.Location.t = curr;

          let range = range |> Exthost.OneBasedRange.toRange;

          let path = Oni_Core.Uri.toFileSystemPath(uri);

          acc
          |> StringMap.update(
               path,
               fun
               | None => Some([range])
               | Some(ranges) => Some([range, ...ranges]),
             );
        },
        StringMap.empty,
        locations,
      );

    map
    |> StringMap.bindings
    |> List.map(((path, ranges)) => {path, ranges});
  };

  let updateLocationTree = (nodes, model) => {
    let locationsView' =
      Component_VimTree.set(
        ~uniqueId=({path, _}) => path,
        ~searchText=
          Component_VimTree.(
            fun
            | Node({data, _}) => data.path
            | Leaf({data, _}) => Oni_Components.LocationListItem.(data.text)
          ),
        nodes,
        model.locationsView,
      );

    {locationNodes: nodes, locationsView: locationsView'};
  };
  let expandLocation = (~filePath, ~lines, model) => {
    let characterIndexToIndex = (idx: CharacterIndex.t) => {
      idx |> CharacterIndex.toInt |> Index.fromZeroBased;
    };

    let expandChildren = (location: location) => {
      let lineCount = Array.length(lines);
      location.ranges
      |> List.filter_map((range: CharacterRange.t) => {
           let line =
             range.start.line |> EditorCoreTypes.LineNumber.toZeroBased;
           if (line >= 0 && line < lineCount) {
             let highlight =
               if (range.start.line == range.stop.line) {
                 Some((
                   range.start.character |> characterIndexToIndex,
                   range.stop.character |> characterIndexToIndex,
                 ));
               } else {
                 None;
               };
             Some(
               Oni_Components.LocationListItem.{
                 file: filePath,
                 location: range.start,
                 text: lines[line],
                 highlight,
               },
             );
           } else {
             None;
           };
         })
      |> List.sort(
           (
             a: Oni_Components.LocationListItem.t,
             b: Oni_Components.LocationListItem.t,
           ) => {
           (a.location.line |> EditorCoreTypes.LineNumber.toZeroBased)
           - (b.location.line |> EditorCoreTypes.LineNumber.toZeroBased)
         });
    };

    let locationNodes' =
      model.locationNodes
      |> List.map(
           fun
           | Tree.Leaf(_) as leaf => leaf
           | Tree.Node({data, _} as prev) =>
             if (data.path == filePath) {
               let children = data |> expandChildren |> List.map(Tree.leaf);

               Node({expanded: true, data, children});
             } else {
               Node(prev);
             },
         );

    model |> updateLocationTree(locationNodes');
  };
};

module Effects = {
  let expandLocationPath = (~font, ~languageInfo, ~buffers, ~filePath) => {
    let toMsg = lines => LocationFileLoaded({filePath, lines});
    Feature_Buffers.Effects.loadFile(
      ~font,
      ~languageInfo,
      ~filePath,
      ~toMsg,
      buffers,
    );
  };
};

let collapseLocations = model => {
  ...model,
  locationsView: Component_VimTree.collapse(model.locationsView),
};

let setLocations = (~maybeActiveBuffer, ~locations, model) => {
  let references = Internal.locationsToReferences(locations);

  let nodes =
    references
    |> List.map(reference =>
         Tree.node(~children=[], ~expanded=false, reference)
       );

  let model' =
    model
    // Un-expand all the nodes
    |> collapseLocations
    |> Internal.updateLocationTree(nodes);

  // Try to expand the current buffer, if it's available
  maybeActiveBuffer
  |> Utility.OptionEx.flatMap(buffer => {
       switch (Buffer.getFilePath(buffer)) {
       | None => None
       | Some(filePath) =>
         let lines = Buffer.getLines(buffer);
         Some(model' |> Internal.expandLocation(~filePath, ~lines));
       }
     })
  |> Option.value(~default=model');
};

let update = (~previewEnabled, ~languageInfo, ~buffers, ~font, msg, model) => {
  switch (msg) {
  | LocationsList(listMsg) =>
    let (locationsView, outmsg) =
      Component_VimTree.update(listMsg, model.locationsView);
    let eff =
      switch (outmsg) {
      | Component_VimTree.Nothing => Outmsg.Nothing
      | Component_VimTree.Touched(item) =>
        previewEnabled
          ? Outmsg.PreviewFile({
              filePath: item.file,
              position: item.location,
            })
          : Outmsg.OpenFile({
              filePath: item.file,
              location: Some(item.location),
              direction: SplitDirection.Current,
            })
      | Component_VimTree.Selected(item) =>
        Outmsg.OpenFile({
          filePath: item.file,
          location: Some(item.location),
          direction: SplitDirection.Current,
        })
      | Component_VimTree.SelectedNode(_) => Nothing
      | Component_VimTree.Collapsed(_) => Nothing
      | Component_VimTree.Expanded({path, _}) =>
        Outmsg.Effect(
          Effects.expandLocationPath(
            ~font,
            ~languageInfo,
            ~buffers,
            ~filePath=path,
          ),
        )
      };
    ({...model, locationsView}, eff);
  | LocationFileLoaded({filePath, lines}) => (
      model |> Internal.expandLocation(~filePath, ~lines),
      Outmsg.Nothing,
    )
  | KeyPress(key) => (
      {
        ...model,
        locationsView: Component_VimTree.keyPress(key, model.locationsView),
      },
      Outmsg.Nothing,
    )
  };
};

module PaneView = {
  open Revery.UI;
  open Oni_Components;

  module Colors = Feature_Theme.Colors;

  module Styles = {
    open Style;

    let pane = [flexGrow(1), flexDirection(`Row)];

    let noResultsContainer = [
      flexGrow(1),
      alignItems(`Center),
      justifyContent(`Center),
    ];

    let title = (~theme) => [
      color(Colors.PanelTitle.activeForeground.from(theme)),
      margin(8),
    ];
  };

  let make =
      (
        ~config,
        ~isFocused: bool,
        ~locationsList:
           Component_VimTree.model(
             location,
             Oni_Components.LocationListItem.t,
           ),
        ~theme,
        ~iconTheme,
        ~languageInfo,
        ~uiFont: UiFont.t,
        ~workingDirectory,
        ~dispatch: Component_VimTree.msg => unit,
        (),
      ) => {
    let innerElement =
      if (Component_VimTree.count(locationsList) == 0) {
        <View style=Styles.noResultsContainer>
          <Text
            style={Styles.title(~theme)}
            fontFamily={uiFont.family}
            fontSize={uiFont.size}
            text="No locations set."
          />
        </View>;
      } else {
        <Component_VimTree.View
          config
          font=uiFont
          isActive=isFocused
          focusedIndex=None
          theme
          model=locationsList
          dispatch
          render={(
            ~availableWidth,
            ~index as _,
            ~hovered as _,
            ~selected as _,
            item,
          ) =>
            switch (item) {
            | Component_VimTree.Node({data, _}) =>
              <FileItemView.View
                theme
                uiFont
                iconTheme
                languageInfo
                item={data.path}
                workingDirectory
              />
            | Component_VimTree.Leaf({data, _}) =>
              <LocationListItem.View
                showPosition=true
                width=availableWidth
                theme
                uiFont
                item=data
              />
            }
          }
        />;
      };

    <View style=Styles.pane> innerElement </View>;
  };
};

let contextKeys = (~isFocused, model) =>
  if (isFocused) {
    Component_VimTree.Contributions.contextKeys(model.locationsView);
  } else {
    WhenExpr.ContextKeys.empty;
  };

open Feature_Pane.Schema;
let pane: Feature_Pane.Schema.t(model, msg) =
  panel(
    ~title="Locations",
    ~id=Some("workbench.panel.locations"),
    ~buttons=
      (~font as _, ~theme as _, ~dispatch as _, ~model as _) =>
        Revery.UI.React.empty,
    ~commands=
      _pane => {
        Component_VimTree.Contributions.commands
        |> List.map(Oni_Core.Command.map(msg => LocationsList(msg)))
      },
    ~contextKeys,
    ~sub=(~isFocused as _, _model) => Isolinear.Sub.none,
    ~view=
      (
        ~config,
        ~editorFont as _,
        ~font,
        ~isFocused,
        ~iconTheme,
        ~languageInfo,
        ~workingDirectory,
        ~theme,
        ~dispatch,
        ~model,
      ) => {
        <PaneView
          uiFont=font
          config
          isFocused
          locationsList={model.locationsView}
          theme
          iconTheme
          languageInfo
          workingDirectory
          dispatch={msg => dispatch(LocationsList(msg))}
        />
      },
    ~keyPressed=key => KeyPress(key),
  );
