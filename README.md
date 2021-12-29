# **Software II Week III 課題 - Paint**

# Introduction

- [課題１](#課題１) - 長方形を描くコマンド `rect` と 円を描くコマンド `circle` を塗りつぶさず線でかけるようにする。

- [課題２](#課題２) - ファイルに保存されたコマンド履歴を読み込み絵を再描画するコマンド `load`を追加

- [課題３](#課題３)　- 以降の描画の文字種を変更する `chpen` コマンドを追加

- [課題４](#課題４) - The following features have been added and are available in addition to the previous paint.c programs:

    - Color Palette Shown above canvas
    - Pen Color Changable
    - Eraser Mode
    - Highlighter Mode (Color Changable)
    - Bucket (Fill) mode (can fill with either pen or highlighter)
    - Redo after Undo
    - Change Pensize
    - Copying to Clipboard
    - Pasting from Clipboard
    - Resizing Canvas dimensions
    - Elongating Canvas along x 
    - Adding Layers
    - Duplicating (copying) Layers
    - Showing/Hiding Layers
    - Inserting/Moving Layers
    - Merging layers
    - Mirror (reversing) layer/canvas along x/y

    [![image.png](https://i.postimg.cc/FKhbB8gZ/image.png)](https://postimg.cc/FYnJfP6d)
    An example with a doraemon.txt file will be shown below to demonstrate the working of this code visually.


- [参考資料](#参考資料) - List of references I used to put this code together

--------
# 課題１

Compile:

```
gcc -o paint1 paint1.c -lm
```

Execute:

```
./paint1 <width> <height>
```

Example:
```
./paint1 80 40
```
A canvas with a height of 40 and a width of 80 will appear. Despite the width being double the height, due to horizontal length being shorter than the vertical length, this particular canvas will look more like a square than a rectangle.

**Rectangle:** `rect <x> <y> <width> <height> `

ex) 
```
rect 5 10 5 5
```

**Circle:** `circle <x> <y> <radius>`

ex)
```
circle 20 20 10
```

# 課題２

Compile:

```
gcc -o paint2 paint2.c -lm
```

Execute:

```
./paint2 <width> <height>
```

Example:
```
./paint2 80 40
```

**Loading a file:** `load <filename>`

ex)

```
load paint2.txt
```

paint2.txt looks like the following:
```
circle 50 20 18
circle 50 23 15 
circle 45 13 6 
circle 55 13 6 
rect 5 5 10 6
circle 47 13 2 
circle 53 13 2 
circle 50 20 3 
line 1 1 5 5 

```

A simple no-color outline of doraemon and a hammer will show up on the screen:

[![image.png](https://i.postimg.cc/bNtJ4j8j/image.png)](https://postimg.cc/WhsTJCh9)

**!!!注意!!!**

The last line of the .txt file to be loaded must have a spacing at the end (either with a spacebar or an enter). Or else, fgets will not retrieve the last line.




# 課題３

Compile:

```
gcc -o paint3 paint3.c -lm
```

Execute:

```
./paint3 <width> <height>
```

Example:
```
./paint3 80 40
```

**Change Pen:** `chpen <pen you want>`

ex) 
```
chpen o
```

changes the pen from the default '*' to '-'



**command:** `rect 5 5 10 10`

|Without Changepen (default) | After chpen (input: chpen o) | 
|----------------------------|----------------------------------|
|[![image.png](https://i.postimg.cc/PrfR31DG/image.png)](https://postimg.cc/yW5jWgnn) | [![image.png](https://i.postimg.cc/kMRQnGdV/image.png)](https://postimg.cc/bd8Dgzr8)|



# 課題４


As mentioned above, this program has the following features in addition to paint3.c:

    - Color Palette Shown above canvas
    - Pen Color Changable
    - Eraser Mode
    - Highlighter Mode (Color Changable)
    - Bucket (Fill) mode (can fill with either pen or highlighter)
    - Redo after Undo
    - Change Pensize
    - Copying to Clipboard
    - Pasting from Clipboard
    - Resizing Canvas dimensions
    - Elongating Canvas along x 
    - Adding Layers
    - Duplicating (copying) Layers
    - Showing/Hiding Layers
    - Inserting/Moving Layers
    - Merging layers
    - Mirror (reversing) layer/canvas along x/y


[![image.png](https://i.postimg.cc/FKhbB8gZ/image.png)](https://postimg.cc/FYnJfP6d)


Compile:

```
gcc -o paint4 paint4.c -lm
```

Execute:

```
./paint4 <width> <height>
```

Example:
```
./paint4 80 40
```

# コマンド一覧

### Standard

These are the commands you need to draw on the board freely


|Command|Description |  Sample command | Output of Sample Command |
|------|--|--------|---|
|`line <x1> <y1> <x2> <y2>` | Draws a line that connects <p> `(x1,y1)` and `(x2,y2)`|`line 1 1 10 5`|[![image.png](https://i.postimg.cc/c45YPkTN/image.png)](https://postimg.cc/tYWYx5Lr)|
| `rect <x> <y> <width> <height> <fill>`| Draw rectangle (fill = 0 --> outline only) | `rect 5 10 10 5 0`| [![image.png](https://i.postimg.cc/Vk0Tcmkm/image.png)](https://postimg.cc/Xr3xfM5z) |
| | (fill = 1 --> filled rectangle) |`rect 5 10 10 5 1`|[![image.png](https://i.postimg.cc/zfr1tVS9/image.png)](https://postimg.cc/qNjYgBFL) |
|`circle <x> <y> <radius> <fill>` | Draw circle (fill = 0 --> outline only) | `circle 10 10 6 0` |[![image.png](https://i.postimg.cc/m20QnFpZ/image.png)](https://postimg.cc/ykyggWz2) |
| | | `circle 10 10 6 1`| [![image.png](https://i.postimg.cc/mrrCzNtw/image.png)](https://postimg.cc/jDGLBf6w) |
|`fill <x> <y>` |If `<x>, <y>` empty, fills it with current pen <p> [![image.png](https://i.postimg.cc/25mGyZDj/image.png)](https://postimg.cc/v4qfSDTC) |`fill 10 12` |[![image.png](https://i.postimg.cc/D0s5md8Z/image.png)](https://postimg.cc/y310bRh4) |
|`chpen <pen>` | changes the pen (back) to `<pen>` <p>[![image.png](https://i.postimg.cc/c45YPkTN/image.png)](https://postimg.cc/tYWYx5Lr) | `chpen '-'` <p> `line 1 1 10 5` | [![image.png](https://i.postimg.cc/mrhFjY7g/image.png)](https://postimg.cc/rdXmFthB)|
|`pensize <size>` | changes the pen size to `<size>` <p>[![image.png](https://i.postimg.cc/c45YPkTN/image.png)](https://postimg.cc/tYWYx5Lr)| <p>`pensize 5` <p> `line 1 1 10 5` |[![image.png](https://i.postimg.cc/1zp4JvfP/image.png)](https://postimg.cc/8Fz1CBRn) |
|`color <color>`| changes pen color to `<color>` <p> look at pen reference color/marker reference color <p> [![image.png](https://i.postimg.cc/1zp4JvfP/image.png)](https://postimg.cc/8Fz1CBRn)|<p>`color 226` <p> `pensize 5` <p> `line 1 1 10 5`|[![image.png](https://i.postimg.cc/SsZhQ9JH/image.png)](https://postimg.cc/zbRM0Vwk)|
|`eraser` | Changes to eraser <p> [![image.png](https://i.postimg.cc/1zp4JvfP/image.png)](https://postimg.cc/8Fz1CBRn)|<p>`eraser` <p>`pensize 1`<p>`rect 3 3 3 5 1` | [![image.png](https://i.postimg.cc/Yqs3HC80/image.png)](https://postimg.cc/dLGdRvbc) |
| `marker`| changes to marker (highlighter) <p> [![image.png](https://i.postimg.cc/SsZhQ9JH/image.png)](https://postimg.cc/zbRM0Vwk)| <p>`marker` <p> `pensize 5` <p> `color 226 `<p> `line 1 1 10 5`|[![image.png](https://i.postimg.cc/cCHXZgn9/image.png)](https://postimg.cc/vgJWtmXf) |
|<p> `copy <x1> <y1> <x2> <y2>`  <p> `paste <x> <y>` |<p>copies a rectangular area and <p> pastes it onto another area <p>[![image.png](https://i.postimg.cc/cCHXZgn9/image.png)](https://postimg.cc/vgJWtmXf) |<p> `copy 0 0 20 20` <p> `paste 20 0` | [![image.png](https://i.postimg.cc/6QCHh4Rg/image.png)](https://postimg.cc/qtMXJR1x)|
|`load <txtfile>`|<p>loads what is in `<txtfile>` <p> if no `<txtfile>` given, <p>`history.txt` is read|`load good_doraemon.txt`|[![image.png](https://i.postimg.cc/L6t41N0M/image.png)](https://postimg.cc/YhSwcfHX)|
|`save <txtfile>`|<p>saves current canvas (and layers) <p>into `<txtfile>`.<p> if no `<txtfile>` given, <p>saved in `history.txt` by default | `save doraemon_copy.txt`|[![image.png](https://i.postimg.cc/YjK9wPnd/image.png)](https://postimg.cc/BPpsTmGF)|



-----------


## Layer commands


Since what we can do is pretty limited on one canvas, I have added the features of stacking layers and being able to show/hide layers, insert or move layers, etc. It is just like a real canvas, where an artist is able to draw on top of their drawings, except that here the artist is freely able to hide or show certain layers.

To explain how these layers work, I will be using a `good_doraemon.txt` file to sample my explanations. Assume that `good_doraemon.txt` has been loaded.


|Command|Description |  Sample command | Output of Sample Command |
|-----|------|-----|-----|
| `layer add`|<p> Adding a layer <p> (currently only 1 layer) <p>[![image.png](https://i.postimg.cc/L8rMQPWV/image.png)](https://postimg.cc/QFkPHFhB)|`layer add` |<p> now there are two layers <p> [![image.png](https://i.postimg.cc/9fjnqYSs/image.png)](https://postimg.cc/87tH2MTb) |
| | <p>Drawing on layer 2 will not influence what is drawn <p> on layer 1. Let's draw a simple crying face on layer 2 <p> on top of our doraemon <p>[![image.png](https://i.postimg.cc/sDMjn3Xs/image.png)](https://postimg.cc/rKL2sXxP)| <p> `marker` <p> `color 233` <p> `circle 40 20 18 0` <p> `rect 30 12 7 3 1` <p> `rect 44 12 7 3 1` <p> `rect 40 25 2 2 1` <p>  `color 11`<p> `fill 40 20`<p> `color 39 `<p> `rect 30 15 7 15 1`<p> `rect 44 15 7 15 1` |[![image.png](https://i.postimg.cc/Bnfrx3Ph/image.png)](https://postimg.cc/WhSYvQnr)  |
| | <p> Notice how the `fill` that is conducted on layer 2 is not influenced  <p> at all by the outlines of doraemon in layer 1. <p> Moreover, a higher layer will overlap the <p> lower layer, as seen here (Layer 2 is on top of layer 1||<p> - Higher Layer = above <p> - You work on the layer you are currently on |
| | <p> Using the concept of layers, we can do many fun things. <p> For instance: <p> - show/hide certain layers. <p> - merge layers that are complete and go together <p> - move/insert layers around | | |
|`layer hide <layer_number>`|<p> hide the layer number designated by `<layer_number>` <p> We will try to hide the crying face (layer 2) <p> [![image.png](https://i.postimg.cc/Bnfrx3Ph/image.png)](https://postimg.cc/WhSYvQnr)|`layer hide 2`|[![image.png](https://i.postimg.cc/FsbYb9nz/image.png)](https://postimg.cc/BX6Z4G3f)|
||<p> The second layer is now hidden. However, it is not gone.|||
|`layer show <layer_number>`|<p> show the layer number designated by `<layer_number>` <p> [![image.png](https://i.postimg.cc/FsbYb9nz/image.png)](https://postimg.cc/BX6Z4G3f)|`layer show 2`|<p> layer 2 appears back again! <p>[![image.png](https://i.postimg.cc/hGT6v5tk/image.png)](https://postimg.cc/Y42by8gR)|
|`layer change <layer_number>`|<p>change to designated `<layer_number>` layer <p>[![image.png](https://i.postimg.cc/YSMfNn3K/image.png)](https://postimg.cc/xXpznPht)| `layer change 1`| <p> Now, we are on layer 1 and we work on the <p>canvas for layer 1 unless changed otherwise <p> <p>[![image.png](https://i.postimg.cc/zDV667mY/image.png)](https://postimg.cc/kDPTtxXT)|
|`layer remove <layer number>`|<p>removes the layer number `<layer number>` <p> [![image.png](https://i.postimg.cc/kMH3gZN5/image.png)](https://postimg.cc/K4ts90HX) | `layer remove 2`| <p> layer 2 removed! <p> [![image.png](https://i.postimg.cc/mD6qhzHn/image.png)](https://postimg.cc/Fd3GBKGb)|
|`mirror <x/y>`| <p> Mirrors the current layer along the x/y axis <p>[![image.png](https://i.postimg.cc/wB52St2N/image.png)](https://postimg.cc/t1TF64LR)|<p>`layer mirror x`  |<p> mirrors the doraemon along the x axis <p>[![image.png](https://i.postimg.cc/3x1VvcMV/image.png)](https://postimg.cc/75CKvXZn)|

Basically, using these, we can change add, modify, mirror, change between layers, and delete unncessary layers. There are additional layer functions as well:

- `layer copy` - copies the current layer and duplicates it onto the top-layer (basically like add layer with a copy of the layer you are on)
- `layer move <layer1> <layer2>` - changes the location of two layers. Useful if you want to merge
- `layer insert <layer> <location>` - inserts the designated layer into your wanted location
- `layer merge` - merges the current layer with the previous one - use `layer move` and `layer insert` to adjust to merge layers that you want to combine

You may test them if you wish.

## Additional functions

|Command|Description |  Sample command | Output of Sample Command |
|-----|------|-----|-----|
|`ratio <ratio>`| <p> changes the ratio along x direction <p> [![image.png](https://i.postimg.cc/wBNCRDNZ/image.png)](https://postimg.cc/jWsgGwZ4)| `ratio 2`| [![image.png](https://i.postimg.cc/xj5Wmfkf/image.png)](https://postimg.cc/23qcRfFM) |
|`resize <x> <y> `| <p> resizes the canvas to `<x>,<y>` <p>[![image.png](https://i.postimg.cc/xj5Wmfkf/image.png)](https://postimg.cc/23qcRfFM)| `resize 80 30`| <p> [![image.png](https://i.postimg.cc/tTCzGj9k/image.png)](https://postimg.cc/p94jK3h5)|


# 参考資料

- [Haoyi's Programming Blog](https://www.lihaoyi.com/post/BuildyourownCommandLinewithANSIescapecodes.html) - Reference for determining 256 colors in C. While the code itself was in Python, helped me get an understanding of utilizing color with the ANSI escape sequence
- [ANSI Color Escape Sequences](https://stackoverflow.com/questions/4842424/list-of-ansi-color-escape-sequences) - really helped me understand how ANSI escape sequences are used