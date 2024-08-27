# Demonstrate usage of plutosvg / plutovg with ImGui

### Intro

By default ImGui + LunaSvg will not be able to render emojis from NotoColorEmoji-Regular.ttf.

This repository will build a simple ImGui application that will display emojis from NotoColorEmoji-Regular.ttf, using plutosvg + plutovg,
as an alternative to lunasvg.

<SVG Fonts include a set of SVG documents. As per the [OpenType specification](https://learn.microsoft.com/en-us/typography/opentype/spec/svg#glyph-identifiers),
some SVG fonts (such as NotoColorEmoji) may group several glyphs in a common svg document (by selecting a subset of the elements in this document).

LunaSvg does support fonts where each glyph is associated to a distinct document. Unfortunately, it is not able to render
a subset of a svg document, and will likely not be able to do so in the future.

Its cousin project plutosvg (by the same author), is able to do it, and provides ready to use freetype hooks.

Example: sammycage/lunasvg#150 shows an example where a single svg document
included in the font may contains thousands of glyphs (each glyph is a subset of the svg document).


### Usage with plutovg / plutosvg

```
git submodule update --init
mkdir build && cd build 
cmake .. 
cmake --build .
./demo
```

### Usage with LunaSvg

```
cmake .. -DUSE_PLUTO_INSTEAD_OF_LUNA_SVG=OFF
cmake --build .
```

If building with LunaSvg, two issues will be visible:

- The initial loading of the font will be extremely slow (up to 30 minutes), since for each glyph 
a huge SVG document that actually include thousands of glyphs will be rendered, instead of a subset of the document.
- The rendering will be incorrect (where many glyphs will show a superposition of several emojis)
