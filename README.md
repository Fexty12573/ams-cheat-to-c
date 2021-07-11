# ams-cheat-to-c
Convert Atmosphere cheat codes to a more readable format, in the form of C 'pseudocode'

Note that while this makes codes more readable, you still need some understanding of the concepts used within them.
For example: pointers, if-statements, loops, etc. Pointers being the most important of the bunch.

Full documentation on atmosphere cheat codes can be found [here](https://github.com/Atmosphere-NX/Atmosphere/blob/master/docs/features/cheats.md)

## Usage
Simply drag and drop a cheat file onto the exe or launch it and manually enter a file path.

Output will be in the same folder as the input file with format `<inputname>.c`

```
atmosphere-cheat-to-c path/to/file
```

## Notes
Cheat codes are in the usual format
```
[Cheat name]
<code>
```

Output will look something like this
```
/* Cheat name */
void Cheat() {
  // code
}
```
