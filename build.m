function build()
%% make package folder
thisFolder = fileparts(mfilename('fullpath'));
%%
packageName = '+halcon';
packageFolder = fullfile(thisFolder, packageName);

if ~isfolder(packageFolder)
    mkdir(packageFolder);
end
copyfile(fullfile(thisFolder,'private\*.h'), packageFolder)

%%
generate_cpp_code(packageFolder);

%%
compile_cpp_code(packageFolder);

%%
generate_help_files(packageFolder);

%% remove help not corresponding to cpp
old = cd(packageFolder);
list = dir('*.m');
list = {list.name};
for i = 1:numel(list)
    fn = regexprep(list{i}, '\.m$', '.cpp');
    if ~isfile(fn)
        delete(list{i});
    end
end
cd(old)

%% remove cpp
delete(fullfile(packageFolder, '*.cpp'));
delete(fullfile(packageFolder, '*.h'));

%% deploy
root = getenv('HALCONROOT');
halconVersion = regexp(root, '(?<=HALCON-)(\d|\.)*', 'match', 'once');
deployFolder = fullfile(thisFolder, '..', ...
    sprintf('halcon-%s-%s-%s', ...
    halconVersion, ...
    computer('arch'), ...
    lower(mex.getCompilerConfigurations('C++').ShortName)));
if ~isfolder(deployFolder)
    mkdir(deployFolder);
end
dn = fullfile(deployFolder, packageName);
if isfolder(dn)
    try
        rmdir(dn);
    catch ex
        disp(getReport(ex));
        warning('Could not remove %s', dn);
    end
end
movefile(packageFolder, deployFolder, 'f');
end
