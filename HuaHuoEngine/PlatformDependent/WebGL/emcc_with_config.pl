use File::Basename qw(dirname);
use File::Spec;
use Config;
use Cwd;
use Cwd 'abs_path';

my $mydir = dirname($0);
my $top = abs_path("$mydir/../..");

# Use currenctly installed Emscripten SDK if available (and activated)
if ($ENV{'EMSDK'} eq '')
{
    my $externalDir = File::Spec->catdir($top, "External", "Emscripten");
    my $nodejs = File::Spec->catdir($top, "External", "nodejs", "builds");
    my $emscriptenFastcomp;

    if ($Config{osname} eq "darwin")
    {
        $emscriptenFastcomp = File::Spec->catdir($externalDir, "EmscriptenFastComp_Mac", "builds");
        $ENV{NODE} = File::Spec->catdir($nodejs, "osx", "bin", "node");
    }
    elsif ($Config{osname} eq 'linux')
    {
        $emscriptenFastcomp = File::Spec->catdir($externalDir, 'EmscriptenFastComp_Linux', 'builds');
        $ENV{NODE} = File::Spec->catdir($nodejs, 'linux64', 'bin', 'node');
    }
    else
    {
        $emscriptenFastcomp = File::Spec->catdir($externalDir, "EmscriptenFastComp_Win", "builds");
        $ENV{NODE} = File::Spec->catdir($nodejs, "win64", "node.exe");
        $ENV{EMSCRIPTEN_NATIVE_OPTIMIZER} = File::Spec->catdir($emscriptenFastcomp, "optimizer.exe");
    }

    $ENV{EMSCRIPTEN} = File::Spec->catdir($externalDir, "Emscripten", "builds");
    $ENV{LLVM} = $emscriptenFastcomp;
    $ENV{EM_CACHE} = File::Spec->catdir($emscriptenFastcomp, "cache");
}
else
{
    $ENV{EMCC_DEBUG} = "1";
}

$ENV{EMSCRIPTEN_TMP} = File::Spec->catdir($top, "build", "temp$$");
$ENV{EM_CONFIG} = splice @ARGV, 0, 1;
$ENV{EMCC_WASM_BACKEND} = "0";

system("@ARGV") && die("failed running emscripten command " + @ARGV);
