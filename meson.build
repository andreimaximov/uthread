project(
    'uthread',
    'cpp',
    default_options : [
        'cpp_std=c++14',
        'warning_level=3',
    ],
    license : 'MIT',
    version : '0.0.1',
)

libevent = dependency('libevent')
threads = dependency('threads')
benchmark = dependency('benchmark', required : false)
gflags = dependency('gflags', required : false)
gmock = dependency('gmock', required : false)
gtest = dependency('gtest', required : false)

incdirs = include_directories('include')

install_subdir('include/cpp', install_dir : get_option('includedir'))

libsrcs = [
    'src/detail/context.cpp',
    'src/detail/stack.cpp',
    'src/detail/task.cpp',
    'src/detail/task_fd.cpp',
    'src/exception.cpp',
    'src/task.cpp',
    'src/task_loop.cpp',
    'src/task_queue.cpp',
    'src/tcp_listener.cpp',
    'src/tcp_stream.cpp',
]

arch_defs = {
    'x86_64-linux'  : 'x86_64_sys_v',
    'x86_64-darwin' : 'x86_64_sys_v',
}

arch_srcs = {
    'x86_64_sys_v': [
        'src/detail/arch/x86_64_sys_v/context.cpp',
        'src/detail/arch/x86_64_sys_v/context.s',
    ],
}

target = host_machine.cpu_family() + '-' + host_machine.system()

if arch_defs.has_key(target)
    libsrcs += arch_srcs[arch_defs[target]]
else
    error(target + ' is not supported!')
endif

lib = library(
    'uthread',
    libsrcs,
    dependencies : [libevent],
    include_directories : incdirs,
    install : true,
)

if gtest.found() and gmock.found()
    test = executable(
        'uthread-test',
        [
            'test/main.cpp',
            'test/detail/context.cpp',
            'test/detail/function.cpp',
            'test/detail/stack.cpp',
            'test/detail/task.cpp',
            'test/detail/task_fd.cpp',
            'test/task.cpp',
            'test/task_loop.cpp',
            'test/task_queue.cpp',
            'test/events.cpp',
            'test/mpmc_queue.cpp',
            'test/socket_addr.cpp',
        ],
        dependencies : [libevent, gtest, gmock],
        include_directories : incdirs,
        link_with : lib,
    )

    test('test', test)
endif

if benchmark.found()
    bench = executable(
        'uthread-bench',
        [
            'bench/main.cpp',
            'bench/detail/context.cpp',
            'bench/detail/task.cpp',
            'bench/task.cpp',
        ],
        dependencies : [libevent, threads, benchmark],
        include_directories : incdirs,
        link_with : lib,
    )

    benchmark('bench', bench)
endif

example_defs = {
    'helloworld'    : 'examples/helloworld.cpp',
    'stackoverflow' : 'examples/stackoverflow.cpp',
    'tcpchat'       : 'examples/tcpchat.cpp',
    'tcpecho'       : 'examples/tcpecho.cpp',
}

example_exes = []

if gflags.found()
    foreach name, cpp : example_defs
        example_exes += executable(
            name,
            [cpp],
            dependencies : [libevent, threads, gflags],
            include_directories : incdirs,
            link_with : lib,
        )
    endforeach
endif

run_target('smoke', command : ['scripts/smoke.py', '-v'], depends : example_exes)
run_target('format', command : 'scripts/clang-format.sh')
