function generate_cpp_code(packageFolder)
% Code which generates all CPP code from MVtec Include file
halconroot = getenv('HALCONROOT');
filename = [halconroot '\include\halconcpp\HOperatorSet.h'];

fid=fopen(filename);
tline_old='';
func=struct();
k=0;
while(true)
    tline = fgetl(fid);
    if ~ischar(tline), break, end
    
    % Process
    if((length(tline)>15) && (strcmp(tline(1:15),'LIntExport void')))
        
        tline = tline(17:end);
        i = find(tline=='(');
        function_comment = tline_old;
        function_name = tline(1:i-1);
        j = find(tline==')');
        tline=tline((i+1):(j-1));
        
        j = find(tline==',');
        js =[1,j+2];
        je =[j-1,length(tline)];
        
        if(je(1)<js(1)), js=[]; end
        
        io_names = cell(length(js),1);
        io_input = false(length(js),1);
        io_type = zeros(length(js),1);
        for i=1:length(js)
            io_part = tline(js(i):je(i));
            while(io_part(1)==' '), io_part=io_part(2:end); end
            while(io_part(end)==' '), io_part=io_part(1:end-1); end
            j = find(io_part==' ');
            io_name=io_part((j(end)+1):end);
            
            if(strcmp(io_part(1:j(1)-1),'const'))
                io_input(i)=true;
                io_part=io_part((j(1)+1):end);
            end
              j = find(io_part==' ');
          
            if(strcmp(io_part(1:(j-1)),'HObject&'))
                io_type(i)=1;
                io_names{i} = ['ho_' io_name];
            elseif(strcmp(io_part(1:(j-1)),'HTuple&'))
                io_type(i)=2;
                io_names{i} = ['ht_' io_name];
            elseif(strcmp(io_part(1:(j-1)),'HObject*'))
                io_type(i)=3;
                io_names{i} = ['ho_' io_name];
            elseif(strcmp(io_part(1:(j-1)),'HTuple*'))
                io_type(i)=4;
                io_names{i} = ['ht_' io_name];
            else
                io_type(i)=5;
                io_names{i} = ['_' io_name];
            end

        end
        k=k+1;
        func(k).name = function_name;
        func(k).comment = function_comment;
        func(k).io_names = io_names;
        func(k).io_input = io_input;
        func(k).io_type = io_type ;
        
    end
    
    tline_old = tline;
end
fclose(fid);

