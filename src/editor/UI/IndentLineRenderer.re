/*
 * IndentLineRenderer.re
 *
 * Logic for rendering indent lines in the buffer view
 */


type bufferPositionToPixel = ((int, int)) => ((float, float));

let render = (
    ~buffer: Buffer.t,
    ~startLine: int,
    ~endLine: int,
    ~lineHeight: float,
    ~indentWidth: int,
    ~bufferPositionToPixel,
    ~cursorLine: int,
    ~theme: Theme.t,
    ()
) => {

};
