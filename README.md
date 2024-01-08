# ascii-snake

<img src=docs/preview.gif width=400px>

## Installation and usage

### Linux

Open a terminal and execute the following steps:

```bash
sudo apt-get install libncurses5-dev libncursesw5-dev # ncurses is the only required dependency
git clone https://github.com/pablozabo/ascii-snake && cd ascii-snake
make
cd build/debug/bin
./snake
```

### Windows

First download and install [mingw](https://www.mingw-w64.org/downloads/#w64devkit),
then open a terminal and execute the following steps:

```bash
git clone https://github.com/pablozabo/ascii-snake && cd ascii-snake
make
cd build\debug\bin
./snake.exe
```

### Controls:

- <kbd>ARROW keys:</kbd> snake movement
- <kbd>ESC or F1 :</kbd> exit
