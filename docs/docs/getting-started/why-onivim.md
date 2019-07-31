---
id: why-onivim
title: Why Onivim?
sidebar_label: Why Onivim?
---

A fair question to ask is - why yet _another_ text editor? Aren't Vim, VSCode, Atom, or Sublime good enough?

We appreciate aspects of these editors, as we've taken ideas and inspiration from each. But, in our opinion, none in isolation are perfect - and we think we can do better, by taking the aspects we appreciate of each and combining it into our vision of a perfect code editor.

## Principles

Onivim 2 is our ideal code editor - in our vision, the fastest way to go from thought to code. We're building it with the following principles in mind: 

### Performant

We built Onivim 2 from the ground up to be performant. Our ideal code editor starts up instantly, is extremely responsive, and never makes us wait.

The current trend of code editors is to build them on a web technology stack - which, in our mind, compromises the user experience. The fact is, hybrid applications built with tools like Electron will always be at a deficit compared to a native solution.

Our 'secret sauce' to a performant, native foundation, while enjoying the productivity of the web stack is [ReasonML](https://reasonml.github.io) (which is built on [OCaml](https://ocaml.org) and [Revery](https://outrunlabs.com/revery)). We believe a code editor is the perfect environment to put this tech to the test!

### Modal

Our ideal code editor is modal - allowing you to be __maximally productive__ using __just the keyboard__. There is a learning curve to modal editing, to be sure, but once you've learned it,
it's tough to go back!

Non-modal text editors, like Atom or VSCode, are optimized for _text insertion_. However, most of our work as developers is actually _navigating_ and _editing_ source code files. Modal editing provides a grammar for navigating and editing using just the keyboard.

As an example, the `dw` command in Vim is not just about deleting a word - it's actually a combination of a _verb_ (`d` -> `delete`) and a _motion_ (`w` ->`word`). There are many such operators and motions in Vim's modal editing language, and they can be combined in interesting ways - once you learn this, and build that muscle memory, you can navigate and edit code as quickly as your fingers can type!

### Modern

Onivim 2 should be aesthetically pleasing, look and feel modern, and be beautiful to use. Editors like VSCode, Atom, and Sublime embody this - but editors like Vim, Emacs, Kakoune are all terminal-based. 

Why, in 2019, do we still use user interfaces that emulate 1970's era hardware?

### Out-of-box productivity

Onivim 2 should be productive and useful out-of-the-box, without needing to spend lots of time configuring to get basics like code completion or snippets working.

We want to save Vim users from having to spend lots of time researching, experimenting, and configuring various language plugins just to get basic language support - with a solution that just works.

Editors like Atom and VSCode, with their focus on _text insertion_ have optimized and maximized that experience - with rich auto-completion and snippets out-of-the-box. Even with significant time investment, it can be hard to match the insert-mode experience that those editors have in Vim.

## Beautiful, Fast, Modern Modal Editing

Editors like VSCode and Atom are modern, extensible, and out-of-the-box productive, but are built on a technology stack that compromises the user experience. VSCode, because of its popularity, has an incredible ecosystem of plugins and extensions.

Vim is an amazing modal editor, but it lacks a modern UI, and requires significant configuration and investment to come even close to the language support of Atom or VSCode.

Sublime shows that you can have a modern, beautiful UI without sacrificing performance - but limited extensibility and modal editing capability.

Each of these editors embodies some of the core principles we value, but none embody them all - and that's why we're building Onivim: a beautiful, fast, and modern modal editor.
