# VidGenX

VidGenX is a simple tool to render videos based on a description in a custom language. It is intended as an educational project to learn how to use Flex/Bison in a toolchain to generate an internal representation, and also to improve software design skills.

## Features

|Feature|Support|Note|
|---|---|---|
|Language definition|✅||
|Interpreter|✅||
|Basic shapes|✅|rectangle, circle|
|Time-based delay|✅||
|Composites with parameters|✅||
|Constants|✅||
|Prototypes|✅||
|Basic animations|✅||
|Enhanced animations|❌||
|Image support|❌||
|Text support|❌||
|Animation-based delay|❌||
|Advanced shapes|❌|ellipse, rounded rectangle, ...|
|Different animation interpolators|❌||

Legend: ✅ - full support, ⏳ - partial support, ❌ - not yet implemented

## Example

To define a video using the language, you can create a sample scene using this code saved in e.g., `sample.vdef`:

```
Config(version = 1, width = 640, height = 480, fps = 30, defaultbackground = RGB('#336633'))

Proto {
    Smiley = Composite(smilefill) {
        Circle(x = 30, y = 30, r = 30, fill = RGB('#FFFF00'), stroke = RGB('#00FF00'), strokewidth = 3)
        Circle(x = 20, y = 20, r = 5, fill = RGB('#888888'), stroke = RGB('#000000'), strokewidth = 2)
        Circle(x = 40, y = 20, r = 5, fill = RGB('#888888'), stroke = RGB('#000000'), strokewidth = 2)
        Rectangle(x = 10, y = 40, width = 40, height = 5, fill = smilefill)
    }
}

Constants {
    angry = RGB('#FF0000')
}

Scene(duration = 4s) {
    // every object has x, y; all parameters (including the x, y) are passed in params block

    v = Rectangle(x = 10, y = 20, width = 100, height = 200, fill = RGB('#669966'))
    s = Smiley(x = 20, y = 30, smilefill = angry)
    
    Wait(duration = 2s)
    vv = Rectangle(x = 110, y = 120, width = 100, height = 200, fill = RGB('#996699'), rotate = 10)
    
    vv.Animate(rotate = 60, duration = 1s, width = 300)
}
```

Then to generate a video, run the compiled program with these arguments:

```
vidgenx.exe sample.vdef output_dir
```

This will generate a set of images and a resulting video into an output directory you specified.

## License

This software is distributed under the MIT license. Please, see attached LICENSE file for more information.