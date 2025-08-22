# huffman-encoder
A lossless file compressor and decompressor using the Huffman encoding.

## Building
```
mkdir build && cd build
```
```
cmake .. && cmake --build .
```

## Using
Compressing a file : 
```
./huffman-encoder -e file.txt
```

Decompressing a file :
```
./huffman-encoder -d file.hff
```

## Performances
Takes about 1.2 seconds to compress a 100mb file on a macbook air m4.
<img width="988" height="120" alt="CleanShot 2025-08-21 at 23 09 50@2x" src="https://github.com/user-attachments/assets/d316f2f6-404c-4147-b36d-98ff9b1dc05e" />

Takes about 0.5 seconds to decompress that 100mb file we just compressed to a 75mb file.
<img width="1046" height="106" alt="CleanShot 2025-08-21 at 23 12 14@2x" src="https://github.com/user-attachments/assets/aaff7d05-db4e-413e-875b-d239d3952ed9" />
