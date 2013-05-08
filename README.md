Gravity Pictures
================

![Gravity Pictures](http://i.imgur.com/QkckHEm.png)


How to
======

Simply run the run script:

```
$ ./run Release
```


Notes
=====

I didn't want to use sdl/glfw/... because I don't need a window. I only want off-screen rendering of opengl, so I use the osx api to create a opengl context. Other than that, the code should be portable (assuming a new enough c++ compiler).