for k=1:length(func)
    function_name = func(k).name;
    function_comment = func(k).comment;
    io_names = func(k).io_names;
    io_input = func(k).io_input ;
    io_type =func(k).io_type;


    i=0; str={};
    i=i+1; str{i}=function_comment;
    i=i+1; str{i}='';
    i=i+1; str{i}='#include "mex.h"';
    i=i+1; str{i}='#include "HalconHelper.h"';
    i=i+1; str{i}='';
    i=i+1; str{i}='using namespace HalconCpp;';
    i=i+1; str{i}='';
    i=i+1; str{i}='void mexFunction( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[] )'; 
    i=i+1; str{i}='{';
    i=i+1; str{i}='    // Disable thread_pool - should allow dynamic unload - there are problems';
    i=i+1; str{i}='    // SetSystem("thread_pool", "false");';
    i=i+1; str{i}='    // Prevent dynamical unloading';
    i=i+1; str{i}='    if (!mexIsLocked())';
    i=i+1; str{i}='    {';
    i=i+1; str{i}='        mexLock();';
    i=i+1; str{i}='    }';
    i=i+1; str{i}='    // Check number of inputs  ';
    ninputs=nnz(io_input); noutputs=nnz(~io_input);
    i=i+1; str{i}=['    if(nrhs<' num2str(ninputs) ') '];
    i=i+1; str{i}= '    { ';
    i=i+1; str{i}=['        mexErrMsgTxt(" ' num2str(ninputs) ' input variables are required"); '];
    i=i+1; str{i}= '    }';
    i=i+1; str{i}='';
    io_names_input = io_names(io_input);
    io_names_output= io_names(~io_input);
    io_type_input  = io_type(io_input);
    io_type_output = io_type(~io_input);

    i=i+1; str{i}='    // All inputs';
    for j=1:length(io_names_input)
        switch(io_type_input(j))
            case 1  % HObject
                i=i+1; str{i}=['    HObject ' io_names_input{j} '; // input'];
            case 2 % HTuple
                i=i+1; str{i}=['    HTuple ' io_names_input{j} '; // input'];
            otherwise
                error('unknown type');
        end
    end
    i=i+1; str{i}='';

    i=i+1; str{i}='    // All outputs';
    for j=1:length(io_names_output)
        switch(io_type_output(j))
            case 3  % HObject
                i=i+1; str{i}=['    HObject ' io_names_output{j} '; // output'];
            case 4 % HTuple
                i=i+1; str{i}=['    HTuple ' io_names_output{j} '; // output'];
            otherwise
                error('unknown type');
        end
    end
    i=i+1; str{i}='';



    i=i+1; str{i}='    // Convert Matlab Arrays to Halcon Objects and Tuples';
    for j=1:length(io_names_input)
        switch(io_type_input(j))
            case 1  % HObject
                i=i+1; str{i}=['    MatlabArray2HalconObject(prhs[' num2str(j-1) '], &'  io_names_input{j} ' );'];
            case 2 % HTuple
                i=i+1; str{i}=['    MatlabArray2HalconTuple(prhs[' num2str(j-1) '], &'  io_names_input{j} ' );'];
            otherwise
                error('unknown type');
        end
    end
    
     
    i=i+1; str{i}='';
    i=i+1; str{i}='    try';
    i=i+1; str{i}='    {';
    
    strf=['        ' function_name '('];
    for j=1:length(io_names)
       if(io_input(j))
           strf=[strf io_names{j}];
       else
           strf=[strf '&' io_names{j}];
       end
       if(j<length(io_names))
           strf=[strf ', '];
       end

    end
    strf=[strf ');'];
    i=i+1; str{i}=strf;

    i=i+1; str{i}='    }';
    i=i+1; str{i}='    catch (HalconCpp::HException &HDevExpDefaultException)';
    i=i+1; str{i}='    {';
    i=i+1; str{i}=['        Herror  err = HDevExpDefaultException.ErrorCode(); '];
    i=i+1; str{i}=['        if( ((int)err)==1203) '];
    i=i+1; str{i}=['        { '];
    i=i+1; str{i}=['            printf("***********************************************************\n"); '];
    i=i+1; str{i}=['            printf("Please note parameters must be of the right type (rounded values often int32, true/false string ..., example :\n"); '];
    i=i+1; str{i}=['            printf("ParSegmentation = {''max_orientation_diff'',''max_curvature_diff'',''output_xyz_mapping'',''min_area''};\n"); '];
    i=i+1; str{i}=['            printf("ValSegmentation = {0.13,0.11,''true'',int32(150)};\n"); '];
    i=i+1; str{i}=['            printf("ObjectModel3DOutID =hSegmentObjectModel3d (ObjectModel3DID, ParSegmentation, ValSegmentation);\n"); '];
    i=i+1; str{i}=['            printf("***********************************************************\n"); '];
    i=i+1; str{i}=['        } '];
    i=i+1; str{i}='        const char* error_text = HDevExpDefaultException.ErrorMessage().Text();';
    i=i+1; str{i}='        mexErrMsgTxt(error_text);';
    i=i+1; str{i}='    }';




    i=i+1; str{i}='';
    i=i+1; str{i}='    // Convert Halcon Objects and Tuples to Matlab Arrays';
    for j=1:length(io_names_output)
        switch(io_type_output(j))
            case 3  % HObject
                i=i+1; str{i}=['    if(nlhs> ' num2str(j-1) ')'];
                i=i+1; str{i}=['    {'];
                i=i+1; str{i}=['        HalconObject2MatlabArray(&'  io_names_output{j} ', &plhs[' num2str(j-1) ']);'];
                i=i+1; str{i}=['    }'];
            case 4 % HTuple
                i=i+1; str{i}=['    if(nlhs> ' num2str(j-1) ')'];
                i=i+1; str{i}=['    {'];
                i=i+1; str{i}=['        HalconTuple2MatlabArray(&'  io_names_output{j} ' ,&plhs[' num2str(j-1) ']);'];
                i=i+1; str{i}=['    }'];
            otherwise
                error('unknown type');
        end
    end

    i=i+1; str{i}='}';
    filename=fullfile(packageFolder, [function_name '.cpp']);
    fid = fopen(filename,'w+');
    for j=1:length(str)
        fprintf(fid,'%s\r\n',str{j});
    end
    fclose(fid);
end