function generate_help_files(packageFolder)
% Code which generates all CPP code from MVtec Include file
halconroot = getenv('HALCONROOT');
folder = [halconroot '/doc/html/reference/operators'];
files = dir([folder '/' '*.html']);

for i=1:length(files)
    fname = files(i).name;
    fnamec='';
    up = true;
    for j=1:(length(fname)-5)
        ch=fname(j);
        if(fname(j)=='_'), up=true; continue; end
        if(up), ch=upper(ch); end
        fnamec=[fnamec ch];
        up=false;
    end
    
    filename =[folder '/' files(i).name];

    fid = fopen(filename, 'r');
    c = fread(fid, inf, 'uint8=>char')';
    fclose(fid);

    cs = strfind(c,'</head>');
    str = c((cs+7):end);
 
    % Strip div for other languages
    divs = strfind(str,'<div');
    dive = strfind(str,'</div>');
    for j=1:length(divs)
        strname = str(divs(j):end);
        strname = strname(1:find(strname=='>',1,'first'));
        k = strfind(strname,'name');
        if(~isempty(k))
            strname = strname(k+6:end);
            strname =strname(1:(find(strname=='"',1,'first')-1));
            if(strcmp(strname,'c')||strcmp(strname,'com')||strcmp(strname,'cpp')||strcmp(strname,'cpp_old')||(strcmp(strname,'dotnet')))
                 str(divs(j):dive(j)+5)=char(14);
            end
        end
        k = strfind(strname,'class');
        if(~isempty(k))
            strname = strname(k+7:end);
            strname =strname(1:(find(strname=='"',1,'first')-1));
            if(strcmp(strname,'tabbar'))
                     str(divs(j):dive(j)+5)=char(14);
            end
        end
    end
    
    % Strip span for other languages
    divs = strfind(str,'<span');
    dive = strfind(str,'</span>');
    for j=1:length(divs)
        strname = str(divs(j):end);
        strname = strname(1:find(strname=='>',1,'first'));
        k = strfind(strname,'name');
        if(~isempty(k))
            strname = strname(k+6:end);
            strname =strname(1:(find(strname=='"',1,'first')-1));
            if(strcmp(strname,'c')||strcmp(strname,'com')||strcmp(strname,'cpp')||strcmp(strname,'cpp_old')||(strcmp(strname,'dotnet')))
                 str(divs(j):dive(j)+6)=char(14);
            end
        end
    end
    
    % Remove HTML tags -> plain text
    str = regexprep(str, '<[^>]*>', ' ');
   
    % Split Lines
    str = regexp(str, '\n+', 'split');
    fid = fopen(fullfile(packageFolder, [fnamec '.m']),'w');
    r=0;
    for j=1:length(str)
        lst =str{j};
        % Remove strange chars
        lst = uint8(lst); lst= lst((lst>=32)&(lst<=126)); lst=char(lst);
        
        % Skip multiple empty lines
        if(isempty(lst)||(all(lst==32)))
            if(r==0)
                r = 1;
            else
                continue;
            end
        else
            r = 0;
        end
        fprintf(fid,'%%%s\n',lst);
    end
    fclose(fid);
     
end