# EerieLeap Lua

Zephyr module wrapper around upstream [Lua](https://github.com/lua/lua), which is the `lua/`
Git submodule pinned at a release tag (currently `v5.4.8`). The wrapper is the Zephyr glue
(`zephyr/module.yml`, `Kconfig.lua_module`, `CMakeLists.txt`) and `patches/`; `VERSION` records
the wrapper, the submodule records the Lua. Originally based on
[lua_zephyr](https://github.com/rodrigopex/lua_zephyr).

- `CONFIG_LUA` builds the core, `lauxlib` and the `table`, `string`, `math` and `utf8`
  libraries; `CONFIG_LUA_LIB_*` add `base` (default on), `coroutine`, `package`, `io`, `os`
  and `debug`. `lua.c`, `luac.c`, `onelua.c`, `linit.c` and `ltests.c` are never built.
- `CONFIG_LUA_32BITS` (default on) selects `float` numbers and 32-bit integers through a
  global compile definition; `patches/luaconf.patch` makes upstream's hard-coded
  `LUA_32BITS` overridable and is applied to the submodule at configure time with `git apply`.
- Sources are compiled as C++, so `LUAI_THROW` is a C++ `throw` and `lua_error()` unwinds
  through C++ callers; the application needs `CONFIG_CPP_EXCEPTIONS`.
- `LUA_COMPAT_*` switches are off, as upstream ships them.
- `src/stubs.c` provides weak `times()` (picolibc) and newlib file stubs. `ltablib.c` seeds the
  `table.sort` pivot with `clock()`, which picolibc implements over `times()`, so the stub is
  needed even without the `io` and `os` libraries.

Upgrading Lua: check out the new tag in `lua/`, confirm `patches/luaconf.patch` still applies
(`git -C lua apply --check ../patches/luaconf.patch`), build, commit the gitlink.

```shell
git submodule update --init --recursive
```
