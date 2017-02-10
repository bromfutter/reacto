#!python

class PlatformBaseClass(object):

    def __init__(self):
        self._build_dirs = {}
        self._build_binaries = {}
        self._build_libraries = {}
        self._coverage_excludes = []
        self._SetupEnv()
        # CPPINCLUDES is added because Scons doesn't support
        # `-include` command line, so we will manage it
        # manually.

    # Returns a cusomized environment
    # This environment will take changes from build scripts.
    def Env(self):
        return self._env

    # Must returns a string with the name of your platform
    def Name(self):
        pass

    # Must return a list of target names availble
    # to user in commandline.
    def TargetNameList(self):
        return []

    # Must return a list of project names that will be
    # built using this platform.
    def BuildList(self):
        return []

    # Should return true or false for each of the
    # configurations
    def ProfileEnabled(self):
        return False
    def CppUTestExtensionsEnabled (self):
        return False

    def CoverageAddExclusion (self, file_path):
        self._coverage_excludes.append(file_path)

    def BuildListAddBinary (self, build_name, path):
        self._build_binaries[build_name] = path

    def BuildListAddLibrary (self, build_name, path):
        self._build_libraries[build_name] = path

    def BuildListAddDirectory (self, build_name, path):
        self._build_dirs[build_name] = path

    # A method that will be called after all build scripts
    # have been loaded.
    # Use this call to add your custom Commands.
    def PostBuildTargets(self):
        pass

    @staticmethod
    def MakeYCMFlags(target, source, env):
        from SCons.Node.FS import File, Dir
        from SCons.Node import NodeList
        def flatten(xs):
            result = []
            if isinstance(xs, (list, tuple, NodeList)):
                for x in xs:
                    result.extend(flatten(x))
            else:
                result.append(xs)
            return result
        def toPath(f):
            if isinstance(f, (File, Dir)):
                return str(f.path)
            return f
        flags = flatten(env['CXXFLAGS']) + flatten(env['CFLAGS']) \
             + ['-D ' + define for define in flatten(env['CPPDEFINES'])] \
             + ['-I ' + toPath(inc) for inc in flatten(env['CPPPATH'])] \
             + ['-l ' + toPath(lib) for lib in flatten(env['LIBS'])] \
             + ['-include ' + toPath(inc) for inc in flatten(env['CINCLUDES'])] \
             + ['-include ' + toPath(inc) for inc in flatten(env['CXXINCLUDES'])]
        with open(target[0].path, "w") as flags_file:
            flags_file.write("flags={}".format(flags))

    class Default(object):
        def __init__(self, target):
            self._target = target
        def __str__(self):
            return self._target
