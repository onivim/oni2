---
id: faq
title: Frequently Asked Questions
sidebar_label: FAQ
---

## Downloads & Updates

### Where can I download a build?

Pre-alpha builds are available for users who have pre-ordered via our [Early Access Portal](https://v2.onivim.io/early-access-portal).

If you haven't pre-ordered, you can still try it out by following our [build instructions](../for-developers/building).

### How do I update my build?

Nightly builds are published to the [Early Access Portal](https://v2.onivim.io/early-access-portal).

Auto-update is a [work-in-progress](https://github.com/onivim/oni2/issues/559), until it is implemented, you'll need to download the latest build manually from the [Early Access Portal](https://v2.onvim.io/early-access-portal).

## Licensing

### Where is my license key?

After purchase, you'll receive an invoice from `paypal@outrunlabs.com` with your key attached.

If you don't receive it - please reach out to [hello@outrunlabs.com](mailto:hello@outrunlabs.com) and we'll get it set for you.

### I already backed Onivim 1, do I need to pay again?

__Nope!__ If you supported the project via any of the crowd-sourcing platforms prior to July 31, 2019, you have a __lifetime commercial license__.

If you did not receive a license key - feel free to send me a mail at bryphe@outrunlabs.com and we'll get you set. THANK YOU for supporting the project!

### Is Onivim 2 open-source?

We're developing Onivim 2 [in the open](https://github.com/onivim/oni2), but it is licensed under a [commercial EULA](https://github.com/onivim/oni2/blob/master/Outrun-Labs-EULA-v1.1.md).

However, we value open source - and to that end, we've implemented a 'time-delay' open source license. __Each commit that makes it to `master` will be dual-licensed under the permissive MIT license after 18 months__.
We maintain a [separate repo](https://github.com/onivim/oni2-mit) containing the MIT licensed code, which is sync'd daily. 

We hope that this can help us strike a balance: __the sustainability of a commercial offering__ while __giving back to the open source community__.

When we talk about sustainability - we mean not only in the context of Onivim 2, but in the broader context of open-source. 

To that end, we dedicate a portion of the proceeds from sales of Onivim 2 to open-source projects we leverage, like:
- [Vim](https://www.vim.org/sponsor/hall_of_honour.php)
- [Neovim](https://salt.bountysource.com/teams/neovim/supporters)

### Do I need multiple licenses to use on multiple machines?

There is no limit to the number of machines you may use the license on, as long as _you_ are the user. The license keys are _per-user_, not _per-computer_.

## Technology

### Why are you using [Revery](https://outrunlabs.com/revery) & [ReasonML](https://reasonml.github.io) instead of <insert favorite tech stack>?

In my opinion, the model of UI as a _pure function_ of `state`, as popularized by React, Redux, and Elm, is the simplest way to build a UI-driven app.

ReasonML (which is really OCaml under the hood) is a perfect fit for that paradigm: a performant, functional-focused language.

## Functionality

### Can I use my `init.vim` or `.vimrc` with Onivim 2?

Eventually! This is the goal of our [Beta Build](https://v2.onivim.io/#timeline) milestone.






