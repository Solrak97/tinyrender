# tinyrender

I've been working in graphics for a while using game engines, as I wanted a deeper understanding of what was going on, I decided to try to build something from scratch.

The tinyrender is a project  based on the material from https://github.com/ssloy/tinyrenderer/wiki, it's really helpful, just some basic notions of linear algebra and C++ are enough to start building something interesting.

---

### What I've learned so far

* Sometimes there is a +1 on the code, it seemed random at first but its quite easy to understand after reading the .obj vectors.
  
     ```
     v -0.201076 -0.720822 -0.633205 
     ```

    Vector coordinates are defined as a float in the range [-1, 1], those points must be scaled to fit on the screen or image, but the target only deals in positive num,bers, so before scaling the point it must be transformed to a positive scale, adding +1 to the original point will 'center' the point to the range [0, 2].
    
    ```c++
    model.vert(face[j]).x + 1) * width / 2;
    ```