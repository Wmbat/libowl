import os
import argparse

def build(args):
    root_dir = os.getcwd()
    build_dir = root_dir + "/build_" + args.build_type + "_" + args.compiler

    if os.path.isdir(build_dir) is not True:
        os.mkdir(build_dir)

    os.chdir(build_dir)

    cmake = 'cmake ' 

    if args.generator == 'ninja':
        cmake += '-G Ninja '

    if args.compiler == 'clang':
        cmake += '-DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ '
    else:
        cmake += '-DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ '

    cmake += '-DCMAKE_BUILD_TYPE='
    if args.build_type == 'release':
        cmake += 'Release '
    else:
        cmake += 'Debug '

    cmake += '-DBUILD_TESTS='
    if args.build_tests:
        cmake += 'ON '
    else:
        cmake += 'OFF '

    cmake += '-DBUILD_BENCH='
    if args.build_bench:
        cmake += 'ON '
    else:
        cmake += 'OFF '

    os.system(cmake + '../')

    if args.generator == 'ninja':
        os.system('ninja')
    else:
        os.system('make -j13')

    os.system("mv compile_commands.json ../")

def main():
    parser = argparse.ArgumentParser(description='Build epona in debug mode')

    parser.add_argument('-G', action='store', default='ninja', 
            dest='generator',
            choices=['ninja', 'make'], 
            help='The generator to build the project')

    parser.add_argument('-C', action='store', default='clang', 
            dest='compiler',
            choices=['clang', 'gcc'], 
            help='The compiler to use')

    parser.add_argument('-B', action='store', default='debug', 
            dest='build_type',
            choices=['debug', 'release'],
            help='The build type')

    parser.add_argument('--build_tests', action='store_true', help='Whether to build unit tests or not')
    parser.add_argument('--build_bench', action='store_true', help='Whether to build benchmarks or not')

    build(parser.parse_args())

if __name__ == "__main__":
    main()
