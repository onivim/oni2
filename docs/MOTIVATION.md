# Oni v2

A next-generation modal editor - built on [Revery](https://github.com/bryphe/revery) ([Reason](https://reasonml.github.io/)/[Ocaml](https://ocaml.org/)) + [Neovim](https://neovim.io/), leveraging the VSCode Extension Host model.

## Motivation + Reflections

Maintaining and working with developers on `Oni` has been an incredible experience, and I'm amazed at how far it has come in the time we've worked on it.

Oni started out as a personal / hobby project for me - and opened the door to meeting and collaborating with some incredible people.

There are some things that went well, and seemed to resonate with developers:
- __Language support__ - we had a lot of love for out-of-box JavaScript/TypeScript functionality!
- __Aesthetics matter__ - having something that looked great out-of-the-box helped stifle Atom / VSCode envy.
- __"It just works"__ - having modal editing + language support work out of the box appealed to users.
- __Sneak mode__ - 'sneak mode' seemed to resonate pretty well with some users.

Some more mixed features:
- __Tutorials__ - Some users appreciated, some did not.
- __Integrated Browser__ - Never was too useful in user's flow.

And, there are also areas that Oni fell short, and didn't meet the goals we set out:
- __Performance__ - as can be attested to by viewing any hacker-news-Oni-thread, the choice of Electron as a technology stack certainly alienated some users. There are still lots of areas we can improve on perf - but we need to decide if its worth investing in our current technology, and fixing the issues, or exploring new technology with the possibility of performance improvements.
- __Language support__ - Although language support worked well in some cases, there was awkwardness with the way the completion model integrated with Neovim. For example, if there were both language server providers and a popupmenu, this could lead to some awkward and problematic UX. In addition, the completion model never worked perfectly with macros or the repeat key, which was a pain point.
- __Enabling power users__ - Power vim users had trouble integrating functionality from the Vim config to Oni's config, and vice versa. For example, calling Oni commands via Ex mode was challenging in some cases, or not possible in others.
- __Sustainability__ - Oni is not a financially sustainable project. This is partly a consequence of being so niche, but maybe there are improvements we can make.

## A New Architecture

![image](https://user-images.githubusercontent.com/13532591/47937221-4691eb80-de9d-11e8-8b7a-1d5cea400863.png)

### Current architectural limitations

There were several specific features that were difficult to implement in Oni v1:
- Smooth scrolling 
- 'Minimap'
- VimL / Ex integration
- Meeting performance goals
    - Startup performance
    - Keypress latency
- Keeping parity / pace with VSCode extensions
    - Code completion
    - Debug support

There were also additional pain points - long builds, cumbersome packaging, etc.

### A new, high-performance foundation

At the time Oni was started, [Electron](https://electronjs.org) was the best choice for a small group of developers to build a cross-platform app. This can be attested to by the prevalence of successful Electron apps, like [VSCode](https://github.com/microsoft/vscode), [Discord](https://discordapp.com), [Slack](https://slack.com), [Atom](https://atom.io)

Electron certainly has known downsides, but the upside is that it can enable a small group to quickly ship apps. JavaScript is one of the most popular languages - so an important benefit is that it always makes your project approachable to a wide audience. One particular upside is that, when working with Electron, you have a high confidence that the core app functionality is consistent across platforms (something you don't get from native widget frameworks).

However, times have changed - I believe there are alternatives now to Electron that can enable the same developer experience and productivity benefits, but without the performance tradeoffs that are necessitated with Electron. 

For our prototyping, we will be leveraging [Reason](https://reasonml.github.io/) - a javascript-like syntax around OCaml - and a framework I'm developing called [Revery](https://github.com/bryphe/revery) - a high-performance alternative to [Electron](https://electronjs.org). I'm building this framework from the ground-up, based on what I've learned from cross-platform development - to create a framework that offers __consistency across platforms__ along with __native performance__.

Why Reason / OCaml? For those unfamiliar, OCaml is a battle-tested, pragmatic, functional programming language. The OCaml compiler is [one of the fastest](https://twitter.com/garybernhardt/status/1007694141231378432?lang=en), and the performance of the native-compiled code approaches native C. It's a great choice for the next generation of Oni! 

### A new view layer

One aspect of Oni's 'view' that caused problems for us over the course of development was the fact that Neovim treats the visible screen as a grid of cells. This makes it tough to know what is beyond the extents of the screen (necessary for smooth scrolling). A naive approach to simply tell Neovim to treat the viewport as larger is busted, because it [breaks features like scrolloff](https://github.com/onivim/oni/issues/405)

For Oni v2, one avenue we will be prototyping is decoupling our 'view' from the character-grid provided by Neovim. This provides several challenges, but it means that, if we control the rendering, we can easily implement smooth scrolling and scrolling decoupled from cursor position. In addition, it's straightforward to implement a minimap renderer.\

### Insert-mode control

One particular pain point with Oni's model today is the integration with native Vim completion and macros, with its own language support. This came up in several awkward bugs (multiple pop-up windows, etc).

I believe the best way to solve this problem is to use the "Apple" approach - control the insert-mode experience from end-to-end. We'll process the insert-mode experience completely in our front-end - which means typing latency can be minimized - and we can engage language services immediately without needing a round-trip through Neovim.

The downside with that approach, though, is that it means some insert mode plugins may not work as expected (or at all) in Oni. I'd be interested to know if there are any insert-mode features or plugins that you'd miss so we can make sure we account for that functionality in some way.

### Reduced RPC calls

Oni v1 communicates with Neovim over `stdio` using the `msgpack-rpc` protocol. This is great, except this 'asynchronicity' was the source of many problems and additional (unnecessary) complexity - but it's complexity we had to deal with.

An example is the code we needed to build up the window metrics we needed to render our 'buffer layers':

Most Neovim UIs use this `stdio` protocol - but there are a few exceptions, like VimR, that use an embedded C library `libnvim`. This is the approach that we plan on prototyping for Oni v2, because it's the fastest way to communicate with nvim. It means that several of our problematic async APIs can now be synchronous - which simplifies the downstream logic.

Having our core editor experience work directly with `libnvim` also opens up other possibilities to improve the deficiences in our prior architecture - for example, we can directly integrate with the completions in order to enable better macro support.

### Embrace VSCode Extensions

A mistake I made early on was to build an independent extension model for Oni. At the time, the VSCode extension implementation was too limiting for some of the functionality I wanted to explore.

However, the VSCode extension ecosystem is burgeoning with high-quality language support, including debuggers, that I would like to leverage for Oni v2. It's hard to bill Oni as giving you the best of VSCode and Vim if you can't install plugins from both places. In addition, VSCode has invested significantly in both their extension host model and the ecosystem of plugins, and has generously licensed the editor under a permissive MIT License, which makes it easy for us to re-use pieces in Oni.

In Oni v1, we already were leveraging some VSCode pieces - in particular, the snippet support and textmate highlighting, but I want to double-down on that investment.

### A minimal version

Another barrier to entry for users migrating from Vim -> Oni is that they might find the aesthetics of a modern UI appealing, but they don't want all the cruft or they are concerned about conflicts with their Vim config. For Oni v2, we should consider an architecture where the node-host-extension-model is _optional_, such that there is a minimal version of the editor available that has a minimized set of conflicts. It's also useful to be able to put the 'full' version in this minimal environment, for debugging purposes.

## A New Model for Sustainability

### Funding / Business Model

Making OSS sustainable is notoriously hard. For Oni v1, unfortunately, the funding model is not sustainable - we've had some incredibly generous backers who have contributed, but it's not enough for us to even have a single full-time developer. Projects like Atom and VSCode have tremendous amount of funding from their parent companies, but unfortunately Oni is not in the same boat.

Because of this, I plan on pursuing a dual-licensing model:
- Oni v2 will be free for __non-commercial and educational__ use - we always want you to be able to use it
- For commercial use, we will follow the Sublime model of a 1-year or 3-year license. The price will be increased after each release.

As a __thank you to all of our existing backers and early adopters__ - _anyone_ who has contributed financially to the Oni project prior to the end of 2018 - they will receive a license for lifetime commercial usage. In addition, _active backers_ will receive early access to the binaries. 

I believe this is a fair model, and it allows us to contribute back to open source projects we depend on. I will donate 30% of the revenue to the open source projects we depend on, in particular Neovim. 

The market is so niche for a developer tool like this, so I don't anticipate it being a commercial success, but I would like to give back more to the contributors and to the open source projects we depend on. We had ~100k downloads in Oni's lifetime - if 1% of those contributed $25, that would be $25,000 over the course of Oni's lifetime, which is significantly more than we have currently. It's still not enough to be a full-time project, but there are potentially other avenus we could pursue as well.

One important core value to the project is to contribute back to open source. Therefore, we will push some of the funds to Neovim, Vim, and other open-source efforts that we depend on. I believe this is the _only sustainable model for open source_ - sell products that then go back to fund the OSS dependencies. 

### Focusing on the right priorities

Another challenge with open source is, as the project becomes popular, it can be difficult and time-consuming to address every issue. This is feasible when a project is in early stages, but especially without funding or full-time support, it is not possible to keep up with incoming issues. At that point, it is necessary to introduce a 'triage' process - where we prioritize issues / work and decide where best to invest time. For Oni, we have a very minimal process - we tag backer-initiated issues with the 'insider' tag, but other issues can still consume attention and time.

Focusing our resources on the right set of work is critical - _especially_ when resources are limited. To that end, I'm considering a more aggressive triage strategy - we will default to _closing issues immediately_ (and tag them with 'backlog'), __unless it was initiated by a user that has a license or an early adopter__. This helps us focus on the right set of priorities because we will only have issues that are relevant to users that have already themselves made an investment in Oni.

This is critical in a small open source project - it's easy to bleed a ton of time just _triaging_ issues, as opposed to doing work that could actually benefit users that have invested in the project!

## Risks

The above document is optimistic and paints a rosy picture for this new architecture, as it should - otherwise it wouldn't be worth pursuing! However, there are several potential risks to call out as well.

- __Unproven technology__ - To my knowledge, this would be the first cross-platform UI desktop app shipped with Reason. The good news is that OCaml is a solid foundation to build on, but there are still a lot of unknowns here. It's also a great opportunity to showcase this technology stack and the way it can improve performance.

- __`libnvim` is not a stable API__ - Most UI clients do not leverage `libnvim`, with the notable exception of VimR. This represents a potential risk. The mitigation here is to continue to use the msgpack API.

- __Developer Experience__ - The technology stack we're using does not have the rich set of developer tools that Electron has - an integrated debugger, a performance profiler, hot reloading. This could be a potential pain point, at least initially.

- __IME Support__ - The core editor is working with `GLFW`, which doesn't have built-in IME support. This would either need to be augmented or contributed back to GLFW. It's a non-trivial challenge to get this work cross-platform.

- __Incompatibilities with Vim/Neovim__ - With the introduction of features like smooth-scrolling, some Vim features like 'scrolloff' may not behave as predictably.

- __Lack of embedded browser__ - Having a whole browser embedded is a common complaint towards the bloat of Electron, but it's great to have for implementing features like markdown preview (and our integrated browser!) 

## Open Questions

- What syntax highlighting strategy would be best? Textmate Highlighting, Treesitter? Could we contribute one of these back to core Neovim, or would it be better for performance to run out-of-band? (tree-sitter integration is ongoing in neovim: https://github.com/neovim/neovim/pull/9219)
- Could there be a story for native-code plugins? This would be great for things like buffer layers. OCaml has a feature called `dynlink` which is for dynamically loading libraries - this could be leveraged for a native-code plugin story (would be ideal for performance-critical extensions like buffer layers)
- How will features like Markdown Preview be implemented in this new architecture?
