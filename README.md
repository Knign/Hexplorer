# Hexplorer

## Compilation
```
gcc hexplorer.c args.c -o hexplorer
```

## Usage
```
hexplorer [FILE]

    Arguments:
        [FILE]                 File to read
    
    Options:
        -l, --line <int>       Bytes per line in output (default: 16)
        -b, --bytes <int>      Number of bytes to read (default: all)
        -o, --offset <int>     Byte offset at which to begin reading
        -f, --file <filename>  Output file to save the hexdump\n (default: STDOUT)
        
    Flags:
        -h, --help             Display this help text and exit
        -v, --version          Display the version number and exit
```
