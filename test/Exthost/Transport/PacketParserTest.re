open TestFramework;
open Exthost;

module Packet = Transport.Packet;
module Parser = Transport.Packet.Parser;

open Packet;

let simpleMessage1 = "Hello World!" |> Bytes.of_string;
let simpleMessage2 = "Hello World2!" |> Bytes.of_string;

let packet1 =
  Packet.create(~bytes=simpleMessage1, ~packetType=Regular, ~id=1);
let packet2 =
  Packet.create(~bytes=simpleMessage2, ~packetType=Regular, ~id=2);

let packet1Bytes = Packet.toBytes(packet1);
let packet1Length = packet1Bytes |> Bytes.length;
let packet1Buffer = packet1Bytes |> Luv.Buffer.from_bytes;

let packet1Split1 = Bytes.sub(packet1Bytes, 0, 4) |> Luv.Buffer.from_bytes;
let packet1Split2 =
  Bytes.sub(packet1Bytes, 4, Bytes.length(packet1Bytes) - 4)
  |> Luv.Buffer.from_bytes;

let doublePackets = Bytes.create(packet1Length * 2);
Bytes.blit(packet1Bytes, 0, doublePackets, 0, packet1Length);
Bytes.blit(packet1Bytes, 0, doublePackets, packet1Length, packet1Length);
let doublePacketBuffer = doublePackets |> Luv.Buffer.from_bytes;

describe("Transport.Packet.parser", ({test, _}) => {
  test("simple, full message packet", ({expect, _}) => {
    let (_parser, messages) = Parser.initial |> Parser.parse(packet1Buffer);

    expect.bool(Packet.equal(messages |> List.hd, packet1)).toBe(true);
  });
  test("split packet", ({expect, _}) => {
    let (parser, messages) = Parser.initial |> Parser.parse(packet1Split1);

    expect.equal(messages, []);

    let (_parser, messages) = parser |> Parser.parse(packet1Split2);

    expect.bool(Packet.equal(messages |> List.hd, packet1)).toBe(true);
  });
  test("double packets", ({expect, _}) => {
    let (_parser, messages) =
      Parser.initial |> Parser.parse(doublePacketBuffer);

    expect.int(List.length(messages)).toBe(2);

    let p1 = List.nth(messages, 0);
    let p2 = List.nth(messages, 1);
    expect.bool(Packet.equal(p1, packet1)).toBe(true);
    expect.bool(Packet.equal(p2, packet1)).toBe(true);
  });
});
