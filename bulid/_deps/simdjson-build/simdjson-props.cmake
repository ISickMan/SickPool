target_include_directories("${target}" ${public} [==[$<BUILD_INTERFACE:/home/sickguy/Documents/Projects/SickPool/server/bulid/_deps/simdjson-src/include>]==] ${private} [==[$<BUILD_INTERFACE:/home/sickguy/Documents/Projects/SickPool/server/bulid/_deps/simdjson-src/src>]==])
target_compile_features("${target}" ${public} [==[cxx_std_11]==])
target_compile_options("${target}" ${private} [==[-mno-avx256-split-unaligned-load]==] [==[-mno-avx256-split-unaligned-store]==])
target_link_libraries("${target}" ${public} [==[Threads::Threads]==])
target_compile_definitions("${target}" ${public} [==[SIMDJSON_THREADS_ENABLED=1]==])
