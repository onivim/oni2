open Utility;

[@deriving show({with_path: false})]
type t = {
  anchor: int,
  focus: int,
};

let initial: t = {anchor: 0, focus: 0};

let create = (~text: string, ~anchor: int, ~focus: int): t => {
  let safeOffset = IntEx.clamp(~lo=0, ~hi=String.length(text));

  let safeAnchor = safeOffset(anchor);
  let safeFocus = safeOffset(focus);

  {anchor: safeAnchor, focus: safeFocus};
};

let length = (selection: t): int => {
  abs(selection.focus - selection.anchor);
};

let offsetLeft = (selection: t): int => {
  min(selection.focus, selection.anchor);
};

let offsetRight = (selection: t): int => {
  max(selection.focus, selection.anchor);
};

let isCollapsed = (selection: t): bool => {
  selection.anchor == selection.focus;
};

let collapsed = (~text: string, offset: int): t => {
  create(~text, ~anchor=offset, ~focus=offset);
};

let extend = (~text: string, ~selection: t, offset: int): t => {
  create(~text, ~anchor=selection.anchor, ~focus=offset);
};

let%test_module "Selection" =
  (module
   {
    let testString = "Some Strin";
    let testStringLength = String.length(testString);
    let create = create(~text=testString);

    let%test_module "initial" =
      (module
       {
         let%test "Returns valid selection" = {
          let {anchor, focus} = initial;
          anchor == 0 && focus == 0;
         };
     });
    let%test_module "create" =
      (module
       {
         let%test "Returns valid selection" = {
           let {anchor, focus} = create(~anchor=3, ~focus=3);
           anchor == 3 && focus == 3
         };
         let%test "Handle different values" = {
           let {anchor, focus} = create(~anchor=3, ~focus=6);
           anchor == 3 && focus == 6
         };

         let%test "Handle values below 0" = {
           let {anchor, focus} = create(~anchor=-1, ~focus=-3);
           anchor == 0 && focus == 0
         };

         let%test "Handle above length" = {
           let {anchor, focus} = create(~anchor=70, ~focus=55);
           anchor == testStringLength && focus == testStringLength
         };
       });
    let%test_module "length" =
      (module
       {
         let%test "Returns range when anchor comes first" = {
           (create(~anchor=3, ~focus=5)
           |> length) == 2;
         };

         let%test "Returns range when anchor comes last" = {
           (create(~anchor=5, ~focus=3)
           |> length) == 2;
         };

         let%test "Returns 0 for collapsed selection" = {
           (create(~anchor=3, ~focus=3)
           |> length) == 0;
         };
         
       });
    let%test_module "offsetLeft" =
      (module
       {
         let%test "Returns anchor" = {
           (create(~anchor=3, ~focus=5)
           |> offsetLeft) == 3;
         };

         let%test "Returns focus" = {
           (create(~anchor=5, ~focus=3)
           |> offsetLeft) == 3;
         };
         let%test "Returns any" = {
           (create(~anchor=3, ~focus=3)
           |> offsetLeft) == 3;
         };

       });
    let%test_module "offsetRight" =
      (module
       {
         let%test "Returns anchor" = {
           (create(~anchor=5, ~focus=3)
           |> offsetRight) == 5;
         };

         let%test "Returns focus" = {
           (create(~anchor=3, ~focus=5)
           |> offsetRight) == 5;
         };
         
         let%test "Returns any" = {
           (create(~anchor=3, ~focus=3)
           |> offsetRight) == 3;
         };
       });
    let%test_module "isCollapsed" =
      (module
       {
         let%test "Returns true" = {
           (create(~anchor=3, ~focus=3)
           |> isCollapsed) == true;
         };
         let%test "Returns false" = {
           (create(~anchor=3, ~focus=7)
           |> isCollapsed) == false;
         };
       });
    let%test_module "collapse" =
      (module
       {

         let collapse = collapsed(~text=testString);
         let%test "Collapse selection with offset" = {
           let {anchor, focus} = collapse(3);
           anchor == 3 && focus == 3
         };
         let%test "Collapse selection with offset less than 0" = {
           let {anchor, focus} = collapse(-20);
           anchor == 0 && focus == 0
         };

         let%test "Collapse selection with offset mor ethan length" = {
           let {anchor, focus} = collapse(testStringLength+70);
           anchor == testStringLength && focus == testStringLength;
         };
       });
    let%test_module "extend" =
      (module
       {

         let extend = extend(~text=testString);
         let%test "Extend when selection is collapsed" = {
           let selection = create(~anchor=3, ~focus=3);
           let { anchor, focus } = extend(~selection, 5);
           anchor  == 3 && focus == 5
         };
         let%test "Extend when selection is not collapsed" = {
           let selection = create(~anchor=3, ~focus=8);
           let { anchor, focus } = extend(~selection, 5);
           anchor  == 3 && focus == 5
         };
         let%test "Doesn't extend when selection is not collapsed in offset" = {
           let selection = create(~anchor=3, ~focus=3);
           let { anchor, focus } = extend(~selection, 3);
           anchor  == 3 && focus == 3
         };
         let%test "Extends when offset is less than 0" = {
           let selection = create(~anchor=3, ~focus=3);
           let { anchor, focus } = extend(~selection, -3);
           anchor  == 3 && focus == 0
         };
         let%test "Extends when offset is more than length" = {
           let selection = create(~anchor=3, ~focus=3);
           let { anchor, focus } = extend(~selection, testStringLength+70);
           anchor  == 3 && focus == testStringLength;
         };
       });
 });
