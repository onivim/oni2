---
id: architecture
title: Architecture Overview
sidebar_label: Architecture
---

## Architecture

Onivim 2 is a [Revery](https://www.github.com/revery-ui/revery) application built with [ReasonML](https://reasonml.github.io).

Even though it is new technology, if you're coming from a web background - it should be easy to get started! The syntax of ReasonML is similar to JavaScript,
and the core ideas are very similar to [React](https://reactjs.org), [Redux](https://redux.js.org/), and the [Elm Architecture](https://guide.elm-lang.org/architecture/) - just used in a way that lets us compile to native code.

We want to have a functional lean to our codebase... and [ReasonML](https://reasonml.github.io) / [OCaml](https://ocaml.org) are the perfect fit for that.
[OCaml](https://ocaml.org) is highly optimized - even down to its garbage collector implementation - for building functional applications in a performant way.

### Key concepts

- __Action__: A payload of information to describe a state change. Just like an [action](https://redux.js.org/basics/actions) in redux.
- __State__ and __Model__: used interchangeably to describe the application state. 
- __Reducer__: A function of (`state`, `action`) that returns a new `state`.
- __Updater__: A function of (`state`, `action`) that returns a tuple of (`state`, `effects`). Similar to a reducer, but also handles side-effects. Inspired by the [Elm Architecture](https://guide.elm-lang.org/architecture/)
- __Store Connector__: Loosely akin to a middleware in React.
- __Store__: A __State__, __Updater__, and __Store Connectors__. Similar to a [store](https://redux.js.org/basics/store) in Redux.

One additional concept is the idea of our UI as a _pure function_ of `state` - this will be familiar for anyone coming from [React](https://reactjs.org).

The state management (actions, state, reducer, updaters, store) are facilitated by a reason-native library called [`isolinear](https://github.com/bryphe/isolinear).

### Source Code Overview

Overview:
 - `src` - editor source code
    - `src/editor` - the source code for Onivim 2 (primarily Reason)
        - `src/editor/Core` - Core types used across the application
        - `src/editor/Extensions` - Modules relating to the extension host or syntax highlighting
        - `src/editor/Model` - description of the _state_ of the application. The most important modules here are:
            - [`State.re`](https://github.com/onivim/oni2/blob/master/src/editor/Model/State.re) - type definition for the application state.
            - [`Reducer.re`](https://github.com/onivim/oni2/blob/master/src/editor/Model/Reducer.re) - top-level _reducer_ function.
            - [`Actions.re`](https://github.com/onivim/oni2/blob/master/src/editor/Model/Actions.re) - type definition for all _actions_.
        - `src/editor/Store` - connects the _state_ of the application with external effects. 
            - [StoreThread.re](https://github.com/onivim/oni2/blob/master/src/editor/Store/StoreThread.re) is the entry point - creating a _store_ and initializing all the _store connectors_.
        - `src/editor/UI` - the user interface for the application.
            - [Root.re](https://github.com/onivim/oni2/blob/master/src/editor/UI/Root.re) is the top-level UI
            - [EditorSurface.re](https://github.com/onivim/oni2/blob/master/src/editor/UI/EditorSurface.re) is where the buffer rendering happens.
        - `src/editor/bin_editor` - The 'main' entry point of `Oni2_editor.exe`.
    - `src/textmate_service` - the source for our textmate syntax highlighting (JavaScript)

### Vim

Vim is at the heart of Onivim 2. There are a few moving parts:

- [libvim](https://github.com/onivim/libvim) - A fork of [Vim](https://www.vim.org) that is platform and terminal agnostic. It is the core buffer editing engine exposed via a simple [API](https://github.com/onivim/libvim/blob/master/src/libvim.h).
- [reason-libvim](https://github.com/onivim/reason-libvim) - Reason bindings for `libvim`. Glance at the [rei](https://github.com/onivim/reason-libvim/blob/master/src/Vim.rei) to get a feel for the API.

Onivim 2 integrates with `reason-libvim` via a _store connector_: [`VimStoreConnector.re`](https://github.com/onivim/oni2/blob/master/src/editor/Store/VimStoreConnector.re).

### Revery

[Revery](https://github.com/revery-ui/revery) was built to support Onivim 2. It is a new UI framework that is intended to build fast, native, cross-platform applications with the ergonomics of React & Redux.

It manages the lifecycle of the application - it provides the `App.start`, `App.createWindow`, and `UI.start` methods we use to kick-off the application: [`Oni2_editor.re`](https://github.com/onivim/oni2/blob/master/src/editor/bin_editor/Oni2_editor.re)

### Extension Host

Coming soon!

## Testing

For a cross-platform project built & maintained by a small team - test coverage is so important!

We have two classes of tests - _unit tests_ and _integration tests_.

### Unit Tests

Unit tests live in the `test` folder and be can be run via `esy '@test' run`. 

We organize our tests 1-to-1 with the `src` code - for example, test cases for `src/editor/Core/LineNumber.re` would live in `test/Core/LineNumberTests.re`.

Unit tests verify a particular piece of code in isolation, and should have minimal dependencies.

### Integration Tests

Unit tests help us verify that individual modules work as expected - it's also important for an application like Onivim 2 to verify that things work when all these pieces come together.

We use _integration tests_ to help us verify this - integration tests live in the `integration_test` folder.

_integration tests_ are each stand-alone executables that initialize an entire _Store_ - just like the Onivim 2 app does! The only difference is that _integration tests_ are headless (no UI).

_integration tests_ can be run in one of two ways:
- They all can be run via `esy '@integrationtest' run`
- Individual tests can be run via `esy '@integrationtest' x Test-Name.exe`

## Learning Resources

If you're interested in learning about ReasonML, we recommend the following:

- [reasonml.github.io](https://reasonml.github.io)
- [sketch.sh](https://sketch.sh)
- [Revery Playground](https://www.outrunlabs.com/revery/playground)

For learning more about the architectural ideas, we recommend:
- [Elm Architecture](https://guide.elm-lang.org/architecture/)
- [Redux](https://redux.js.org/)
- [React](https://reactjs.org/)
