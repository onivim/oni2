type t = {
  // Bytes
  bytes: Bytes.t,
  // Current idx we're writing
  idx: int,
  // total size of the buffer
  totalSize: int,
};

let create = size => {bytes: Bytes.create(size), idx: 0, totalSize: size};

let isFull = ({idx, totalSize, _}) => idx >= totalSize;

let empty = Bytes.create(0) |> Luv.Buffer.from_bytes;

let getBytes = ({bytes, _}) => bytes;

let write = (buffer: Luv.Buffer.t, {bytes, idx, totalSize}) => {
  let size = Luv.Buffer.size(buffer);

  let remainingBytes = totalSize - idx;

  let bytesToWrite = min(size, remainingBytes);

  if (bytesToWrite < size) {
    let bufferToWrite =
      Luv.Buffer.sub(~offset=0, ~length=bytesToWrite, buffer);
    let remainingBuffer =
      Luv.Buffer.sub(
        ~offset=bytesToWrite,
        ~length=size - bytesToWrite,
        buffer,
      );

    Luv.Buffer.blit_to_bytes(~destination_offset=idx, bufferToWrite, bytes);
    ({bytes, idx: idx + bytesToWrite, totalSize}, remainingBuffer);
  } else {
    // Writing whole buffer
    Luv.Buffer.blit_to_bytes(~destination_offset=idx, buffer, bytes);
    ({bytes, idx: idx + bytesToWrite, totalSize}, empty);
  };
};
