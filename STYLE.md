# Styling

SRTOS code uses `clang-format` to automatically format code. Many IDEs have features to auto-format code on file save, but it is up to the user to set this up if desired.

When executed in the `Src/` folder, `clang-format -style=gnu -i -- **.c **.h` will format each file in accordance with the expected standard. You do not need to format files like this, but as long as they are formatted with the GNU Coding Standard, it is acceptable.

Code is styled according to the [GNU Coding Standard.](https://www.gnu.org/prep/standards/standards.html)

All comments in SRTOS **must** use C's multiline comment syntax (`/* Text Here */`). When comments are multiline,
