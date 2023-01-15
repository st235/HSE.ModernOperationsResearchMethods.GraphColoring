# Graph Coloring

## Build

```bash
g++ -std=c++17 graph_colorization.cpp
```

## Algorithm

I am using DSatur algorithm to colorize the graph.

Algorithm uses greedy approach by considering the verticies
in a specific order and assining to them the smallest available color
not used in the local neighbourhood.

The ordering of the consideration relies on an heuristic:
the ordering is built dynamically choosing next the vertex adjacent to
the largest number of different colors.

## Misc

I modififed `Check` method. Instead of considering `0` as no color, I am considering
`kColorNoColor = -1` as I am using colors as an index in other arrays in the algorithm
and it is more convenient to use them this way.

## Output

```bash
            Instance    Colors      Time, sec
         myciel3.col         3              0
         myciel7.col         7          0.004
 latin_square_10.col         1              0
         school1.col        16          0.023
     school1_nsh.col        27          0.017
      mulsol.i.1.col        48          0.005
      inithx.i.1.col        53          0.024
            anna.col        10          0.001
            huck.col        10              0
            jean.col         9              0
       miles1000.col        41          0.004
       miles1500.col        72          0.006
      fpsol2.i.1.col        64          0.014
        le450_5a.col         9          0.007
       le450_15b.col        16          0.011
       le450_25a.col        24          0.011
        games120.col         8          0.001
      queen11_11.col        13          0.002
        queen5_5.col         4              0
```
