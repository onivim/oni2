---
id: faq
title: Frequently Asked Questions
sidebar_label: FAQ
---

## Licensing

### Is Onivim 2 open-source?

We're developing Onivim 2 [in the open](https://github.com/onivim/oni2), but it is licensed under a [commercial EULA](https://github.com/onivim/oni2/blob/master/Outrun-Labs-EULA-v1.1.md).

However, we value open source - and to that end, we've implemented a 'time-delay' open source license. __Each commit that makes it to `master` will be dual-licensed under the permissive MIT license after 18 months__.
We maintain a [separate repo](https://github.com/onivim/oni2-mit) containing the MIT licensed code, which is sync'd daily. 

We hope that this can help us strike a balance: __the sustainability of a commercial offering__ while __giving back to the open source community__.

### Where can I download a build?

Pre-alpha builds are available for users who have pre-ordered via our [Early Access Portal](https://v2.onivim.io/early-access-portal).

If you haven't pre-ordered, you can still try it out by following our [build instructions](../for-developers/building).

### I already backed Onivim 1, do I need to pay again?

__Nope!__ If you supported the project via any of the crowd-sourcing platforms prior to July 31, 2019, you have a __lifetime commercial license__.

If you did not receive a license key - feel free to send me a mail at bryphe@outrunlabs.com and we'll get you set. THANK YOU for supporting the project!

### Do I need multiple licenses to use on multiple machines?

There is no limit to the number of machines you may use the license on, as long as _you_ are the user. The license keys are _per-user_, not _per-computer_.

## Technology

### Why are you using [Revery](https://outrunlabs.com/revery) & [ReasonML](https://reasonml.github.io) instead of <insert favorite tech stack>?

In my opinion, the model of UI as a _pure function_ of `state`, as popularized by React, Redux, and Elm, is the simplest way to build a UI-driven app.

ReasonML (which is really OCaml under the hood) is a perfect fit for that paradigm: a performant, functional-focused language.




