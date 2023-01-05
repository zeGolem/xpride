# xpride

A pride/sexuality/gender flags viewer written in C/xcb

It supports scaling the flag to any resolution and window aspect ratio

## Dependencies

You'll need a working X11 server for this program to work, as well as the xcb
library

## Building and running

Use the provided build script to build the program:

```console
 $ ./build.sh
```

If you don't have clang on your system, specify another compiler with:

```console
 $ CC=gcc ./build.sh
```

You can then run it with a flag:

```console
 $ ./xpride res/pride.flag

```

To avoid typing the full path to the flag each time, you can also copy the
flags in res/ to /usr/share/flags/:

```console
 # mkdir -p /usr/share/flags/
 # cp res/* /usr/share/flags/
```

(make sure to run these commands as root)

Then, you can just specify the flag's name:

```console
 $ ./xpride pride
```

## Debugging, hacking and contributing

You can build a debug version of the program with the following command:

```console
 $ ./build.sh debug
```

This will add debugging symbols and address sanitization

If you want to submit a patch, make sure you run `clang-format` before every
commit!

```console
 $ clang-format -i src/*
```

### For adding new flags

The flag format is really simple for now, it's a newline-separated list of
hexadecimal colors. You can look in res/ for examples!

### Submitting patches

You can either create a pull request on 
[the GitHub page](https://github.com/zeGolem/xpride), or send it to me (zeGolem)
via email. My address is available in the git log.
