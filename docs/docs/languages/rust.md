---
id: rust
title: Rust
sidebar_label: Rust
---
Onivim supports rust via the [`matklad.rust-analyzer`](https://open-vsx.org/extension/matklad/rust-analyzer) extension:

![rust](https://user-images.githubusercontent.com/13532591/89589799-443f8c80-d7fb-11ea-9d95-c2a89c7c1204.gif)

The following features are supported:

- Code completion
- Diagnostics
- Go-to definition
- Hover
- Document highlights

### Setup

1) Ensure you have `rust` and `cargo` installed: https://www.rust-lang.org/learn/get-started

2) Install the extension via the command-line:

- `oni2 --install-extension matklad.rust-analyzer`

### Usage

Once installed, run `oni2` in the your project's folder - for example: `oni2 /path/to/rust-project`, and then open a `.rs` file

The first time the extension activates, it may prompt you to download `rust-analyzer` binaries.

### More Info

- [Rust-Analyzer Documentation](https://rust-analyzer.github.io/manual.html#installation)
