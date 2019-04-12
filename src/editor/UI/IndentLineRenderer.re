/*
 * IndentLineRenderer.re
 *
 * Logic for rendering indent lines in the buffer view
 */

open Oni_Core;
open Oni_Model;


type bufferPositionToPixel = ((int, int)) => ((float, float));

let render = (
    ~buffer: Buffer.t,
    ~startLine: int,
    ~endLine: int,
    ~lineHeight: float,
    ~fontWidth: float,
    ~bufferPositionToPixel,
    ~cursorLine: int,
    ~theme: Theme.t,
    ~indentationSettings: IndentationSettings.t,
    ()
) => {

    /* First, render *all* indent guides */

    let l = ref(startLine);

    while (l^ < endLine) {

        let line = l^;
        let lineText = Buffer.getLine(line);

        let level = Indentation.getLevel(indentationSettings, lineText);

        let (x, y) = bufferPositionToPixel(line, 0);
        let indentationWidthInPixels = float_of_int(indentationSettings.tabSize *. fontWidth);

        let i = ref(0);
        while (i^ < level) {
              Shapes.drawRect(
                ~transform,
                ~x=x +. indentationWidthInPixels *. float_of_int(i^),
                ~y=y,
                ~width=1,
                ~height=lineHeight,
                ~color=Colors.white,
                (),
              );

            incr(i);
        }

        incr(l);
    };

    /* Next, render _active_ indent guide */

};
