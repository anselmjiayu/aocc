# aocc
![title](/aocc.png)

Solutions to [Advent of Code] 2023, written in Clojure and C99.

## usage

### C
There are basically no prerequisites. Every file is self-contained, for example:
```sh
cc -Wall -g d1.c && ./a.out -p 1 ../inputs/1.txt
```
### Clojure
The project uses [Leiningen]. To run a specific task, type
```sh
lein run 2 1 ../inputs/2.txt
```

[Advent of Code]: https://adventofcode.com/
[Leiningen]: https://leiningen.org/
