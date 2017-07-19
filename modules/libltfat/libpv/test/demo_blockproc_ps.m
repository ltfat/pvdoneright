function demo_blockproc_ps(source,varargin) %RUNASSCRIPT
%DEMO_BLOCKPROC_BASICLOOP Basic real-time audio manipulation
%   Usage: demo_blockproc_basicloop('gspi.wav')
%
%   For additional help call |demo_blockproc_basicloop| without arguments.
%
%   The demo runs simple playback loop allowing to set gain in dB.
% 

if demo_blockproc_header(mfilename,nargin)
   return;
end

maxstretch = 4;
% Basic Control pannel (Java object)
p = blockpanel({
               {'GdB','Gain',-20,20,0,21},...
               {'stretch','Stretch',-1,1,1,501},...
               {'bypass','Bypass',0,1,0,2}
               });
         
% Setup blocktream
try
    fs = block(source,varargin{:},'loadind',p,'single');
catch
    % Close the windows if initialization fails
    blockdone(p);
    err = lasterror;
    error(err.message);
end

% Set buffer length to 30 ms
Lin = floor(30e-3*fs);
Linvar = 0;

plan = libpointer();
status = calllib('libpv','ltfat_pv_init_s', maxstretch, 1, Lin + Linvar ,plan);

fout = zeros(ceil((Lin + Linvar)*maxstretch),1,'single');

flag = 1;
%Loop until end of the stream (flag) and until panel is opened
while flag && p.flag
   gain = blockpanelget(p,'GdB');
   stretch = maxstretch^(blockpanelget(p,'stretch'));
   bypass = blockpanelget(p,'bypass');
   
   Lintmp =  Lin;
 
   %calllib('libpv','print_pos', plan);
   Lout = calllib('libpv','ltfat_pv_nextoutlen_s',plan,Lintmp);
   %fprintf('stretch=%.3f, Lout=%i, Lin=%i\n',stretch,Lout,Lin);
       
   gain = 10^(gain/20);
   
   [f,flag] = blockread(Lintmp);
   
   bufInPtr = libpointer('singlePtr',f(:,1));
   bufOutPtr = libpointer('singlePtr',fout);
   status = calllib('libpv','ltfat_pv_execute_s',plan,bufInPtr,Lintmp,1,stretch,Lout,bufOutPtr);
   

   fout2 = bufOutPtr.Value;
   foutres = fout2(1:Lout)*gain;
   foutres = dctresample(foutres,Lintmp);
   
   if bypass
       foutres = f;
   end
   % The following does nothing in the rec only mode.
   blockplay(foutres);
   % The following does nothing if 'outfile' was not specified 
   blockwrite(foutres);
end
blockdone(p);
calllib('libpv','ltfat_pv_done_s',plan);