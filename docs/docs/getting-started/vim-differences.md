---
id: vim-differences
title: Vim Differences
sidebar_label: Vim Differences
---

Coming from Vim? Onivim is actually Vim at the core, built on top of a fork of Vim called [libvim](https://github.com/onivim/libvim).

However, there are several key aspects that are different or may be unexpected coming from Vim.

## Default settings

Onivim changes several default settings:

- `set nocompatible`
- `set autoread`
- `set autoindent`

## Tabs, Buffers, and Windows

An often-confusing aspect of Vim is the relationship of tabs, buffers, and windows. The confusion comes from the fact that the word 'tab' is pretty overloaded, especially in terms of user's mental models coming from other applications.

In Vim, a 'tab' is a collection of windows. This is different from most 'modern' code editors - where a tab is a buffer in a split.

We made the trade-off of moving to the 'modern' definition: each window split in Onivim contains a list of buffers (editors), and thus, each UI 'tab' is really just a buffer.

However, we see value in the concept of the 'Vim-tab' - a grouping of window splits - and we want to keep that moving forward. We're working on adding a concept of [workspaces](https://github.com/onivim/oni2/issues/440), which encompass a grouping of window splits, as well some other properties (like the current working directory). Let us know if you have feedback on that concept by posting on that issue!




