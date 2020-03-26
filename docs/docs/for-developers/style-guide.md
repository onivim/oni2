---
id: style-guide
title: Style Guide
sidebar_label: Style Guide
---

## Style Guide

This style guide describes our code standards. It's a living document based on best practices we learn as we develop Onivim 2.

### Function Naming

- Use `toString` for a function that converts a type to a string.

> Note: This goes against the OCaml convention of using a `show` method for `t => string`. The reason for this is that we've found the name `show` to be ambiguous - for example, for `Window.show` - what would the expected behavior be? `Window.toString` is much clearer.

### Variable Naming

- For `option` types, use a variable name prefixed with `maybe`.

> Example: `let maybeInt = Some(1)`

### Error Handling

#### Exceptions vs Result

In the OCaml standard library, there are often two 'versions' of a function - one that returns a result, and one that may throw an exception, ie:

- `Sys.getenv: t => string` - may raise an exception if the environment variable 'var' is not present
- `Sys.getenv_opt: t => option(string)` - returns an `option(string)`

However, for our code, we've flipped this - the 'default' version should be the one that returns an `option` or a `result`. The reason for this is that this documents the error in the type system - ensuring that the consumer handles it, and avoiding crashes!

Therefore, for function names, we'd recommend:

- `validate: string => result(t, string)`
- `validateExn: string => t`

Where the `Exn` postfix shows that the function may throw an exception.
