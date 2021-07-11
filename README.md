# ams-cheat-to-c
Convert Atmosphere cheat codes to a more readable format, in the form of C pseudocode

## Usage
Simply drag and drop a cheat file onto the exe or launch it and manually enter a file path.

Output will be in the same folder as the exe with the name `out.c`

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
