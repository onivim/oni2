---
id: reason-ocaml
title: Reason & OCaml
sidebar_label: Reason & OCaml
---

Being authored in [Reason](https://reasonml.github.io/), Onivim supports [Reason](https://reasonml.github.io) as well as sister languages like 
[OCaml](https://ocaml.org). 

Onivim comes out-of-the-box with support for syntax highlighting and language detection for Reason & OCaml.

However, to get the most out of Onivim for these languages, you'll need a language server - we recommend [ocaml-lsp](https://github.com/ocaml/ocaml-lsp).

## Setup for a native `esy` project

If you're building a Reason or OCaml project with `esy`, simply add the following to the `devDependencies` of your `package.json` or `esy.json`:

```
  "devDependencies": {
    "@opam/ocaml-lsp-server": "ocaml/ocaml-lsp:ocaml-lsp-server.opam"
  }
```

Run `esy` to install and build, and open Onivim in the directory of your project.

## Setup for a native `opam` project

You can install `ocaml-lsp-server` via these OPAM commands:

```
$ opam pin add ocaml-lsp-server https://github.com/ocaml/ocaml-lsp.git
$ opam install ocaml-lsp-server
```

Build your project, and then open Onivim in the directory of your project.

## FAQ

### I'm not getting completions or diagnostics - what can I try?

The tooling depends on the project being built - so make sure your project is built before you open Onivim.

### Why isn't `ocaml-lsp` bundled with Onivim?

The ocamllsp binary depends on Merlin, which is tightly coupled to the OCaml compiler version being used - therefore, we rely on the environment to supply the correct `ocamllsp` binary for the current compiler version.
