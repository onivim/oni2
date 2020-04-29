module Transport = Exthost_Transport;

module NamedPipe: {
  type t;

  let create: string => t;
  let toString: t => string;
};
