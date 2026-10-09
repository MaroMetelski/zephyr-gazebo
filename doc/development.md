# Development guidelines

## Linting & formatting

### C code

Use `clang-format` to format or check *C* source and header files.
The project inherits the coding style from Zephyr. Use `zephyr/.clang-format`
file for formatting (`--style 'file:<workspace>/zephyr/.clang-format'`).

To check the style, use the `-n` (`--dry-run`) flag:
```
clang-format -n --style='file:<workspace>/zephyr/.clang-format' <file>
```

To format in place, use the `-i` flag:
```
clang-format -i --style='file:<workspace>/zephyr/.clang-format' <file>
```

### Python

Use `black` with default settings to format Python scripts.

```
pip install -r scripts/requirements-dev.txt
```

Check:
```
python3 -m black --check <file>
```

Format in place:
```
python3 -m black <file>
```
