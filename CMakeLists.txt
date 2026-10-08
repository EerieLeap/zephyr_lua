# Zephyr module wrapper around upstream Lua (lua/, a Git submodule pinned at a release tag).
#
# The sources are compiled as C++ so that LUAI_THROW is a C++ throw and lua_error() unwinds
# through the C++ frames of the callers (longjmp would skip their destructors); this needs
# CONFIG_CPP_EXCEPTIONS in the application.

if(CONFIG_LUA)
    set(LUA_MODULE_DIR "${ZEPHYR_CURRENT_MODULE_DIR}")
    set(LUA_SRC_DIR "${LUA_MODULE_DIR}/lua")

    if(NOT EXISTS "${LUA_SRC_DIR}/lua.h")
        message(FATAL_ERROR "Lua sources not found in ${LUA_SRC_DIR}: run 'git submodule update --init --recursive'.")
    endif()

    # Upstream luaconf.h hard-codes LUA_32BITS and has no user hook, so a one-hunk patch makes
    # it overridable from the command line. Applied at configure time, once.
    # Several builds may configure at once (twister), so the check-and-apply runs under a lock
    # and a failed apply is re-checked: another configure may have applied it in between.
    file(GLOB LUA_PATCHES "${LUA_MODULE_DIR}/patches/*.patch")
    set(LUA_PATCH_LOCK "${LUA_MODULE_DIR}/patches/.apply.lock")
    file(LOCK ${LUA_PATCH_LOCK} TIMEOUT 120)
    foreach(patch ${LUA_PATCHES})
        execute_process(
            COMMAND git apply --reverse --check ${patch}
            WORKING_DIRECTORY ${LUA_SRC_DIR}
            RESULT_VARIABLE patch_already_applied
            OUTPUT_QUIET
            ERROR_QUIET
        )
        if(NOT patch_already_applied EQUAL 0)
            message(STATUS "Applying Lua patch: ${patch}")
            execute_process(
                COMMAND git apply ${patch}
                WORKING_DIRECTORY ${LUA_SRC_DIR}
                RESULT_VARIABLE patch_apply_result
                OUTPUT_QUIET
                ERROR_QUIET
            )
            if(NOT patch_apply_result EQUAL 0)
                execute_process(
                    COMMAND git apply --reverse --check ${patch}
                    WORKING_DIRECTORY ${LUA_SRC_DIR}
                    RESULT_VARIABLE patch_already_applied
                    OUTPUT_QUIET
                    ERROR_QUIET
                )
                if(NOT patch_already_applied EQUAL 0)
                    file(LOCK ${LUA_PATCH_LOCK} RELEASE)
                    message(FATAL_ERROR "Failed to apply Lua patch: ${patch}")
                endif()
            endif()
        endif()
    endforeach()
    file(LOCK ${LUA_PATCH_LOCK} RELEASE)

    # The core, the auxiliary library and the libraries that are always opened.
    set(LUA_SOURCES
        lapi.c lcode.c lctype.c ldebug.c ldo.c ldump.c lfunc.c lgc.c llex.c lmem.c
        lobject.c lopcodes.c lparser.c lstate.c lstring.c ltable.c ltm.c lundump.c lvm.c lzio.c
        lauxlib.c
        ltablib.c lstrlib.c lmathlib.c lutf8lib.c
    )

    # Optional libraries; io and os are what drag stdio and OS stubs into an image.
    if(CONFIG_LUA_LIB_BASE)
        list(APPEND LUA_SOURCES lbaselib.c)
    endif()
    if(CONFIG_LUA_LIB_COROUTINE)
        list(APPEND LUA_SOURCES lcorolib.c)
    endif()
    if(CONFIG_LUA_LIB_PACKAGE)
        list(APPEND LUA_SOURCES loadlib.c)
    endif()
    if(CONFIG_LUA_LIB_IO)
        list(APPEND LUA_SOURCES liolib.c)
    endif()
    if(CONFIG_LUA_LIB_OS)
        list(APPEND LUA_SOURCES loslib.c)
    endif()
    if(CONFIG_LUA_LIB_DEBUG)
        list(APPEND LUA_SOURCES ldblib.c)
    endif()

    list(TRANSFORM LUA_SOURCES PREPEND "${LUA_SRC_DIR}/")

    # Weak libc stubs (times() for the clock() call in ltablib.c's sort pivot seed, and the
    # newlib file stubs); plain C, built on every target.
    list(APPEND LUA_SOURCES "${LUA_MODULE_DIR}/src/stubs.c")

    # Global: every translation unit that includes luaconf.h must agree on the number types.
    zephyr_include_directories("${LUA_SRC_DIR}")
    if(CONFIG_LUA_32BITS)
        zephyr_compile_definitions(LUA_32BITS=1)
    else()
        zephyr_compile_definitions(LUA_32BITS=0)
    endif()

    zephyr_library()
    zephyr_library_sources(${LUA_SOURCES})

    set(LUA_CXX_SOURCES ${LUA_SOURCES})
    list(REMOVE_ITEM LUA_CXX_SOURCES "${LUA_MODULE_DIR}/src/stubs.c")
    set_source_files_properties(${LUA_CXX_SOURCES} PROPERTIES LANGUAGE CXX)
endif()
