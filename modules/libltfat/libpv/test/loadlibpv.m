function loadlibpv(varargin)

definput.keyvals.lib='libpv.so';
definput.flags.phase={'load','reload'};
[flags,~,lib]=ltfatarghelper({'lib'},definput,varargin);

[~,libname]=fileparts(lib);
    
if libisloaded(libname) 
    if flags.do_reload
        unloadlibrary(libname);
    else
        error('%s: libltfat is already loaded. Use ''reload'' to force reload.',upper(mfilename));
    end
end

warning('off');
currdir = fileparts(mfilename('fullpath'));
libpath = [currdir, filesep, '..',filesep,'build',filesep,lib];
headerpath = [currdir, filesep,'..', filesep,'build',filesep,'pv.h'];
loadlibrary(libpath,headerpath,'mfilename','libpvprotofile.m');
warning('on');






