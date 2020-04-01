function compile_cpp_code(packageFolder)
%%
halconroot = getenv('HALCONROOT');
HalconPathInclude = [halconroot '\include'];
HalconPathLib     = [halconroot '\lib\' getenv('HALCONARCH')];
%%
old = cd(packageFolder);
files=dir('*.cpp');
for i=1:length(files)
    try
        Filename =  files(i).name;
        disp(Filename);
        mex(['-L' HalconPathLib],'-lhalconcpp',['-I' HalconPathInclude ],Filename);
    catch ex
        disp(getReport(ex));
        warning(Filename);
    end
end
cd(old);

return;
end
