if (NOT EXISTS "/Users/adib/Dev/Personal/Libs/SDL2-2.26.5/sdl-bin/install_manifest.txt")
    message(FATAL_ERROR "Cannot find install manifest: \"/Users/adib/Dev/Personal/Libs/SDL2-2.26.5/sdl-bin/install_manifest.txt\"")
endif(NOT EXISTS "/Users/adib/Dev/Personal/Libs/SDL2-2.26.5/sdl-bin/install_manifest.txt")

file(READ "/Users/adib/Dev/Personal/Libs/SDL2-2.26.5/sdl-bin/install_manifest.txt" files)
string(REGEX REPLACE "\n" ";" files "${files}")
foreach (file ${files})
    message(STATUS "Uninstalling \"$ENV{DESTDIR}${file}\"")
    execute_process(
        COMMAND /opt/homebrew/Cellar/cmake/3.26.3/bin/cmake -E remove "$ENV{DESTDIR}${file}"
        OUTPUT_VARIABLE rm_out
        RESULT_VARIABLE rm_retval
    )
    if(NOT ${rm_retval} EQUAL 0)
        message(FATAL_ERROR "Problem when removing \"$ENV{DESTDIR}${file}\"")
    endif (NOT ${rm_retval} EQUAL 0)
endforeach(file)

