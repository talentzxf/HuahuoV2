package WebGL;

use strict;
use warnings;
use File::Path qw(mkpath rmtree);
use File::Basename qw(dirname basename);
use File::Find;
use lib File::Spec->rel2abs(dirname(__FILE__));
use File::chdir;
use Tools qw (UCopy Jam JamRaw);
use Tools qw (PackageAndSendResultsToBuildServer);

sub Register
{
    Main::RegisterTarget(
        "WebGLSupport",
        {
            windowspreparefunction => \&PrepareWebGLSupport,
            windowsbuildfunction => \&BuildWebGLSupport,
            macbuildfunction => \&BuildWebGLSupport,
            linuxbuildfunction => \&BuildWebGLSupport,
            options => {
                "codegen" => ["debug", "release"],
                "incremental" => [0, 1],
                "zipresults" => [0, 1]
            }
        }
    );
}

sub BuildWebGLEditorExtensions
{
    my ($root) = @_;

    Jam($root, "WebGLEditorExtensions", "", "", 1);
}

sub CreatePlatformArtifact
{
    my ($targetPath, $platform) = @_;

    my $platformPath = $targetPath . "_$platform";
    UCopy($targetPath, $platformPath);

    rmtree("$platformPath/BuildTools/lib/modules_testing");

    if ($platform eq "Mac")
    {
        rmtree("$platformPath/BuildTools/Emscripten_FastComp_Win");
        rmtree("$platformPath/BuildTools/Emscripten_Win");
        rmtree("$platformPath/BuildTools/Emscripten_FastComp_Linux");
        rmtree("$platformPath/BuildTools/Emscripten_Linux");
    }
    elsif ($platform eq 'Win')
    {
        rmtree("$platformPath/BuildTools/Emscripten_FastComp_Mac");
        rmtree("$platformPath/BuildTools/Emscripten_Mac");
        rmtree("$platformPath/BuildTools/Emscripten_FastComp_Linux");
        rmtree("$platformPath/BuildTools/Emscripten_Linux");
    }
    elsif ($platform eq 'Linux')
    {
        rmtree("$platformPath/BuildTools/Emscripten_FastComp_Win");
        rmtree("$platformPath/BuildTools/Emscripten_Win");
        rmtree("$platformPath/BuildTools/Emscripten_FastComp_Mac");
        rmtree("$platformPath/BuildTools/Emscripten_Mac");
    }

    PackageAndSendResultsToBuildServer($platformPath, "WebGLSupport");
}

sub BuildWebGLSupport
{
    my ($root, $targetPath, $options) = @_;

    my @cmd = ($root, 'WebGLPlayer', 'WebGLPlayerNoDevelopment', 'WebGLPlayerUnitTests');

    # add multithreaded variations
    push @cmd, ('WebGLPlayerThreaded', 'WebGLPlayerNoDevelopmentThreaded', 'WebGLPlayerUnitTestsThreaded');

    JamRaw(@cmd);

    # build size-optimized variations separately, because building too many targets at once
    # will result in extremely long path name for the .dag file, which can not be properly handled on Windows
    @cmd = ($root, 'WebGLPlayerNoDevelopmentOptSize', 'WebGLPlayerNoDevelopmentThreadedOptSize');

    JamRaw(@cmd);

    if ($options->{zipresults})
    {
        CreatePlatformArtifact($targetPath, "Mac");
        CreatePlatformArtifact($targetPath, "Win");
        CreatePlatformArtifact($targetPath, 'Linux');
        PackageAndSendResultsToBuildServer($targetPath);
    }
}

sub PrepareWebGLSupport
{
}

1;
